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

    /**
     * Current lua thread
     */
    static lua_State* T = nullptr;

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
        static int createRow(lua_State* L) {
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
            return lua_yield(L, 0);
        }
    };


    namespace Proxy {
        int lua_print(lua_State* L) {
            int nargs = lua_gettop(L);

            for( auto i = 1; i <= nargs; ++i ) {

                IO::write( lua_tostring(L, i) );
                IO::write(" ");
            }

            IO::write( "\n" );
            return 0;
        }

        int lua_millis(lua_State* L) {
            lua_pushnumber(L, millis());
            return 1;
        }
    }

    void init() {
        L = luaL_newstate();
        luaL_openlibs(L);

        lua_register(L, "print", Proxy::lua_print);
        lua_register(L, "millis", Proxy::lua_millis);

        lua_createtable(L, 0, 0);
        lua_pushcfunction(L, Matrix::show);
        lua_setfield(L, -2, "show");

        // create a raw object for every led matrix row
        for( auto i = 1; i <= 8; ++i ) {
            lua_pushnumber(L, i);
            Matrix::createRow( L );
            lua_rawseti(L, 1, i);
            lua_pop(L, 1);
        }

        lua_setglobal(L, "LEDHat");
    }

    void execute(const std::string& code) {
        // Stop active thread
        if( T != nullptr ) {
            lua_pop(L, 1); // Pop the thread reference from lua main stack
            T = nullptr; // no active thread at the moment
        }

        // Create new thread
        T = lua_newthread(L);

        luaL_loadstring(T, code.c_str()) == LUA_OK;
    }

    void resume() {
        if( T == nullptr ) {
            return;
        }

        int nres;
        auto status = lua_resume(T, NULL, 0, &nres );
        switch( status ) {
            case LUA_YIELD:
                return;

            default: // for errors
                IO::write( "Error: " );
                IO::write( lua_tolstring(T, -1, NULL) );
                IO::write('\n');
                [[fallthrough]]

            case LUA_OK:
                lua_pop(L, 1); // Pop the thread reference from lua main stack
                T = nullptr; // no active thread at the moment
        }
    }

}