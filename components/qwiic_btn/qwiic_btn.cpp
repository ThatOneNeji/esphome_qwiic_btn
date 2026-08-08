#include "qwiic_btn.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome::qwiic_btn {

static const char *const TAG = "qwiic_btn";

void QwiicBtnComponent::setup() {
  // Verify I2C communication by reading and verifying the chip ID
  uint8_t chip_id;
  if (!this->read_byte(QWIIC_BUTTON_ID, &chip_id)) {
    ESP_LOGE(TAG, "Failed to read chip ID");
    this->error_code_ = ERROR_COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  if (chip_id != QWIIC_BUTTON_DEVICE_ID) {
    ESP_LOGE(TAG, "Unknown chip ID 0x%02X", chip_id);
    this->error_code_ = ERROR_WRONG_CHIP_ID;
    this->mark_failed();
    return;
  }

  if (!this->write_register_word_(QWIIC_BUTTON_DEBOUNCE_TIME, this->debounce_time_)) {
    ESP_LOGE(TAG, "Failed to configure debounce time");
    this->error_code_ = ERROR_COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  // Force the LED to a known state on every boot. The device otherwise keeps whatever brightness it last had
  // (its own power-on default, or a leftover value from before a reboot), independent of anything the `light`/
  // `output` platforms do once they start up.
  //
  // This has to be deferred: a `light` on top of our `output` applies its own restore_mode state on the first
  // loop() tick (after every component's setup() has run), which would otherwise land after — and silently
  // overwrite — a write made here directly in setup(). The delay just needs to clear that first tick.
  this->set_timeout(100, [this]() { this->set_led_brightness(this->default_led_brightness_); });

  // Publish the starting state so the binary_sensor isn't left in an unknown state. In LATCH_MODE this is just
  // the initial (unpressed) latch_state_, not the raw pressed level.
  if (this->read_status_()) {
    this->last_is_pressed_ = this->status_reg_.is_pressed;
    if (this->pressed_binary_sensor_ != nullptr) {
      bool initial_state = this->press_mode_ == LATCH_MODE ? this->latch_state_ : this->status_reg_.is_pressed;
      this->pressed_binary_sensor_->publish_state(initial_state);
    }
  }
}

void QwiicBtnComponent::loop() {
  if (!this->read_status_()) {
    ESP_LOGW(TAG, ESP_LOG_MSG_COMM_FAIL);
    return;
  }

  this->update_pressed_state_(this->status_reg_.is_pressed);

  if (this->status_reg_.event_available) {
    this->clear_events_();
  }
}

bool QwiicBtnComponent::read_status_() { return this->read_byte(QWIIC_BUTTON_STATUS, &this->status_reg_.reg); }

void QwiicBtnComponent::update_pressed_state_(bool is_pressed) {
  if (this->press_mode_ == LATCH_MODE) {
    // Toggle only on the rising edge (button just pressed down), then hold until the next press
    if (is_pressed && !this->last_is_pressed_) {
      this->latch_state_ = !this->latch_state_;
      if (this->pressed_binary_sensor_ != nullptr)
        this->pressed_binary_sensor_->publish_state(this->latch_state_);
    }
  } else if (this->pressed_binary_sensor_ != nullptr) {
    this->pressed_binary_sensor_->publish_state(is_pressed);
  }
  this->last_is_pressed_ = is_pressed;
}

void QwiicBtnComponent::clear_events_() {
  // Writing 0x00 clears event_available and has_been_clicked; is_pressed simply reflects live hardware state
  if (!this->write_byte(QWIIC_BUTTON_STATUS, 0x00))
    ESP_LOGW(TAG, "Failed to clear events");
}

void QwiicBtnComponent::set_led_brightness(uint8_t brightness) {
  ESP_LOGV(TAG, "Setting LED brightness to %u", brightness);
  // Steady (non-pulsing) brightness: granularity=1, cycle/off time=0, mirroring the Arduino library's
  // LEDon()/LEDoff() helpers.
  bool success = this->write_byte(QWIIC_BUTTON_LED_BRIGHTNESS, brightness);
  success &= this->write_byte(QWIIC_BUTTON_LED_PULSE_GRANULARITY, 1);
  success &= this->write_register_word_(QWIIC_BUTTON_LED_PULSE_CYCLE_TIME, 0);
  success &= this->write_register_word_(QWIIC_BUTTON_LED_PULSE_OFF_TIME, 0);
  if (!success) {
    ESP_LOGW(TAG, "Failed to set LED brightness");
  }
}

bool QwiicBtnComponent::write_register_word_(uint8_t a_register, uint16_t value) {
  const uint8_t data[2] = {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>(value >> 8)};
  return this->write_bytes(a_register, data, 2);
}

void QwiicBtnComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Qwiic Button:");
  ESP_LOGCONFIG(TAG, "  Press Mode: %s", this->press_mode_ == LATCH_MODE ? "LATCH" : "MOMENTARY");
  ESP_LOGCONFIG(TAG, "  Default LED Brightness: %u", this->default_led_brightness_);

  switch (this->error_code_) {
    case NONE:
      break;
    case ERROR_COMMUNICATION_FAILED:
      ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
      break;
    case ERROR_WRONG_CHIP_ID:
      ESP_LOGE(TAG, "Unknown chip ID");
      break;
  }

  LOG_I2C_DEVICE(this);
  LOG_BINARY_SENSOR("  ", "Pressed", this->pressed_binary_sensor_);
}

void QwiicBtnLEDOutput::write_state(float state) {
  this->parent_->set_led_brightness(static_cast<uint8_t>(lroundf(state * 255.0f)));
}

}  // namespace esphome::qwiic_btn
