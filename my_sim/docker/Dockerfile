FROM centos:7.7.1908

MAINTAINER "Andrew Smith <atsmith3@illinois.edu>"

ARG user="andrew"

RUN yum -y update 

RUN yum -y update && \
  yum -y install csh ksh glibc.i686 sqlite elfutils-libelf-devel elfutils-libelf-devel.i686 @development-tools glibc-devel.i686

RUN yum -y update && \
  yum -y install vim gcc clang libgcc libgcc.i686 gcc-c++ libstdc++.i686 libstdc++ zlib zlib.i686

RUN groupadd -g 1000 sim
RUN useradd -rm -d /home/${user} -s /bin/bash -g root -G sim -u 1000 ${user}
RUN mkdir -p /run_vsim
RUN chown ${user} /run_vsim

WORKDIR /home/andrew/research/power_converter/my_sim
