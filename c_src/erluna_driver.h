#ifndef ERLUNA_DRIVER_H
#define ERLUNA_DRIVER_H

typedef struct {
    lua_State      *L;
    char           *args;
    ei_x_buff      *result;
} async_erluna_t;

#endif /* ERLUNA_DRIVER_H */
