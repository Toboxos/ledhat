#include <string>

namespace IO {

    /**
     * Initializes theIO system
     * 
     * Sets up the serial & bluetooth serial connections
     */
    void init();

     /**
     *  Writes a single char to the serial connection
     */
    void write(const char msg);

    /**
     *  Writes a message to the serial connection
     */
    void write(const std::string& msg);

    /**
     * Returns the number of bytes available to read from the serial connection
     */
    int available();

    /**
     * Read from the serial connection. 
     * 
     * \return -1 (int) if read failed. value as uint8_t if read was successfull
     */
    int read();
} 