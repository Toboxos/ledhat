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
    IO::write("Filed closed!\n");
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
    static std::string ioBuffer;

    while( IO::available() ) {
        int i = IO::read();
        if( i == -1 ) {
            continue;
        }

        char c = (char) i;
        IO::write( c );

        switch( c )  {
            case '\r':
                break;

            case '\b':
                ioBuffer.pop_back();
                break;

            case '\n':
                if( ioBuffer.size() > 0 && ioBuffer[0] == '/' ) { // Command
                    if( cmdParser.parseCommand( ioBuffer ) ) {
                        ioBuffer.clear();
                        break;
                    }
                }

                if( file ) { // File Upload
                    file.print( ioBuffer.c_str() );
                    file.print('\n');
                    ioBuffer.clear();
                    break;
                }


                LuaScripting::sendLine(ioBuffer);
                ioBuffer.clear();
                break;

            default:
                ioBuffer += c;
        }
    }

}

void loop() {
    handleIO();
    LuaScripting::resume();
}
