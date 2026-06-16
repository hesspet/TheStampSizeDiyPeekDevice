//
// I2C Scanner – prüft, ob am I2C-Bus Geräte erreichbar sind
//

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// ESP32-C3 OLED-Entwicklungsboard mit 0,42 Zoll OLED-Modul
constexpr int I2C_SDA = 5;
constexpr int I2C_SCL = 6;
constexpr int BUTTON_PIN = 9;

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

void drawTextScreen(const char *firstLine, const char *secondLine, const char *thirdLine = nullptr, bool inverted = false)
{
    display.clearBuffer();
    display.setFontMode(1);
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, 72, 40);
        display.setDrawColor(0);
    }

    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 10);
    display.print(firstLine);
    display.setCursor(0, 24);
    display.print(secondLine);

    if (thirdLine != nullptr)
    {
        display.setCursor(0, 38);
        display.print(thirdLine);
    }

    display.sendBuffer();
}

void setup()
{
    // Button initialisieren
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.begin(115200);
    delay(1000);

    Wire.begin(I2C_SDA, I2C_SCL);
    display.begin();

    Serial.println("start vor Wire.begin()");

    // Display: Scanning anzeigen
    drawTextScreen("LipoTest", "scanning");

    Serial.println("LipoTest startet...");

    byte foundDeviceCount = 0;

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0)
        {
            foundDeviceCount++;
            Serial.print("I2C-Geraet gefunden bei 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    Serial.println("Scan fertig.");

    // Display: Ergebnis anzeigen
    char resultLine[24];
    snprintf(resultLine, sizeof(resultLine), "%d found", foundDeviceCount);
    drawTextScreen("LipoTest", resultLine);
}

void loop()
{
}
