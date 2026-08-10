# Unihiker K10 Skills

Local Codex skills for working with the DFRobot Unihiker K10 board. Install these folders into your Codex skills directory, then Codex can load the right K10 reference material, helper scripts, firmware bundle, and troubleshooting notes when you ask it to build or flash K10 projects.

> **TTS compatibility:** K10 speech synthesis exists only in the Chinese firmware. The skills must verify that firmware variant before using Arduino/PlatformIO `ASR::setAsrSpeed()` / `ASR::speak()` or MicroPython TTS APIs. CN model-data selection alone is not proof of Chinese-firmware TTS support.

## What's Included

| Skill | Use it for | Current notes |
| --- | --- | --- |
| `unihiker-k10-arduino` | Arduino/C++ sketches, K10 BSP setup, serial upload, Arduino API lookup, screen/sensor/RGB/audio/AI examples. | Uses FQBN `UNIHIKER:esp32:k10`; includes Windows `arduino-cli.exe`; documents Arduino CLI `build_cache.*` rather than older cache keys. |
| `unihiker-k10-platformio` | PlatformIO CLI projects, K10 Arduino/C++ builds, serial upload, monitoring, ASR audio diagnostics, and workshop offline support bundles. | Uses DFRobot's `platform-unihiker`; documents stale startup DMA and ES7243E/I2S wake-word troubleshooting; includes macOS and Windows offline installers. |
| `unihiker-k10-box-platformio` | PlatformIO/LVGL projects for the K10 information-technology experiment box, including all box sensors and actuators. | Separates K10-native and box hardware; covers QMI8658, line tracking, IO controller, LVGL formatting crashes, motor startup, and safe actuator tests. |
| `k10-compile-server` | Remote LAN compilation for PlatformIO K10 projects, browser Web Serial flashing, firmware download, and server-side USB flash. | Use an existing HTTPS compile server on port 8900, or self-host from `rockets-cn/unihiker-k10-compile-server`; preferred client upload needs only Chrome/Edge. |
| `unihiker-k10-micropython` | Flashing MicroPython, uploading `main.py`, MicroPython API lookup, REPL-oriented troubleshooting. | Bundles K10 MicroPython firmware `v0.9.2`; only `main.py` auto-runs after reset. |
| `unihiker-k10-ota` | Adding HTTP OTA update support to Arduino or PlatformIO projects. | Requires a custom partition table with `ota_0` and `ota_1`; AI projects must preserve the K10 model partitions. |

## Agent Usage

Agents should read `AGENT_INDEX.md` first, then load only the matching skill for the user's task. Use this routing:

| User intent | Load |
| --- | --- |
| Arduino sketch, C++ API, serial upload, K10 BSP setup | `unihiker-k10-arduino/SKILL.md` |
| PlatformIO CLI project, PlatformIO upload, offline workshop bundle | `unihiker-k10-platformio/SKILL.md` |
| K10 experiment-box sensors, QMI8658, line tracker, LVGL dashboard, motors, buzzer, or traffic lights | `unihiker-k10-box-platformio/SKILL.md` |
| LAN compile server build, browser Web Serial flash, firmware download, server-side flash | `k10-compile-server/SKILL.md` |
| MicroPython firmware, `main.py`, `mpremote`, Python API | `unihiker-k10-micropython/SKILL.md` |
| Wireless Arduino firmware update, HTTP OTA, ESP-NOW maintenance OTA mode | `unihiker-k10-ota/SKILL.md` |
| AI model recovery, voice/TTS/face AI with OTA partitions | `references/k10-ai-model-flash.md` plus the matching toolchain skill |

Do not guess K10 APIs from general ESP32 or MicroPython knowledge. Read the selected skill and its relevant `references/` material before writing code or commands.

## Install

Copy or symlink every skill folder into your Codex skills directory:

