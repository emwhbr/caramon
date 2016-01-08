/*
 Usage: tx433 <systemCode> <unitCode> <command>
 Command is 0 for OFF and 1 for ON
 */

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    
  /*
    output PIN is hardcoded for testing purposes
    see https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    for pin mapping of the raspberry pi GPIO connector
  */
  
  // EMWHBR: wiringPi pin 5 is BCM_GPIO 24, P1-18
  int PIN = 5;

  /**
   * Switch a remote switch on (Type B with two rotary/sliding switches)
   *
   * @param nAddressCode  Number of the switch group (1..4)
   * @param nChannelCode  Number of the switch itself (1..4)
 */

  if (argc != 4) {
    printf("Usage: tx433 <systemCode> <unitCode> <1|0>\n");
    exit(EXIT_FAILURE);
  }

  int systemCode = atoi(argv[1]);
  int unitCode   = atoi(argv[2]);
  int command    = atoi(argv[3]);
  
  if (wiringPiSetup () == -1) {
    exit(EXIT_FAILURE);
  }
  
  RCSwitch mySwitch = RCSwitch();
  mySwitch.enableTransmit(PIN);
  
  switch(command) {
  case 1:
    mySwitch.switchOn(systemCode, unitCode);
    break;
  case 0:
    mySwitch.switchOff(systemCode, unitCode);
    break;
  default:
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
