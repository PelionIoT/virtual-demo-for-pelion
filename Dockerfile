FROM ubuntu:20.04

ENV VIRTUAL_DEMO_VERSION v1.2

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
    autoconf-archive \
    libcmocka0 \
    libcmocka-dev \
    procps \
    iproute2 \
    pkg-config \
    libtool \
    libssl-dev \
    uthash-dev \
    autoconf \
    doxygen \
    libjson-c-dev \
    libini-config-dev \
    libcurl4-openssl-dev \
    acl \
    uuid-dev \
    libdbus-1-dev \
    libglib2.0-dev \
    rustc \
    clang \
&& rm -rf /var/lib/apt/lists/*

# install python deps.
RUN pip3 install mercurial requests click tornado posix_ipc mbed-cli manifest-tool

# factory-provisioning deps

# tpm2-tss
RUN git clone https://github.com/tpm2-software/tpm2-tss.git \
    && cd tpm2-tss \
    && git checkout 3.1.0 \
    && ./bootstrap \
    && ./configure --prefix=/usr \
    && make install

# tpm2-abrmd
RUN git clone https://github.com/tpm2-software/tpm2-abrmd.git \
    && cd tpm2-abrmd \
    && git checkout 2.4.0 \
    && ./bootstrap \
    && ./configure --with-dbuspolicydir=/etc/dbus-1/system.d \
      --with-udevrulesdir=/usr/lib/udev/rules.d \
      --with-systemdsystemunitdir=/usr/lib/systemd/system \
      --libdir=/usr/lib64 --prefix=/usr \
    && make install

# tpm2-tools
RUN git clone https://github.com/tpm2-software/tpm2-tools.git \
   && cd tpm2-tools \
   && git checkout 5.1.1 \
   && ./bootstrap \
   && ./configure --prefix=/usr \
   && make install

# ibmswtpm2
RUN mkdir ibmswtpm2 \
   && cd ibmswtpm2 \
   && curl -L https://sourceforge.net/projects/ibmswtpm2/files/ibmtpm1661.tar.gz/download --output ibmswtpm2.tar.gz \
   && tar -xvzf ibmswtpm2.tar.gz \
   && rm ibmswtpm2.tar.gz \
   && cd src \
   && make install

# parsec
RUN git clone https://github.com/parallaxsecond/parsec.git \
    && cd parsec \
    && git checkout 0.8.1 \
    && cargo build --manifest-path ./Cargo.toml --features "tpm-provider,direct-authenticator" \
    && cp target/debug/parsec /usr/bin

# parsec-tool
RUN git clone https://github.com/parallaxsecond/parsec-tool \
    && cd parsec-tool \
    && git checkout 0.4.0 \
    && cargo build \
    && cp target/debug/parsec-tool /usr/bin

# add our custom parsec tpm configuration
ADD etc/config-parsec.toml /build

# generate parsec run-time directories
RUN mkdir /run/parsec \
    && mkdir -p /var/lib/parsec/mappings

#
# --factory-configurator-client-example / mbed-cloud-client-example / sim-webapp--
#

# Add factory-configurator-client-example
ADD factory-configurator-client-example /build/factory-configurator-client-example

# Add mbed-cloud-client-example
ADD mbed-cloud-client-example /build/mbed-cloud-client-example

# Add sim-webapp
ADD sim-webapp /build/sim-webapp

#
# --factory-configurator-client-example--
#
# change working directory
WORKDIR /build/factory-configurator-client-example

# deploy mbed lib deps.
RUN mbed config root . \
 && mbed deploy

# initialize cmake and & build demo
RUN python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
WORKDIR /build/factory-configurator-client-example/__x86_x64_NativeLinux_mbedtls
RUN cmake -G "Unix Makefiles" -DPARSEC_TPM_SE_SUPPORT=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake \
      -DEXTERNAL_DEFINE_FILE=./../linux-psa-config.cmake \
     && make factory-configurator-client-example.elf

#
# --mbed-cloud-client-example--
#
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

# add pre-build production client
RUN mkdir /build/mbed-cloud-client-example-production \
    && curl -k -SL https://github.com/PelionIoT/virtual-demo-for-pelion/releases/download/${VIRTUAL_DEMO_VERSION}/mbed-cloud-client-example-precompiled-bin.v4.11-prod.elf -o /build/mbed-cloud-client-example-production/mbedCloudClientExample.elf \
    && chmod +x /build/mbed-cloud-client-example-production/mbedCloudClientExample.elf

EXPOSE 8888

WORKDIR /build/sim-webapp

CMD ["python3", "sim-webapp.py"]