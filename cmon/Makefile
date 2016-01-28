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

include ./common_defs.mk

# ------ Application

APP_OBJS = $(OBJ_DIR)/cmon_alive.o \
           $(OBJ_DIR)/cmon_climate_data.o \
           $(OBJ_DIR)/cmon_climate_data_queue.o \
           $(OBJ_DIR)/cmon_climate_logger.o \
           $(OBJ_DIR)/cmon_climate_sampler.o \
           $(OBJ_DIR)/cmon_controller_data.o \
           $(OBJ_DIR)/cmon_controller_data_queue.o \
           $(OBJ_DIR)/cmon_core.o \
           $(OBJ_DIR)/cmon_exception.o \
           $(OBJ_DIR)/cmon_ext_sensor.o \
           $(OBJ_DIR)/cmon_fallback.o \
           $(OBJ_DIR)/cmon_file.o \
           $(OBJ_DIR)/cmon_int_sensor.o \
           $(OBJ_DIR)/cmon_io.o \
           $(OBJ_DIR)/cmon_led.o \
           $(OBJ_DIR)/cmon_log.o \
           $(OBJ_DIR)/cmon_log_disk.o \
           $(OBJ_DIR)/cmon_log_net.o \
           $(OBJ_DIR)/cmon_main.o \
           $(OBJ_DIR)/cmon_power_switch.o \
           $(OBJ_DIR)/cmon_power_switch_rc.o \
           $(OBJ_DIR)/cmon_temp_controller.o \
           $(OBJ_DIR)/cmon_thread_utility.o \
           $(OBJ_DIR)/cmon_utility.o \
           $(OBJ_DIR)/delay.o \
           $(OBJ_DIR)/ds18b20_io.o \
           $(OBJ_DIR)/gpio.o \
           $(OBJ_DIR)/hdc1008_io.o \
           $(OBJ_DIR)/io_pin.o \
           $(OBJ_DIR)/io_pin_in.o \
           $(OBJ_DIR)/io_pin_out.o \
           $(OBJ_DIR)/shell_cmd.o \
           $(OBJ_DIR)/pid_ctrl.o \
           $(OBJ_DIR)/signal_support.o \
           $(OBJ_DIR)/socket_support.o \
           $(OBJ_DIR)/thread.o \
           $(OBJ_DIR)/timer.o

APP_BASENAME = $(OBJ_DIR)/cmon
APP_NAME = $(APP_BASENAME)_$(KIND).$(ARCH_TYPE)

# ------ Targets
.PHONY : clean help

-include $(APP_OBJS:.o=.d)

app : $(APP_OBJS)
	$(CC) -o $(APP_NAME) $(APP_OBJS) $(LIB_DIRS) $(LIBS)

all : app

clean :
	rm -f $(APP_OBJS)
	rm -f $(APP_OBJS:.o=.d)
	rm -f $(OBJ_DIR)/*.$(ARCH_TYPE)
	rm -f $(SRC_DIR)/*~
	rm -rf *~

help:
	@echo "Usage: make clean"
	@echo "       make app"
	@echo "       make all"