```bash
mkdir -p ~/.agents/skills
cp -R unihiker-k10-arduino ~/.agents/skills/
cp -R unihiker-k10-platformio ~/.agents/skills/
cp -R unihiker-k10-box-platformio ~/.agents/skills/
cp -R k10-compile-server ~/.agents/skills/
cp -R unihiker-k10-micropython ~/.agents/skills/
cp -R unihiker-k10-ota ~/.agents/skills/
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE\.agents\skills"
Copy-Item -Recurse .\unihiker-k10-arduino "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\unihiker-k10-platformio "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\unihiker-k10-box-platformio "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\k10-compile-server "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\unihiker-k10-micropython "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\unihiker-k10-ota "$env:USERPROFILE\.agents\skills\"
```

Restart Codex after installing or updating skills so it reloads the skill metadata.

## Arduino Quick Start

1. Install `arduino-cli`. On Windows, the Arduino skill already includes `arduino-cli.exe` for offline use.

2. Configure the K10 BSP and ESP32 Core package indexes. Use the China mirror pair when you are in mainland China; otherwise use the official Espressif URL.

   ```bash
   arduino-cli config add board_manager.additional_urls https://downloadcd.dfrobot.com.cn/UNIHIKER/package_unihiker_index.json
   arduino-cli config add board_manager.additional_urls https://dl.espressif.com/dl/package_esp32_index.json
   arduino-cli core update-index
   arduino-cli core install UNIHIKER:esp32
   ```

   Mainland China ESP32 Core mirror:

   ```bash
   arduino-cli config add board_manager.additional_urls https://jihulab.com/esp-mirror/espressif/arduino-esp32/-/raw/gh-pages/package_esp32_index_cn.json
   ```

3. Enable Arduino CLI's official build cache for faster repeat builds:

   ```bash
   arduino-cli config set build_cache.path ~/.cache/arduino-build-cache
   arduino-cli config set build_cache.compilations_before_purge 0
   ```

   On Windows PowerShell:

   ```powershell
   arduino-cli config set build_cache.path "$env:LOCALAPPDATA\arduino\build-cache"
   arduino-cli config set build_cache.compilations_before_purge 0
   ```

4. Put each sketch in a same-named directory, for example `hello/hello.ino`.

5. Upload by USB:

   ```bash
   bash unihiker-k10-arduino/scripts/upload-arduino.sh hello/hello.ino /dev/cu.usbmodem2201
   ```

   On Windows PowerShell:

   ```powershell
   .\unihiker-k10-arduino\scripts\upload-arduino.ps1 .\hello\hello.ino COM3
   ```

For optimized Arduino OTA-capable builds, use the `compile-ota` helper from the Arduino skill after adding an OTA partition table:

```bash
bash unihiker-k10-arduino/scripts/compile-ota.sh hello
```

```powershell
.\unihiker-k10-arduino\scripts\compile-ota.ps1 hello
```

## PlatformIO Quick Start

Use the PlatformIO skill when you want K10 Arduino/C++ development through `pio` instead of `arduino-cli`.

1. Install PlatformIO Core and initialize a K10 project:

   ```bash
   bash unihiker-k10-platformio/scripts/init-k10-platformio-project.sh my-k10-project
   ```

2. Build and upload:

   ```bash
   pio run -d my-k10-project
   pio run -d my-k10-project -t upload --upload-port /dev/cu.usbmodem2201
   ```

3. Monitor serial output:

   ```bash
   pio device monitor -d my-k10-project --port /dev/cu.usbmodem2201 --baud 115200
   ```

For workshops, prepare one offline bundle or self-contained installer per OS/CPU architecture so students do not all download the K10 framework and ESP32 toolchains at the same time:

```bash
bash unihiker-k10-platformio/scripts/prepare-offline-bundle.sh /tmp/k10-platformio-bundle.tgz
bash unihiker-k10-platformio/scripts/install-offline-bundle.sh /tmp/k10-platformio-bundle.tgz
```

On a prepared Windows x64 teacher machine, create a USB-friendly self-extracting installer after one successful K10 PlatformIO build:

```powershell
powershell -ExecutionPolicy Bypass -File .\unihiker-k10-platformio\scripts\prepare-windows-offline-installer.ps1 C:\tmp\K10P-windows-x64.exe
```

Students can run `K10P-windows-x64.exe` from the USB drive; it installs the bundled PlatformIO environment to `C:\K10P`.

