#pragma once
#include <string>
#include <functional>>

namespace LuaScripting {
    /**
     * Initialize the lua VM
     */
    void init();

    /**
     * Executes the given lua code
     *
     * @param code Lua code to run
     */
    void execute(const std::string& code);
}