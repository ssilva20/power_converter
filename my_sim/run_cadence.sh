#!/bin/bash

if [[ -z $(docker images -q centos7:cadence) ]]; then
  pushd docker && docker build -t centos7:cadence . && popd
fi

docker run --rm -t -i --memory 16g --network=host \
  --user $(id -u):$(id -g) \
  --name=$1 \
  -v /software:/software \
  -v /dev/shm/:/dev/shm/ \
  -v /home/andrew/cadence:/home/andrew/cadence \
  -v /home/andrew/research/power_converter:/home/andrew/research/power_converter \
  centos7:cadence \
  ./run_vsim.sh $1 $2
