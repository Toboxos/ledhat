#pragma once
#include <string>

namespace LuaScripting {
    /**
     * Initialize the lua VM
     */
    void init();

    /**
     * Loads the given code to the lua engine.
     *
     * The code will not start running immediately. It will start as a serperate lua thread
     * which has to be resumed using the resume() method
     *
     * @param code Lua code to run
     */
    void execute(const std::string& code);

    /**
     * Resums the current lua thread.
     *
     * The current active lua thread will run until the next yield
     */
    void resume();
}