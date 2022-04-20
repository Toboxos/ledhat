#include <HardwareSerial.h>
#include <BluetoothSerial.h>

#include "IO.h"

namespace IO {
    BluetoothSerial espBT;

    void init() {
        espBT.begin( "LED HAT" );
        espBT.setPin("2259");
        Serial.begin( 115200 );
    }

    template <>
    void write<std::string>( const std::string& msg ) {
        espBT.write( (const uint8_t*) msg.c_str(), msg.size() );
        Serial.write( (const uint8_t*) msg.c_str(), msg.size() );
    }

    int available() {
        return espBT.available() + Serial.available();
    }

    int read() {
        if( espBT.available() ) {
            int c = espBT.read();
            if( c > -1 ) {
                return c;
            }
        }

        if( Serial.available() ) {
            int c = Serial.read();
            if( c > -1 ) {
                return c;
            }
        }

        return -1;
    }
}