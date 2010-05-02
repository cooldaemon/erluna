## erluna
erluna is Lua bindings for Erlang.

### Building from Source
    % cd /path/to
    % git clone git://github.com/cooldaemon/erluna.git
    % cd ./erluna
    % make

### How to Use
    {ok, Lua} = erluna:start(),
    ok = Lua:eval("lua_value1 = 1 + 1"),
    {ok, 2} = Lua:get("lua_value1"),
    ok = Lua:eval("function lua_fun(x) return x * 2 end"),
    ok = Lua:eval("lua_value2 = lua_fun(5)"),
    {ok, 10} = Lua:get("lua_value2"),
    ok = Lua:set("lua_value3", [1, {foo, bar}, 3]),
    {ok, [{1, 1}, {2, 3}, {"foo", "bar"}]} = Lua:get("lua_value3"),
    Lua:stop().

