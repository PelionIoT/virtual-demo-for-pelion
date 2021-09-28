FROM ubuntu:20.04

ENV CLOUD_SDK_API_KEY YOUR_PELION_API_KEY

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

WORKDIR /build

# noninteractive env variable required to allow cmake install without region prompt
ENV DEBIAN_FRONTEND=noninteractive

# install packages
RUN apt-get update && apt-get -y install \
    git vim curl \
    build-essential \
    automake make cmake \
    python3 python3-pip \
 && rm -rf /var/lib/apt/lists/*

# install python deps.
RUN pip3 install mercurial requests click tornado posix_ipc mbed-cli manifest-tool

# Add pelion-client and webapp
ADD mbed-cloud-client-example /build/mbed-cloud-client-example
ADD sim-webapp /build/sim-webapp

# change working directory
WORKDIR /build/mbed-cloud-client-example

# deploy mbed lib deps.
RUN mbed config root . \
 && mbed deploy

# initialize cmake and & build demo
RUN python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
WORKDIR /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls
RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake \
     -DEXTERNAL_DEFINE_FILE=./../define.txt \
    && make mbedCloudClientExample.elf

EXPOSE 8888

WORKDIR /build/sim-webapp

CMD ["python3", "sim-webapp.py"]
