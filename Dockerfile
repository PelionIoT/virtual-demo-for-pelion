FROM ubuntu

#noninteractive env variable required to allow cmake install without region prompt
ENV DEBIAN_FRONTEND=noninteractive

#install packages
RUN apt-get update
RUN apt-get install -y curl wget
RUN apt-get install -y python3 python3-pip git mercurial
RUN apt-get install -y make
RUN apt-get install -y cmake
RUN pip3 install requests
RUN pip3 install click
RUN python3 -m pip install mbed-cli

#download and extract gcc. set location with mbed -G, remove download tar bz2 to save space
WORKDIR /mbed
RUN wget -nv --progress=bar https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
RUN tar -xf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
RUN rm *.bz2
RUN mbed config -G ARM_PATH /mbed

#clone repo, update to copy it over given that its already been cloned to get the Dockerfile
WORKDIR /home
RUN git clone --branch docker https://github.com/armmbed/virtual-demo-for-pelion

#mbed deploy
WORKDIR virtual-demo-for-pelion/mbed-cloud-client-example
RUN mbed deploy

#copy my certificate
COPY mbed-cloud-client-example/mbed_cloud_dev_credentials.c .

#run the compliation and make processes
RUN python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
WORKDIR __x86_x64_NativeLinux_mbedtls
RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define.txt
RUN make mbedCloudClientExample.elf
WORKDIR Release

#start the app on docker run
CMD ./mbedCloudClientExample.elf

