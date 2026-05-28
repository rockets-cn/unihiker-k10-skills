---
name: unihiker-k10-ota
description: Add HTTP OTA (Over-The-Air) firmware update capability to Unihiker K10 Arduino projects. Use when you need wireless firmware updates without USB cable.
---

# Unihiker K10 - HTTP OTA

## Overview

Enable wireless firmware updates for K10 Arduino projects via HTTP POST.

**Core principle:** K10's default partition table has no OTA partitions. You must switch to a custom partition table with `ota_0` + `ota_1` before `Update.begin()` can work.

**Why not ArduinoOTA?** The standard `ArduinoOTA` library (UDP-based) requires the ESP32 to connect back to the host computer on a random port, which is often blocked by Windows Firewall. HTTP OTA uses a simple host→device upload direction and works reliably on all networks.

## When to Use

- Your K10 is installed in a location difficult to reach with USB
- You want to update firmware without opening the enclosure
- You need a scriptable/automated deployment pipeline
- ArduinoOTA network port upload fails with "No response from device"

## Prerequisites

- Existing K10 Arduino project with `WebServer` running
- `arduino-cli` installed and K10 BSP (`UNIHIKER:esp32:k10`) available
- Device and computer on the same network (or connected to K10's AP)

## Quick Start

### Step 1: Add Custom Partition Table

Create `partitions.csv` in your sketch directory:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x640000,
app1,     app,  ota_1,   0x650000,0x640000,
spiffs,   data, spiffs,  0xc90000,0x370000,
```

Compile with the custom partition:

```bash
arduino-cli compile --fqbn UNIHIKER:esp32:k10 . \
  --output-dir build \
  --build-property "build.partitions=custom"
```

Optional speed-up for repeated compiles:

```bash
# Use all CPU cores and keep build artifacts in stable project-local folders.
arduino-cli compile --fqbn UNIHIKER:esp32:k10 . \
  --build-path .arduino-build \
  --output-dir build \
  --build-property "build.partitions=custom" \
  -j 0
```

Arduino CLI already has a built-in `build_cache`. To use a longer-lived cache, configure the official `build_cache.*` keys rather than `compiler.cache.*`:

```bash
arduino-cli config set build_cache.path ~/.cache/arduino-build-cache
arduino-cli config set build_cache.compilations_before_purge 0
```

On Windows PowerShell:

```powershell
arduino-cli config set build_cache.path "$env:LOCALAPPDATA\arduino\build-cache"
arduino-cli config set build_cache.compilations_before_purge 0
```

### Step 2: Add OTA Endpoint to Firmware

Include the `Update` library and add a POST handler:

```cpp
#include <Update.h>

void handleOta() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  if (!Update.hasError()) {
    ESP.restart();  // or schedule a delayed restart
  }
}

void handleOtaUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

// In setup() or startNetwork():
server.on("/ota", HTTP_POST, handleOta, handleOtaUpload);
```

### Step 3: First USB Upload (Required Once)

The first upload must be via USB to flash the new partition table:

```bash
arduino-cli upload -p COM4 --fqbn UNIHIKER:esp32:k10 .
```

### Step 4: Update Over WiFi

After the first USB upload, use any of these methods:

**curl:**
```bash
curl -F "file=@build/your_sketch.ino.bin" http://192.168.9.42/ota
```

**Python script (works on Windows, macOS, and Linux):**
```bash
python scripts/ota_upload.py build/your_sketch.ino.bin --ip 192.168.9.42
```

**PowerShell 7+ (works on Windows, macOS, and Linux):**
```powershell
pwsh ./scripts/ota_upload.ps1 -Bin build/your_sketch.ino.bin -Ip 192.168.9.42
```

## Important Notes

- **Partition change erases flash layout.** The first USB upload after adding `partitions.csv` will reformat the flash partition table. `Preferences` / NVS data may be lost.
- **Every OTA-enabled sketch must include the OTA code.** If you upload a sketch without `/ota` handler, you lose OTA capability and must return to USB.
- **Do not use `delay()` in `loop()` for long periods.** Use non-blocking `millis()` patterns so the WebServer can process the upload request.
- **Content-Length:** Arduino WebServer's `server.header("Content-Length")` does not work in POST handlers. Use `server.clientContentLength()` instead if you need the raw body size.
- **Compile cache:** Use Arduino CLI's official `build_cache.*` settings and `--build-path` for repeat builds. Do not document `compiler.cache.enable`, `compiler.cache.path`, or `ccache` as required OTA setup because they are not part of the current Arduino CLI configuration reference.

## Files

```
unihiker-k10-ota/
├── SKILL.md                           # This file
├── references/
│   └── ota-implementation.md          # Detailed implementation guide
└── scripts/
    ├── ota_upload.py                  # Python OTA uploader
    └── ota_upload.ps1                 # PowerShell OTA uploader
```

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| `BEGIN_FAIL` | No OTA partitions in partition table | Add `partitions.csv` with `ota_0` + `ota_1` and reflash via USB |
| `FAIL` after upload | `Update.write()` failed mid-stream | Check serial log; likely flash write error or insufficient space |
| `NO_CONTENT` | `Content-Length` header missing | Ensure client sends valid `multipart/form-data` with file data |
| Device does not restart | `ESP.restart()` called before response sent | Use `scheduleRestart()` with a small delay instead |
| Network port not found | mDNS/ArduinoOTA not running | HTTP OTA does not need network port detection; use the device's IP directly |
