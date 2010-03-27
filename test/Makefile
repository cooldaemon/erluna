## Licensed under the Apache License, Version 2.0 (the "License"); you may not
## use this file except in compliance with the License.  You may obtain a copy
## of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
## License for the specific language governing permissions and limitations
## under the License.

TEST_SERVER=$(shell erl -noshell -eval 'io:format("~s~n", [code:lib_dir(test_server)]).' -s init stop)

EMULATOR = beam
INCLUDES = -I$(ROOT)/include -I$(TEST_SERVER)/include -I$(COMMON_TEST)/include
SOURCES  = $(wildcard *.erl)
OBJECTS  = $(SOURCES:%.erl=%.$(EMULATOR))

COMPILE_FLAGS += \
	+warn_unused_vars +nowarn_shadow_vars +warn_unused_import

%.$(EMULATOR): %.erl
	erlc -W $(COMPILE_FLAGS) $(INCLUDES) $<

all: $(OBJECTS)

clean:	
	rm -rf *.$(EMULATOR) erl_crash.dump log/*

