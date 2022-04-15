#include <Arduino.h>
#include <BluetoothSerial.h>

#include "CommandParser.h"
#include "LEDHat.h"
#include "LuaScripting.h"

BluetoothSerial espBT;
CommandParser cmdParser;

CRGB gColor(0, 5, 0);
int gScrollSpeed = 40;


/**
 * Callback for providing the pixel color to the LEDHat
 */
CRGB color(unsigned int, unsigned int) {
    return gColor;
}

/**
 * Callback for logging message via bluetooth
 */
void logBT(const std::string& msg) {
    espBT.write( (uint8_t*) msg.c_str(), msg.size() );
    espBT.write('\n');
}

void logSerial(const std::string& msg) {
    Serial.write( msg.c_str() );
}

/**
 * Showing text when /text command is used
 */
void showText(const std::string& text) {
    LEDHat::Instance().scrollText( text.c_str(), gScrollSpeed );
}

/**
 * Sets the speed how fast the text is moving when showText() is called
 */
void setSpeed(const std::string& speed) {
    auto fromChar = [](char c) -> unsigned int {
        c = std::tolower( c );

        if( c >= '0' && c <= '9' ) {
            return c - '0';
        }

        return 0;
    };

    gScrollSpeed = 0;
    for( auto& c : speed ) {
        gScrollSpeed *= 10;
        gScrollSpeed += fromChar(c);
    }
}

/**
 * Sets the color in which the text is drawn
 *
 * @param color Color as hex string (e.g. 02AB3F)
 */
void setColor(const std::string& color) {
    if( color.size() != 6 ) {
        return;
    }

    auto fromHex = [](char c) -> unsigned int {
        c = std::tolower( c );

        if( c >= '0' && c <= '9') {
            return c - '0';
        }

        if( c >= 'a' && c <= 'f' ) {
            return c - 'a';
        }

        return 0;
    };

    gColor.red = (fromHex(color[0]) << 16) + fromHex(color[1]);
    gColor.green = (fromHex(color[2]) << 16) + fromHex(color[3]);
    gColor.blue = (fromHex(color[4]) << 16) + fromHex(color[5]);
}

void setup() {
    espBT.begin( "LED HAT" );
    Serial.begin( 115200 );

    LEDHat::Instance().setup();
    LEDHat::Instance().setColorProvider( color );

    cmdParser.addCommandHandler( "text", showText );
    cmdParser.addCommandHandler( "color", setColor );
    cmdParser.addCommandHandler( "speed", setSpeed );
    cmdParser.setLogger( logBT );


    LuaScripting::init();
    LuaScripting::setLogger( logSerial );
    LuaScripting::execute(" \
        function fibonacci(n) \
            a, b = 0, 1 \
            \
            for i = 1, n do \
                print( 'a=', a, ' b=', b ) \
                a, b = b, a + b \
            end \
            return a \
        end \
        \
        fibonacci(3) \
    ");
}

std::string cmdBuffer;
void loop() {
    while( espBT.available() ) {
        char c = espBT.read();
        if( c == '\n' ) {
            cmdParser.parseCommand( cmdBuffer );
            cmdBuffer = "";
        } else if( c != '\r' ) {
           cmdBuffer += c;
        }
    }
}
