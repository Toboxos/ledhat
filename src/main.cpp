#include <Arduino.h>
#include <BluetoothSerial.h>
#include <SPIFFS.h>

#include "CommandParser.h"
#include "IO.h"
#include "LEDHat.h"
#include "LuaScripting.h"


File file;
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

void execute(const std::string& code) {
    LuaScripting::execute( code );
}

void uploadFile(const std::string& filename) {
    file = SPIFFS.open( ("/" + filename).c_str(), FILE_WRITE );
    if( !file ) {
        IO::write( "Failed to open file!\n" );
        return;
    }
    IO::write("Content will be written to " + filename + "\n");
}

void closeFile(const std::string& _) {
    file.close();
    IO::write("Filed closed!");
}

void loadFile(const std::string& filename) {
    file = SPIFFS.open( ("/" + filename).c_str() );
    if( !file ) {
        IO::write( "Failed to open file!\n" );
        return;
    }

    std::string code;
    while( file.available() ) {
        code += file.readString().c_str();
    }

    file.close();
    LuaScripting::execute( code );
}

void dumpFile(const std::string& filename) {
    file = SPIFFS.open( ("/" + filename).c_str() );
    if( !file ) {
        IO::write( "Failed to open file!\n" );
        return;
    }

    std::string code;
    while( file.available() ) {
        code += file.readString().c_str();
    }

    file.close();
    IO::write(code);
    IO::write('\n');
}

void setup() {
    IO::init();
    LEDHat::Instance().setup();
    LEDHat::Instance().setColorProvider( color );

    SPIFFS.begin( true );

    cmdParser.addCommandHandler( "text", showText );
    cmdParser.addCommandHandler( "color", setColor );
    cmdParser.addCommandHandler( "speed", setSpeed );
    cmdParser.addCommandHandler( "execute", execute );
    cmdParser.addCommandHandler( "upload", uploadFile );
    cmdParser.addCommandHandler( "close", closeFile );
    cmdParser.addCommandHandler( "load", loadFile );
    cmdParser.addCommandHandler( "dump", dumpFile );

    LuaScripting::init();
}

std::string cmdBuffer;

void loop() {
    while( IO::available() ) {
        char c = IO::read();

        if( c == '\n' ) {
            if( !cmdParser.parseCommand( cmdBuffer ) && file ) {
                file.print( cmdBuffer.c_str() );
                file.print("\n");
            }
            cmdBuffer = "";
        } else if( c != '\r' ) {
           cmdBuffer += c;
        }
    }
}
