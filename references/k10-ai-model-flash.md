# K10 AI Model Flashing and Recovery

This reference is shared by the K10 Arduino, PlatformIO, OTA, and troubleshooting skills.

## What Is Stored Where

UNIHIKER K10 uses fixed flash regions for the built-in AI support files. Keep these offsets unchanged unless DFRobot publishes a new partition table.

| Region | Offset | Size in factory table | Purpose |
| --- | --- | --- | --- |
| `model` | `0x510000` | `4563K` | Speech recognition model image |
| `voice_data` | `0x985000` | `2542K` | TTS voice data |
| `fr` | `0xC01000` | `100K` | Face-recognition related data |

Local BSP evidence:

- Arduino BSP: `~/Library/Arduino15/packages/UNIHIKER/hardware/esp32/0.0.3/tools/partitions/`
- PlatformIO framework: `~/.platformio/packages/framework-arduinounihiker/tools/partitions/`
- `srmodels.bin` is about 3.1 MB and is used by the Arduino menu item labeled `CN`.
- `srmodels4.bin` is about 4.2 MB and is used by the Arduino menu item labeled `EN`.
- `srmodels5.bin` is about 4.5 MB in the BSP but is not selected by the current Arduino/PlatformIO upload menu.
- `esp_tts_voice_data_xiaoxin.dat` is about 2.5 MB and is flashed with both CN and EN model selections.

## Arduino CLI Model Upload

The Arduino BSP exposes model flashing through the board menu.

- Normal app upload without model refresh:
  `--fqbn UNIHIKER:esp32:k10:Model=None`
- Chinese model refresh:
  `--fqbn UNIHIKER:esp32:k10:Model=Hi_eps`
- English model refresh:
  `--fqbn UNIHIKER:esp32:k10:Model=Ni_hao_xiao_zhi`

The `boards.txt` labels are historically confusing: `Hi_eps` is displayed as `CN`, while `Ni_hao_xiao_zhi` is displayed as `EN`.

## PlatformIO Model Upload

DFRobot's PlatformIO builder reads `-DModel=...` from `build_flags`.

Use these environments in projects that may need AI recovery:

```ini
[k10_base]
platform = https://github.com/DFRobot/platform-unihiker.git
board = unihiker_k10
framework = arduino
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1

[env:unihiker]
extends = k10_base
build_flags =
    ${k10_base.build_flags}
    -DModel=None

[env:unihiker-init-cn]
extends = k10_base
build_flags =
    ${k10_base.build_flags}
    -DModel=CN

[env:unihiker-init-en]
extends = k10_base
build_flags =
    ${k10_base.build_flags}
    -DModel=EN
```

Use `unihiker-init-cn` or `unihiker-init-en` once over USB when the model partitions may be blank or damaged. Use `unihiker` for normal rebuilds, uploads, and OTA app images so model data is not rewritten every time.

## OTA Partition Rule

Do not use a generic two-slot OTA table that lets app partitions grow past `0x510000`; it will overlap the model region.

Use a model-preserving OTA table:

```csv
# Name,     Type, SubType, Offset,   Size,     Flags
nvs,        data, nvs,     0x9000,   0x5000,
otadata,    data, ota,     0xe000,   0x2000,
app0,       app,  ota_0,   0x10000,  0x280000,
app1,       app,  ota_1,   0x290000, 0x280000,
model,      data, spiffs,  0x510000, 4563K,
voice_data, data, fat,     0x985000, 2542K,
fr,         data, ,        0xC01000, 100K,
coredump,   data, coredump,,         1K,
spiffs,     data, spiffs,  0xC1B000, 0x3E5000,
```

The practical limit is that each OTA app slot is 2.5 MB. Check firmware size after every build. If the image no longer fits, choose one of these paths:

1. Keep AI models and reduce firmware size.
2. Keep OTA and AI models but move rarely used features out of the app.
3. Drop OTA and use the factory single-app table for larger USB-only firmware.
4. Drop built-in AI model regions only when the program explicitly does not need K10 AI functions.

## What Can Be Combined

Safe combinations:

- Voice recognition + RGB + WiFi AP/STA + HTTP OTA, if the app fits inside the OTA slot.
- Model-preserving OTA partition table + `-DModel=None`, if the board already has valid model data.
- Model-preserving OTA partition table + `-DModel=CN` or `-DModel=EN` for a one-time USB initialization.

Use caution:

- OTA upload while time-critical wireless or control loops are running. Pause nonessential work during upload.
- AI functions with poor-quality TF cards. Official FAQ notes TF-card faults can trigger AI-related crashes.
- Heavy screen drawing inside callbacks, threads, or interrupt-like handlers. Official FAQ recommends updating variables there and drawing from the main loop.

Do not combine without a deliberate recovery plan:

- Generic large OTA partitions with built-in AI model functions, because the app slots can overwrite `model`, `voice_data`, or `fr`.
- Full flash erase followed by `-DModel=None`, because model partitions may remain blank.
- Uploading firmware without an OTA endpoint after switching to OTA workflow, because wireless update capability is lost until USB flashing restores it.

## Recovery Paths

Official UNIHIKER documentation provides two recovery flows:

- Mind+ restore: hold BOOT while connecting USB, release after the port appears, then click `Restore Initial Settings`; press RST when done.
- Factory programme: after restoring initial settings in Mind+, download the official factory programme zip from the K10 hardware download page, paste it into Mind+ manual edit mode, and upload it.

Use Mind+ restore when a board keeps rebooting or when model files appear damaged. Use the Arduino/PlatformIO model initialization environments when the development toolchain is already set up and only the model partitions need to be refreshed.
