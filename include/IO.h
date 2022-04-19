#include <string>
#include <sstream>

namespace IO {

    /**
     * Initializes theIO system
     *
     * Sets up the serial & bluetooth serial connections
     */
    void init();

    /**
     * Writes the given variable to the serial connection
     */
    template <typename T>
    void write(const T& value) {
        std::stringstream ss;
        ss << value;

        write<std::string>( ss.str() );
    }

    /**
     *  Writes a string to the serial connection
     */
    template <>
    void write<std::string>(const std::string& msg);

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