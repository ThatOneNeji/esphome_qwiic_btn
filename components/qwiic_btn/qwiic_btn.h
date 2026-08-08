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

class QwiicBtnComponent : public Component, public i2c::I2CDevice {
  SUB_BINARY_SENSOR(pressed)

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_debounce_time(uint16_t debounce_time) { this->debounce_time_ = debounce_time; }

  // brightness: 0 (off) - 255 (full brightness); sets a steady, non-pulsing LED output
  void set_led_brightness(uint8_t brightness);

 protected:
  uint16_t debounce_time_{20};

  enum ErrorCode {
    NONE = 0,
    ERROR_COMMUNICATION_FAILED,
    ERROR_WRONG_CHIP_ID,
  } error_code_{NONE};

  StatusRegister status_reg_{.reg = 0};

  bool read_status_();
  void clear_events_();
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
