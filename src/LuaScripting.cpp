#include <sstream>

#include "IO.h"
#include "LEDHat.h"
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

    void log(const std::string& msg);
    void log(int value);

    struct Row {
        static int get(lua_State* L) {
            const auto col = luaL_checkinteger(L, 2);
        
            lua_getfield(L, 1, "row");
            lua_Integer row = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            auto pixel = LEDHat::Instance().getPixel(row - 1, col - 1);

            lua_createtable(L, 0, 0); // -4

            lua_pushinteger(L, pixel.r ); // -3
            lua_pushinteger(L, pixel.g ); // -2
            lua_pushinteger(L, pixel.b ); // -1

            lua_rawseti(L, -4, 3); // b
            lua_rawseti(L, -3, 2); // g
            lua_rawseti(L, -2, 1); // r

            return 1;
        }

        static int set(lua_State* L) {
            const auto col = luaL_checkinteger(L, 2);
        
            lua_getfield(L, 1, "row");
            lua_Integer row = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 3, 1); // r  -3
            lua_rawgeti(L, 3, 2); // g  -2 
            lua_rawgeti(L, 3, 3); // b  -1

            lua_Integer r = luaL_checkinteger(L, -3);
            lua_Integer g = luaL_checkinteger(L, -2);
            lua_Integer b = luaL_checkinteger(L, -1);

            lua_pop(L, 3);

            LEDHat::Instance().setPixel(row - 1, col - 1, CRGB(r, g, b) );
            return 0;
        }
    };

    struct Matrix {
        static int index(lua_State* L) {
            lua_createtable(L, 0, 0);
            
            // set row attribute
            lua_Number row = luaL_checknumber(L, 2);

            lua_pushnumber(L, row);
            lua_setfield(L, -2, "row");

            // set metatable
            lua_createtable(L, 0, 0);

            lua_pushcfunction(L, Row::get);
            lua_setfield(L, -2, "__index");

            lua_pushcfunction(L, Row::set);
            lua_setfield(L, -2, "__newindex");

            lua_setmetatable(L, -2);


            return 1;
        }

        static int show(lua_State* L) {
            LEDHat::Instance().show();
            return 0;
        }
    };


    void log(const std::string& msg) {
        IO::write( msg );
    }

    void log(int number) {
        std::stringstream ss;
        ss << number;

        log( ss.str() );
    }

    int lua_print(lua_State* L) {
        int nargs = lua_gettop(L);

        for( auto i = 1; i <= nargs; ++i ) {

            log( lua_tostring(L, i) );
            log(" ");
        }

        log( "\n" );
        return 1;
    }

    int lua_panic(lua_State* L) {
        log("Lua error");
        return 0;
    }

    int lua_millis(lua_State* L) {
        lua_pushnumber(L, millis());
        return 1;
    }

    void init() {
        L = luaL_newstate();
        luaL_openlibs(L);

        lua_atpanic(L, lua_panic);

        lua_pushcfunction(L, lua_print);
        lua_setglobal(L, "print");

        lua_createtable(L, 0, 0);
        lua_pushcfunction(L, Matrix::show);
        lua_setfield(L, -2, "show");

        lua_pushcfunction(L, lua_millis);
        lua_setfield(L, -2, "millis");

        // create a raw object for every led matrix row
        for( auto i = 1; i <= 8; ++i ) {
            lua_pushnumber(L, i);
            Matrix::index( L );
            lua_rawseti(L, 1, i);
            lua_pop(L, 1);
        }

        lua_setglobal(L, "LEDHat");
    }

    void execute(const std::string& code) {
        if( luaL_loadstring(L, code.c_str()) == LUA_OK ) {
            if( lua_pcall(L, 0, 0, 0) == LUA_OK ) {
                // If it was executed successfuly we 
                // remove the code from the stack
                lua_pop(L, lua_gettop(L));
            } else {
                IO::write( "Error: " );
                IO::write( lua_tolstring(L, -1, NULL) );
                IO::write('\n');
            }
        }
    }

}