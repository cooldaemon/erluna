//#include <stdio.h>
#include <string.h> // strcmp

#include <ei.h>
#include <erl_driver.h>

#include <lua.h>
#include <lauxlib.h>

#include "erluna_driver.h"
#include "erluna_term.h"

static void lua_to_number(async_erluna_t *data, int index);
static void lua_to_boolean(async_erluna_t *data, int index);
static void lua_to_table(async_erluna_t *data, int index);

static int erlang_to_ulong(async_erluna_t *data, int *index);
static int erlang_to_long(async_erluna_t *data, int *index);
static int erlang_to_double(async_erluna_t *data, int *index);
static int erlang_to_atom(async_erluna_t *data, int *index, int size);
static int erlang_to_string(async_erluna_t *data, int *index, int size);
static int erlang_to_list(async_erluna_t *data, int *index);
static int erlang_to_tuple(async_erluna_t *data, int *index);

void lua_to_erlang(async_erluna_t *data, int index)
{
    switch (lua_type(data->L, index)) {
        case LUA_TNIL:
            ei_x_encode_atom(data->result, "nil");
            break;
        case LUA_TNUMBER:
            lua_to_number(data, index);
            break;
        case LUA_TBOOLEAN:
            lua_to_boolean(data, index);
            break;
        case LUA_TSTRING:
            ei_x_encode_string(data->result, lua_tostring(data->L, index));
            break;
        case LUA_TTABLE:
            lua_to_table(data, index);
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

static void lua_to_number(async_erluna_t *data, int index)
{
    double number = (double)lua_tonumber(data->L, index);

    if ((long long)number == number) {
        ei_x_encode_longlong(data->result, (long long)number);
    } else {
        ei_x_encode_double(data->result, number);
    }
}

static void lua_to_boolean(async_erluna_t *data, int index)
{
    if(lua_toboolean(data->L, index)) {
        ei_x_encode_atom(data->result, "true");
    } else {
        ei_x_encode_atom(data->result, "false");
    }
}

static void lua_to_table(async_erluna_t *data, int index)
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

int erlang_to_lua(async_erluna_t *data, int *index)
{
    int type, size;
    ei_get_type(data->args, index, &type, &size);

    switch (type) {
        case ERL_SMALL_INTEGER_EXT:
            return erlang_to_ulong(data, index);
        case ERL_INTEGER_EXT:
            return erlang_to_long(data, index);
        case ERL_FLOAT_EXT:
            return erlang_to_double(data, index);
        case ERL_ATOM_EXT:
            return erlang_to_atom(data, index, size);
        case ERL_REFERENCE_EXT:
        case ERL_NEW_REFERENCE_EXT:
        case ERL_PORT_EXT:
        case ERL_PID_EXT:
        case ERL_SMALL_TUPLE_EXT:
        case ERL_LARGE_TUPLE_EXT:
            break;
        case ERL_NIL_EXT:
            lua_pushnil(data->L);
            return 1;
        case ERL_STRING_EXT:
            return erlang_to_string(data, index, size);
        case ERL_LIST_EXT:
            return erlang_to_list(data, index);
        case ERL_BINARY_EXT:
        case ERL_SMALL_BIG_EXT:
        case ERL_LARGE_BIG_EXT:
        case ERL_NEW_FUN_EXT:
        case ERL_FUN_EXT:
        default:
            break;
    }

    return 0;
}

static int erlang_to_ulong(async_erluna_t *data, int *index)
{
    unsigned long value;
    ei_decode_ulong(data->args, index, &value);
    lua_pushinteger(data->L, value);
    return 1;
}

static int erlang_to_long(async_erluna_t *data, int *index)
{
    long value;
    ei_decode_long(data->args, index, &value);
    lua_pushinteger(data->L, value);
    return 1;
}

static int erlang_to_double(async_erluna_t *data, int *index)
{
    double value;
    ei_decode_double(data->args, index, &value);
    lua_pushnumber(data->L, value);
    return 1;
}

static int erlang_to_atom(async_erluna_t *data, int *index, int size)
{
    int boolvalue;
    if (ei_decode_boolean(data->args, index, &boolvalue) == 0) {
        lua_pushboolean(data->L, boolvalue);
        return 1;
    }

    char *atom = driver_alloc(sizeof(char) * (size + 1));

    ei_decode_atom(data->args, index, atom);
    if (strcmp(atom, "nil") == 0) {
        lua_pushnil(data->L);
    } else {
        lua_pushlstring(data->L, atom, size);
    }

    driver_free(atom);
    return 1;
}

static int erlang_to_string(async_erluna_t *data, int *index, int size)
{
    char *string = driver_alloc(sizeof(char) * (size + 1));

    ei_decode_string(data->args, index, string);
    lua_pushlstring(data->L, string, size);

    driver_free(string);
    return 1;
}

static int erlang_to_list(async_erluna_t *data, int *index)
{
    int arity;
    ei_decode_list_header(data->args, index, &arity);

    lua_newtable(data->L);

    for (int i = 1, n = 1; i <= arity; i++) {
        int type, size;
        ei_get_type(data->args, index, &type, &size);
        switch (type) {
            case ERL_SMALL_TUPLE_EXT:
            case ERL_LARGE_TUPLE_EXT:
                if (erlang_to_tuple(data, index) == 0) {
                    return 0;
                }
                break;
            default:
            {
                if (erlang_to_lua(data, index)) {
                    lua_rawseti(data->L, -2, n);
                    n++;
                } else {
                    return 0;
                }
                break;
            }
        }
    }

    return 1;
}

static int erlang_to_tuple(async_erluna_t *data, int *index)
{
    int arity;
    ei_decode_tuple_header(data->args, index, &arity);
    if (arity != 2) {
        return 0;
    }

    int type, size;

    // key
    ei_get_type(data->args, index, &type, &size);
    switch (type) {
        case ERL_SMALL_INTEGER_EXT:
        case ERL_INTEGER_EXT:
        case ERL_FLOAT_EXT:
        case ERL_ATOM_EXT:
        case ERL_NIL_EXT:
        case ERL_STRING_EXT:
            erlang_to_lua(data, index);
            break;
        default:
            return 0;
    }

    // value
    ei_get_type(data->args, index, &type, &size);
    switch (type) {
        case ERL_SMALL_INTEGER_EXT:
        case ERL_INTEGER_EXT:
        case ERL_FLOAT_EXT:
        case ERL_ATOM_EXT:
        case ERL_NIL_EXT:
        case ERL_STRING_EXT:
        case ERL_LIST_EXT:
            erlang_to_lua(data, index);
            break;
        default:
            return 0;
    }

    lua_rawset(data->L, -3);
    return 1;
}

