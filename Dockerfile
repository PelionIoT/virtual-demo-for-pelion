FROM ubuntu:18.04

ENV CLOUD_SDK_API_KEY YOUR_PELION_API_KEY

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

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
RUN mbed config root . \
 && mbed deploy \
 && rm -rf mbed-os/ \
 && rm -rf drivers/

# Add manifest tool (2.x series as supported by cloud-client>=4.7)
RUN pip3 uninstall --yes manifest-tool && pip3 install manifest-tool==2.1.0

# initialize cmake
RUN python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
WORKDIR /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls
RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake \
     -DEXTERNAL_DEFINE_FILE=./../define_linux_update.txt -DFOTA_ENABLE=ON -DFOTA_TRACE=ON \
     && make mbedCloudClientExample.elf

EXPOSE 8888

WORKDIR /build/sim-webapp

CMD ["python3", "sim-webapp.py"]