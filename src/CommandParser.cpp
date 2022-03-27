#include "CommandParser.h"

void CommandParser::parseCommand(const std::string& text) {
    _logger("Parsing: " + text);

    if( text.size() == 0 || text[0] != '/' ) {
        _logger("Not in command format!");
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
    _logger("Command: '" + command + "' Arg: '" + arg + "'");
    auto handler = _handlers.find( command );
    if( handler == _handlers.end() ) {
        return;
    }

    handler->second( arg );
}
