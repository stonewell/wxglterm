FROM ubuntu:17.10

ADD sources.list /etc/apt/sources.list
RUN cat /etc/apt/sources.list

RUN apt-get update
RUN apt-get install -y software-properties-common python-software-properties
RUN apt-get install -y build-essential cmake libboost-thread-dev libboost-date-time-dev
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get install -y gcc g++ cmake libglfw3-dev libglew-dev libfontconfig1-dev libfreetype6-dev python3-dev libglm-dev git

WORKDIR /wxglterm
RUN git clone https://github.com/stonewell/wxglterm.git /wxglterm


