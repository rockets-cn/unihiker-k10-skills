# Agent Index

This file is the cold-start map for agents working in this repository. Read it before choosing a K10 skill.

## Read Order

1. Read this file.
2. Select exactly the skill that matches the task.
3. Read that skill's `SKILL.md`.
4. Read only the referenced API or implementation files needed for the task.
5. Prefer the provided scripts for upload, flashing, compile, and OTA work.
6. For AI, voice, OTA, partition-table, or factory-recovery work, read `references/k10-ai-model-flash.md`.
7. When adding or materially changing a skill, update both `README.md` and this file in the same change.

## Task Routing

| Task | Primary skill | Required follow-up |
| --- | --- | --- |
| Create, fix, compile, or upload an Arduino sketch | `unihiker-k10-arduino` | Read Arduino API references before using K10-specific classes. |
| Set up Arduino CLI or K10 BSP | `unihiker-k10-arduino` | Configure both the K10 BSP URL and an ESP32 Core URL before installing `UNIHIKER:esp32`. |
| Create, fix, compile, upload, or monitor a PlatformIO K10 project | `unihiker-k10-platformio` | Read PlatformIO workflow notes and the bundled K10 Arduino API references before using K10-specific classes. |
| Build or diagnose a K10 experiment-box PlatformIO/LVGL project | `unihiker-k10-box-platformio` | Read `references/hardware-map.md`; keep native K10 devices separate from box IO `0x20`, line tracker `0x30`, and QMI8658 `0x6B`. |
| Diagnose missing box sensor values, LVGL float crashes, or non-moving box motors | `unihiker-k10-box-platformio` | Read `references/troubleshooting.md` and verify the physical symptom, not only serial control-stage logs. |
| Compile a PlatformIO K10 project through the LAN server and flash it from a client browser | `k10-compile-server` | Prefer `--web-serial` / `-WebSerial`; use `references/server-setup.md` if the user needs to self-host `rockets-cn/unihiker-k10-compile-server`. |
| Prepare or install a PlatformIO offline workshop bundle or USB installer | `unihiker-k10-platformio` | Use the PlatformIO bundle scripts; prepare one bundle or self-contained installer per OS/CPU architecture. |
| Diagnose macOS PlatformIO offline installer pip dependency errors | `unihiker-k10-platformio` | Read `references/platformio-workshop.md`; missing `typing-extensions` means the installer was built with an older wheelhouse. |
| Diagnose serial upload or port issues | `unihiker-k10-arduino` or `unihiker-k10-micropython` | Use the matching `find-port` or upload script instead of inventing port detection logic. |
| Diagnose PlatformIO build, package, upload, or port issues | `unihiker-k10-platformio` | Use `k10-pio.sh`, `pio device list`, and `doctor-offline.sh` before changing project code. |
| Diagnose K10 ASR wake-word, ES7243E, microphone, or all-zero I2S input | `unihiker-k10-platformio` | Read `references/k10-asr-audio-troubleshooting.md`; distinguish model loading from audio transport and drain startup DMA before judging the microphone. |
| Flash MicroPython firmware | `unihiker-k10-micropython` | Use the bundled firmware and the documented BOOT/RST download-mode sequence. |
| Upload or debug MicroPython code | `unihiker-k10-micropython` | Use `main.py` for auto-run behavior; non-entry files require REPL import or `main.py` import. |
| Add HTTP OTA to an Arduino or PlatformIO project | `unihiker-k10-ota` | Add OTA partitions, add an HTTP `/ota` endpoint, and perform one USB upload before wireless updates. |
| Combine K10 AI functions with OTA or recover missing AI models | `unihiker-k10-ota`, `unihiker-k10-arduino`, or `unihiker-k10-platformio` | Read `references/k10-ai-model-flash.md`; preserve model offsets and use a model-refresh upload only when needed. |
| Add OTA to an ESP-NOW project | `unihiker-k10-ota` | Use a maintenance OTA mode with AP/STA networking; do not claim HTTP OTA works over pure ESP-NOW packets. |
| Improve screen animation, sensor display refresh, dashboards, or status UIs | `unihiker-k10-arduino`, `unihiker-k10-platformio`, or `unihiker-k10-micropython` | Prefer partial redraws; avoid full-screen clearing unless the full-screen refresh rate is proven above 30 fps. |

