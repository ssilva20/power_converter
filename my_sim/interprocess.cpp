#include <stdio.h>
#include <sys/mman.h>
#include <sys/shm.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#include "interprocess.h"
#ifdef WITH_VPI
#include "vpi_ams.h"
#include "vpi_user.h"
#include "vpi_user_cds.h"
#endif

#ifdef INTERPROCESS_PYTHON
#include <pybind11/pybind11.h>
#endif

int shm_fd = 0;

mapped* shm_ptr = NULL;

char name_buff[256] = "";

// shm_init
//   helper to initialize data/mutexes in mapped struct
void init_shm(mapped* p) {
  sem_init(&p->pv.sem, 1, 1);
  p->pv.new_data = NO_NEW_DATA;
  p->pv.data.v_set = 0;
  p->pv.data.curr_r_load = 0;
  p->pv.data.sim_over = 0;
  
  sem_init(&p->vp.sem, 1, 1);
  p->vp.new_data = NO_NEW_DATA;
  p->vp.data.curr_v = 0;
  p->vp.data.sim_done = 0;
}

// shm_create
//   Creates, mmaps and initializes data/mutexes in shared mem region if INIT.
//   Opens shm and mmaps if !INIT
// Returns:
//   ptr to shared mem struct on success
//   NULL on failure
int create_shm(int should_init, char* name) {
#ifdef WITH_VPI
  vpiHandle systfref, argsiter, argh;
  struct t_vpi_value value;
  systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  argsiter = vpi_iterate(vpiArgument, systfref);/* get iterator (list) of passed arguments */
  argh = vpi_scan(argsiter);/* get the one argument - add loop for more args */
  if(!argh){
    vpi_printf("$VPI missing parameter.\n");
    return 0;
  }

  value.format = vpiIntVal;
  vpi_get_value(argh, &value);
  should_init = value.value.integer;
  
  argh = vpi_scan(argsiter);/* get the one argument - add loop for more args */
  if(!argh){
    vpi_printf("$VPI missing parameter.\n");
    return 0;
  }

  value.format = vpiStringVal;
  vpi_get_value(argh, &value);
  name = value.value.str;
  vpi_free_object(argsiter);
  vpi_printf("$create_shm called with args %d %s\n", should_init, name);
#endif

  if(name == NULL) {
    fprintf(stderr,"bad pointer to name\n");
    exit(-1);
  }
  strncpy(name_buff, name, 256);
  if(should_init == INIT) {
    shm_fd = shm_open(name_buff, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) {
      fprintf(stderr,"Failed to create & open /dev/shm/%s\n", name_buff);
      exit(-1);
    }
    ftruncate(shm_fd, SHM_LENGTH);
    shm_ptr = (mapped*)mmap(NULL, SHM_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if(shm_ptr == NULL) {
      fprintf(stderr,"mmap failed\n");
      shm_unlink(name_buff);
      exit(-1);
    }
    // Initialize Data:
    init_shm(shm_ptr);
    printf("Shared memory region at: %p with size: %d\n", shm_ptr, SHM_LENGTH); 
  }
  else {
    shm_fd = shm_open(name_buff, O_RDWR, 0666);
    if(shm_fd == -1) {
      fprintf(stderr,"Failed to open /dev/shm/%s\n", name_buff);
      exit(-1);
    }
    shm_ptr = (mapped*)mmap(NULL, SHM_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if(shm_ptr == NULL) {
      fprintf(stderr,"mmap failed\n");
      shm_unlink(name_buff);
      exit(-1);
    }
  }
  return 0;
}

// shm_destroy
//   Unmaps and Unlinks shared memory region
// Input:
//   ptr to shared mem region
void destroy_shm() {
  munmap(shm_ptr, SHM_LENGTH);
  shm_unlink(name_buff);
}

//*******************************************************************
//
// VPI Getters & Setter functions
//
//*******************************************************************
// driver_data_ready:
//   Call to block simulation until data is sent to it:
void wait_driver_data() {
  //printf("Shared memory region at: %p with size: %lu\n", shm_ptr, SHM_LENGTH); 
  while(1) {
    sem_wait(&shm_ptr->pv.sem);
    //printf("pv.nd:%d, pv.vs:%d, pv.er:%d, pv.ts:%d\n", shm_ptr->pv.new_data, shm_ptr->pv.data.v_set, shm_ptr->pv.data.curr_r_load, shm_ptr->pv.data.sim_over);
    if(shm_ptr->pv.new_data == NEW_DATA) {
      sem_post(&shm_ptr->pv.sem);
      return;
    }
    sem_post(&shm_ptr->pv.sem);
  }
}

uint32_t get_size() {
  return sizeof(uint32_t);
}

// get_setpoint_voltage
//   get the new voltage setpoint:
int get_voltage_setpoint() {
#ifdef WITH_VPI
  vpiHandle systfref, argsiter, argh;
  struct t_vpi_value value;
#endif
  int ret = 0;

  //systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  //argsiter = vpi_iterate(vpiArgument, systfref);/* get iterator (list) of passed arguments */
  //argh = vpi_scan(argsiter);/* get the one argument - add loop for more args */
  //if(!argh){
  //  vpi_printf("$VPI missing parameter.\n");
  //  return 0;
  //}
    
  //value.format = vpiStringVal;
  //vpi_get_value(argh, &value);
  //char *s = value.value.str;
                
  ////Do shell command on DOS!
  //int result=reinterpret_cast<int>(ShellExecute(NULL, "open", s, NULL, NULL, SW_SHOWNORMAL));
  //value.value.integer = result;//;
  //value.format = vpiIntVal;/* return the result */

  //vpi_put_value(systfref, &value, NULL, vpiNoDelay);
  //vpi_free_object(argsiter);

  sem_wait(&shm_ptr->pv.sem);
#ifdef WITH_VPI
  systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  value.value.integer = (int)shm_ptr->pv.data.v_set;
  value.format = vpiIntVal;/* return the result */
  vpi_put_value(systfref, &value, NULL, vpiNoDelay);
#else
  ret = (int)shm_ptr->pv.data.v_set;
#endif
  sem_post(&shm_ptr->pv.sem);
  return ret;
}

// get_setpoint_voltage
//   get the new voltage setpoint:
int get_effective_resistance() {
#ifdef WITH_VPI
  vpiHandle systfref, argsiter, argh;
  struct t_vpi_value value;
#endif
  int ret = 0;

  sem_wait(&shm_ptr->pv.sem);
#ifdef WITH_VPI
  systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  value.value.integer = (int)shm_ptr->pv.data.curr_r_load;
  value.format = vpiIntVal;/* return the result */
  vpi_put_value(systfref, &value, NULL, vpiNoDelay);
#else
  ret = (int)shm_ptr->pv.data.curr_r_load;
#endif
  sem_post(&shm_ptr->pv.sem);
  return ret;
}

// get_terminate_simulation
//   get the new voltage setpoint:
uint32_t get_terminate_simulation() {
#ifdef WITH_VPI
  vpiHandle systfref, argsiter, argh;
  struct t_vpi_value value;
#endif
  int ret = 0;

  sem_wait(&shm_ptr->pv.sem);
#ifdef WITH_VPI
  systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  value.value.integer = (int)shm_ptr->pv.data.sim_over;
  value.format = vpiIntVal;/* return the result */
  vpi_put_value(systfref, &value, NULL, vpiNoDelay);
#else
  ret = (int)shm_ptr->pv.data.sim_over;
#endif
  sem_post(&shm_ptr->pv.sem);
  return ret;
}

// ack_driver_data
void ack_driver_data() {
  sem_wait(&shm_ptr->pv.sem);
  shm_ptr->pv.new_data = NO_NEW_DATA; 
  sem_post(&shm_ptr->pv.sem);
}

int send_voltage(int voltage) {
#ifdef WITH_VPI
  vpiHandle systfref, argsiter, argh;
  struct t_vpi_value value;
  systfref = vpi_handle(vpiSysTfCall, NULL); /* get system function that invoked C routine */
  argsiter = vpi_iterate(vpiArgument, systfref);/* get iterator (list) of passed arguments */
  argh = vpi_scan(argsiter);/* get the one argument - add loop for more args */
  if(!argh){
    vpi_printf("$VPI missing parameter.\n");
    return 0;
  }
  value.format = vpiIntVal;
  vpi_get_value(argh, &value);
  voltage = value.value.integer;
#endif
  sem_wait(&shm_ptr->vp.sem);
  shm_ptr->vp.data.curr_v = (uint32_t)voltage;
  shm_ptr->vp.new_data = NEW_DATA;
  sem_post(&shm_ptr->vp.sem);

  // Wait for ack:
  while(1) {
    sem_wait(&shm_ptr->vp.sem);
    if(shm_ptr->vp.new_data == NO_NEW_DATA) {
      sem_post(&shm_ptr->vp.sem);
      return 0;
    }
    sem_post(&shm_ptr->vp.sem);
  }
  return 0;
}

uint32_t get_voltage() {
  uint32_t ret = 0;
  while(1) {
    sem_wait(&shm_ptr->vp.sem);
    if(shm_ptr->vp.new_data == NEW_DATA) {
      ret = shm_ptr->vp.data.curr_v;
      shm_ptr->vp.new_data = NO_NEW_DATA;
      sem_post(&shm_ptr->vp.sem);
      return ret;
    }
    sem_post(&shm_ptr->vp.sem);
  }
  return ret;
}

void set_driver_signals(uint32_t voltage_setpoint, uint32_t resistance, uint32_t terminate_sim) {
  // Wait for the verilog simulation to consume the previous data:
  while(1) {
    sem_wait(&shm_ptr->pv.sem);
    if(shm_ptr->pv.new_data == NO_NEW_DATA) {
      printf("Sending V:%d R:%d\n", voltage_setpoint, resistance);
      shm_ptr->pv.data.v_set = voltage_setpoint;
      shm_ptr->pv.data.curr_r_load = resistance;
      shm_ptr->pv.data.sim_over = terminate_sim;
      shm_ptr->pv.new_data = NEW_DATA;
      sem_post(&shm_ptr->pv.sem);
      return;
    }
    sem_post(&shm_ptr->pv.sem);
  }
}

#ifdef WITH_VPI
// register_create_shm
void register_create_shm() {
    s_vpi_systf_data data;
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname ="$create_shm";
    data.user_data ="$create_shm";
    data.calltf=create_shm;
    data.compiletf=0;
    data.sizetf=0;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_destroy_shm
void register_destroy_shm() {
    s_vpi_systf_data data;
    data.type = vpiSysTask;
    data.tfname ="$destroy_shm";
    data.calltf=destroy_shm;
    data.compiletf=0;
    data.sizetf=0;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_wait_driver_data
void register_wait_driver_data() {
    s_vpi_systf_data data;
    data.type = vpiSysTask;
    data.tfname ="$wait_driver_data";
    data.calltf=wait_driver_data;
    data.compiletf=0;
    data.sizetf=0;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_wait_driver_data
void register_get_voltage_setpoint() {
    s_vpi_systf_data data;
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname ="$get_voltage_setpoint";
    data.user_data ="$get_voltage_setpoint";
    data.calltf=get_voltage_setpoint;
    data.compiletf=0;
    data.sizetf=get_size;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_wait_driver_data
void register_get_effective_resistance() {
    s_vpi_systf_data data;
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname ="$get_effective_resistance";
    data.user_data ="$get_effective_resistance";
    data.calltf=get_effective_resistance;
    data.compiletf=0;
    data.sizetf=get_size;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_wait_driver_data
void register_get_terminate_simulation() {
    s_vpi_systf_data data;
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname ="$get_terminate_simulation";
    data.calltf=get_terminate_simulation;
    data.compiletf=0;
    data.sizetf=get_size;
    data.user_data=0;
    vpi_register_systf(&data);
}

// register_ack_driver_data
void register_ack_driver_data() {
    s_vpi_systf_data data;
    data.type = vpiSysTask;
    data.tfname ="$ack_driver_data";
    data.calltf=ack_driver_data;
    data.compiletf=0;
    data.sizetf=0;
    data.user_data=0;
    vpi_register_systf(&data);
}

void register_send_voltage() {
    s_vpi_systf_data data;
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname ="$send_voltage";
    data.calltf=send_voltage;
    data.compiletf=0;
    data.sizetf=0;
    data.user_data=0;
    vpi_register_systf(&data);
}
#endif // WITH_VPI

#ifdef INTERPROCESS_PYTHON
PYBIND11_MODULE(interprocess, m) {
  m.doc() = "Interprocess Communication routines for interfacing with simulation"; // optional module docstring
  m.def("create_shm", &create_shm, "Create/Open and Map a shared mem region");
  m.def("destroy_shm", &destroy_shm, "Destroy and UnMap a shared mem region");
  m.def("set_driver_signals", &set_driver_signals, "Send voltage and resistance to simulation");
  m.def("get_voltage", &get_voltage, "Get the instantaneous voltage value from the sim");
}
#endif
