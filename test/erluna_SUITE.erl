%% @author Masahito Ikuta <cooldaemon@gmail.com> [http://d.hatena.ne.jp/cooldaemon/]
%% @copyright Masahito Ikuta 2010
%% @doc This module is test suite for data store manager.

%% Copyright 2008 Masahito Ikuta
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

-module(erluna_SUITE).
-compile(export_all).

-include("ct.hrl").
-include("erluna_test.hrl").

all() -> [test_set, test_eval].

test_set() -> [].
test_set(_Conf) ->
  {ok, Lua} = erluna:start(),
  
  ?assertMatch(Lua:set("lua_value1", 1), ok, set_number),
  ?assertMatch(Lua:get("lua_value1"), {ok, 1}, get_number),

  ?assertMatch(Lua:set("lua_value2", 0.3), ok, set_float),
  ?assertMatch(Lua:get("lua_value2"), {ok, 0.3}, get_float),

  ?assertMatch(Lua:set("lua_value3", foo), ok, set_atom),
  ?assertMatch(Lua:get("lua_value3"), {ok, "foo"}, get_atom),

  ?assertMatch(Lua:set("lua_value4", "bar"), ok, set_string),
  ?assertMatch(Lua:get("lua_value4"), {ok, "bar"}, get_string),

  ?assertMatch(Lua:set("lua_value5", [1, {foo, bar}, 3]), ok, set_list1),
  ?assertMatch(
    Lua:get("lua_value5"),
    {ok, [{1, 1}, {2, 3}, {"foo", "bar"}]},
    get_list1
  ),

  ?assertMatch(Lua:set("lua_value6", [{foo, [{bar, baz}]}]), ok, set_list2),
  ?assertMatch(
    Lua:get("lua_value6"),
    {ok, [{"foo", [{"bar", "baz"}]}]},
    get_list2
  ),

  ?assertMatch(Lua:stop(), ok, stop),
  ok.

test_eval() -> [].
test_eval(_Conf) ->
  {ok, Lua} = erluna:start(),
  
  ?assertMatch(Lua:eval("lua_value1 = 1 + 1"), ok, eval_number),
  ?assertMatch(Lua:get("lua_value1"), {ok, 2}, get_number),

  ?assertMatch(Lua:eval("lua_value2 = 1.1 + 2.2"), ok, eval_float),
  ?assertMatch(Lua:get("lua_value2"), {ok, 3.3000000000000003}, get_float),

  ?assertMatch(Lua:eval("lua_value3 = [[foo]]"), ok, eval_string),
  ?assertMatch(Lua:get("lua_value3"), {ok, "foo"}, set_string),

  ?assertMatch(Lua:eval("function lua_fun(x) return x * 2 end"), ok, eval_function1),
  ?assertMatch(Lua:eval("lua_value4 = lua_fun(5)"), ok, eval_function2),
  ?assertMatch(Lua:get("lua_value4"), {ok, 10}, get_function),

  ?assertMatch(Lua:stop(), ok, stop),
  ok.

