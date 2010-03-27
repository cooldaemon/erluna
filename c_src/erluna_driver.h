#ifndef ERLUNA_DRIVER_H
#define ERLUNA_DRIVER_H

typedef struct {
    lua_State      *L;
    char           *args;
    ErlDrvTermData *result;
    int            result_size;
} async_erluna_t;

#endif /* ERLUNA_DRIVER_H */
