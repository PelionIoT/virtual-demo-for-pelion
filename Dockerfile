FROM ubuntu:20.04

ENV CLOUD_SDK_API_KEY YOUR_PELION_API_KEY

WORKDIR /build

# noninteractive env variable required to allow cmake install without region prompt
ENV DEBIAN_FRONTEND=noninteractive

# install packages
RUN apt-get update && apt-get -y install \
    git mercurial \
    build-essential \
    automake make cmake \
    python3 python3-pip \
 && rm -rf /var/lib/apt/lists/*

# install mbed-cli
RUN pip3 install requests click mbed-cli mbed-cloud-sdk tornado posix_ipc

# Add pelion-client and webapp
ADD mbed-cloud-client-example /build/mbed-cloud-client-example
ADD sim-webapp /build/sim-webapp

#
# phase-1: initialize pelion-client
#
WORKDIR /build/mbed-cloud-client-example

# deploy mbed lib deps and remove unused to save space
RUN mbed deploy \
 && rm -rf mbed-os/ \
 && rm -rf drivers/

# initialize cmake
RUN python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
WORKDIR /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls
RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define.txt \
    && make mbedCloudClientExample.elf

EXPOSE 8888

WORKDIR /build/sim-webapp

CMD ["python3", "sim-webapp.py"]