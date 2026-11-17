#pragma once

#include <Arduino.h>
#include <api/HardwareSPI.h>

using arduino::SPISettings;

/**
 * Hardware SPI master (SPI0) of BK7236.
 *
 * Pins are fixed by the SoC: SCK P14, MOSI P16, MISO P17. The chip select is not
 * driven by the hardware - the application controls it with a GPIO (any pin, also P15).
 */
class ArduinoSPI : public arduino::HardwareSPI {
  private:
	bool initialized = false;
	SPISettings settings;
	bool configure(const SPISettings &settings);

  public:
	uint8_t transfer(uint8_t data) override;
	uint16_t transfer16(uint16_t data) override;
	void transfer(void *buf, size_t count) override;
	void transferBytes(const uint8_t *txBuf, uint8_t *rxBuf, size_t count);

	void usingInterrupt(int interruptNumber) override {}

	void notUsingInterrupt(int interruptNumber) override {}

	void beginTransaction(SPISettings settings) override;
	void endTransaction(void) override;

	void attachInterrupt() override {}

	void detachInterrupt() override {}

	void begin() override;
	bool begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss = -1);
	void end() override;

	void setFrequency(uint32_t frequency);
	void setBitOrder(BitOrder order);
	void setDataMode(uint8_t mode);
};

extern ArduinoSPI SPI;
