# Pelion Virtual demo

Fristly, you need to build the linux cloud client as follows:
```
$ cd device-simulator-linux-client/
$ mbed deploy
# Copy paste a Pelion certificate in device-simulator-linux-client
$ python pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
$ __x86_x64_NativeLinux_mbedtls
$ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define.txt
$ make mbedCloudClientExample.elf
```

Then you can launch the webapp
```
$ cd webapp
$ npm install
$ npm start
```
