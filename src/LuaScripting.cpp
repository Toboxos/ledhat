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

    /* Proxy functions calls from lua to the LEDHat */
    namespace LEDHatProxy {

        int getLED(lua_State* L) {
            // get col from 2nd function argument
            const auto col = luaL_checkinteger(L, 2);

            // get the row from table (1st argument) field 'row'
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

        int setLED(lua_State* L) {
            // get col from 2nd function argument
            const auto col = luaL_checkinteger(L, 2);

            // get the row from table (1st argument) field 'row'
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

        int createRow(lua_State* L, int row) {
            // table for the row
            lua_createtable(L, 0, 0);

            // Adding the row as an field
            lua_pushnumber(L, row);
            lua_setfield(L, -2, "row");

            // set metatable
            lua_createtable(L, 0, 0);

            lua_pushcfunction(L, getLED);
            lua_setfield(L, -2, "__index");

            lua_pushcfunction(L, setLED);
            lua_setfield(L, -2, "__newindex");

            lua_setmetatable(L, -2);


            return 1;
        }

        int show(lua_State* L) {
            LEDHat::Instance().show();
            return lua_yield(L, 0);
        }

        int clear(lua_State* L) {
            LEDHat::Instance().clear();
        }

        int drawText(lua_State* L) {
            auto text = luaL_checkstring(L, 1); // 1. arg = text
            auto offsetX = luaL_checkinteger(L, 2); // 2. arg = offsetX
            auto offsetY = luaL_checkinteger(L, 3); // 3. arg = offsetY
            auto wrapArround = lua_toboolean(L, 4); // 4.arg = wrapArround
            auto clear = lua_toboolean(L, 5); // 5.arg = clear

            LEDHat::Instance().drawText(text, offsetX, offsetY, wrapArround, clear);
            return 0;
        }
    }


    /* Proxy functions from lua to Arduino functions */
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

        // registering proxy functions
        lua_register(L, "print", Proxy::lua_print);
        lua_register(L, "millis", Proxy::lua_millis);

        // Create LEDHat table for lua
        lua_createtable(L, 0, 0);
        {
            // registering show function
            lua_pushcfunction(L, LEDHatProxy::show);
            lua_setfield(L, -2, "show");

            // registering clear function
            lua_pushcfunction(L, LEDHatProxy::clear);
            lua_setfield(L, -2, "clear");

            // registering drawText function
            lua_pushcfunction(L, LEDHatProxy::drawText);
            lua_setfield(L, -2, "drawText");

            // create a raw object for every led matrix row
            for( auto i = 1; i <= 8; ++i ) {
                LEDHatProxy::createRow( L, i );
                lua_rawseti(L, -2, i);
            }

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