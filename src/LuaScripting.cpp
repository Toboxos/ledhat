#include <sstream>

#include "LuaScripting.h"

extern "C" {
    #include <lauxlib.h>
    #include <lualib.h>
}

namespace LuaScripting {

    /**
     * Global lua state
     */
    static lua_State* L = nullptr;

    /**
     * Callback to log
     */
    static LoggerFunction LOGGER;

    void log(const std::string& msg) {
        if( !LOGGER ) {
            return;
        }

        LOGGER( msg );
    }

    void log(int number) {
        std::stringstream ss;
        ss << number;

        log( ss.str() );
    }

    int lua_print(lua_State* L) {
        int nargs = lua_gettop(L);

        for( auto i = 1; i <= nargs; ++i ) {

            /* String */
            if( lua_isstring(L, i) ) {
               log( luaL_checkstring(L, i) );
            }

            /* Integer */
            else if( lua_isinteger(L, i) ) {
                log( luaL_checkinteger(L, i) );
            }
        }

        log( "\n" );
        return 1;
    }

    void init() {
        L = luaL_newstate();
        luaL_openlibs(L);

        lua_pushcfunction(L, lua_print);
        lua_setglobal(L, "print");
    }

    void setLogger(const LoggerFunction& logger) {
        LOGGER = logger;
    }

    void execute(const std::string& code) {
        if( luaL_loadstring(L, code.c_str()) == LUA_OK ) {
            if( lua_pcall(L, 0, 0, 0) == LUA_OK ) {
                // If it was executed successfuly we 
                // remove the code from the stack
                lua_pop(L, lua_gettop(L));
            }
        }
    }

}