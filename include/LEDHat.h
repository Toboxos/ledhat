#pragma once
#include <FastLED.h>
#include <functional>

#include "characters.h"

class LEDHat
{
public:
    using ColorProvider = std::function<CRGB(unsigned int row, unsigned int col)>;

    /**
     * Singleton instance function
     *
     * @returns The singleton instance of the LEDHat
     */
    static LEDHat &Instance();

    /**
     * Setup method. Must be called before any other method is called
     */
    void setup();

    /**
     * Clears the the led matrix
     */
    void clear();

    /**
     * Sets a color provider
     *
     * A color provider gets the row and column of a pixel which is drawn and returns the color of the pixel
     */
    void setColorProvider( const ColorProvider& colorProvider ) { _colorProvider = colorProvider; }

    /**
     * Prints the given character at given position. Wrap around automatically.
     *
     * Prints ths given character at given position. If the columns exceeds the maximum column count
     * of an 2D band a automatic wrap around is done so the printing continues on the beginning.
     * To prevent overwriting existing characters the maxWrapAround paramter is used to determine
     * the maximum colum from start until which a character is allowed to be printed.
     *
     *                                        \/ colum count exceeded
     * +-------------------------------------+
     * |                                     |*
     * |                                     |
     * +-------------------------------------+
     *
     *  \/ wrap around is performed
     * +-------------------------------------+
     * |*                                    |
     * |                                     |
     * +-------------------------------------+
     *
     * Wrap around is only done in positive direction (excedding the max column count).
     * If a light of the character has an negative led column it is not drawn.
     *
     * @param[in] c Character to print
     * @param[in] row Row where to start printing character
     * @param[in] col Column where to start printing character
     * @param[in] maxWrapAround Maximum column until which a character is allowed to be printed after wrap around
     */
    void drawCharacter(const Character &c, int row, int col, int maxWrapAround = 0);

    /**
     * Draws the given text onto the led buffer on given position. (does not flush the leds)
     *
     * @param[in] text The text to be drawn
     * @param[in] offsetX Start colum position of the text
     * @param[in] offsetY Start row position of the text
     * @param[in] allowWrapAround Defines if a wrap around is allowed
     */
    void drawText(const char *text, int offsetX = 0, int offsetY = 0, bool allowWrapAround = true);

    /**
     * Scrolls a text around the led matrix. (flushed the leds. blocks)
     *
     * @param[in] text
     * @param[in] duration Duration in ms between each scroll step
     */
    void scrollText(const char *text, unsigned int duration);

private:
    LEDHat() = default;

    /**
     * Computes the row & column on the led matrix given the index in the linear led buffer
     *
     * @param[in] idx Index in the linear led buffer
     * @param[out] row Row on the led matrix
     * @param[out] col Column on the led matrix
     */
    void indexToCoordinate(unsigned int idx, unsigned int &row, unsigned int &col);

    /**
     * Computes the index in the linear led buffer given the row & column on the led matrix
     *
     * @param[in] row Row on led matrix
     * @param[in] col Column on led matrix
     * @return Index in the linear led buffer
     */
    int coordinateToIndex(int row, int col);

    /**
     * Callback providing the color for a pixel
     */
    ColorProvider _colorProvider;

    /**
     * Total number of leds of the led matrix
     */
    const static unsigned int NUM_LEDS = 512;

    /**
     * Rows of the led matrix
     */
    const static unsigned int ROWS = 8;

    /**
     * Columns of the led matrix
     */
    const static unsigned int COLS = NUM_LEDS / 8;

    /**
     * The pin of the ESP32 where the led matrix is connected
     */
    const static unsigned int PIN = 13;

    /**
     * The led buffer containing the pixel values
     */
    CRGB _ledBuffer[NUM_LEDS];
};