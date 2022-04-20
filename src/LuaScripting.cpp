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

    /**
     * Buffer for data received from the serial connection
     */
    std::string dataBuffer;


    /* Proxy functions calls from lua to the LEDHat */
    namespace LEDHatProxy {

        static void stack_dump(lua_State* L, const char* stackname) {
            int i;
            int top = lua_gettop(L);
            printf("--------------- %s STACK ---------------\n", stackname);
            for (i = top; i >= 1; i--) {
                int t = lua_type(L, i);
                printf("[%2d - %8s] : ", i, lua_typename(L, t));
                switch (t) {
                case LUA_TSTRING:
                    printf("%s", lua_tostring(L, i));
                    break;
                case LUA_TBOOLEAN:
                    printf(lua_toboolean(L, i) ? "true" : "false");
                    break;
                case LUA_TNUMBER:
                    printf("%g", lua_tonumber(L, i));
                    break;
                case LUA_TNIL:
                    printf("nil");
                    break;
                case LUA_TNONE:
                    printf("none");
                    break;
                case LUA_TFUNCTION:
                    printf("<function %p>", lua_topointer(L, i));
                    break;
                case LUA_TTABLE:
                    printf("<table %p>", lua_topointer(L, i));
                    break;
                case LUA_TTHREAD:
                    printf("<thread %p>", lua_topointer(L, i));
                    break;
                case LUA_TUSERDATA:
                    printf("<userdata %p>", lua_topointer(L, i));
                    break;
                case LUA_TLIGHTUSERDATA:
                    printf("<lightuserdata %p>", lua_topointer(L, i));
                    break;
                default:
                    printf("unknown %s", lua_typename(L, t));
                    break;
                }
                printf("\n");
            }
            printf("--------------- %s STACK ---------------\n", stackname);
        }

        namespace Helpers {
            CRGB lua_tocolor(lua_State* L, int idx) {
                lua_pushvalue(L, idx); // push table to top of stack

                lua_rawgeti(L, -1, 1); // r     -3
                lua_rawgeti(L, -2, 2); // g     -2
                lua_rawgeti(L, -3, 3); // b     -1

                lua_Integer r = luaL_checkinteger(L, -3);
                lua_Integer g = luaL_checkinteger(L, -2);
                lua_Integer b = luaL_checkinteger(L, -1);

                lua_pop(L, 4); // pop color values + table

                return CRGB(r, g, b);
            }
        }

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
            auto color = Helpers::lua_tocolor(L, 2); // 2. arg = color
            auto offsetX = luaL_checkinteger(L, 3); // 3. arg = offsetX
            auto offsetY = luaL_checkinteger(L, 4); // 4. arg = offsetY
            auto wrapArround = lua_toboolean(L, 5); // 5.arg = wrapArround

            LEDHat::Instance().drawText(text, color, offsetX, offsetY, wrapArround);
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
    
        int readLine(lua_State* L) {
            lua_pushstring(L, dataBuffer.c_str() );
            dataBuffer.clear();

            return lua_yield(L, 1);
        }
    }

    void init() {
        L = luaL_newstate();
        luaL_openlibs(L);

        // registering proxy functions
        lua_register(L, "print", Proxy::lua_print);
        lua_register(L, "millis", Proxy::lua_millis);
        lua_register(L, "readLine", Proxy::readLine );

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


        // if thread was yielded use the same stack to resume
        auto top = 0;
        if( lua_status(T) == LUA_YIELD ) {
            top = lua_gettop( T );
        }

        int nres;
        auto status = lua_resume(T, NULL, top, &nres );
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

    void sendLine(const std::string& line) {
        dataBuffer = line;
    }

}