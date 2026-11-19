/* This file was auto-generated from t3-u.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         18 // Total GPIO count
#define NUM_DIGITAL_PINS   18 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  6  // ADC inputs
#define NUM_ANALOG_OUTPUTS 6  // PWM & DAC outputs
#define PINS_GPIO_MAX      41 // Last usable GPIO number

// SPI Interfaces
// --------------
#define PIN_SPI0_CS    15u // GPIO15
#define PIN_SPI0_MISO  17u // GPIO17
#define PIN_SPI0_MOSI  16u // GPIO16
#define PIN_SPI0_SCK   14u // GPIO14
#define PINS_SPI0_CS   {15u}
#define PINS_SPI0_MISO {17u}
#define PINS_SPI0_MOSI {16u}
#define PINS_SPI0_SCK  {14u}

// Wire Interfaces
// ---------------
#define PIN_WIRE1_SCL_0 0u  // GPIO0
#define PIN_WIRE1_SCL_1 14u // GPIO14
#define PIN_WIRE1_SDA_0 1u  // GPIO1
#define PIN_WIRE1_SDA_1 15u // GPIO15
#define PINS_WIRE1_SCL  {0u, 14u}
#define PINS_WIRE1_SDA  {1u, 15u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  10u // GPIO10
#define PIN_SERIAL0_TX  11u // GPIO11
#define PIN_SERIAL1_RX  1u  // GPIO1
#define PIN_SERIAL1_TX  0u  // GPIO0
#define PIN_SERIAL2_RX  40u // GPIO40
#define PIN_SERIAL2_TX  41u // GPIO41
#define PINS_SERIAL0_RX {10u}
#define PINS_SERIAL0_TX {11u}
#define PINS_SERIAL1_RX {1u}
#define PINS_SERIAL1_TX {0u}
#define PINS_SERIAL2_RX {40u}
#define PINS_SERIAL2_TX {41u}

// Pin function macros
// -------------------
#define PIN_ADC1  25u // GPIO25
#define PIN_ADC2  24u // GPIO24
#define PIN_ADC12 0u  // GPIO0
#define PIN_ADC13 1u  // GPIO1
#define PIN_ADC14 12u // GPIO12
#define PIN_ADC15 13u // GPIO13
#define PIN_CS    15u // GPIO15
#define PIN_MISO  17u // GPIO17
#define PIN_MOSI  16u // GPIO16
#define PIN_P0    0u  // GPIO0
#define PIN_P1    1u  // GPIO1
#define PIN_P10   10u // GPIO10
#define PIN_P11   11u // GPIO11
#define PIN_P12   12u // GPIO12
#define PIN_P13   13u // GPIO13
#define PIN_P14   14u // GPIO14
#define PIN_P15   15u // GPIO15
#define PIN_P16   16u // GPIO16
#define PIN_P17   17u // GPIO17
#define PIN_P18   18u // GPIO18
#define PIN_P24   24u // GPIO24
#define PIN_P25   25u // GPIO25
#define PIN_P32   32u // GPIO32
#define PIN_P34   34u // GPIO34
#define PIN_P36   36u // GPIO36
#define PIN_P40   40u // GPIO40
#define PIN_P41   41u // GPIO41
#define PIN_PWM0  18u // GPIO18
#define PIN_PWM4  24u // GPIO24
#define PIN_PWM5  25u // GPIO25
#define PIN_PWM6  32u // GPIO32
#define PIN_PWM8  34u // GPIO34
#define PIN_PWM10 36u // GPIO36
#define PIN_RX0   10u // GPIO10
#define PIN_RX1   1u  // GPIO1
#define PIN_RX2   40u // GPIO40
#define PIN_SCK   14u // GPIO14
#define PIN_TX0   11u // GPIO11
#define PIN_TX1   0u  // GPIO0
#define PIN_TX2   41u // GPIO41

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define HAS_SPI0                1
#define HAS_WIRE1               1
#define SERIAL_INTERFACES_COUNT 3
#define SPI_INTERFACES_COUNT    1
#define WIRE_INTERFACES_COUNT   1

// Arduino pin names
// -----------------
#define PIN_D0  14u // GPIO14
#define PIN_D1  16u // GPIO16
#define PIN_D2  40u // GPIO40
#define PIN_D3  41u // GPIO41
#define PIN_D4  25u // GPIO25
#define PIN_D5  1u  // GPIO1
#define PIN_D6  0u  // GPIO0
#define PIN_D7  24u // GPIO24
#define PIN_D8  32u // GPIO32
#define PIN_D9  34u // GPIO34
#define PIN_D10 36u // GPIO36
#define PIN_D11 18u // GPIO18
#define PIN_D12 11u // GPIO11
#define PIN_D13 10u // GPIO10
#define PIN_D14 12u // GPIO12
#define PIN_D15 13u // GPIO13
#define PIN_D16 17u // GPIO17
#define PIN_D17 15u // GPIO15
#define PIN_A0  25u // GPIO25
#define PIN_A1  1u  // GPIO1
#define PIN_A2  0u  // GPIO0
#define PIN_A3  24u // GPIO24
#define PIN_A4  12u // GPIO12
#define PIN_A5  13u // GPIO13

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
static const unsigned char A3 = PIN_A3;
static const unsigned char A4 = PIN_A4;
static const unsigned char A5 = PIN_A5;
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
