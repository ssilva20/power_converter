#!/bin/bash

SIM_ROOT="/home/andrew/power_converter"
SIM_DIR="my_sim"

if [[ -z $(docker images -q centos7:cadence) ]]; then
  pushd docker && docker build --build-arg user=$(whoami) --build-arg wd=$SIM_ROOT/$SIM_DIR -t centos7:cadence . && popd
fi

docker run --rm -t -i --memory 16g --network=host \
  --user $(id -u):$(id -g) \
  --name=$1 \
  -v /software:/software \
  -v /dev/shm/:/dev/shm/ \
  -v /home/andrew/cadence:/home/andrew/cadence \
  -v $SIM_ROOT:$SIM_ROOT \
  centos7:cadence

#docker run --rm -t -i --memory 16g --network=host \
#  --user $(id -u):$(id -g) \
#  --name=$1 \
#  -v /software:/software \
#  -v /dev/shm/:/dev/shm/ \
#  -v /home/andrew/cadence:/home/andrew/cadence \
#  -v $SIM_ROOT:$SIM_ROOT \
#  centos7:cadence \
#  ./run_vsim.sh $1 $2
