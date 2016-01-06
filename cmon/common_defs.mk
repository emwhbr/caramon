# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

# ----- Toolchain setup

ARCH_TYPE = arm
CFLAGS_ARCH_TUNING=
TC_PREFIX=arm-unknown-linux-gnueabihf-

AR  = $(TC_PREFIX)ar
CC  = $(TC_PREFIX)gcc
CPP = $(TC_PREFIX)g++
AS  = $(TC_PREFIX)gcc
LD  = $(TC_PREFIX)gcc

# ----- Bonden naive setup

ifeq "$(BUILD_TYPE)" "RELEASE"
        OPTIMIZE = -O3
        KIND = rel
else 
        OPTIMIZE = -O0 -g3
        KIND = dbg
        DEBUG_PRINTS = -DDEBUG_PRINTS
endif

OBJ_DIR = ./obj
SRC_DIR = ./src

# ----- Compiler flags

CFLAGS = -Wall -Wextra -Werror -Dlinux -Wno-packed-bitfield-compat
CFLAGS += $(CFLAGS_ARCH_TUNING)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEBUG_PRINTS)

COMP_FLAGS = $(CFLAGS) -c
COMP_FLAGS_C = $(COMP_FLAGS) -std=c99
COMP_FLAGS_CPP = $(COMP_FLAGS) -std=c++98 -pedantic -Wno-long-long -Wno-variadic-macros

# ----- Includes

APP_INCLUDE = -I$(SRC_DIR)

INCLUDES = $(APP_INCLUDE)

# ----- Linker paths

LIB_DIRS =

# ----- Linker libraries

LIBSX = -lstdc++ -lgcc -lpthread -lrt

LIBS =  $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h .d

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS_C) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : %.c
	@$(CC) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_C) $(INCLUDES) $< > $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CPP) $(COMP_FLAGS_CPP) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : $(SRC_DIR)/%.cpp
	@$(CPP) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_CPP) $(INCLUDES) $< > $@
