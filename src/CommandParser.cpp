#include "CommandParser.h"
#include "IO.h"

bool CommandParser::parseCommand(const std::string& text) {
    if( text.size() == 0 || text[0] != '/' ) {
        return false;
    }

    auto commandEnd = text.find(' ', 1);
    if( commandEnd == std::string::npos ) {
        callHandler( text.substr(1), "" );
        return true;
    }

    callHandler(  text.substr(1, commandEnd - 1),  text.substr( commandEnd + 1 ) );
    return true;
}

void CommandParser::callHandler(const std::string& command, const std::string& arg) {
    auto handler = _handlers.find( command );
    if( handler == _handlers.end() ) {
        return;
    }

    handler->second( arg );
}
