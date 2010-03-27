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

include vsn.mk

ifndef ROOT
	ROOT=$(shell pwd)
endif

COMMON_TEST=$(shell erl -noshell -eval 'io:format("~s~n", [code:lib_dir(common_test)]).' -s init stop)
RUN_TEST_CMD=$(COMMON_TEST)/priv/bin/run_test

all: build

build: so beam

so:
	cd c_src && ROOT=$(ROOT) make

beam:
	cd src && ROOT=$(ROOT) make

test: test_compile
	${RUN_TEST_CMD} -dir . \
		-logdir test/log -cover test/erluna.coverspec \
		-I$(ROOT)/include \
		-pa $(ROOT)/ebin

test_compile: build
	cd test && ROOT=$(ROOT) COMMON_TEST=$(COMMON_TEST) make

docs:
	erl -noshell -run edoc_run application "'erluna'" \
		'"."' '[{def,{vsn, "$(ERLUNA_VSN)"}}]'

clean:	
	rm -rf erl_crash.dump doc/*
	cd c_src; ROOT=$(ROOT) make clean
	cd src; ROOT=$(ROOT) make clean
	cd test; ROOT=$(ROOT) make clean

