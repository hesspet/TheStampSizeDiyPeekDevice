#include "display/M5StickHardware.h"

M5StickHardware::M5StickHardware()
{
}

void M5StickHardware::begin()
{
    auto configuration = M5.config();
    M5.begin(configuration);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(128);
    M5.Display.fillScreen(ColorBackground);
    M5.Display.setTextColor(ColorForeground, ColorBackground);
}

void M5StickHardware::clearBuffer()
{
    M5.Display.fillScreen(ColorBackground);
}

void M5StickHardware::sendBuffer()
{
    // M5GFX zeichnet direkt.
}

uint8_t *M5StickHardware::getBufferPtr()
{
    return nullptr;
}

size_t M5StickHardware::getBufferSize()
{
    return 0;
}

void M5StickHardware::setFont(const void *font)
{
    const uint8_t textSize = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(font));
    if (textSize > 0)
    {
        M5.Display.setTextSize(textSize);
    }
}

void M5StickHardware::setFontMode(uint8_t /*mode*/)
{
}

void M5StickHardware::enableUTF8Print()
{
}

void M5StickHardware::setCursor(int16_t x, int16_t y)
{
    M5.Display.setCursor(x, y);
}

void M5StickHardware::print(const char *text)
{
    M5.Display.print(text);
}

void M5StickHardware::print(int32_t value)
{
    M5.Display.print(value);
}

int16_t M5StickHardware::getStrWidth(const char *text)
{
    return M5.Display.textWidth(text);
}

int16_t M5StickHardware::getDisplayWidth()
{
    return M5.Display.width();
}

int16_t M5StickHardware::getDisplayHeight()
{
    return M5.Display.height();
}

void M5StickHardware::setDrawColor(uint16_t color)
{
    drawColor = color;
    M5.Display.setTextColor(color);
}

void M5StickHardware::drawBox(int16_t x, int16_t y, int16_t w, int16_t h)
{
    M5.Display.fillRect(x, y, w, h, drawColor);
}

void M5StickHardware::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    M5.Display.drawLine(x1, y1, x2, y2, drawColor);
}

void M5StickHardware::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3)
{
    M5.Display.fillTriangle(x1, y1, x2, y2, x3, y3, drawColor);
}

void M5StickHardware::drawCircle(int16_t x, int16_t y, int16_t radius)
{
    M5.Display.drawCircle(x, y, radius, drawColor);
}

void M5StickHardware::drawDisc(int16_t x, int16_t y, int16_t radius)
{
    M5.Display.fillCircle(x, y, radius, drawColor);
}

void M5StickHardware::drawFrame(int16_t x, int16_t y, int16_t w, int16_t h)
{
    M5.Display.drawRect(x, y, w, h, drawColor);
}

void M5StickHardware::drawPixel(int16_t x, int16_t y)
{
    M5.Display.drawPixel(x, y, drawColor);
}

void M5StickHardware::fillScreen(uint16_t color)
{
    M5.Display.fillScreen(color);
    M5.Display.setTextColor(
        color == ColorBackground ? ColorForeground : ColorInvertForeground,
        color);
}

void M5StickHardware::setDisplayRotation(uint8_t rotation)
{
    M5.Display.setRotation(rotation);
}

void M5StickHardware::enterHardwareSleep()
{
    M5.Display.setBrightness(0);
    M5.Display.sleep();
}

void M5StickHardware::wakeFromHardwareSleep()
{
    M5.Display.wakeup();
    M5.Display.setBrightness(128);
}

void M5StickHardware::deactivateBeforeDeepSleep()
{
    M5.Display.fillScreen(ColorBackground);
    M5.Display.setBrightness(0);
    M5.Display.sleep();
}

void *M5StickHardware::getRawDisplay()
{
    return &M5.Display;
}

M5GFX &M5StickHardware::getDisplay()
{
    return M5.Display;
}

int32_t M5StickHardware::getBatteryLevelPercent()
{
    int32_t batteryLevelPercent = M5.Power.getBatteryLevel();
    if (batteryLevelPercent >= 0)
    {
        return batteryLevelPercent > 100 ? 100 : batteryLevelPercent;
    }

    const int16_t batteryVoltageMillivolts = getBatteryVoltageMillivolts();
    if (batteryVoltageMillivolts <= 0)
    {
        return -1;
    }

    const int32_t calculatedLevelPercent =
        (static_cast<int32_t>(batteryVoltageMillivolts) - 3300) * 100 / (4150 - 3300);

    if (calculatedLevelPercent < 0) return 0;
    if (calculatedLevelPercent > 100) return 100;
    return calculatedLevelPercent;
}

int16_t M5StickHardware::getBatteryVoltageMillivolts()
{
    return M5.Power.getBatteryVoltage();
}

void M5StickHardware::drawBatteryLevelLine()
{
    const int32_t batteryLevelPercent = getBatteryLevelPercent();
    if (batteryLevelPercent < 0)
    {
        return;
    }

    const int16_t displayWidth = getDisplayWidth();
    constexpr int16_t lineHeight = 4;
    const int16_t filledWidth = static_cast<int16_t>(
        (static_cast<int32_t>(displayWidth) * batteryLevelPercent + 99) / 100);

    uint16_t batteryColor = ColorForeground;
    if (batteryLevelPercent <= 20)
    {
        batteryColor = ColorBatteryCritical;
    }
    else if (batteryLevelPercent <= 40)
    {
        batteryColor = ColorBatteryWarning;
    }

    M5.Display.fillRect(0, 0, displayWidth, lineHeight, ColorBatteryTrack);
    if (filledWidth > 0)
    {
        M5.Display.fillRect(0, 0, filledWidth, lineHeight, batteryColor);
    }
}
