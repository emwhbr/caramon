// ************************************************************************
// *                                                                      *
// * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>

#include <sstream>

#include "cmon_io.h"
#include "cmon_product_info.h"
#include "cmon_utility.h"
#include "cmon_file.h"
#include "cmon_exception.h"
#include "cmon_core.h"
#include "delay.h"
#include "signal_support.h"
#include "gpio.h"
#include "cmon_led.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define CMON_NAME  "CMON-MAIN"

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////
static void app_terminate(void);
static bool app_parse_command_line(int argc, char *argv[]);
static void app_report_help(const char *app_name);
static void app_report_prod_info(void);
static bool app_check_switches(void);
static void app_report_switches(void);
static bool cmon_ios_initialize(void);
static bool cmon_gpio_initialize(void);
static void cmon_gpio_finalize(void);
static void cmon_signal_handler(int sig);
static bool cmon_initialize(void);
static bool cmon_finalize(void);
static bool cmon_check_ok(void);
static void cmon_delete_terminate_error_file(void);
static void cmon_update_terminate_error_file(void);
static void cmon_exit_on_error(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////
class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate(app_terminate);
  }
};
// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// Set by command line arguments
static int g_fallback = 0;
static int g_disk_no_log = 0;
static int g_net_no_log = 0;
static int g_pid_ctrl = 0;
static int g_verbose = 0;
static string g_logfile = "";
static string g_terminate_errorfile = "";

// CMON core object pointer
static cmon_core *g_cmon_core = NULL;

// Set by signal handler
static volatile sig_atomic_t g_received_sig_terminate = 0;

/////////////////////////////////////////////////////////////////////////////
//               Private functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static void app_terminate(void)
{
   // Only log this event, no further actions for now
  cmon_io_put("%s : unhandled exception, termination handler activated\n",
	      CMON_NAME);

  // The terminate function should not return
  abort();
}

///////////////////////////////////////////////////////////////

