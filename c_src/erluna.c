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
static void set_result(async_erluna_t *data, ErlDrvTermData *spec, int spec_size);
static void set_atom(async_erluna_t *data, char *atom);
static void set_double(async_erluna_t *data, double number);
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

            char lua_source[sizeof(size) + 1];
            ei_decode_string(data->args, &i, lua_source);

            eval(data, lua_source);
            break;
        case COMMAND_GET_GLOBAL:
            if (type != ERL_STRING_EXT) {
                set_error(data, "Arguments type error.");
                return;
            }

            char name[sizeof(size) + 1];
            ei_decode_string(data->args, &i, name);

            get_global(data, name);
            break;
        default:
            set_error(data, "Command not found.");
            break;
    }
}

static void set_ok(async_erluna_t *data)
{
    ErlDrvTermData spec[] = {ERL_DRV_ATOM, driver_mk_atom("ok")};
    set_result(data, spec, sizeof(spec));
}

static void set_error(async_erluna_t *data, const char *message)
{
    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("error"),
        ERL_DRV_STRING, (ErlDrvTermData)message, strlen(message),
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
}

static void set_atom(async_erluna_t *data, char *atom)
{
    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("ok"),
        ERL_DRV_ATOM, driver_mk_atom(atom),
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
}

static void set_string(async_erluna_t *data, const char *string)
{
    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("ok"),
        ERL_DRV_STRING, (ErlDrvTermData)string, strlen(string),
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
}

static void set_double(async_erluna_t *data, double number)
{
    int i = 0, size;
    ei_encode_version(NULL, &i);

    char *buf;
    if ((long long)number == number) {
        ei_encode_longlong(NULL, &i, (long long)number);
        size = i;
        buf = malloc(sizeof(char) * (size + 1));
        i = 0;
        ei_encode_version(buf, &i);
        ei_encode_longlong(buf, &i, (long long)number);
    } else {
        ei_encode_double(NULL, &i, number);
        buf = malloc(sizeof(char) * (size + 1));
        i = 0;
        ei_encode_version(buf, &i);
        ei_encode_double(buf, &i, number);
    }

    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("ok"),
            ERL_DRV_ATOM, driver_mk_atom("number"),
            ERL_DRV_STRING, (ErlDrvTermData)buf, size,
            ERL_DRV_TUPLE, 2,
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
    free(buf);
}

static void set_result(async_erluna_t *data, ErlDrvTermData *spec, int spec_size)
{
    data->result = (ErlDrvTermData *)driver_alloc(spec_size);
    memcpy(data->result, spec, spec_size);
    data->result_size = spec_size / sizeof(spec[0]);
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
            set_double(data, (double)lua_tonumber(data->L, -1));
            lua_pop(data->L, 1);
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


