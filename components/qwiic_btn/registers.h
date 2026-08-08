/*
 * Virtual I2C register map of the SparkFun Qwiic Button / Qwiic Switch.
 * Firmware (ATTiny841): https://github.com/sparkfun/Qwiic_Button/blob/master/Firmware/Qwiic_Button/registers.h
 * Arduino library: https://github.com/sparkfun/SparkFun_Qwiic_Button_Arduino_Library
 *
 * Multi-byte registers are laid out little-endian, matching the AVR firmware's native struct layout: the low byte
 * lives at the lower register address and the high byte at the next (auto-incremented) address.
 */

#pragma once

#include <cstdint>

namespace esphome::qwiic_btn {

enum QwiicButtonRegister : uint8_t {
  QWIIC_BUTTON_ID = 0x00,
  QWIIC_BUTTON_FIRMWARE_MINOR = 0x01,
  QWIIC_BUTTON_FIRMWARE_MAJOR = 0x02,
  QWIIC_BUTTON_STATUS = 0x03,
  QWIIC_BUTTON_INTERRUPT_CONFIG = 0x04,
  QWIIC_BUTTON_DEBOUNCE_TIME = 0x05,  // uint16_t, milliseconds
  QWIIC_BUTTON_PRESSED_QUEUE_STATUS = 0x07,
  QWIIC_BUTTON_PRESSED_QUEUE_FRONT = 0x08,  // uint32_t
  QWIIC_BUTTON_PRESSED_QUEUE_BACK = 0x0C,   // uint32_t
  QWIIC_BUTTON_CLICKED_QUEUE_STATUS = 0x10,
  QWIIC_BUTTON_CLICKED_QUEUE_FRONT = 0x11,    // uint32_t
  QWIIC_BUTTON_CLICKED_QUEUE_BACK = 0x15,     // uint32_t
  QWIIC_BUTTON_LED_BRIGHTNESS = 0x19,         // uint8_t, 0-255
  QWIIC_BUTTON_LED_PULSE_GRANULARITY = 0x1A,  // uint8_t
  QWIIC_BUTTON_LED_PULSE_CYCLE_TIME = 0x1B,   // uint16_t, milliseconds
  QWIIC_BUTTON_LED_PULSE_OFF_TIME = 0x1D,     // uint16_t, milliseconds
  QWIIC_BUTTON_I2C_ADDRESS = 0x1F,
};

static const uint8_t QWIIC_BUTTON_DEVICE_ID = 0x5D;
static const uint8_t QWIIC_BUTTON_DEFAULT_I2C_ADDRESS = 0x6F;

// QWIIC_BUTTON_STATUS bitfield
union StatusRegister {
  struct {
    bool event_available : 1;  // set by the device on a new press/click event; write 0 to clear
    bool has_been_clicked : 1;
    bool is_pressed : 1;
    bool : 5;
  };
  uint8_t reg;
};

}  // namespace esphome::qwiic_btn
