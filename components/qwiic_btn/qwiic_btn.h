/*
 * Adds support for the SparkFun Qwiic Button/Switch, which communicates over an I2C bus.
 * A firmware running on an ATTiny841 exposes the button's pressed state and a controllable status LED as I2C
 * registers.
 * Firmware: https://github.com/sparkfun/Qwiic_Button (accessed August 2026)
 * SparkFun's Arduino library: https://github.com/sparkfun/SparkFun_Qwiic_Button_Arduino_Library (accessed August
 * 2026)
 */

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"
#include "registers.h"

namespace esphome::qwiic_btn {

// MOMENTARY: the binary_sensor reflects the button's live pressed state.
// LATCH: the binary_sensor toggles on each press (rising edge) and holds that state until the next press.
enum PressMode {
  MOMENTARY_MODE,
  LATCH_MODE,
};

class QwiicBtnComponent : public Component, public i2c::I2CDevice {
  SUB_BINARY_SENSOR(pressed)

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_debounce_time(uint16_t debounce_time) { this->debounce_time_ = debounce_time; }
  void set_press_mode(PressMode press_mode) { this->press_mode_ = press_mode; }
  void set_default_led_brightness(uint8_t brightness) { this->default_led_brightness_ = brightness; }

  // brightness: 0 (off) - 255 (full brightness); sets a steady, non-pulsing LED output
  void set_led_brightness(uint8_t brightness);

 protected:
  uint16_t debounce_time_{20};
  PressMode press_mode_{MOMENTARY_MODE};
  bool last_is_pressed_{false};
  bool latch_state_{false};
  uint8_t default_led_brightness_{0};

  enum ErrorCode {
    NONE = 0,
    ERROR_COMMUNICATION_FAILED,
    ERROR_WRONG_CHIP_ID,
  } error_code_{NONE};

  StatusRegister status_reg_{.reg = 0};

  bool read_status_();
  void clear_events_();
  void update_pressed_state_(bool is_pressed);
  bool write_register_16_(uint8_t a_register, uint16_t value);
};

class QwiicBtnLEDOutput final : public output::FloatOutput {
 public:
  void set_parent(QwiicBtnComponent *parent) { this->parent_ = parent; }

 protected:
  void write_state(float state) override;

  QwiicBtnComponent *parent_;
};

}  // namespace esphome::qwiic_btn
