#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "asr.h"
#include "unihiker_k10.h"

namespace {
constexpr char STA_SSID[] = "DFRobot-guest";
constexpr char STA_PASSWORD[] = "dfrobot@qq.com@2017";
constexpr char AP_SSID[] = "K10-VoiceRGB-OTA";
constexpr char AP_PASSWORD[] = "12345678";

constexpr uint8_t SCREEN_DIR = 2;
constexpr uint8_t RGB_LEVEL = 9;

constexpr uint8_t CMD_LIGHT_ON = 1;
constexpr uint8_t CMD_LIGHT_OFF = 2;
constexpr uint8_t CMD_RED = 3;
constexpr uint8_t CMD_GREEN = 4;
constexpr uint8_t CMD_BLUE = 5;
constexpr uint8_t CMD_WHITE = 6;

UNIHIKER_K10 k10;
ASR asr;
WebServer server(80);

char cmdLightOn[] = "kai deng";
char cmdLightOff[] = "guan deng";
char cmdRed[] = "hong se";
char cmdGreen[] = "lv se";
char cmdBlue[] = "lan se";
char cmdWhite[] = "bai se";

const char *lastCommand = "Ready";
bool lastWakeState = false;
bool otaUploadActive = false;
bool restartPending = false;
uint32_t restartAtMs = 0;
uint32_t lastStaAttemptMs = 0;
uint32_t lastStatusDrawMs = 0;

void setAllRgb(uint8_t brightness, uint32_t color, const char *label) {
  k10.rgb->brightness(brightness);
  k10.rgb->write(-1, color);
  lastCommand = label;
  Serial.println(label);
}

void drawStatus(bool force = false) {
  const uint32_t now = millis();
  if (!force && now - lastStatusDrawMs < 1000) {
    return;
  }
  lastStatusDrawMs = now;

  const bool staReady = WiFi.status() == WL_CONNECTED;
  k10.canvas->canvasText(staReady ? "STA: connected" : "STA: connecting",
                         0, 0, staReady ? 0x008000 : 0xFF8800,
                         k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText(WiFi.localIP().toString(),
                         0, 22, 0x0000FF, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText(String("AP: ") + WiFi.softAPIP().toString(),
                         0, 44, 0x0000FF, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText(lastCommand,
                         0, 66, 0xFF0000, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText(lastWakeState ? "Wake: listening" : "Say: ni hao xiao xing",
                         0, 88, 0x008000, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->updateCanvas();
}

void scheduleRestart() {
  restartPending = true;
  restartAtMs = millis() + 1200;
}

void handleRoot() {
  String body;
  body.reserve(512);
  body += "K10 Voice RGB OTA\n";
  body += "STA SSID: ";
  body += STA_SSID;
  body += "\nSTA IP: ";
  body += WiFi.localIP().toString();
  body += "\nAP SSID: ";
  body += AP_SSID;
  body += "\nAP IP: ";
  body += WiFi.softAPIP().toString();
  body += "\nOTA endpoint: POST /ota multipart field file\n";
  body += "Last command: ";
  body += lastCommand;
  body += "\n";
  server.send(200, "text/plain", body);
}

void handleOtaResult() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  if (!Update.hasError()) {
    lastCommand = "OTA OK, restart";
    drawStatus(true);
    scheduleRestart();
  }
}

void handleOtaUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    otaUploadActive = true;
    Serial.printf("OTA start: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    otaUploadActive = false;
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    otaUploadActive = false;
    Serial.println("OTA aborted");
  }
}

void startNetwork() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  const bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("AP %s: %s %s\n", apStarted ? "started" : "failed",
                AP_SSID, WiFi.softAPIP().toString().c_str());

  WiFi.begin(STA_SSID, STA_PASSWORD);
  lastStaAttemptMs = millis();
  Serial.printf("STA connecting: %s\n", STA_SSID);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/ota", HTTP_POST, handleOtaResult, handleOtaUpload);
  server.begin();
  Serial.println("HTTP server started on / and /ota");
}

void keepStaConnected() {
  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return;
  }

  const uint32_t now = millis();
  if ((status == WL_IDLE_STATUS || status == WL_NO_SSID_AVAIL) && now - lastStaAttemptMs < 30000) {
    return;
  }

  if (now - lastStaAttemptMs >= 30000) {
    lastStaAttemptMs = now;
    Serial.println("STA reconnecting...");
    WiFi.disconnect(false);
    delay(50);
    WiFi.begin(STA_SSID, STA_PASSWORD);
  }
}

void initVoiceCommands() {
  asr.asrInit(CONTINUOUS, CN_MODE, 6000);
  while (asr._asrState == 0) {
    server.handleClient();
    delay(20);
  }

  asr.addASRCommand(CMD_LIGHT_ON, cmdLightOn);
  asr.addASRCommand(CMD_LIGHT_OFF, cmdLightOff);
  asr.addASRCommand(CMD_RED, cmdRed);
  asr.addASRCommand(CMD_GREEN, cmdGreen);
  asr.addASRCommand(CMD_BLUE, cmdBlue);
  asr.addASRCommand(CMD_WHITE, cmdWhite);
}

void handleVoiceRgb() {
  if (otaUploadActive) {
    return;
  }

  lastWakeState = asr.isWakeUp();

  if (asr.isDetectCmdID(CMD_LIGHT_ON)) {
    setAllRgb(RGB_LEVEL, 0xFFFFFF, "Light on");
    drawStatus(true);
  } else if (asr.isDetectCmdID(CMD_LIGHT_OFF)) {
    setAllRgb(0, 0x000000, "Light off");
    drawStatus(true);
  } else if (asr.isDetectCmdID(CMD_RED)) {
    setAllRgb(RGB_LEVEL, 0xFF0000, "Red");
    drawStatus(true);
  } else if (asr.isDetectCmdID(CMD_GREEN)) {
    setAllRgb(RGB_LEVEL, 0x00FF00, "Green");
    drawStatus(true);
  } else if (asr.isDetectCmdID(CMD_BLUE)) {
    setAllRgb(RGB_LEVEL, 0x0000FF, "Blue");
    drawStatus(true);
  } else if (asr.isDetectCmdID(CMD_WHITE)) {
    setAllRgb(RGB_LEVEL, 0xFFFFFF, "White");
    drawStatus(true);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  k10.begin();
  k10.initScreen(SCREEN_DIR);
  k10.creatCanvas();
  k10.setScreenBackground(0xFFFFFF);

  startNetwork();
  initVoiceCommands();

  setAllRgb(0, 0x000000, "Ready");
  drawStatus(true);
}

void loop() {
  server.handleClient();
  keepStaConnected();
  handleVoiceRgb();
  drawStatus();

  if (restartPending && millis() >= restartAtMs) {
    ESP.restart();
  }

  delay(10);
}
