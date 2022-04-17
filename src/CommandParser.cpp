#include "CommandParser.h"
#include "IO.h"

void CommandParser::parseCommand(const std::string& text) {
    IO::write("Parsing: " + text);
    IO::write('\n');

    if( text.size() == 0 || text[0] != '/' ) {
        IO::write("Not in command format!");
        return;
    }

    auto commandEnd = text.find(' ', 1);
    if( commandEnd == std::string::npos ) {
        callHandler( text.substr(1), "" );
        return;
    }

    callHandler(  text.substr(1, commandEnd - 1),  text.substr( commandEnd + 1 ) );
}

void CommandParser::callHandler(const std::string& command, const std::string& arg) {
    IO::write("Command: '" + command + "' Arg: '" + arg + "'");
    IO::write('\n');
    auto handler = _handlers.find( command );
    if( handler == _handlers.end() ) {
        return;
    }

    handler->second( arg );
}
