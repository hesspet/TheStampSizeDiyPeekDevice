//
// LipoTest – INA219 Strom- und Spannungsmessung mit OLED-Anzeige und InfluxDB
//

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ESP32-C3 OLED-Entwicklungsboard mit 0,42 Zoll OLED-Modul
constexpr int I2C_SDA = 5;
constexpr int I2C_SCL = 6;
constexpr int BUTTON_PIN = 9;
constexpr uint8_t RELAIS_PIN = 4; // Abschalter
constexpr uint8_t BLUE_LED_PIN = 8; 

// WLAN-Konfiguration
constexpr char WIFI_SSID[] = "Agathas-Netz-16";
constexpr char WIFI_PASS[] = "1234567890123050363";

// InfluxDB-Konfiguration
const char* INFLUX_URL =
    "http://homeserver:8086/api/v2/write?org=home&bucket=LIPO&precision=s";
const char* INFLUX_TOKEN = "7_QhmRrXgvwFgGajRG1vEPXj-kWYqq2ucXbpjADZr6bo4u77ir3RFkGGdyaOPFTC4rnoDra654FTN5OKpadJTA==";

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
Adafruit_INA219 ina219(0x40);

float accumulated_mAh = 0.0f;
float accumulated_mWh = 0.0f;
unsigned long lastMillis = 0;

void drawTextScreen(const char* firstLine, const char* secondLine, const char* thirdLine = nullptr, bool inverted = false)
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

void connectWifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Verbinde WLAN");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WLAN verbunden: ");
    Serial.println(WiFi.localIP());
}

bool sendToInflux(float voltage_V, float current_mA, float power_mW, float accumulated_mAh, float accumulated_mWh)
{
    if (WiFi.status() != WL_CONNECTED)
        return false;

    HTTPClient http;
    http.begin(INFLUX_URL);
    http.addHeader("Authorization", String("Token ") + INFLUX_TOKEN);
    http.addHeader("Content-Type", "text/plain");

    String line;
    line += "ina219,device=esp32_current_monitor ";
    line += "voltage_V=" + String(voltage_V, 4);
    line += ",current_mA=" + String(current_mA, 4);
    line += ",power_mW=" + String(power_mW, 4);
    line += ",accumulated_mAh=" + String(accumulated_mAh, 6);
    line += ",accumulated_mWh=" + String(accumulated_mWh, 6);
    
    int httpCode = http.POST(line);
    http.end();

    return httpCode >= 200 && httpCode < 300;
}

void setup()
{
    // Button initialisieren
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAIS_PIN, OUTPUT);

    Serial.begin(115200);
    delay(1000);

    Wire.begin(I2C_SDA, I2C_SCL);
    display.begin();

    Serial.println("LipoTest startet...");

    // INA219 initialisieren
    if (!ina219.begin())
    {
        Serial.println("INA219 nicht gefunden!");
        drawTextScreen("LipoTest", "INA219 Fehler");
        while (1) { delay(1000); }
    }

    ina219.setCalibration_32V_2A();
    Serial.println("INA219 bereit.");

    // WLAN verbinden
    connectWifi();

    drawTextScreen("LipoTest", "WiFi verbunden");
    delay(1500);

    lastMillis = millis();

    digitalWrite(RELAIS_PIN, true); // entladen beginnt
}

void loop()
{
    unsigned long now = millis();
    if (now - lastMillis >= 1000)
    {
       
        float deltaHours = (now - lastMillis) / 3600000.0f;
        lastMillis = now;

        float current_mA = ina219.getCurrent_mA();
        float voltage_V  = ina219.getBusVoltage_V();
        float power_mW   = ina219.getPower_mW();

        accumulated_mAh += current_mA * deltaHours;
        accumulated_mWh += voltage_V * current_mA * deltaHours;

        // Serielle Ausgabe
        Serial.print("I=");
        Serial.print(current_mA, 2);
        Serial.print(" mA, U=");
        Serial.print(voltage_V, 3);
        Serial.print(" V, P=");
        Serial.print(power_mW, 2);
        Serial.print(" mW, Kapazitaet=");
        Serial.print(accumulated_mAh, 3);
        Serial.print(" mAh, Energie=");
        Serial.print(accumulated_mWh, 3);
        Serial.print(" mWh : ");

        // InfluxDB senden
        bool ok = sendToInflux(voltage_V, current_mA, power_mW, accumulated_mAh, accumulated_mWh);
        Serial.println(ok ? "InfluxDB: OK" : "InfluxDB: Fehler");

        // OLED-Ausgabe
        char line1[24];
        char line2[24];
        char line3[24];
        snprintf(line1, sizeof(line1), "mA: %.1f", current_mA);
        snprintf(line2, sizeof(line2), " V: %.3f", voltage_V);
        snprintf(line3, sizeof(line3), "Ah: %.1f", accumulated_mAh);
        drawTextScreen(line1, line2, line3);

        // undervoltage protection
        if (voltage_V < 3.3f) 
        {
            digitalWrite(RELAIS_PIN, false);
        }
    }
}
