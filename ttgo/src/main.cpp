#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>

static constexpr uint8_t I2C_SLAVE_ADDRESS = 0x55;
static constexpr uint8_t SDA_PIN = 21;
static constexpr uint8_t SCL_PIN = 22;

TFT_eSPI tft = TFT_eSPI();

volatile uint8_t receivedValue = 0;
volatile bool newValueReceived = false;

void receiveEvent(int byteCount)
{
    if (byteCount < 1)
    {
        return;
    }

    receivedValue = Wire.read();

    while (Wire.available())
    {
        Wire.read();
    }

    newValueReceived = true;
}

void showScreen(
    uint16_t backgroundColor,
    uint16_t textColor,
    const char *line1,
    const char *line2,
    const char *line3 = nullptr)
{
    tft.fillScreen(backgroundColor);
    tft.setTextColor(textColor, backgroundColor);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextWrap(false, false);
    tft.setCursor(8, 20);
    tft.print(line1);
    tft.setCursor(8, 45);
    tft.print(line2);

    if (line3 != nullptr)
    {
        tft.setCursor(8, 70);
        tft.print(line3);
    }
}

void showScreenForValue(uint8_t value)
{
    switch (value)
    {
    case 1:
        showScreen(
            TFT_GREEN,
            TFT_BLACK,
            "Device: Good",
            "Everyting works");
        break;

    case 2:
        showScreen(
            tft.color565(255, 165, 0),
            TFT_BLACK,
            "Device: Ok",
            "May need mantance");
        break;

    case 3:
        showScreen(
            TFT_RED,
            TFT_WHITE,
            "Device: Warning",
            "Needs maintenance.",
            "Turn off now.");
        break;

    default:
        showScreen(
            TFT_WHITE,
            TFT_BLACK,
            "Error",
            "No valid input.");
        break;
    }
}

void setup()
{
    tft.init();
    tft.setRotation(1);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    showScreenForValue(0);

    Wire.begin(I2C_SLAVE_ADDRESS, SDA_PIN, SCL_PIN, 100000);
    Wire.onReceive(receiveEvent);
}

void loop()
{
    if (newValueReceived)
    {
        noInterrupts();
        uint8_t value = receivedValue;
        newValueReceived = false;
        interrupts();

        showScreenForValue(value);
    }
}