The macOS self-contained installer bundles PlatformIO Python wheels, including conditional dependencies such as `typing-extensions` for student Macs running Python older than 3.13. If an older installer fails offline with `No matching distribution found for typing-extensions`, rebuild it with the current `prepare-macos-offline-installer.sh`.

## K10 Experiment Box Quick Start

The K10 experiment box is not just a K10 board with extra widgets. Its peripherals share the I2C wiring with the board but use a separate driver and different device identities:

| Hardware layer | Device | Address | Actual role |
| --- | --- | --- | --- |
| K10 native | AHT20 | `0x38` | Temperature and humidity |
| K10 native | LTR303 | `0x29` | Ambient light |
| K10 native | SC7A20H | `0x19` | Board accelerometer; it may be absent on a box assembly |
| Experiment box | IO controller | `0x20` | Knob, sound, keys, ultrasonic, IR, motors, buzzer, and traffic lights |
| Experiment box | Line tracker | `0x30` | Five-channel raw values, thresholds, and digital states |
| Experiment box | QMI8658 | `0x6B` | Box acceleration and gyroscope data; expected chip ID is `0x05` |

Use `unihiker-k10-box-platformio` for box projects. Start with a normal K10 PlatformIO project, then install the pinned and PlatformIO-compatible DFRobot box driver:

```bash
bash unihiker-k10-platformio/scripts/init-k10-platformio-project.sh my-k10-box-project
bash unihiker-k10-box-platformio/scripts/install-k10-box-driver.sh my-k10-box-project
pio run -d my-k10-box-project
```

The tested LVGL layout uses four pages:

1. K10 temperature, humidity, ambient light, microphone, and buttons, plus box QMI8658 acceleration.
2. Box knob, sound, infrared code, ultrasonic distance, keys, conductance, and obstacle sensor.
3. Box gyroscope and five-channel line-tracker data.
4. A manually triggered actuator test for traffic lights, K10 RGB, both buzzers, and both DC motors.

Observed hardware and software constraints are captured in the skill:

- Do not report `k10.getAccelerometerX/Y/Z()` zeros as valid box acceleration when startup reports `SC7A20H Device not found`; read and label the box QMI8658 instead.
- Format floating-point values with `snprintf`, then update LVGL with `lv_label_set_text`. The bundled LVGL configuration disables float formatting, and a large `lv_label_set_text_fmt` call can crash when opening a data-heavy page.
- Treat ultrasonic `0xFFFF` as no valid reading and display `--`.
- Motor log output is not proof of movement. The tested startup pulse uses duty `255` for roughly 800-1000 ms, with a stop interval before reversal; verify wheel motion or a simultaneous QMI8658 change.
- Never start actuator tests at boot. Require an explicit action and stop motors, buzzers, and LEDs on completion, cancellation, page exit, and startup.

For exact APIs and diagnosis, read `unihiker-k10-box-platformio/references/hardware-map.md`, `lvgl-dashboard.md`, and `troubleshooting.md`.

## MicroPython Quick Start

1. Install the flashing and upload tools:

   ```bash
   pip install esptool mpremote
   ```

2. Flash the bundled K10 MicroPython firmware. Enter download mode first by holding BOOT, pressing RST, then releasing BOOT.

   ```bash
   bash unihiker-k10-micropython/scripts/flash-micropython.sh /dev/cu.usbmodem2201
   ```

3. Upload `main.py`:

   ```bash
   bash unihiker-k10-micropython/scripts/upload-micropython.sh main.py /dev/cu.usbmodem2201
   ```

Only `main.py` runs automatically on boot. Other Python files need to be imported from the REPL or by `main.py`.

## HTTP OTA Quick Start

Use the OTA skill when an Arduino project needs wireless updates after the first USB flash.

1. Add `partitions.csv` to the sketch directory with `ota_0` and `ota_1` app partitions.

2. Add an HTTP POST `/ota` endpoint that calls the ESP32 `Update` API.

3. Do one USB upload after changing the partition table:

   ```bash
   arduino-cli upload -p /dev/cu.usbmodem2201 --fqbn UNIHIKER:esp32:k10 hello
   ```

4. Build and upload future firmware over WiFi:

   ```bash
   bash unihiker-k10-arduino/scripts/compile-ota.sh hello -i 192.168.9.42
   ```

   Or upload an existing binary:

   ```bash
   python unihiker-k10-ota/scripts/ota_upload.py hello/build/hello.ino.bin --ip 192.168.9.42
   ```

