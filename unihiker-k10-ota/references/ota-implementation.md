# K10 HTTP OTA Implementation Guide

## Table of Contents

1. [Why HTTP OTA Instead of ArduinoOTA](#why-http-ota)
2. [Partition Table Requirements](#partition-table)
3. [Firmware Code Changes](#firmware-code)
4. [Build and Upload Workflow](#build-upload)
5. [OTA Update Workflow](#ota-update)
6. [Reference: Complete Minimal Example](#minimal-example)

---

## Why HTTP OTA Instead of ArduinoOTA

`ArduinoOTA` uses a UDP-based protocol:
1. Host sends an authentication challenge to the device (port 3232)
2. Device verifies password
3. **Device opens a TCP connection back to the host** on a random port
4. Host streams the firmware over this reverse connection

Step 3 is the failure point on Windows because:
- Windows Defender Firewall blocks inbound connections from the ESP32
- No admin privileges available to add firewall rules
- `arduino-cli` network upload cannot pass the password non-interactively

HTTP OTA flips the direction:
- Host opens a TCP connection **to** the device (outbound — always allowed)
- Host POSTs the firmware as `multipart/form-data`
- Device receives and writes to flash using the `Update` library

---

## Partition Table Requirements

The K10 BSP (`UNIHIKER:esp32` v0.0.3) ships with `large_spiffs_16MB.csv`:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
factory,  app,  factory, 0x10000, 0x500000,
model,    data, spiffs,  0x510000,4563k,
voice_data,data, fat,    0x985000,2542k,
fr,       data, ,        0xC01000,100K,
coredump, data, coredump,,        1K,
```

**Problem:** There is no `ota_0` / `ota_1` / `otadata` partition. `Update.begin()` fails immediately because it cannot find an inactive OTA slot to write to.

**Solution:** Create `partitions.csv` in your sketch directory with OTA partitions:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x640000,
app1,     app,  ota_1,   0x650000,0x640000,
spiffs,   data, spiffs,  0xc90000,0x370000,
```

- `app0` and `app1` are each ~6.5 MB — plenty of room for the ~1.1 MB K10 titrator firmware
- `otadata` is required for the bootloader to know which app partition to boot from
- `spiffs` is reduced compared to the factory table, but still 3.5 MB

**Compile with the custom partition:**

```bash
arduino-cli compile --fqbn UNIHIKER:esp32:k10 . \
  --output-dir build \
  --build-property "build.partitions=custom"
```

The first USB upload after this change will write the new partition table to flash. This is a one-time operation.

---

## Firmware Code Changes

### 1. Include Update Library

```cpp
#include <Update.h>
```

### 2. Add Upload Handler

Use the **four-argument** `server.on()` overload to register both a final handler and an upload-progress handler:

```cpp
void handleOta() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  if (!Update.hasError()) {
    scheduleRestart("OTA update done");  // or ESP.restart()
  }
}

void handleOtaUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("OTA: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}
```

### 3. Register the Route

```cpp
server.on("/ota", HTTP_POST, handleOta, handleOtaUpload);
```

### 4. Restart Scheduling (Recommended)

Restarting immediately inside the handler can cut off the HTTP response. Use a delayed restart:

```cpp
bool restartPending = false;
uint32_t restartAtMs = 0;

void scheduleRestart(const String &message) {
  restartPending = true;
  restartAtMs = millis() + 1200;  // 1.2s delay
}

void loop() {
  // ... existing loop code ...
  if (restartPending && millis() >= restartAtMs) {
    ESP.restart();
  }
}
```

---

## Build and Upload Workflow

### Initial Setup (USB Required)

```bash
# Compile with custom partition table
arduino-cli compile --fqbn UNIHIKER:esp32:k10 . \
  --output-dir build \
  --build-property "build.partitions=custom"

# Upload via USB (also flashes the new partition table)
arduino-cli upload -p COM4 --fqbn UNIHIKER:esp32:k10 .
```

### Subsequent Updates (WiFi OTA)

```bash
# Compile only
arduino-cli compile --fqbn UNIHIKER:esp32:k10 . --output-dir build

# Upload via HTTP
curl -F "file=@build/your_sketch.ino.bin" http://192.168.9.42/ota
```

---

## OTA Update Workflow

1. **Ensure the device is running an OTA-enabled sketch** (has `/ota` endpoint)
2. **Get the device IP** from the web UI, serial output, or router
3. **Compile** the new firmware
4. **POST the `.bin` file** to `http://<ip>/ota`
5. **Wait for `OK` response** (~5–10 seconds for a 1.1 MB firmware)
6. **Device restarts automatically** after a short delay
7. **Verify** by checking the web UI or JSON status endpoint

---

## Reference: Complete Minimal Example

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

void handleOta() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  if (!Update.hasError()) {
    delay(100);
    ESP.restart();
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
    if (!Update.end(true)) {
      Update.printError(Serial);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP("K10-OTA-Test", "12345678");

  server.on("/ota", HTTP_POST, handleOta, handleOtaUpload);
  server.begin();

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
```

---

## Common Pitfalls

| Pitfall | Why It Happens |
|---------|---------------|
| `Update.begin(size)` with exact size fails | ESP32 flash requires 4 KB alignment. Use `UPDATE_SIZE_UNKNOWN` instead. |
| `Update.writeStream(server.client())` hangs | `writeStream()` waits for the client to close the connection, but the client waits for the HTTP response. Deadlock. Use chunked `client.read()` with a known `Content-Length` instead. |
| `server.header("Content-Length")` returns empty | Arduino WebServer stores `Content-Length` in `_clientContentLength`, not the headers map. Use `server.clientContentLength()` (public in ESP32 core). |
| `FAIL` after full upload | `Update.end(true)` failed. Most common cause: firmware size exceeds OTA partition size. Ensure partition is large enough. |
