#include <stdio.h>
#include <string.h> // strlen
//#include <unistd.h> // sleep

#include <ei.h>
#include <erl_driver.h>

#include <lua.h>
#include <lauxlib.h>

#include "erluna.h"
#include "erluna_driver.h"

#define COMMAND_EVAL       1
#define COMMAND_EVAL_FILE  2
#define COMMAND_APPLY      3
#define COMMAND_GET_GLOBAL 4

static void set_ok(async_erluna_t *data);
static void set_error(async_erluna_t *data, const char *message);
static void set_atom(async_erluna_t *data, const char *atom);
static void set_int64(async_erluna_t *data, long long number);
static void set_float(async_erluna_t *data, double number);
static void set_string(async_erluna_t *data, const char *string);

static void eval(async_erluna_t *data, const char *lua_source);
static void get_global(async_erluna_t *data, const char *name);

void erluna_dispatch(void *async_handle)
{
    async_erluna_t *data = (async_erluna_t *)async_handle;

    int i = 0, ver;
    if (0 != ei_decode_version(data->args, &i, &ver)) {
        set_error(data, "Can't decode arguments.");
        return;
    }

    int type, size;
    ei_get_type(data->args, &i, &type, &size);
    if (type != ERL_SMALL_TUPLE_EXT) {
        set_error(data, "Arguments type error.");
        return;
    }

    int arity;
    ei_decode_tuple_header(data->args, &i, &arity);

    ei_get_type(data->args, &i, &type, &size);
    if (type != ERL_SMALL_INTEGER_EXT) {
        set_error(data, "Arguments type error.");
        return;
    }

    char command;
    ei_decode_char(data->args, &i, &command);

    ei_get_type(data->args, &i, &type, &size);

    switch (command) {
        case COMMAND_EVAL:
            if (type != ERL_STRING_EXT) {
                set_error(data, "Arguments type error.");
                return;
            }

            char *lua_source = driver_alloc(sizeof(char) * (size + 1));
            ei_decode_string(data->args, &i, lua_source);
            eval(data, lua_source);
            driver_free(lua_source);
            break;
        case COMMAND_GET_GLOBAL:
            if (type != ERL_STRING_EXT) {
                set_error(data, "Arguments type error.");
                return;
            }

            char *name = driver_alloc(sizeof(char) * (size + 1));
            ei_decode_string(data->args, &i, name);
            get_global(data, name);
            driver_free(name);
            break;
        default:
            set_error(data, "Command not found.");
            break;
    }
}

static void set_ok(async_erluna_t *data)
{
    ei_x_encode_atom(data->result, "ok");
}

static void set_error(async_erluna_t *data, const char *message)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "error");
    ei_x_encode_string(data->result, message);
}

static void set_atom(async_erluna_t *data, const char *atom)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "ok");
    ei_x_encode_atom(data->result, atom);
}

static void set_string(async_erluna_t *data, const char *string)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "ok");
    ei_x_encode_string(data->result, string);
}

static void set_int64(async_erluna_t *data, long long number)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "ok");
    ei_x_encode_longlong(data->result, number);
}

static void set_float(async_erluna_t *data, double number)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "ok");
    ei_x_encode_double(data->result, number);
}

static void eval(async_erluna_t *data, const char *lua_source)
{
//    printf("debug %s", lua_source);
//    fflush(stdout);

    if (!luaL_dostring(data->L, lua_source)) {
        set_ok(data);
        return;
    }

    set_error(data, lua_tostring(data->L, -1));
    lua_pop(data->L, 1);
}

static void get_global(async_erluna_t *data, const char *name)
{
    lua_getglobal(data->L, name);

    double number;
    switch (lua_type(data->L, -1)) {
        case LUA_TNIL:
            lua_pop(data->L, 1);
            set_atom(data, "nil");
            break;
        case LUA_TBOOLEAN:
            if(lua_toboolean(data->L, -1)) {
                set_atom(data, "true");
            } else {
                set_atom(data, "false");
            }
            lua_pop(data->L, 1);
            break;
        case LUA_TNUMBER:
            number = (double)lua_tonumber(data->L, -1);
            lua_pop(data->L, 1);
            if ((long long)number == number) {
                set_int64(data, (long long)number);
            } else {
                set_float(data, number);
            }
            break;
        case LUA_TSTRING:
            set_string(data, lua_tostring(data->L, -1));
            lua_pop(data->L, 1);
            break;
        default:
            set_error(data, "Unknown type.");
            break;
    }
}

