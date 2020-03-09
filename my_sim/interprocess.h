#ifndef INTERPROCESS_H
#define INTERPROCESS_H
#include <fcntl.h> 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#define NEW_DATA    1
#define NO_NEW_DATA 0    // Also doubles as the data was already consumed

#define SHM_LENGTH  sizeof(mapped)
//#define SHM_NAME    "shm_cadence"

#define INIT        1
#define NINIT       0

typedef struct {
  uint32_t v_set;               // the next voltage setpoint
  uint32_t curr_r_load;         // the current load of the system
  uint32_t sim_over;            // Terminate Simulation
} v_incoming_signals;

typedef struct {
  uint32_t curr_v;      // Current Clock Count
  uint32_t sim_done;     // Terminate Simulation
} p_incoming_signals;

typedef struct {
  sem_t sem;
  uint8_t new_data;
  v_incoming_signals data;
} p_to_v;

typedef struct {
  sem_t sem;
  uint8_t new_data;
  p_incoming_signals data;
} v_to_p;

typedef struct {
// Process to Verilog Sim
  p_to_v pv;
// Verilog Sim to Process
  v_to_p vp;
} mapped;

// shm_create
//   Creates, mmaps and initializes data/mutexes in shared mem region if INIT.
//   Opens shm and mmaps if !INIT
// Returns:
//   ptr to shared mem struct on success
//   NULL on failure
int create_shm(int process, char* name);

// shm_destroy
//   Unmaps and Unlinks shared memory region
// Input:
//   ptr to shared mem region
void destroy_shm();

void wait_driver_data();
int get_voltage_setpoint();
int get_effective_resistance();
uint32_t get_terminate_simulation();
void ack_driver_data();
int send_voltage(int voltage);

void set_driver_signals(uint32_t voltage_setpoint, uint32_t resistance, uint32_t terminate_sim);
uint32_t get_voltage();

#ifdef WITH_VPI
void register_create_shm();
void register_destroy_shm();
void register_wait_driver_data();
void register_get_voltage_setpoint();
void register_get_effective_resistance();
void register_get_terminate_simulation();
void register_ack_driver_data();
void register_send_powersupply_stats();
#endif // WITH_VPI

#endif
