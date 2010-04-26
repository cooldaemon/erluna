//#include <stdio.h>
//#include <string.h> // strlen
//#include <unistd.h> // sleep

#include <ei.h>
#include <erl_driver.h>

#include <lua.h>
#include <lauxlib.h>

#include "erluna.h"
#include "erluna_driver.h"
#include "erluna_term.h"

#define COMMAND_EVAL       1
#define COMMAND_EVAL_FILE  2
#define COMMAND_APPLY      3
#define COMMAND_GET_GLOBAL 4

static void set_error(async_erluna_t *data, const char *message);

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
        {
            if (type != ERL_STRING_EXT) {
                set_error(data, "Arguments type error.");
                return;
            }

            char *lua_source = driver_alloc(sizeof(char) * (size + 1));
            ei_decode_string(data->args, &i, lua_source);
            eval(data, lua_source);
            driver_free(lua_source);
            break;
        }
        case COMMAND_GET_GLOBAL:
        {
            if (type != ERL_STRING_EXT) {
                set_error(data, "Arguments type error.");
                return;
            }

            char *name = driver_alloc(sizeof(char) * (size + 1));
            ei_decode_string(data->args, &i, name);
            get_global(data, name);
            driver_free(name);
            break;
        }
        default:
            set_error(data, "Command not found.");
            break;
    }
}

static void set_error(async_erluna_t *data, const char *message)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "error");
    ei_x_encode_string(data->result, message);
}

static void eval(async_erluna_t *data, const char *lua_source)
{
    if (!luaL_dostring(data->L, lua_source)) {
        ei_x_encode_atom(data->result, "ok");
        return;
    }

    set_error(data, lua_tostring(data->L, -1));
    lua_pop(data->L, 1);
}

static void get_global(async_erluna_t *data, const char *name)
{
    ei_x_encode_tuple_header(data->result, 2);
    ei_x_encode_atom(data->result, "ok");
    lua_getglobal(data->L, name);
    lua_to_erlang(data, -1);
    lua_pop(data->L, 1);
}

