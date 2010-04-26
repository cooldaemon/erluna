//#include <stdio.h>

#include <ei.h>
#include <erl_driver.h>

#include <lua.h>
#include <lauxlib.h>

#include "erluna_driver.h"
#include "erluna_term.h"

static void to_number(async_erluna_t *data, int index);
static void to_boolean(async_erluna_t *data, int index);
static void to_table(async_erluna_t *data, int index);

void lua_to_erlang(async_erluna_t *data, int index)
{
    switch (lua_type(data->L, index)) {
        case LUA_TNIL:
            ei_x_encode_atom(data->result, "nil");
            break;
        case LUA_TNUMBER:
            to_number(data, index);
            break;
        case LUA_TBOOLEAN:
            to_boolean(data, index);
            break;
        case LUA_TSTRING:
            ei_x_encode_string(data->result, lua_tostring(data->L, index));
            break;
        case LUA_TTABLE:
            to_table(data, index);
            break;
        case LUA_TFUNCTION:
            ei_x_encode_atom(data->result, "function");
            break;
        case LUA_TUSERDATA:
            ei_x_encode_atom(data->result, "userdata");
            break;
        case LUA_TTHREAD:
            ei_x_encode_atom(data->result, "thread");
            break;
        case LUA_TLIGHTUSERDATA:
            ei_x_encode_atom(data->result, "lightuserdata");
            break;
        default: // LUA_TNONE
            return;
    }
}

static void to_number(async_erluna_t *data, int index)
{
    double number = (double)lua_tonumber(data->L, index);

    if ((long long)number == number) {
        ei_x_encode_longlong(data->result, (long long)number);
    } else {
        ei_x_encode_double(data->result, number);
    }
}

static void to_boolean(async_erluna_t *data, int index)
{
    if(lua_toboolean(data->L, index)) {
        ei_x_encode_atom(data->result, "true");
    } else {
        ei_x_encode_atom(data->result, "false");
    }
}

static void to_table(async_erluna_t *data, int index)
{
    int arity = 0;

    lua_pushnil(data->L);
    while (lua_next(data->L, index - 1)) {
        arity++;
        lua_pop(data->L, 1);
    }
//    printf("arity %d\n", arity); fflush(stdout);

    ei_x_encode_list_header(data->result, arity);

    lua_pushnil(data->L);
    while (lua_next(data->L, index - 1)) {
        ei_x_encode_tuple_header(data->result, 2);
        lua_to_erlang(data, -2);
        lua_to_erlang(data, -1);
        lua_pop(data->L, 1);
    }

    ei_x_encode_empty_list(data->result);
}

