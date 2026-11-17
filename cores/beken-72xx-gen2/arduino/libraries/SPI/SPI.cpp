#if LT_ARD_HAS_SPI || DOXYGEN

#include "SPI.h"

extern "C" {
#include <driver/gpio.h>
#include <driver/spi.h>
#include <os/os.h>

#include "spi_ll.h"

extern bk_err_t gpio_dev_unmap(gpio_id_t gpio_id);
}

#define SPI_ID	   SPI_ID_0
#define SPI_HW	   ((spi_hw_t *)SPI_LL_REG_BASE(SPI_ID))
#define SPI_PIN_CS GPIO_15

ArduinoSPI SPI;

// a byte takes 80 us at 100 kHz; allow some milliseconds before giving up
#define SPI_TIMEOUT_US		  10000
// waiting for the end of the last frame (not reported as an error)
#define SPI_FINISH_TIMEOUT_US 1000

bool ArduinoSPI::configure(const SPISettings &settings) {
	uint8_t mode		= settings.getDataMode();
	spi_config_t config = {};
	config.role			= SPI_ROLE_MASTER;
	config.bit_width	= SPI_BIT_WIDTH_8BITS;
	config.polarity		= (mode & 0b10) ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
	config.phase		= (mode & 0b01) ? SPI_PHASE_2ND_EDGE : SPI_PHASE_1ST_EDGE;
	// 3-wire: the hardware does not use the chip select pin
	config.wire_mode = SPI_3WIRE_MODE;
	config.baud_rate = settings.getClockFreq();
	config.bit_order = settings.getBitOrder() == LSBFIRST ? SPI_LSB_FIRST : SPI_MSB_FIRST;
#if CONFIG_SPI_BYTE_INTERVAL
	config.byte_interval = 0;
#endif
	// CONFIG_SPI is not set, so ARMINO's driver_init() skips this (no-op if already done)
	if (bk_spi_driver_init() != BK_OK)
		return false;
	if (this->initialized)
		bk_spi_deinit(SPI_ID);
	this->initialized = false;
	if (bk_spi_init(SPI_ID, &config) != BK_OK)
		return false;
	// leave the chip select pin to the application
	gpio_dev_unmap(SPI_PIN_CS);
	this->settings	  = settings;
	this->initialized = true;
	return true;
}

void ArduinoSPI::begin() {
	if (!this->initialized)
		configure(this->settings);
}

bool ArduinoSPI::begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss) {
	// only the SoC's SPI0 pins are supported
	if ((sck != -1 && sck != GPIO_14) || (miso != -1 && miso != GPIO_17) || (mosi != -1 && mosi != GPIO_16))
		return false;
	begin();
	return this->initialized;
}

void ArduinoSPI::end() {
	if (!this->initialized)
		return;
	bk_spi_deinit(SPI_ID);
	this->initialized = false;
}

void ArduinoSPI::beginTransaction(SPISettings settings) {
	if (!this->initialized || settings != this->settings)
		configure(settings);
}

void ArduinoSPI::endTransaction(void) {}

void ArduinoSPI::setFrequency(uint32_t frequency) {
	beginTransaction(SPISettings(frequency, this->settings.getBitOrder(), this->settings.getDataMode()));
}

void ArduinoSPI::setBitOrder(BitOrder order) {
	beginTransaction(SPISettings(this->settings.getClockFreq(), order, this->settings.getDataMode()));
}

void ArduinoSPI::setDataMode(uint8_t mode) {
	beginTransaction(SPISettings(this->settings.getClockFreq(), this->settings.getBitOrder(), mode));
}

static bool waitFor(bool (*ready)(spi_hw_t *), spi_hw_t *hw, uint32_t timeoutUs) {
	// fast path: micros() itself takes several microseconds
	if (ready(hw))
		return true;
	uint32_t start = micros();
	while (!ready(hw)) {
		if (micros() - start > timeoutUs)
			return false;
	}
	return true;
}

static bool isTxFinished(spi_hw_t *hw) {
	return spi_ll_is_tx_finish_int_triggered(hw, spi_ll_get_interrupt_status(hw));
}

void ArduinoSPI::transferBytes(const uint8_t *txBuf, uint8_t *rxBuf, size_t count) {
	if (!this->initialized || !count)
		return;
	// full-duplex transfer with polling; ARMINO's bk_spi_transmit() is half-duplex (write, then read)
	spi_hw_t *hw = SPI_HW;
	while (count) {
		// the transfer length register is limited, split longer buffers
		uint32_t len = count > SPI_F_TX_TRANS_LEN_M ? SPI_F_TX_TRANS_LEN_M : count;
		spi_ll_disable_tx(hw);
		spi_ll_disable_rx(hw);
		spi_ll_clear_tx_fifo(hw);
		spi_ll_clear_rx_fifo(hw);
		while (spi_ll_is_rx_fifo_rd_ready(hw))
			spi_ll_read_byte(hw);
		spi_ll_clear_tx_finish_int_status(hw);
		spi_ll_clear_rx_finish_int_status(hw);
		spi_ll_set_tx_trans_len(hw, len);
		spi_ll_set_rx_trans_len(hw, len);
		spi_ll_enable_tx_rx(hw);
		for (uint32_t i = 0; i < len; i++) {
			if (!waitFor(spi_ll_is_tx_fifo_wr_ready, hw, SPI_TIMEOUT_US))
				goto timeout;
			spi_ll_write_byte(hw, txBuf ? txBuf[i] : 0xFF);
			// read back each byte, so that the RX FIFO can't overflow
			if (!waitFor(spi_ll_is_rx_fifo_rd_ready, hw, SPI_TIMEOUT_US))
				goto timeout;
			uint8_t data = spi_ll_read_byte(hw);
			if (rxBuf)
				rxBuf[i] = data;
		}
		// let the hardware complete the last frame before disabling it,
		// otherwise a slow clock leaves a partial frame for the next transfer
		waitFor(isTxFinished, hw, SPI_FINISH_TIMEOUT_US);
		spi_ll_disable_tx(hw);
		spi_ll_disable_rx(hw);
		if (txBuf)
			txBuf += len;
		if (rxBuf)
			rxBuf += len;
		count -= len;
	}
	return;

timeout:
	spi_ll_disable_tx(hw);
	spi_ll_disable_rx(hw);
	LT_E("SPI transfer timeout");
}

uint8_t ArduinoSPI::transfer(uint8_t data) {
	uint8_t rx = 0;
	transferBytes(&data, &rx, 1);
	return rx;
}

uint16_t ArduinoSPI::transfer16(uint16_t data) {
	uint8_t tx[2], rx[2];
	if (this->settings.getBitOrder() == LSBFIRST) {
		tx[0] = data;
		tx[1] = data >> 8;
	} else {
		tx[0] = data >> 8;
		tx[1] = data;
	}
	transferBytes(tx, rx, 2);
	if (this->settings.getBitOrder() == LSBFIRST)
		return rx[0] | (rx[1] << 8);
	return (rx[0] << 8) | rx[1];
}

void ArduinoSPI::transfer(void *buf, size_t count) {
	transferBytes((const uint8_t *)buf, (uint8_t *)buf, count);
}

#endif
