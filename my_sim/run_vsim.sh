#!/bin/bash

VERILOG_TB_PATH="/home/andrew/research/power_converter/my_sim"

#export LD_LIBRARY_PATH=/software/cadence-Aug2016/INNOVUS161/tools.lnx86/lib:/software/cadence-Aug2016/MMSIM151/tools.lnx86/lib:/software/cadence-Aug2016/INCISIVE152/vmanager/linux:/software/cadence-Aug2016/INCISIVE152/tools.lnx86/lib:/software/cadence-Aug2016/ETS131/tools.lnx86/lib:/software/cadence-Aug2016/EDI142/tools.lnx86/lib:/software/cadence-Aug2016/ADW166/tools.lnx86/lib:$LD_LIBRARY_PATH
export PATH=/software/cadence-Aug2016/INNOVUS161/tools.lnx86/bin:/software/cadence-Aug2016/INNOVUS1611/share/bin:/software/cadence-Aug2016/INNOVUS161/bin:/software/cadence-Aug2016/MMSIM151/tools.lnx86/bin:/software/cadence-Aug2016/MMSIM151/share/bin:/software/cadence-Aug2016/MMSIM151/bin:/software/cadence-Aug2016/INCISIVE152/tools.lnx86/bin:/software/cadence-Aug2016/INCISIVE152/share/bin:/software/cadence-Aug2016/INCISIVE152/bin:/software/cadence-Aug2016/IC617/tools.lnx86/leapfrog/bin:/software/cadence-Aug2016/IC617/tools.lnx86/dfII/bin:/software/cadence-Aug2016/IC617/tools.lnx86/bin:/software/cadence-Aug2016/IC617/share/bin:/software/cadence-Aug2016/IC617/bin:/software/cadence-Aug2016/ETS131/tools.lnx86/bin:/software/cadence-Aug2016/ETS131/share/bin:/software/cadence-Aug2016/ETS131/bin:/software/cadence-Aug2016/EDI142/tools.lnx86/bin:/software/cadence-Aug2016/EDI142/share/bin:/software/cadence-Aug2016/EDI142/bin:/software/cadence-Aug2016/ADW166/tools.lnx86/bin:/software/cadence-Aug2016/ADW166/share/bin:/software/cadence-Aug2016/ADW166/bin:$PATH
export LM_LICENSE_FILE=5280@cadence.webstore.illinois.edu
export OA_UNSUPPORTED_PLAT=linux_rhel50_gcc48x

pushd /run_vsim
ln -s $VERILOG_TB_PATH/interprocess.cpp ./interprocess.c
ln -s $VERILOG_TB_PATH/interprocess.h .
ln -s $VERILOG_TB_PATH/tb.vams .
ln -s $VERILOG_TB_PATH/scf.scs .

# BUILD:
#gcc hello_vpi.c -m32 -fPIC -shared -I/software/cadence-Aug2016/INCISIVE152/tools.lnx86/include -o hello_vpi.so
gcc interprocess.c -O0 -g -DWITH_VPI -std=c11 -D_XOPEN_SOURCE=500 -m32 -fPIC -fpermissive -shared -I/software/cadence-Aug2016/INCISIVE152/tools.lnx86/include -lpthread -o interprocess.so

# RUN:
#irun tb.vams -top DAC6_TB +access+r -v ./hello_vpi.so -analogcontrol scf.scs
ncverilog \
	+define+SHM_NAME=\\\"${1}\\\" \
  +define+STEP_SIZE=${2} \
  tb.vams \
  +access+r -loadvpi ./interprocess.so:register_create_shm \
  -loadvpi ./interprocess.so:register_destroy_shm \
  -loadvpi ./interprocess.so:register_wait_driver_data \
  -loadvpi ./interprocess.so:register_get_voltage_setpoint \
  -loadvpi ./interprocess.so:register_get_effective_resistance \
  -loadvpi ./interprocess.so:register_get_terminate_simulation \
  -loadvpi ./interprocess.so:register_ack_driver_data \
  -loadvpi ./interprocess.so:register_send_voltage \
  -top buck_model \
  -analogcontrol scf.scs

