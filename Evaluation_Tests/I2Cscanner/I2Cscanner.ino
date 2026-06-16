//
// Simple Routine, die einfach prüft ob iam I2C Bus überhaupt etwas zu finden ist
//

#include <Arduino.h>
#include <Wire.h>

//  ESP32-C3 OLED-Entwicklungsboard mit 0,42 Zoll OLED-Modul
constexpr int I2C_SDA = 5;
constexpr int I2C_SCL = 6;
constexpr int BUTTON_PIN = 9;

void setup()
{
    // Initialize button
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor

    Serial.begin(115200);
    delay(1000);
    Serial.println("start vor Wire.begin()");

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.println("I2C Scanner startet...");

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        // Serial.print(address, HEX);
        // Serial.print(':');
        // Serial.println(error);
        if (error == 0)
        {
            Serial.print("I2C-Geraet gefunden bei 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    Serial.println("Scan fertig.");
}

void loop()
{
}