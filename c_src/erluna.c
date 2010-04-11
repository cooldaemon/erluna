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

static void set_int64(async_erluna_t *data, long long number)
{
    long long *p_number = driver_alloc(sizeof(long long));
    data->free = (void *)p_number;
    *p_number = number;

    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("ok"),
        ERL_DRV_INT64, (ErlDrvTermData)p_number,
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
}

static void set_float(async_erluna_t *data, double number)
{
    double *p_number = driver_alloc(sizeof(double));
    data->free = (void *)p_number;
    *p_number = number;

    ErlDrvTermData spec[] = {
        ERL_DRV_ATOM, driver_mk_atom("ok"),
        ERL_DRV_FLOAT, (ErlDrvTermData)p_number,
        ERL_DRV_TUPLE, 2
    };
    set_result(data, spec, sizeof(spec));
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


