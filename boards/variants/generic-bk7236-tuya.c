/* This file was auto-generated from generic-bk7236-tuya.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P48
	{GPIO48, PIN_GPIO | PIN_IRQ,                                          PIN_NONE, 0},
	// D1: P24, PWM4, ADC2
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC,                      PIN_NONE, 0},
	// D2: P25, PWM5, ADC1
	{GPIO25, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC,                      PIN_NONE, 0},
	// D3: ADC4, P28
	{GPIO28, PIN_GPIO | PIN_IRQ | PIN_ADC,                                PIN_NONE, 0},
	// D4: P32, PWM6
	{GPIO32, PIN_GPIO | PIN_IRQ | PIN_PWM,                                PIN_NONE, 0},
	// D5: P34, PWM8
	{GPIO34, PIN_GPIO | PIN_IRQ | PIN_PWM,                                PIN_NONE, 0},
	// D6: P36, PWM10
	{GPIO36, PIN_GPIO | PIN_IRQ | PIN_PWM,                                PIN_NONE, 0},
	// D7: P1, UART2_RX, ADC13, I2C1_SDA, SWDIO
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_SWD | PIN_UART, PIN_NONE, 0},
	// D8: P0, UART2_TX, ADC12, I2C1_SCL, SWCLK
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_SWD | PIN_UART, PIN_NONE, 0},
	// D9: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                               PIN_NONE, 0},
	// D10: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                               PIN_NONE, 0},
	// D11: P12, ADC14, DP
	{GPIO12, PIN_GPIO | PIN_IRQ | PIN_ADC,                                PIN_NONE, 0},
	// D12: P13, ADC15, DN
	{GPIO13, PIN_GPIO | PIN_IRQ | PIN_ADC,                                PIN_NONE, 0},
	// D13: P14, SCK, I2C1_SCL
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,                      PIN_NONE, 0},
	// D14: P15, CS, I2C1_SDA
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,                      PIN_NONE, 0},
	// D15: P16, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                                PIN_NONE, 0},
	// D16: P17, MISO
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_SPI,                                PIN_NONE, 0},
	// D17: P18, PWM0
	{GPIO18, PIN_GPIO | PIN_IRQ | PIN_PWM,                                PIN_NONE, 0},
	// D18: P19, PWM1
	{GPIO19, PIN_GPIO | PIN_IRQ | PIN_PWM,                                PIN_NONE, 0},
	// D19: P41, UART3_TX
	{GPIO41, PIN_GPIO | PIN_IRQ | PIN_UART,                               PIN_NONE, 0},
	// D20: P40, UART3_RX
	{GPIO40, PIN_GPIO | PIN_IRQ | PIN_UART,                               PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[8]),  // GPIO0 (D8)
	[1]  = &(lt_arduino_pin_info_list[7]),  // GPIO1 (D7)
	[10] = &(lt_arduino_pin_info_list[10]), // GPIO10 (D10)
	[11] = &(lt_arduino_pin_info_list[9]),  // GPIO11 (D9)
	[12] = &(lt_arduino_pin_info_list[11]), // GPIO12 (D11)
	[13] = &(lt_arduino_pin_info_list[12]), // GPIO13 (D12)
	[14] = &(lt_arduino_pin_info_list[13]), // GPIO14 (D13)
	[15] = &(lt_arduino_pin_info_list[14]), // GPIO15 (D14)
	[16] = &(lt_arduino_pin_info_list[15]), // GPIO16 (D15)
	[17] = &(lt_arduino_pin_info_list[16]), // GPIO17 (D16)
	[18] = &(lt_arduino_pin_info_list[17]), // GPIO18 (D17)
	[19] = &(lt_arduino_pin_info_list[18]), // GPIO19 (D18)
	[24] = &(lt_arduino_pin_info_list[1]),  // GPIO24 (D1)
	[25] = &(lt_arduino_pin_info_list[2]),  // GPIO25 (D2)
	[28] = &(lt_arduino_pin_info_list[3]),  // GPIO28 (D3)
	[32] = &(lt_arduino_pin_info_list[4]),  // GPIO32 (D4)
	[34] = &(lt_arduino_pin_info_list[5]),  // GPIO34 (D5)
	[36] = &(lt_arduino_pin_info_list[6]),  // GPIO36 (D6)
	[40] = &(lt_arduino_pin_info_list[20]), // GPIO40 (D20)
	[41] = &(lt_arduino_pin_info_list[19]), // GPIO41 (D19)
	[48] = &(lt_arduino_pin_info_list[0]),  // GPIO48 (D0)
};
// clang-format on
