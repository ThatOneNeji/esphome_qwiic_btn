# esphome_qwiic_btn

[![CI](https://github.com/ThatOneNeji/esphome_qwiic_btn/actions/workflows/ci.yml/badge.svg)](https://github.com/ThatOneNeji/esphome_qwiic_btn/actions/workflows/ci.yml)

An [ESPHome](https://esphome.io/) external component for the
[SparkFun Qwiic Button](https://www.sparkfun.com/sparkfun-qwiic-button-red-led.html) /
[Qwiic Switch](https://www.sparkfun.com/sparkfun-qwiic-switch.html) breakout boards: read the button's pressed
state, and control its onboard LED (on/off and brightness) over I2C.

## Installation

Add this repository as a [git external component](https://esphome.io/components/external_components#git):

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/ThatOneNeji/esphome_qwiic_btn
      ref: main
```

Minimum ESPHome version: `2026.7.0`.

## Quick start

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22

qwiic_btn:
  id: my_button
  address: 0x6F

binary_sensor:
  - platform: qwiic_btn
    qwiic_btn_id: my_button
    name: "Qwiic Button"

output:
  - platform: qwiic_btn
    id: qwiic_btn_led
    qwiic_btn_id: my_button

light:
  - platform: monochromatic
    name: "Qwiic Button LED"
    output: qwiic_btn_led
```

See [`example.yaml`](example.yaml) / [`example.esp-idf.yaml`](example.esp-idf.yaml) for complete configs, and
**[components/qwiic_btn/README.md](components/qwiic_btn/README.md)** for the full hardware overview, wiring,
configuration reference, and a troubleshooting guide.

## Components

| Component | Description |
| ----------- | -------------- |
| [`qwiic_btn`](components/qwiic_btn/README.md) | I2C hub, `binary_sensor` (button state), and `output` (LED) platforms for the SparkFun Qwiic Button/Switch. |

## Development

[pre-commit](https://pre-commit.com/) is used for linting/formatting (same tools as [ESPHome](https://github.com/esphome/esphome) itself: `ruff`, `flake8`, `clang-format`, `yamllint`, `pyupgrade`):

```sh
pip install pre-commit
pre-commit install       # optional: run automatically on every commit
pre-commit run --all-files
```

CI (`.github/workflows/ci.yml`) runs the same pre-commit checks, plus `esphome config`/`esphome compile` against
the example configs (via the local-source fixtures in `.github/ci/`) on every push and pull request.

## License

[MIT](LICENSE)
