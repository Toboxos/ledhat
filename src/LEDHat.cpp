#include "LEDHat.h"

LEDHat &LEDHat::Instance()
{
    static LEDHat instance;
    return instance;
}

void LEDHat::setup()
{
    FastLED.addLeds<NEOPIXEL, PIN>(_ledBuffer, NUM_LEDS);
}

void LEDHat::indexToCoordinate(unsigned int idx, unsigned int &row, unsigned int &col)
{
    col = idx / ROWS;
    row = idx % ROWS;

    if (col % 2 == 1)
    {
        row = (ROWS - 1) - row;
    }
}

int LEDHat::coordinateToIndex(int row, int col)
{
    int idx = col * ROWS;
    if (col % 2 == 0)
    {
        idx += row;
    }
    else
    {
        idx += ((ROWS - 1) - row);
    }

    return idx;
}

void LEDHat::clear()
{
    for (auto i = 0; i < NUM_LEDS; ++i)
    {
        _ledBuffer[i].red = _ledBuffer[i].green = _ledBuffer[i].blue = 0;
    }
}

void LEDHat::drawCharacter(const Character &c, int row, int col, CRGB color, int maxWrapAround /*= 0*/)
{
    for (auto x = 0; x < c.width; ++x)
    {
        for (auto y = 0; y < c.height; ++y)
        {
            auto fixedCol = (col + x) % COLS; // column after wrap around

            // wrap around is performed and maxWrapAround is hit --> return (we do not need to anything more)
            if (col + x > COLS && fixedCol > maxWrapAround)
            {
                return;
            }

            if (fixedCol < 0)
            {
                continue;
            }

            auto idx = coordinateToIndex(row + y, fixedCol);

            if (c.data[y * c.width + x] - '0')
            {
                _ledBuffer[idx] = color;
            }
        }
    }
}

void LEDHat::drawText(const char *text, CRGB color, int offsetX /*= 0*/, int offsetY /*= 0*/, bool allowWrapAround /*= true*/)
{
    const auto startPos = offsetY;

    auto len = strlen(text);
    for (auto i = 0; i < len; ++i)
    {
        Character c;
        if (!getCharacter(text[i], c))
        {
            continue;
        }

        if (allowWrapAround)
        {
            drawCharacter(c, offsetY, offsetX, color, startPos - 1); // Wrap around is allowed until 1 column before start of first character
        }
        else
        {
            drawCharacter(c, offsetY, offsetX, color, 0); // No wrap around is allowed
        }

        offsetX += c.width;
    }
}

void LEDHat::setPixel(int y, int x, CRGB color) {
   auto idx = coordinateToIndex(y, x);

   _ledBuffer[idx] = color; 
}

CRGB LEDHat::getPixel(int y, int x ) {
   auto idx = coordinateToIndex(y, x);

   return _ledBuffer[idx];
}

void LEDHat::show() {
    FastLED.show();
}
