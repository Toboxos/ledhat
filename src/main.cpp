#include <Arduino.h>
#include <BluetoothSerial.h>
#include <SPIFFS.h>

#include "CommandParser.h"
#include "IO.h"
#include "LEDHat.h"
#include "LuaScripting.h"


File file;
CommandParser cmdParser;

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
    SPIFFS.begin( true );
    LEDHat::Instance().setup();

    cmdParser.addCommandHandler( "execute", execute );
    cmdParser.addCommandHandler( "upload", uploadFile );
    cmdParser.addCommandHandler( "close", closeFile );
    cmdParser.addCommandHandler( "load", loadFile );
    cmdParser.addCommandHandler( "dump", dumpFile );

    LuaScripting::init();
}


void handleIO() {
    static std::string cmdBuffer;

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

void loop() {
    handleIO();
    LuaScripting::resume();
}