HTTP OTA needs AP or STA networking. ESP-NOW-only packets are not an IP transport, so ESP-NOW projects should enter a maintenance OTA mode, start AP/STA networking, pause time-critical traffic, and service the HTTP endpoint there.

## K10 AI Models and Recovery

K10 built-in AI support files live in fixed flash regions. Projects that use voice recognition, TTS, face recognition, or other built-in AI features must not let OTA app partitions overlap these regions:

| Region | Offset | Factory size |
| --- | --- | --- |
| `model` | `0x510000` | `4563K` |
| `voice_data` | `0x985000` | `2542K` |
| `fr` | `0xC01000` | `100K` |

Safe pattern for AI + OTA projects:

- Use OTA app slots that end before `0x510000`.
- Use normal app builds with `Model=None` / `-DModel=None` when the board already has valid model data.
- Use a one-time USB model refresh only when model data may be blank or damaged.
- Use Mind+ `Restore Initial Settings` as the official recovery path when the board repeatedly reboots or AI model files appear damaged.

Toolchain-specific recovery notes are in `references/k10-ai-model-flash.md` and `unihiker-k10-platformio/references/k10-ai-model-flash.md`.

## Board and Toolchain Rules

- Arduino and MicroPython firmware are mutually exclusive. Flash the firmware that matches the workflow you are using.
- Arduino sketches use `UNIHIKER:esp32:k10`; `esp32:unihiker` is not the correct FQBN.
- Arduino `.ino` files must live in a directory with the same base name.
- K10 Arduino canvas drawing methods are called through `k10.canvas->`, not directly on `k10`.
- Prefer local or partial screen redraws for animations, dashboards, sensor values, status text, and voice/OTA state. Repeated full-screen clears or full-background redraws cause visible flicker and are uncomfortable; use full-screen refresh only for initialization, page switches, exit cleanup, or when measured full-screen refresh is above 30 fps.
- Avoid documenting `compiler.cache.*` or `ccache` as standard setup. Current Arduino CLI uses `build_cache.*`.
- MicroPython firmware `v0.9.2` has a known AI + WiFi resource conflict. Use one at a time unless you have validated a newer firmware path.
- OTA-enabled Arduino sketches must keep the OTA endpoint in future builds. Uploading a sketch without it removes wireless update capability until the next USB flash.
- Generic large OTA partition tables can overwrite K10 AI model data. Preserve the fixed `model`, `voice_data`, and `fr` offsets for AI-enabled projects.

## Repository Layout

```text
unihiker-k10-arduino/       Arduino skill, upload scripts, API references, examples
unihiker-k10-platformio/    PlatformIO skill, offline bundle scripts, API references
unihiker-k10-box-platformio/ Experiment-box PlatformIO/LVGL skill, driver installer, hardware and debugging references
k10-compile-server/         LAN compile server skill, remote build and browser/server USB upload scripts
unihiker-k10-micropython/   MicroPython skill, flash/upload scripts, firmware, API reference
unihiker-k10-ota/           HTTP OTA skill, implementation guide, upload scripts
references/                 Shared repository-level K10 notes, including AI model recovery
sketches/                   Example Arduino sketches
```

`SKILL.md` files are the agent-facing entry points. Keep them concise and put detailed API material in `references/`. Put deterministic or error-prone operations in `scripts/` so agents can run them instead of retyping long commands.

## Maintenance Checks

When adding a new skill or materially changing an existing one, update both `README.md` and `AGENT_INDEX.md` in the same change so human-facing and agent-facing routing stay aligned.

Validate skill frontmatter after changing `SKILL.md`:

```bash
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-arduino
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-platformio
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-box-platformio
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py k10-compile-server
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-micropython
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-ota
```

Run shell syntax checks after editing Bash scripts:

```bash
bash -n unihiker-k10-arduino/scripts/*.sh
bash -n unihiker-k10-platformio/scripts/*.sh
bash -n k10-compile-server/scripts/*.sh
bash -n unihiker-k10-micropython/scripts/*.sh
```