## Non-Negotiable Constraints

- Arduino and MicroPython firmware are mutually exclusive on the board.
- Speech synthesis is available only in the Chinese K10 firmware. Confirm the firmware variant before using TTS APIs; retaining or refreshing CN model data alone is insufficient.
- Arduino FQBN is `UNIHIKER:esp32:k10`.
- PlatformIO board is `unihiker_k10` with `platform = https://github.com/DFRobot/platform-unihiker.git`.
- Arduino sketches must use same-named sketch directories.
- PlatformIO projects use `platformio.ini` and `src/main.cpp`; do not mix Arduino CLI FQBN settings into PlatformIO config.
- K10 experiment-box work must distinguish native SC7A20H `0x19` from box QMI8658 `0x6B`; absent native acceleration must not be reported as valid zeros.
- Experiment-box actuator tests must be explicitly triggered and must stop motors, buzzer, and LEDs on completion, cancellation, page exit, and startup.
- Arduino canvas calls use `k10.canvas->`.
- Screen updates must default to partial redraws. Full-screen clearing or full-background redraw causes visible flicker and is only acceptable for initialization, page changes, exit cleanup, or when the measured full-screen refresh rate is above 30 fps.
- Current Arduino CLI cache settings use `build_cache.*`; do not introduce `compiler.cache.*` or `ccache` as standard setup.
- MicroPython `v0.9.2` has an AI + WiFi resource conflict; do not combine them unless the user explicitly validates a newer path.
- HTTP OTA requires OTA partitions and IP networking. ESP-NOW is not an HTTP transport.
- If an OTA-enabled sketch is replaced by a sketch without the OTA endpoint, wireless update capability is lost until USB flashing restores it.
- K10 built-in AI model data lives at fixed flash offsets starting at `0x510000`; OTA app partitions must not overlap these regions.
- A full flash erase followed by `Model=None` can leave AI model partitions blank. Use Mind+ Restore Initial Settings or a one-time Arduino/PlatformIO model-refresh upload.

## Script Preferences

Use scripts when the task involves deterministic device operations:

| Operation | Preferred scripts |
| --- | --- |
| Arduino serial upload | `unihiker-k10-arduino/scripts/upload-arduino.sh`, `upload-arduino.ps1`, `upload_k10.py`, or `upload-k10.bat` |
| Arduino OTA compile/upload | `unihiker-k10-arduino/scripts/compile-ota.sh` or `compile-ota.ps1` |
| PlatformIO project setup/build/upload/monitor | `unihiker-k10-platformio/scripts/init-k10-platformio-project.sh` or `k10-pio.sh` / `k10-pio.ps1` |
| Install the pinned K10 experiment-box driver | `unihiker-k10-box-platformio/scripts/install-k10-box-driver.sh` |
| LAN server compile and Web Serial firmware upload | `k10-compile-server/scripts/compile-project.sh --web-serial` or `compile-project.ps1 -WebSerial` |
| PlatformIO workshop offline bundle | `unihiker-k10-platformio/scripts/prepare-offline-bundle.sh`, `prepare-macos-offline-installer.sh`, `prepare-windows-offline-installer.ps1`, `install-offline-bundle.sh`, `install-offline-bundle.ps1`, or `doctor-offline.sh` |
| MicroPython firmware flashing | `unihiker-k10-micropython/scripts/flash-micropython.sh` or `flash-mp-auto.sh` |
| MicroPython file upload | `unihiker-k10-micropython/scripts/upload-micropython.sh` |
| HTTP OTA binary upload | `unihiker-k10-ota/scripts/ota_upload.py` or `ota_upload.ps1` |

## Verification Commands

Run the relevant checks after documentation or skill changes:

```bash
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-arduino
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-platformio
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-box-platformio
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py k10-compile-server
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-micropython
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-ota
bash -n unihiker-k10-arduino/scripts/*.sh
bash -n unihiker-k10-platformio/scripts/*.sh
bash -n unihiker-k10-box-platformio/scripts/*.sh
bash -n k10-compile-server/scripts/*.sh
bash -n unihiker-k10-micropython/scripts/*.sh
```

For code generation tasks, verification should also include the appropriate compile, upload dry run, or firmware-flashing preflight when hardware access is available.
