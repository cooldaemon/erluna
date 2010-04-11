%% @author Masahito Ikuta <cooldaemon@gmail.com> [http://d.hatena.ne.jp/cooldaemon/]
%% @copyright Masahito Ikuta 2010
%% @doc Test Include file.

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

-define(_ASSERT(Expr, Expect, Message), 
  case Expr of
    Expect -> ok;
    Other  -> ct:fail({Message, Other})
  end
).

-define(assertEqual(Expr, Expect, Message), 
  (fun () ->
    ExpectResult = Expect,
    ?_ASSERT(Expr, ExpectResult, Message)
  end)()
).

-define(assertMatch(Expr, Expect, Message), 
  (fun () ->
    ?_ASSERT(Expr, Expect, Message)
  end)()
).

