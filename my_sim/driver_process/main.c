#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "interprocess.h"

static void int_handler(int signum) {
  destroy_shm();
  exit(-1);
}

int main(int argc, char** argv) {
  create_shm(0, argv[1]);

  // Install Signal Handler:
  struct sigaction sa;
  sa.sa_handler = int_handler;
  sigaction(SIGINT, &sa, NULL);
  
  set_driver_signals(10, 10, 0);
  get_voltage();
  set_driver_signals(2, 40, 0);
  get_voltage();
  set_driver_signals(1000, 15, 0);
  get_voltage();
  set_driver_signals(3, 256, 1);
  get_voltage();

  destroy_shm();
  return 0;
}