static bool app_parse_command_line(int argc, char *argv[])
{
  bool command_line_ok = true;

  int option_index = 0;
  int c;
  struct option long_options[] = {
    {"disk-no-log",     no_argument,       0, 'd'},
    {"fallback",        no_argument,       0, 'f'},
    {"help",            no_argument,       0, 'h'},
    {"log",             required_argument, 0, 'l'},
    {"net-no-log",      no_argument,       0, 'n'},
    {"pid-ctrl",        no_argument,       0, 'p'},
    {"terminate-error", required_argument, 0, 't'},
    {"verbose",         no_argument,       0, 'v'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  // At least one argument must be specified
  if (argc == 1) {
    return false;
  }

  while (1) {
    c = getopt_long(argc, argv, "dfhl:npt:vV",
		    long_options, &option_index);
    
    // Detect the end of the options
    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      // If option sets a flag, do nothing else now
      if (long_options[option_index].flag) {
	break;
      }
      break;

    case 'd':
      g_disk_no_log = 1;
      break;

    case 'f':
      g_fallback = 1;
      break;

    case 'h':
      app_report_help(basename(argv[0]));
      exit(EXIT_SUCCESS);
      break;

    case 'l':
      g_logfile = optarg;
      break;

    case 'n':
      g_net_no_log = 1;
      break;

    case 'p':
      g_pid_ctrl = 1;
      break;

    case 't':
      g_terminate_errorfile = optarg;
      break;

    case 'v':
      g_verbose = 1;
      break;

    case 'V':
      app_report_prod_info();
      exit(EXIT_SUCCESS);
      break;

    default:
      command_line_ok = false;
      break;
    }
  }

  return command_line_ok;
}

////////////////////////////////////////////////////////////////

static void app_report_help(const char *app_name)
{
  ostringstream oss_msg;

  oss_msg << app_name << " [options]" << "\n\n"

	  << "options\n\n"

	  << "-d, --disk-no-log\n"
	  << "    Disable data logging to local disk.\n\n"

	  << "-f, --fallback\n"
	  << "    Fallback mode with data logging disabled and minimum functionality.\n"
	  << "    Temperature is controlled by fixed rate PWM with no feedback.\n\n"

	  << "-h, --help\n"
	  << "    Print help information and exit.\n\n"

	  << "-l, --log <full-path-to-logfile>\n"
	  << "    Logfile for progress information.\n\n"

	  << "-n, --net-no-log\n"
	  << "    Disable data logging to network.\n\n"

	  << "-p, --pid-ctrl\n"
	  << "    Enable PID temperature controller.\n\n"

	  << "-t, --terminate-error <full-path-to-error-file>\n"
	  << "    File for counting bad terminations.\n\n"

	  << "-v, --verbose\n"
	  << "    Be verbose and print detailed information.\n\n"
    
	  << "-V, --version\n"
	  << "    Print product information and exit.\n";
  
  printf(oss_msg.str().c_str());
}

////////////////////////////////////////////////////////////////

static void app_report_prod_info(void)
{
  if (!g_fallback) {
    cmon_io_put("CMON: %s-%s\n",
		CMON_PRODUCT_NUMBER,
		CMON_RSTATE);
  }
  else {
    cmon_io_put("CMON(fallback): %s-%s\n",
		CMON_PRODUCT_NUMBER,
		CMON_RSTATE);
  }
}

////////////////////////////////////////////////////////////////

static bool app_check_switches(void)
{
  return true;
}

////////////////////////////////////////////////////////////////

static void app_report_switches(void)
{
  ostringstream oss_msg;

  oss_msg << "Command line switches:\n";
  oss_msg << "\tfallback        : " << (g_fallback ? "on" : "off") << "\n";
  oss_msg << "\tdisk-no-log     : " << (g_disk_no_log ? "on" : "off") << "\n";
  oss_msg << "\tlogfile         : " << g_logfile << "\n";
  oss_msg << "\tnet-no-log      : " << (g_net_no_log ? "on" : "off") << "\n";
  oss_msg << "\tpid-ctrl        : " << (g_pid_ctrl ? "on" : "off") << "\n";
  oss_msg << "\tterminate-error : " << g_terminate_errorfile << "\n";
  oss_msg << "\tverbose         : " << (g_verbose ? "on" : "off");

  cmon_io_put("%s\n", oss_msg.str().c_str());
}

////////////////////////////////////////////////////////////////

static bool cmon_ios_initialize(void)
{
  // Initialize I/O singleton object
  try {
    cmon_io_initialize(g_logfile);
    return true;
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("Initialize CMON I/O, exception:\n%s\n",
		gxp.get_details().c_str());
    return false;
  }
  catch (...) {
    cmon_io_put("Initialize CMON I/O, unexpected exception\n");
    return false;
  }
}

////////////////////////////////////////////////////////////////

static bool cmon_gpio_initialize(void)
{
  long rc;

  // Initialize GPIO framework common for entire application
  try {
    rc = gpio_initialize();
    if (rc != GPIO_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
		"Initialize GPIO failed", NULL);
    }
    cmon_led_initialize();
    cmon_led(CMON_LED_ALIVE,   false);   // Turn status LED off
    cmon_led(CMON_LED_SYSFAIL, false);   // Turn status LED off

    return true;
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("Initialize CMON GPIO, exception:\n%s\n",
		gxp.get_details().c_str());
    return false;
  }
  catch (...) {
    cmon_io_put("Initialize CMON GPIO, unexpected exception\n");
    return false;
  }
}

////////////////////////////////////////////////////////////////

static void cmon_gpio_finalize(void)
{
  try {
    cmon_led_finalize();
    gpio_finalize();
  }
  catch (...) {
    cmon_io_put("Finalize CMON GPIO, unexpected exception\n");
  }
}

////////////////////////////////////////////////////////////////

static void cmon_signal_handler(int sig)
{
  switch (sig) {
  case CMON_SIG_TERMINATE:
    g_received_sig_terminate = 1;
    break;
  default:
    ;
  }
}

////////////////////////////////////////////////////////////////

static bool cmon_initialize(void)
{
  long rc;

  // Create the CMON core object
  g_cmon_core = new cmon_core(g_fallback == 1,
			      g_disk_no_log == 1,
                              g_net_no_log == 1,
			      g_pid_ctrl == 1,
                              g_verbose == 1);
  
  // Initialize and start CMON
  try {
    // Mask signals to explicitly wait for in this application.
    // All threads inherit the signal mask from their creator.
    // The semantics of sigwait require that all threads (including
    // the thread calling sigwait) have the signal masked, for
    // reliable operation. Otherwise, a signal that arrives
    // while the sigwaiter is not blocked in sigwait might be
    // delivered to another thread.
    // JOE: No signals here yet.....

    // Install signal handler for CMON termination
    rc = sig_sup_define_signal_handler(CMON_SIG_TERMINATE,
				       cmon_signal_handler);
    if (rc != SIGNAL_SUPPORT_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_SIGNAL_OPERATION_FAILED,
		"Install signal handler failed", NULL);
    }

    // Initialize the CMON core object
    g_cmon_core->initialize();
    return true;
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("Initialize CMON, exception:\n%s\n",
		gxp.get_details().c_str());
    return false;
  }
  catch (...) {
    cmon_io_put("Initialize CMON, unexpected exception\n");
    return false;
  }
}

