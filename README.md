# Unihiker K10 Skills

Local Codex skills for working with the DFRobot Unihiker K10 board. Install these folders into your Codex skills directory, then Codex can load the right K10 reference material, helper scripts, firmware bundle, and troubleshooting notes when you ask it to build or flash K10 projects.

## What's Included

| Skill | Use it for | Current notes |
| --- | --- | --- |
| `unihiker-k10-arduino` | Arduino/C++ sketches, K10 BSP setup, serial upload, Arduino API lookup, screen/sensor/RGB/audio/AI examples. | Uses FQBN `UNIHIKER:esp32:k10`; includes Windows `arduino-cli.exe`; documents Arduino CLI `build_cache.*` rather than older cache keys. |
| `unihiker-k10-micropython` | Flashing MicroPython, uploading `main.py`, MicroPython API lookup, REPL-oriented troubleshooting. | Bundles K10 MicroPython firmware `v0.9.2`; only `main.py` auto-runs after reset. |
| `unihiker-k10-ota` | Adding HTTP OTA update support to Arduino projects. | Requires a custom partition table with `ota_0` and `ota_1`; includes Python and PowerShell upload helpers. |

## Agent Usage

Agents should read `AGENT_INDEX.md` first, then load only the matching skill for the user's task. Use this routing:

| User intent | Load |
| --- | --- |
| Arduino sketch, C++ API, serial upload, K10 BSP setup | `unihiker-k10-arduino/SKILL.md` |
| MicroPython firmware, `main.py`, `mpremote`, Python API | `unihiker-k10-micropython/SKILL.md` |
| Wireless Arduino firmware update, HTTP OTA, ESP-NOW maintenance OTA mode | `unihiker-k10-ota/SKILL.md` |

Do not guess K10 APIs from general ESP32 or MicroPython knowledge. Read the selected skill and its relevant `references/` material before writing code or commands.

## Install

Copy or symlink every skill folder into your Codex skills directory:

```bash
mkdir -p ~/.agents/skills
cp -R unihiker-k10-arduino ~/.agents/skills/
cp -R unihiker-k10-micropython ~/.agents/skills/
cp -R unihiker-k10-ota ~/.agents/skills/
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE\.agents\skills"
Copy-Item -Recurse .\unihiker-k10-arduino "$env:USERPROFILE\.agents\skills\"
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

## Board and Toolchain Rules

- Arduino and MicroPython firmware are mutually exclusive. Flash the firmware that matches the workflow you are using.
- Arduino sketches use `UNIHIKER:esp32:k10`; `esp32:unihiker` is not the correct FQBN.
- Arduino `.ino` files must live in a directory with the same base name.
- K10 Arduino canvas drawing methods are called through `k10.canvas->`, not directly on `k10`.
- Prefer local or partial screen redraws for animations and sensor refreshes. Repeated full-screen clears cause flicker and unnecessary work.
- Avoid documenting `compiler.cache.*` or `ccache` as standard setup. Current Arduino CLI uses `build_cache.*`.
- MicroPython firmware `v0.9.2` has a known AI + WiFi resource conflict. Use one at a time unless you have validated a newer firmware path.
- OTA-enabled Arduino sketches must keep the OTA endpoint in future builds. Uploading a sketch without it removes wireless update capability until the next USB flash.

## Repository Layout

```text
unihiker-k10-arduino/       Arduino skill, upload scripts, API references, examples
unihiker-k10-micropython/   MicroPython skill, flash/upload scripts, firmware, API reference
unihiker-k10-ota/           HTTP OTA skill, implementation guide, upload scripts
sketches/                   Example Arduino sketches
```

`SKILL.md` files are the agent-facing entry points. Keep them concise and put detailed API material in `references/`. Put deterministic or error-prone operations in `scripts/` so agents can run them instead of retyping long commands.

## Maintenance Checks

Validate skill frontmatter after changing `SKILL.md`:

```bash
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-arduino
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-micropython
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-ota
```

Run shell syntax checks after editing Bash scripts:

```bash
bash -n unihiker-k10-arduino/scripts/*.sh
bash -n unihiker-k10-micropython/scripts/*.sh
```
