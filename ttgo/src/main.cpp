#include <Arduino.h>
#include <TFT_eSPI.h>

constexpr int kBacklightPin = TFT_BL;

TFT_eSPI tft = TFT_eSPI();

void setup() {
  pinMode(kBacklightPin, OUTPUT);
  digitalWrite(kBacklightPin, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLUE);

  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println("Hello");
  tft.setCursor(20, 80);
  tft.println("TTGO T-Display");
}

void loop() {
}