////////////////////////////////////////////////////////////////

static bool cmon_finalize(void)
{
  try {
    g_cmon_core->finalize();  // Finalize CMON core object
    return true;
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("Finalize CMON, exception:\n%s\n",
		gxp.get_details().c_str());
    return false;
  }
  catch (...) {
    cmon_io_put("Finalize CMON, unexpected exception\n");
    return false;
  }
}

////////////////////////////////////////////////////////////////

static bool cmon_check_ok(void)
{
  try {
    g_cmon_core->check_ok();
    return true;
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("Check CMON, exception:\n%s\n",
		gxp.get_details().c_str());
    return false;
  }
  catch (...) {
    cmon_io_put("Check CMON, unexpected exception\n");
    return false;
  }
}

////////////////////////////////////////////////////////////////

static void cmon_delete_terminate_error_file(void)
{
  try {
    if (!g_terminate_errorfile.empty()) {
      cmon_file error_file(g_terminate_errorfile);
      if (error_file.exists()) {
	cmon_delete_file(g_terminate_errorfile.c_str());
      }
    }
  }
  catch (...) {
    cmon_io_put("Delete CMON terminate error file, unexpected exception\n");
  }
}

////////////////////////////////////////////////////////////////

static void cmon_update_terminate_error_file(void)
{
  try {
    if (!g_terminate_errorfile.empty()) {
      cmon_file error_file(g_terminate_errorfile);
      char buffer[64];
      int terminate_errors;

      if (error_file.exists()) {
	// Get current number of terminate errors from file
	error_file.open_file(CMON_FILE_READONLY);
	error_file.read_file((uint8_t *)buffer, sizeof(buffer), false);
	terminate_errors = atoi(buffer);
	error_file.close_file();
	
	// Update terminate errors
	error_file.open_file(CMON_FILE_WRITEONLY);
	sprintf(buffer, "%d\n", ++terminate_errors);
	error_file.write_file((const uint8_t *)buffer, strlen(buffer));
	error_file.close_file();
      }
      else {
	// Create initial terminate errors
	error_file.open_file(CMON_FILE_WRITEONLY);
	sprintf(buffer, "%d\n", 1);
	error_file.write_file((const uint8_t *)buffer, strlen(buffer));
	error_file.close_file();
      }      
    }
  }
  catch (...) {
    cmon_io_put("Update CMON terminate error file, unexpected exception\n");
  }
}

////////////////////////////////////////////////////////////////

static void cmon_exit_on_error(void)
{  
  delete g_cmon_core;
  try {
    cmon_update_terminate_error_file(); // Log this occurrence of bad termination
    cmon_led(CMON_LED_SYSFAIL, true);   // Turn status LED on
    cmon_gpio_finalize();               // Finalize GPIO framework
  }
  catch (...) {}

  cmon_io_put("%s : terminated bad\n", CMON_NAME);
  exit(EXIT_FAILURE);
}

/////////////////////////////////////////////////////////////////////////////
//               Main application
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  // Parse command line arguments
  if (!app_parse_command_line(argc, argv)) {
    app_report_help(basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  // Check command line arguments
  if (!app_check_switches()) {
    app_report_help(basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  // Initialize I/O singleton object
  if (!cmon_ios_initialize()) {
    cmon_exit_on_error();
  } 

  // Report product information
  app_report_prod_info(); 
  if (g_verbose) {    
    app_report_switches();
  }

  // Initialize GPIO framework
  if (!cmon_gpio_initialize()) {
    cmon_exit_on_error();
  } 
  
  // Initialize CMON
  if (!cmon_initialize()) {
    cmon_finalize();
    cmon_exit_on_error();
  }

  /////////////////////////////////////////
  // Main supervision and control loop
  /////////////////////////////////////////
  for (;;) {
    // Check if time to quit
    if (g_received_sig_terminate) {
      cmon_io_put("%s : got signal to terminate\n", CMON_NAME);
      g_received_sig_terminate = 0;
      if (!cmon_finalize()) {
	cmon_exit_on_error();
      }
      break;
    }
    
    // Check CMON
    if (!cmon_check_ok()) {
      cmon_io_put("%s : status not OK, terminating\n", CMON_NAME);
      cmon_finalize();
      cmon_exit_on_error();
    }
    
    // Take it easy
    delay(0.1);
  }
  
  // Cleanup and exit
  delete g_cmon_core;
  cmon_delete_terminate_error_file(); // Reset any previous errors on OK termination
  cmon_gpio_finalize();               // Finalize GPIO framework

  cmon_io_put("%s : terminated ok\n", CMON_NAME); 
  exit(EXIT_SUCCESS);
}
