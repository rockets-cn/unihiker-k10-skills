#include <Arduino.h>
#include "asr.h"
#include "unihiker_k10.h"

UNIHIKER_K10 k10;
ASR asr;

constexpr uint8_t SCREEN_DIR = 2;
constexpr uint8_t CMD_LIGHT_ON = 1;
constexpr uint8_t CMD_LIGHT_OFF = 2;
constexpr uint8_t CMD_RED = 3;
constexpr uint8_t CMD_GREEN = 4;
constexpr uint8_t CMD_BLUE = 5;
constexpr uint8_t CMD_WHITE = 6;

String lastCommand = "Say: ni hao xiao xing";
bool lastWakeState = false;

char cmdLightOn[] = "kai deng";
char cmdLightOff[] = "guan deng";
char cmdRed[] = "hong se";
char cmdGreen[] = "lv se";
char cmdBlue[] = "lan se";
char cmdWhite[] = "bai se";

void setAllRgb(uint8_t brightness, uint32_t color, const String &label) {
  k10.rgb->brightness(brightness);
  k10.rgb->write(-1, color);
  lastCommand = label;
  Serial.println(label);
}

void drawStatus(bool wakeState) {
  k10.canvas->canvasText(wakeState ? "Wake: listening" : "Wake: sleeping",
                         0, 0, 0x0000FF, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText(lastCommand,
                         0, 24, 0xFF0000, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->canvasText("kai/guan/hong/lv/lan/bai",
                         0, 48, 0x008000, k10.canvas->eCNAndENFont16, 50, true);
  k10.canvas->updateCanvas();
}

void setup() {
  Serial.begin(115200);
  k10.begin();

  asr.asrInit(CONTINUOUS, CN_MODE, 6000);
  while (asr._asrState == 0) {
    delay(100);
  }

  k10.initScreen(SCREEN_DIR);
  k10.creatCanvas();
  k10.setScreenBackground(0xFFFFFF);

  asr.addASRCommand(CMD_LIGHT_ON, cmdLightOn);
  asr.addASRCommand(CMD_LIGHT_OFF, cmdLightOff);
  asr.addASRCommand(CMD_RED, cmdRed);
  asr.addASRCommand(CMD_GREEN, cmdGreen);
  asr.addASRCommand(CMD_BLUE, cmdBlue);
  asr.addASRCommand(CMD_WHITE, cmdWhite);

  setAllRgb(0, 0x000000, "Ready");
  drawStatus(false);
}

void loop() {
  bool wakeState = asr.isWakeUp();

  if (asr.isDetectCmdID(CMD_LIGHT_ON)) {
    setAllRgb(9, 0xFFFFFF, "Light on");
  } else if (asr.isDetectCmdID(CMD_LIGHT_OFF)) {
    setAllRgb(0, 0x000000, "Light off");
  } else if (asr.isDetectCmdID(CMD_RED)) {
    setAllRgb(9, 0xFF0000, "Red");
  } else if (asr.isDetectCmdID(CMD_GREEN)) {
    setAllRgb(9, 0x00FF00, "Green");
  } else if (asr.isDetectCmdID(CMD_BLUE)) {
    setAllRgb(9, 0x0000FF, "Blue");
  } else if (asr.isDetectCmdID(CMD_WHITE)) {
    setAllRgb(9, 0xFFFFFF, "White");
  }

  if (wakeState != lastWakeState) {
    lastWakeState = wakeState;
  }
  drawStatus(wakeState);
  delay(100);
}
