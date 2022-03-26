#include <Arduino.h>
#include <BluetoothSerial.h>
#include <FastLED.h>

#include "characters.h"

BluetoothSerial espBT;

// LED config
#define BRIGHTNESS 10
#define NUM_LEDS 512
CRGB leds[NUM_LEDS];
constexpr unsigned int MAX_COL = 512 / 8;


// buffer for bluetooth communication
struct {
    char data[1024];
    unsigned int offset;
} buffer;

/**
 * Computes the row & column on the led matrix given the index in the linear led buffer
 *
 * @param[in] idx Index in the linear led buffer
 * @param[out] row Row on the led matrix
 * @param[out] col Column on the led matrix
 */
void indexToCoordinate(unsigned int idx, unsigned int& row, unsigned int& col) {
    col = idx / 8;
    row = idx % 8;

    if( col % 2 == 1) {
        row = 7 - row;
    }
}

/**
 * Computes the index in the linear led buffer given the row & column on the led matrix
 *
 * @param[in] row Row on led matrix
 * @param[in] col Column on led matrix
 * @return Index in the linear led buffer
 */
int coordinateToIndex(int row, int col) {
    int idx = col * 8;
    if( col % 2 == 0) {
        idx += row;
    } else {
        idx += (7 - row);
    }

    return idx;
}

/**
 * Gets the corresponding Character entry
 *
 * @param[in] c ASCII character
 * @param[out] character Character entry
 * @returns bool Returns true if character entry was found. Otherwise false is returned
 */
bool getCharacter(const char c, Character& character) {
    auto characterIndex = c - ' ';

    // check boundaries
    if( characterIndex < 0 || characterIndex >= sizeof(characters) / sizeof(Character) ) {
        return false;
    }

    character = characters[ characterIndex ];
    return true;
}

/**
 * Clears the leds in the led buffer. (does not flush led buffer)
 */
void clear() {
    for( auto i = 0; i < NUM_LEDS; ++i ) {
        leds[i].red = leds[i].green = leds[i].blue = 0;
    }
}

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
void drawCharacter(const Character& c, int row, int col, int maxWrapAround = 0 ) {
    for( auto y = 0; y < c.height; ++y ) {
        for( auto x = 0; x < c.width; ++x ) {
            auto fixedCol = (col + x ) % MAX_COL; // column after wrap around

            // wrap around is performed and maxWrapAround is hit --> return (we do not need to anything more)
            if( col + x > MAX_COL && fixedCol > maxWrapAround ) {
                return;
            }

            if( fixedCol < 0 ) {
                continue;
            }

            auto idx = coordinateToIndex(row + y, fixedCol);
            leds[idx].red = BRIGHTNESS * (c.data[y * c.width + x] - '0');
            leds[idx].green = 0;
            leds[idx].blue = 0;
        }
    }
}

/**
 * Draws the given text onto the led buffer on given position. (does not flush the leds)
 *
 * @param[in] text The text to be drawn
 * @param[in] offsetX Start colum position of the text
 * @param[in] offsetY Start row position of the text
 * @param[in] allowWrapAround Defines if a wrap around is allowed
 */
void drawText(const char* text, int offsetX = 0, int offsetY = 0, bool allowWrapAround = true) {
    const auto startPos = offsetY;

    auto len = strlen( text );
    for( auto i = 0; i  < len; ++i ) {
        Character c;
        if( !getCharacter(text[i], c) ) {
            continue;
        }

        if( allowWrapAround ) {
            drawCharacter(c, offsetY, offsetX, startPos - 1); // Wrap around is allowed until 1 column before start of first character
        } else {
            drawCharacter(c, offsetY, offsetX, 0); // No wrap around is allowed
        }

        offsetX += c.width + 1;
    }
}

/**
 * Scrolls a text around the led matrix. (flushed the leds. blocks)
 *
 * @param[in] text
 * @param[in] duration Duration in ms between each scroll step
 */
void scrollText(const char* text, unsigned int duration) {
    const auto len = strlen( text );

    // Compute the boundary of text when printed
    auto stringDisplaySize = 0;
    for( auto i = 0; i < len; ++i ) {
        Character c;
        if( !getCharacter(text[i], c) ) {
            continue;
        }

        stringDisplaySize += c.width + 1;
    }

    // Start printing the text at last column without wrap around.
    // Scroll text in from the right and out to the left
    for( int i = MAX_COL; i >= -stringDisplaySize; --i ) {
        char s[10];
        itoa(i, s, 10);
        espBT.write( (uint8_t*) s, strlen(s) );

        clear();
        drawText( text, i, 1, false );
        FastLED.show();
        delay( duration );
    }
}



/**
 * Reads and evaluates the input from bluetooth
 */
void readBluetooth() {
    while( espBT.available() ) {
        char c = espBT.read();
        if( c == '\n' ) {
            buffer.data[ buffer.offset ] = '\0';
            scrollText( buffer.data, 30 );
            buffer.offset = 0;
        } else {
            buffer.data[ buffer.offset++ ] = c;
        }
    }
}

void setup() {
    buffer.offset = 0;

    espBT.begin( "LED HAT" );
    FastLED.addLeds<NEOPIXEL, 13>( leds, NUM_LEDS );
}

void loop() {
    readBluetooth();
}
