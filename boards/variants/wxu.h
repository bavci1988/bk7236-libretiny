/* This file was auto-generated from wxu.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         18 // Total GPIO count
#define NUM_DIGITAL_PINS   18 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  3  // ADC inputs
#define NUM_ANALOG_OUTPUTS 16 // PWM & DAC outputs
#define PINS_GPIO_MAX      47 // Last usable GPIO number

// SPI Interfaces
// --------------
#define PIN_SPI0_CS_0   21u // PA_21
#define PIN_SPI0_CS_1   38u // PB_6
#define PIN_SPI0_MISO_0 11u // PA_11
#define PIN_SPI0_MISO_1 20u // PA_20
#define PIN_SPI0_MISO_2 37u // PB_5
#define PIN_SPI0_MOSI_0 19u // PA_19
#define PIN_SPI0_MOSI_1 36u // PB_4
#define PIN_SPI0_SCK_0  22u // PA_22
#define PIN_SPI0_SCK_1  39u // PB_7
#define PINS_SPI0_CS    {21u, 38u}
#define PINS_SPI0_MISO  {11u, 20u, 37u}
#define PINS_SPI0_MOSI  {19u, 36u}
#define PINS_SPI0_SCK   {22u, 39u}

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL  12u // PA_12
#define PIN_WIRE0_SDA  13u // PA_13
#define PIN_WIRE1_SCL  46u // PB_14
#define PIN_WIRE1_SDA  47u // PB_15
#define PINS_WIRE0_SCL {12u}
#define PINS_WIRE0_SDA {13u}
#define PINS_WIRE1_SCL {46u}
#define PINS_WIRE1_SDA {47u}

// Serial ports
// ------------
#define PIN_SERIAL0_CTS  34u // PB_2
#define PIN_SERIAL0_RTS  35u // PB_3
#define PIN_SERIAL0_RX   33u // PB_1
#define PIN_SERIAL0_TX   32u // PB_0
#define PIN_SERIAL1_CTS  37u // PB_5
#define PIN_SERIAL1_RTS  36u // PB_4
#define PIN_SERIAL1_RX   47u // PB_15
#define PIN_SERIAL1_TX   46u // PB_14
#define PIN_SERIAL2_CTS  20u // PA_20
#define PIN_SERIAL2_RTS  19u // PA_19
#define PIN_SERIAL2_RX   21u // PA_21
#define PIN_SERIAL2_TX   22u // PA_22
#define PINS_SERIAL0_CTS {34u}
#define PINS_SERIAL0_RTS {35u}
#define PINS_SERIAL0_RX  {33u}
#define PINS_SERIAL0_TX  {32u}
#define PINS_SERIAL1_CTS {37u}
#define PINS_SERIAL1_RTS {36u}
#define PINS_SERIAL1_RX  {47u}
#define PINS_SERIAL1_TX  {46u}
#define PINS_SERIAL2_CTS {20u}
#define PINS_SERIAL2_RTS {19u}
#define PINS_SERIAL2_RX  {21u}
#define PINS_SERIAL2_TX  {22u}

// Pin function macros
// -------------------
#define PIN_ADC1  11u // PA_11
#define PIN_ADC2  12u // PA_12
#define PIN_ADC3  13u // PA_13
#define PIN_CTS0  34u // PB_2
#define PIN_CTS1  37u // PB_5
#define PIN_CTS2  20u // PA_20
#define PIN_MISO0 37u // PB_5
#define PIN_PA11  11u // PA_11
#define PIN_PA12  12u // PA_12
#define PIN_PA13  13u // PA_13
#define PIN_PA19  19u // PA_19
#define PIN_PA20  20u // PA_20
#define PIN_PA21  21u // PA_21
#define PIN_PA22  22u // PA_22
#define PIN_PA23  23u // PA_23
#define PIN_PB00  32u // PB_0
#define PIN_PB01  33u // PB_1
#define PIN_PB02  34u // PB_2
#define PIN_PB03  35u // PB_3
#define PIN_PB04  36u // PB_4
#define PIN_PB05  37u // PB_5
#define PIN_PB06  38u // PB_6
#define PIN_PB07  39u // PB_7
#define PIN_PB14  46u // PB_14
#define PIN_PB15  47u // PB_15
#define PIN_PWM5  33u // PB_1
#define PIN_PWM7  35u // PB_3
#define PIN_RTS0  35u // PB_3
#define PIN_RTS1  36u // PB_4
#define PIN_RTS2  19u // PA_19
#define PIN_RX0   33u // PB_1
#define PIN_RX1   47u // PB_15
#define PIN_RX2   21u // PA_21
#define PIN_SCL0  12u // PA_12
#define PIN_SCL1  46u // PB_14
#define PIN_SDA0  13u // PA_13
#define PIN_SDA1  47u // PB_15
#define PIN_TX0   32u // PB_0
#define PIN_TX1   46u // PB_14
#define PIN_TX2   22u // PA_22

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define HAS_SPI0                1
#define HAS_WIRE0               1
#define HAS_WIRE1               1
#define SERIAL_INTERFACES_COUNT 3
#define SPI_INTERFACES_COUNT    1
#define WIRE_INTERFACES_COUNT   2

// Arduino pin names
// -----------------
#define PIN_D0  39u // PB_7
#define PIN_D1  36u // PB_4
#define PIN_D2  47u // PB_15
#define PIN_D3  46u // PB_14
#define PIN_D4  11u // PA_11
#define PIN_D5  21u // PA_21
#define PIN_D6  22u // PA_22
#define PIN_D7  35u // PB_3
#define PIN_D8  20u // PA_20
#define PIN_D9  19u // PA_19
#define PIN_D10 13u // PA_13
#define PIN_D11 23u // PA_23
#define PIN_D12 32u // PB_0
#define PIN_D13 33u // PB_1
#define PIN_D14 34u // PB_2
#define PIN_D15 12u // PA_12
#define PIN_D16 37u // PB_5
#define PIN_D17 38u // PB_6
#define PIN_A0  11u // PA_11
#define PIN_A1  12u // PA_12
#define PIN_A2  13u // PA_13

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;
static const unsigned char D8 = PIN_D8;
static const unsigned char D9 = PIN_D9;
static const unsigned char D10 = PIN_D10;
static const unsigned char D11 = PIN_D11;
static const unsigned char D12 = PIN_D12;
static const unsigned char D13 = PIN_D13;
static const unsigned char D14 = PIN_D14;
static const unsigned char D15 = PIN_D15;
static const unsigned char D16 = PIN_D16;
static const unsigned char D17 = PIN_D17;
