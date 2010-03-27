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

CC     = gcc
CFLAGS = -std=c99

ERL_ROOT = $(shell erl -noshell -eval 'io:format("~s~n", [code:root_dir()]).' -s init stop)/usr
INCLUDES += -I$(ERL_ROOT)/include
LIBS     += -L$(ERL_ROOT)/lib -lerts

EI_ROOT  = $(shell erl -noshell -eval 'io:format("~s~n", [code:lib_dir(erl_interface)]).' -s init stop)
INCLUDES += -I$(EI_ROOT)/include
LIBS     += -L$(EI_ROOT)/lib -lei -lerl_interface

OPTIONS = -fpic -O2

ifeq ($(shell uname), Darwin)
  OPTIONS   += -fno-common -bundle -undefined suppress -flat_namespace
  USR_LOCAL = /opt/local
else
  USR_LOCAL = /usr/local
endif

INCLUDES += -I$(USR_LOCAL)/include
LIBS     += -L$(USR_LOCAL)/lib -llua

TARGET  = $(ROOT)/priv/lib/erluna_drv.so
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS) $(OPTIONS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f $(TARGET) $(OBJECTS)

