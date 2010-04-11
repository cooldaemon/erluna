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

all() -> [testcase1].

testcase1() -> [].
testcase1(_Conf) ->
  {ok, Lua} = erluna:start(),
  
  ?assertMatch(Lua:eval("lua_value1 = 1 + 1"), ok, case1),
  ?assertMatch(Lua:get_global("lua_value1"), {ok, 2}, case2),

  ?assertMatch(Lua:eval("lua_value2 = 1.1 + 2.2"), ok, case3),
  ?assertMatch(Lua:get_global("lua_value2"), {ok, 3.3000000000000003}, case4),

  ?assertMatch(Lua:eval("lua_value3 = [[foo]]"), ok, case5),
  ?assertMatch(Lua:get_global("lua_value3"), {ok, "foo"}, case6),

  ?assertMatch(Lua:eval("function lua_fun(x) return x * 2 end"), ok, case7),
  ?assertMatch(Lua:eval("lua_value4 = lua_fun(5)"), ok, case8),
  ?assertMatch(Lua:get_global("lua_value4"), {ok, 10}, case9),

  ?assertMatch(Lua:stop(), ok, case10),
  ok.

