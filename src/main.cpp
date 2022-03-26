#include <Arduino.h>
#include <BluetoothSerial.h>

#include "LEDHat.h"

BluetoothSerial espBT;


// buffer for bluetooth communication
struct {
    char data[1024];
    unsigned int offset;
} buffer;

CRGB color(unsigned int, unsigned int) {
    return CRGB(0, 5, 0);
}

/**
 * Reads and evaluates the input from bluetooth
 */
void readBluetooth() {
    while( espBT.available() ) {
        char c = espBT.read();
        if( c == '\n' ) {
            buffer.data[ buffer.offset ] = '\0';
            LEDHat::Instance().scrollText( buffer.data, 30 );
            buffer.offset = 0;
        } else {
            buffer.data[ buffer.offset++ ] = c;
        }
    }
}

void setup() {
    buffer.offset = 0;

    espBT.begin( "LED HAT" );
    LEDHat::Instance().setup();
    LEDHat::Instance().setColorProvider( color );
}

void loop() {
    readBluetooth();
}
