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

enum COMMAND {
    EVAL,
    EVAL_FILE,
    APPLY,
    GET,
    SET
};

static void set_error(async_erluna_t *data, const char *message);

static void eval(async_erluna_t *data, const char *lua_source);
static void eval_file(async_erluna_t *data, const char *lua_source);
static void get_global(async_erluna_t *data, const char *name);
static void set_global(async_erluna_t *data, int *i, const char *name);

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
    if (type != ERL_STRING_EXT) {
        set_error(data, "Arguments type error.");
        return;
    }

    char *first_arg = driver_alloc(sizeof(char) * (size + 1));
    ei_decode_string(data->args, &i, first_arg);

    switch (command) {
        case EVAL:
            eval(data, first_arg);
            break;
        case EVAL_FILE:
            eval_file(data, first_arg);
            break;
        case GET:
            get_global(data, first_arg);
            break;
        case SET:
            set_global(data, &i, first_arg);
            break;
        default:
            set_error(data, "Command not found.");
            break;
    }

    driver_free(first_arg);
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

static void eval_file(async_erluna_t *data, const char *lua_filepath)
{
    if (!luaL_dofile(data->L, lua_filepath)) {
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

static void set_global(async_erluna_t *data, int *index, const char *name)
{
    if (erlang_to_lua(data, index)) {
        lua_setglobal(data->L, name);
        ei_x_encode_atom(data->result, "ok");
    } else {
        lua_settop(data->L, 0); // clean up
        set_error(data, "Can't set value.");
    }
}

