#pragma once
#include <string>
#include <functional>>

namespace LuaScripting {
    using LoggerFunction = std::function<void(const std::string&)>;

    /**
     * Initialize the lua VM
     */
    void init();

    /**
     * Sets the logging function lua outputs will go to
     */
    void setLogger( const LoggerFunction& logger );

    /**
     * Executes the given lua code
     *
     * @param code Lua code to run
     */
    void execute(const std::string& code);
}