# qwiic_btn

ESPHome component for the [SparkFun Qwiic Button/Switch](https://www.sparkfun.com/products/15932) breakout.

The board runs an ATTiny841 firmware that exposes the button's pressed state and a controllable status LED over
I2C. This component owns the I2C connection (the "hub") and two platforms hang off it: `binary_sensor` for the
button state and `output` for the LED.

## Hub configuration

```yaml
qwiic_btn:
  id: my_button
  address: 0x6F                 # optional, this is the factory default
  debounce_time: 20ms           # optional, mechanical debounce time written to the device, default 20ms, max 65535ms
  default_led_brightness: 0%    # optional, LED brightness forced at boot, before any automation runs, default 0%
```

| Option                    | Default | Description                                                                                   |
|----------------------------|---------|-------------------------------------------------------------------------------------------------|
| `id`                        | -       | ID to reference this hub from `binary_sensor:`/`output:` entries.                              |
| `address`                   | `0x6F`  | I2C address of the board.                                                                       |
| `debounce_time`              | `20ms`  | Mechanical debounce time (max `65535ms`), configured on the device.                             |
| `default_led_brightness`     | `0%`    | LED brightness written to the device during `setup()`, before `light`/`output` automations run. Overrides whatever the device's own power-on/leftover brightness happens to be, so boot behavior is deterministic. |

## Reading the button

```yaml
binary_sensor:
  - platform: qwiic_btn
    qwiic_btn_id: my_button
    name: "Qwiic Button"
    mode: MOMENTARY  # optional, MOMENTARY (default) or LATCH
```

Accepts all standard `binary_sensor` options (`device_class`, `filters`, `on_press`/`on_release` automations, etc).

| `mode`      | Behavior                                                                                                                            |
|-------------|----------------------------------------------------------------------------------------------------------------------------------------|
| `MOMENTARY` | (default) State reflects the live (debounced) `isPressed` bit of the device's status register.                                       |
| `LATCH`     | State toggles each time the button is pressed down, and holds until the next press (push-on/push-off). Resets to `false` on boot.    |

## Controlling the LED

The LED is exposed as a standard ESPHome `output` (0.0 = off, 1.0 = full brightness), so it can be wired up to a
`light` for on/off + brightness control from Home Assistant:

```yaml
output:
  - platform: qwiic_btn
    id: qwiic_btn_led
    qwiic_btn_id: my_button

light:
  - platform: monochromatic
    name: "Qwiic Button LED"
    output: qwiic_btn_led
```

Or drive it directly from an automation without a `light`:

```yaml
on_...:
  - output.turn_on: qwiic_btn_led
  - output.set_level:
      id: qwiic_btn_led
      level: 50%
  - output.turn_off: qwiic_btn_led
```
