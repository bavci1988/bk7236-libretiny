/* This file was auto-generated from wxu.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PB07, PWM3, SPI0_SCK, FSCK
	{PB_7,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI,                       PIN_NONE, 0},
	// D1: PB04, PWM0, SPI0_MOSI, UART1_RTS, FD0
	{PB_4,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D2: PB15, PWM6, UART1_RX, I2C1_SDA
	{PB_15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,            PIN_NONE, 0},
	// D3: PB14, PWM5, UART1_TX, I2C1_SCL
	{PB_14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,            PIN_NONE, 0},
	// D4: PA11, ADC1, SPI0_MISO, WAKE1
	{PA_11, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_SPI,                       PIN_NONE, 0},
	// D5: PA21, PWM2, SPI0_CS, UART2_RX, WAKE11
	{PA_21, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D6: PA22, PWM3, SPI0_SCK, UART2_TX, WAKE12
	{PA_22, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D7: PB03, PWM7, UART0_RTS, TDI, FD3
	{PB_3,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_UART,           PIN_NONE, 0},
	// D8: PA20, PWM1, SPI0_MISO, UART2_CTS, WAKE10
	{PA_20, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D9: PA19, PWM0, SPI0_MOSI, UART2_RTS, WAKE9
	{PA_19, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D10: PA13, ADC3, PWM5, I2C0_SDA, WAKE3
	{PA_13, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C,             PIN_NONE, 0},
	// D11: PA23, TEST, WAKE13
	{PA_23, PIN_GPIO | PIN_IRQ,                                           PIN_NONE, 0},
	// D12: PB00, PWM4, UART0_TX, TMS, SWDIO
	{PB_0,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SWD | PIN_UART, PIN_NONE, 0},
	// D13: PB01, PWM5, UART0_RX, TCK, SWCLK
	{PB_1,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SWD | PIN_UART, PIN_NONE, 0},
	// D14: PB02, PWM6, UART0_CTS, TDO, FD2, BOOT
	{PB_2,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_UART,           PIN_NONE, 0},
	// D15: PA12, ADC2, PWM4, I2C0_SCL, WAKE2
	{PA_12, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C,             PIN_NONE, 0},
	// D16: PB05, PWM1, SPI0_MISO, UART1_CTS, FD1
	{PB_5,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D17: PB06, PWM2, SPI0_CS, FCS
	{PB_6,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI,                       PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[11] = &(lt_arduino_pin_info_list[4]),  // PA_11 (D4)
	[12] = &(lt_arduino_pin_info_list[15]), // PA_12 (D15)
	[13] = &(lt_arduino_pin_info_list[10]), // PA_13 (D10)
	[19] = &(lt_arduino_pin_info_list[9]),  // PA_19 (D9)
	[20] = &(lt_arduino_pin_info_list[8]),  // PA_20 (D8)
	[21] = &(lt_arduino_pin_info_list[5]),  // PA_21 (D5)
	[22] = &(lt_arduino_pin_info_list[6]),  // PA_22 (D6)
	[23] = &(lt_arduino_pin_info_list[11]), // PA_23 (D11)
	[32] = &(lt_arduino_pin_info_list[12]), // PB_0 (D12)
	[33] = &(lt_arduino_pin_info_list[13]), // PB_1 (D13)
	[34] = &(lt_arduino_pin_info_list[14]), // PB_2 (D14)
	[35] = &(lt_arduino_pin_info_list[7]),  // PB_3 (D7)
	[36] = &(lt_arduino_pin_info_list[1]),  // PB_4 (D1)
	[37] = &(lt_arduino_pin_info_list[16]), // PB_5 (D16)
	[38] = &(lt_arduino_pin_info_list[17]), // PB_6 (D17)
	[39] = &(lt_arduino_pin_info_list[0]),  // PB_7 (D0)
	[46] = &(lt_arduino_pin_info_list[3]),  // PB_14 (D3)
	[47] = &(lt_arduino_pin_info_list[2]),  // PB_15 (D2)
};
// clang-format on
