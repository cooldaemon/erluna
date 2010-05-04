#include <stdio.h>
#include <string.h> # memcpy

#include "ei.h"
#include "erl_driver.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "erluna.h"
#include "erluna_driver.h"

typedef struct {
    ErlDrvPort port;
    lua_State  *L;
} erluna_t;

static ErlDrvData start(ErlDrvPort port, char *command);
static void stop(ErlDrvData handle);
static void output(ErlDrvData handle, char *buf, int len);
static void ready_async(ErlDrvData handle, ErlDrvThreadData async_handle);

static ErlDrvEntry erluna_driver_entry = {
    NULL,
    start,       /* open_port/2 */
    stop,        /* port_close/1 */
    output,      /* port_command/2 */
    NULL,
    NULL,
    "erluna_drv",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ready_async, /* driver_async via C */
    NULL,
    NULL,
    NULL
};

DRIVER_INIT(erluna_driver)
{
    return &erluna_driver_entry;
}


static ErlDrvData start(ErlDrvPort port, char *command)
{ 
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    erluna_t *data = (erluna_t *)driver_alloc(sizeof(erluna_t));
    data->port = port;
    data->L    = L;

    return (ErlDrvData)data;
}

static void stop(ErlDrvData handle)
{
    erluna_t *data = (erluna_t *)handle;
    lua_close(data->L);
    driver_free((char *)handle);
}

static void output(ErlDrvData handle, char *buf, int len)
{
    erluna_t *data = (erluna_t *)handle;

    async_erluna_t *async_data = (async_erluna_t *)driver_alloc(
        sizeof(async_erluna_t)
    );

    async_data->L      = data->L;

    async_data->args   = driver_alloc(len);
    memcpy(async_data->args, buf, len);

    async_data->result = (ei_x_buff *)driver_alloc(sizeof(ei_x_buff));
    ei_x_new_with_version(async_data->result);

    driver_async(data->port, NULL, erluna_dispatch, async_data, free);
}

static void ready_async(ErlDrvData handle, ErlDrvThreadData async_handle)
{
    erluna_t       *data       = (erluna_t *)handle;
    async_erluna_t *async_data = (async_erluna_t *)async_handle;

    driver_output(
        data->port,
        async_data->result->buff,
        async_data->result->index
    );

    ei_x_free(async_data->result);

    driver_free(async_data->result);
    driver_free(async_data->args);
    driver_free(async_data);
}

