# qwiic_btn

ESPHome component for the [SparkFun Qwiic Button](https://www.sparkfun.com/sparkfun-qwiic-button-red-led.html) /
[Qwiic Switch](https://www.sparkfun.com/sparkfun-qwiic-switch.html) breakout boards.

- [Hardware](#hardware)
- [Hub configuration (`qwiic_btn:`)](#hub-configuration-qwiic_btn)
- [Reading the button (`binary_sensor:`)](#reading-the-button-binary_sensor)
- [Controlling the LED (`output:` / `light:`)](#controlling-the-led-output--light)
- [Full example](#full-example)
- [Troubleshooting](#troubleshooting)

## Hardware

The board is a small I2C peripheral built around an ATtiny841: a physical button/switch on one side, and an
addressable status LED (independent of the button) on the other. It connects over the
[Qwiic connector system](https://www.sparkfun.com/qwiic) (JST SH 4-pin: 3.3V, GND, SDA, SCL) — no soldering
required, just chain it onto any other Qwiic device or a breakout onto your ESP's I2C pins.

| | |
|---|---|
| Default I2C address | `0x6F` |
| Expected chip/device ID | `0x5D` (read from register `0x00`; the component checks this at boot and fails setup if it doesn't match) |
| Logic level | 3.3V (do not power from 5V) |
| Firmware source | <https://github.com/sparkfun/Qwiic_Button> |
| Reference Arduino library | <https://github.com/sparkfun/SparkFun_Qwiic_Button_Arduino_Library> |
| Hookup guide | <https://learn.sparkfun.com/tutorials/qwiic-button-hookup-guide> |

The board's I2C address can be changed in hardware (solder jumper) or via the device's own `I2C_ADDRESS` register;
this component does not currently expose a way to change it remotely — set `address:` to match whatever the board
is already configured for.

Wiring straight to an ESP32 (no other Qwiic devices on the bus):

| Qwiic pin | ESP32 |
|-----------|-------|
| 3.3V      | 3.3V  |
| GND       | GND   |
| SDA       | any GPIO (e.g. `GPIO21`) |
| SCL       | any GPIO (e.g. `GPIO22`) |

If you see intermittent `Communication failed` warnings in the logs but the device otherwise shows up fine at
boot, check your wiring/cabling first — see [Troubleshooting](#troubleshooting).

## Hub configuration (`qwiic_btn:`)

The `qwiic_btn:` component is the I2C "hub" — it owns the connection to the board. The `binary_sensor:` and
`output:` platforms below both attach to it by ID; you need exactly one `qwiic_btn:` entry per physical board.

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22

qwiic_btn:
  id: my_button
  address: 0x6F                 # optional, this is the factory default
  debounce_time: 20ms           # optional, mechanical debounce time written to the device, default 20ms, max 65535ms
  default_led_brightness: 0%    # optional, LED brightness forced at boot, before any automation runs, default 0%
```

| Option                  | Default | Description                                                                                                                                                                            |
|--------------------------|---------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `id`                      | -       | ID to reference this hub from `binary_sensor:`/`output:` entries.                                                                                                                     |
| `i2c_id`                  | -       | ID of the `i2c:` bus to use, if you have more than one.                                                                                                                                |
| `address`                 | `0x6F`  | I2C address of the board.                                                                                                                                                              |
| `debounce_time`           | `20ms`  | Mechanical debounce time (max `65535ms`), written to the device's own debounce register.                                                                                              |
| `default_led_brightness`  | `0%`    | LED brightness written directly to the device during `setup()`, before any `light`/`output` automation runs. Makes boot-time LED state deterministic rather than depending on whatever brightness the device happened to power on with or last had. **Note:** this writes straight to the hardware, bypassing the `light` component's own state tracking — if you have a `light:` on top of the `output:`, it will still report "off"/0% right after boot until you interact with it (press the button, change the brightness), even though the physical LED is already at this brightness. If you need Home Assistant/the `light` entity itself to reflect this value at boot too, use `esphome: on_boot: then: - light.turn_on: {id: ..., brightness: ...}` instead of this option. |

## Reading the button (`binary_sensor:`)

```yaml
binary_sensor:
  - platform: qwiic_btn
    qwiic_btn_id: my_button
    name: "Qwiic Button"
    mode: MOMENTARY  # optional, MOMENTARY (default) or LATCH
```

Accepts all standard `binary_sensor` options (`device_class`, `filters`, `on_press`/`on_release`/`on_state`
automations, etc). The state is polled every loop; there's no interrupt line.

| `mode`      | Behavior                                                                                                                          |
|-------------|------------------------------------------------------------------------------------------------------------------------------------|
| `MOMENTARY` | (default) State reflects the live (debounced) `isPressed` bit of the device's status register.                                   |
| `LATCH`     | State toggles each time the button is pressed down, and holds until the next press (push-on/push-off). Resets to `false` on boot. |

## Controlling the LED (`output:` / `light:`)

The LED is exposed as a standard ESPHome `output` (`0.0` = off, `1.0` = full brightness), so the idiomatic way to
use it is wiring it up to a `light` for on/off + brightness control (and, through that, Home Assistant):

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

Or drive the `output` directly from an automation, without a `light:` at all:

```yaml
on_...:
  - output.turn_on: qwiic_btn_led
  - output.set_level:
      id: qwiic_btn_led
      level: 50%
  - output.turn_off: qwiic_btn_led
```

### Linking the button to the LED

A common pattern is having the button's own press toggle its LED (e.g. as a lit push-on/push-off switch), with a
separately-controllable brightness (for example from a Home Assistant `number`). Because `light.control`/
`light.turn_on` only change the fields you explicitly pass, and ESPHome forces a light to 100% if you turn it on
without specifying a brightness while its last-known brightness was 0%, **always set `state` and `brightness`
together** in the same call — setting only `state` can cause an unexpected jump to full brightness:

```yaml
binary_sensor:
  - platform: qwiic_btn
    id: my_button_sensor
    qwiic_btn_id: my_button
    name: "Qwiic Button"
    mode: LATCH
    on_state_change:
      - then:
          - light.control:
              id: my_button_led
              state: !lambda "return id(my_button_sensor).state;"
              brightness: !lambda "return id(led_level).state / 100.0;"

output:
  - platform: qwiic_btn
    id: qwiic_btn_led
    qwiic_btn_id: my_button

light:
  - platform: monochromatic
    id: my_button_led
    name: "Qwiic Button LED"
    output: qwiic_btn_led
    internal: true  # only exposing the number + binary_sensor, not the raw light entity

number:
  - platform: template
    name: "LED Brightness"
    id: led_level
    min_value: 0
    max_value: 100
    step: 1
    mode: box
    optimistic: true
    restore_value: true
    on_value:
      then:
        - light.turn_on:
            id: my_button_led
            brightness: !lambda "return id(led_level).state / 100.0;"
```

## Full example

See [`example.yaml`](../../example.yaml) (Arduino framework) and [`example.esp-idf.yaml`](../../example.esp-idf.yaml)
(ESP-IDF framework) in the repository root for complete, buildable configs.

## Troubleshooting

**Intermittent `Communication failed` warnings, especially right after boot, but the device otherwise works.**
Check the physical I2C wiring/cabling first — loose or marginal connections on the Qwiic cable are the most common
cause. If it persists, try lowering the `i2c:` bus `frequency:`.

**`internal: true` on the `light:`/`binary_sensor:` doesn't hide it from the device's own web page.** Check your
`web_server:` config for `include_internal: true` — that option deliberately shows internal entities on the local
page (while still hiding them from Home Assistant), overriding `internal:`.

**Pressing the button turns the LED on, but it ramps up to 100% instead of the brightness you expected.** This is
ESPHome's `light` component intentionally making an otherwise-invisible "on" call visible: if you call
`light.control`/`light.turn_on` with only `state:` and no `brightness:`, and the light's last-known brightness was
`0`, ESPHome forces it to 100% rather than turning "on" at 0% brightness. Always set `brightness:` alongside
`state:` — see [Linking the button to the LED](#linking-the-button-to-the-led) above.

**`default_led_brightness` doesn't seem to survive boot / gets overwritten back to off.** This can happen if a
`light:` on top of the `output:` has a `restore_mode` that turns it off at boot (e.g. the default `ALWAYS_OFF`) —
its own boot-time state application can land after this component's and win. The component defers its own write
briefly to land after that, but if you're still seeing this, enable `logger: level: DEBUG` (or `VERBOSE`) and look
for `Setting LED brightness to N` / `Failed to set LED brightness` log lines from the `qwiic_btn` tag to confirm
whether the write is happening and succeeding.
