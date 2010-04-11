%% @author Masahito Ikuta <cooldaemon@gmail.com> [http://d.hatena.ne.jp/cooldaemon/]
%% @copyright Masahito Ikuta 2010
%% @doc erluna is Lua bindings for Erlang.
%%
%%  erluna is Lua bindings for Erlang.
%%
%%  Here's a quick example illustrating how to use erluna:
%%  ```
%%    {ok, Lua} = erluna:start(),
%%
%%    Lua:eval("lua_value1 = 1 + 1"),
%%    {ok, 2} = Lua:get_global("lua_value1"),
%%
%%    Lua:eval("function lua_fun(x) return x * 2 end"),
%%    Lua:eval("lua_value2 = lua_fun(5)"),
%%    {ok, 10} = Lua:get_global("lua_value2"),
%%
%%    Lua:stop().
%%  '''

%% Copyright 2010 Masahito Ikuta
%%
%% Licensed under the Apache License, Version 2.0 (the "License");
%% you may not use this file except in compliance with the License.
%% You may obtain a copy of the License at
%%
%% http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing, software
%% distributed under the License is distributed on an "AS IS" BASIS,
%% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
%% See the License for the specific language governing permissions and
%% limitations under the License.

-module(erluna).

-export([start/0, stop/1]).
-export([eval/2, async_eval/2]).
%-export([eval_file/2, async_eval_file/2]).
%-export([apply/3]).
-export([get_global/2, async_get_global/2]).

-include("erluna.hrl").

start() ->
  case erl_ddll:load_driver(driver_dir(), ?DRIVER_NAME) of
    ok                      -> open();
    {error, already_loaded} -> open();
    Error                   -> Error
  end.

driver_dir() ->
  Path = lists:map(
    fun (Path) -> filename:join([Path, "..", "priv", "lib"]) end,
    code:get_path()
  ),
  case file:path_open(Path, ?DRIVER_NAME ++ ".so", [read]) of
    {ok, IoDevice, FullName} ->
      file:close(IoDevice),
      filename:dirname(FullName);
    _Other ->
      "./"
  end.

open() ->
  {ok, #erluna{port = open_port({spawn, ?DRIVER_NAME}, [])}}.

stop(Lua) ->
  erlang:port_close(Lua#erluna.port),
  ok.

eval(Source, Lua) ->
  async_eval(Source, Lua),
  receive Result -> Result end.

async_eval(Source, Lua) ->
  erlang:port_command(
    Lua#erluna.port,
    term_to_binary({?COMMAND_EVAL, Source})
  ).

get_global(Name, Lua) ->
  async_get_global(Name, Lua),
  receive Result -> Result end.

async_get_global(Name, Lua) ->
  erlang:port_command(
    Lua#erluna.port,
    term_to_binary({?COMMAND_GET_GLOBAL, Name})
  ).

