# Virtual demo for Pelion


The virtual demo for Pelion is a software only device that connects to the Pelion Device Management service so that the service can be used and demonstrated without the requirement for any device hardware. The project was specifically created to help discussion of high level device management topics with the constraints of travel due to the Covid-19 pandemic but can be used any time that a screen share demo would be useful when a face to face physical demo isn't possible.

An example demo walkthrough script is provided that gives an example of where the virtual demo can be used:
[Pelion-virtual-demo-script.md](Pelion-virtual-demo-script.md)

![Pelion Virtual Demo GUI](images/PDMvirtualDemo.jpg)

To use this demo you'll need to have a Pelion Device Management account, visit the [Pelion Device Management](portal.mbedcloud.com) site to sign up for an account if you don't already have one.

Running the demo will register a device to your account and allow you to see and write values to and from the device from the device management service in real time via your Internet link.

![Screenshot of demo and browser](images/BrowserScreenShot.png)

**Note** This demo is great for:

* Demoing a connected device on a computer so that you can explain IoT device management without the overhead of configuring and running up physical hardware
* Running a real Pelion Device Management demo in a video call screen share
* Quickly and easily connecting a device so that you can experiment with the Pelion Device Management Portal in real time
* Testing out the FOTA firmware update tools that Pelion Device Management provides
* Connecting a device as part of your work to implement and test the REST API for Pelion Device Management. Pelion doesn't know that this is a virtual demo device

The demo is *not intended* to be used as a:

* Starter template for a constrained, low power IoT device. The Linux platform that we use so that we can demo the device on a computer is too heavy weight, we suggest you apply the concepts explained here to an application running on an mbed enabled board.
* Method of running firmware update management for Linux IoT devices. The firmware update method demoed here is for single-application updates. When you're working with real Linux IoT devices you should implement update methods for Linux packages and any application libraries that you use. We suggest that you take a look at Pelion Edge if you're implementing that type of product.

## Quick start:
The virtual demo can be run using a docker container without any code changes or compilation. Docker needs to be installed on the host machine [https://www.docker.com/products/docker-desktop](https://www.docker.com/products/docker-desktop).
A docker image has been prepared and uploaded to the docker hub site, the commands below will pull the pelion/virtual-demo image from docker hub and run and instance on your machine

1. Generate an Access key from [Pelion Device management Portal](https://portal.mbedcloud.com/). You'll find the Access keys in the Access Management section.

2. Start the `pelion/virtual-demo` container image replacing `CLOUD_SDK_API_KEY` with your key:

    ```
    docker run --name pelion-demo -p 8888:8888 -e CLOUD_SDK_API_KEY=<YOUR_PELION_API_KEY> pelion/virtual-demo
    ```
    
3. Point your web browser to [http://localhost:8888](http://localhost:8888) to access the user interface of the virtual device.

4. Note the deviceID and find the device in your device list on the Pelion Device Management Portal. You can now browse resources and see the data updates from the virtual device in real time.

Kill the demo with CTRL-C. The docker container running this instance of the demo will be suspended. You can choose to come back to this same container and resume it at a later date (the deviceID will be preserved), or you can issue a fresh `docker run` command to create a new device instance and a new device in your device directory.
```
docker restart pelion-demo
docker attach pelion-demo
```

## Performing firmware update
The docker image comes already pre-configured with the necessary tools to perform a firmware update. In particular, In particular, build tools and the[manifest-tool](https://github.com/PelionIoT/manifest-tool) is included to sign and produce a firmware manifest ready to be uploaded to Pelion portal to start an update campaign. 

> For more information about Pelion Device Management update, please consult our [documentation page.](https://developer.pelion.com/docs/device-management/current/updating-firmware/index.html)

Let's change the firmware code running on the virtual device, to simulate a code change and subsequent firmware update. With the docker container running, open a new terminal. Then:

1. Clone the virtual demo repository locally:

    ```
    $ git clone https://github.com/PelionIoT/virtual-demo-for-pelion.git && \
    cd virtual-demo-for-pelion
    ```

2. Switch to the directory where the 'firmware' source code is located:
    ```
    $ cd mbed-cloud-client-example
    ```

3. Copy credential sources and manifest-tool configuration from the running docker container to your local folder::

    ```
    $ docker cp pelion-demo:/build/mbed-cloud-client-example/mbed_cloud_dev_credentials.c . && \
    docker cp pelion-demo:/build/mbed-cloud-client-example/update_default_resources.c . && \
    docker cp pelion-demo:/build/mbed-cloud-client-example/.manifest-dev-tool .
    ```

    > NOTE: If you are planning to apply a delta patch update, you must also save the existing running firmware, otherwise you can skip the following step:
    
    ```
    $ mkdir firmwares
    $ docker cp pelion-demo:/build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf ./firmwares/current_fw.bin


4. Using `vi` editor open `main.cpp` and insert the following code to simulate a code change (insert at line :273):
    ```
    for (int i=0; i<5; i++) {
        printf("!! new firmware !! \n");
    }
    ```
    Save and exit.


5.  Bootstrap a new development container of the virtual demo image to use it for building our new firmware. Notice that we local mount the credential sources and the manifest configuration we copied in step 3 above, so that they are available from inside the new container: 
    ```
    $ docker run -it --name pelion-demo-dev \
     -v $(pwd)/main.cpp:/build/mbed-cloud-client-example/main.cpp \
     -v $(pwd)/mbed_cloud_dev_credentials.c:/build/mbed-cloud-client-example/mbed_cloud_dev_credentials.c \
     -v $(pwd)/update_default_resources.c:/build/mbed-cloud-client-example/update_default_resources.c \
     -v $(pwd)/.manifest-dev-tool/:/build/mbed-cloud-client-example/.manifest-dev-tool/ \
     pelion/virtual-demo bash
    ```
    > NOTE: If planning to perform delta patch update, we need to local mount the 'firmwares/' directory too. Simply append `-v $(pwd)/firmwares/:/build/mbed-cloud-client-example/firmwares` in the command above.


You can now choose either to perform a full firmware image update or a delta patch. Follow the appropriate section below.

### Full Image Update

1. Switch to the firmware source code directory:
    ```
    cd /build/mbed-cloud-client-example/
    ```

2. Build the new firmware image by invoking the `make` tool:
    ```
    make -C __x86_x64_NativeLinux_mbedtls/ mbedCloudClientExample.elf
    ```

3. Upon completion of the build, a new firmware binary is generated:
    ```
    $ ls -l /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf
    
    -rwxr-xr-x 1 root root 6562112 Jan 19 14:32 /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf
    ```
4. We now need to genarate the firmware manifest describing the update, upload it to the portal and start an update campaign. The `manifest-tool` can conveniently perform all this in one step. Simple execute:

    ```
    $ manifest-dev-tool update -p __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf -w -n -v 0.2.0

    2021-01-19 13:29:51,924 INFO FW version: 0.2.0
    2021-01-19 13:29:51,929 INFO Uploading FW image __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf
    2021-01-19 13:29:53,529 INFO Uploaded FW image http://firmware-catalog-media-ca57.s3.dualstack.us-east-1.amazonaws.com/u7chWKTSvQdSK5PXUFyxWu
    2021-01-19 13:29:53,531 INFO Vendor-ID: f214c5a40a7c41588f4a8f2357880e4a
    2021-01-19 13:29:53,532 INFO Class-ID: 66eb99760e98456d8436dc223c7332ff
    2021-01-19 13:29:53,681 INFO Created manifest in v3 schema for full update campaign
    2021-01-19 13:29:54,225 INFO Uploaded manifest ID: 01771ad664fd0000000000010010028e
    2021-01-19 13:29:54,636 INFO Created Campaign ID: 01771ad6669b00000000000100100323
    2021-01-19 13:29:56,384 INFO Started Campaign ID: 01771ad6669b00000000000100100323
    2021-01-19 13:29:56,749 INFO Campaign state: publishing
    2021-01-19 13:29:59,846 INFO Campaign state: autostopped
    2021-01-19 13:29:59,846 INFO Campaign is finished in state: autostopped
    2021-01-19 13:30:00,606 INFO ----------------------------
    2021-01-19 13:30:00,607 INFO     Campaign Summary
    2021-01-19 13:30:00,608 INFO ----------------------------
    2021-01-19 13:30:00,608 INFO  Successfully updated:   1
    2021-01-19 13:30:00,609 INFO  Failed to update:       0
    2021-01-19 13:30:00,609 INFO  Skipped:                0
    2021-01-19 13:30:00,610 INFO  Pending:                0
    2021-01-19 13:30:00,610 INFO  Total in this campaign: 1
    ```

    At the console prompt of the virtual demo, notice the firmware update being initiated, then downloaded and applied:
    ```
    [FOTA INFO] fota.c:596: Firmware update initiated.
    [FOTA DEBUG] fota.c:628: Pelion FOTA manifest is valid
    [FOTA DEBUG] fota.c:651: get manifest : curr version 131072, new version 196608
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 3
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA DEBUG] fota.c:555: Download Authorization requested
    [FOTA] ---------------------------------------------------
    [FOTA] Updating component MAIN from version 0.0.0 to 0.2.0
    [FOTA] Update priority 0
    [FOTA] Update size 6562112B
    [FOTA] ---------------------------------------------------
    [FOTA] Download authorization granted
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    [FOTA INFO] fota.c:1351: Download authorization granted.
    [FOTA DEBUG] fota_block_device_linux.c:77: FOTA BlockDevice init file is fota_candidate
    [FOTA DEBUG] fota.c:1128: FOTA BlockDevice initialized
    [FOTA DEBUG] fota.c:522: New FOTA key saved
    [FOTA DEBUG] fota.c:533: FOTA encryption engine initialized
    [FOTA DEBUG] fota.c:1216: Erasing storage at 0, size 4294967296
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 4
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA] Downloading firmware. 0%
    [FOTA] Downloading firmware. 5%
    [FOTA] Downloading firmware. 10%
    [FOTA] Downloading firmware. 15%
    [FOTA] Downloading firmware. 20%
    [FOTA] Downloading firmware. 25%
    [FOTA] Downloading firmware. 30%
    [FOTA] Downloading firmware. 35%
    [FOTA] Downloading firmware. 40%
    [FOTA] Downloading firmware. 45%
    [FOTA] Downloading firmware. 50%
    [FOTA] Downloading firmware. 55%
    [FOTA] Downloading firmware. 60%
    [FOTA] Downloading firmware. 65%
    [FOTA] Downloading firmware. 70%
    [FOTA] Downloading firmware. 75%
    [FOTA] Downloading firmware. 80%
    [FOTA] Downloading firmware. 85%
    [FOTA] Downloading firmware. 90%
    [FOTA] Downloading firmware. 95%
    [FOTA] Downloading firmware. 100%
    [FOTA INFO] fota.c:1509: Firmware download finished
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 6
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA DEBUG] fota.c:1460: Install Authorization requested
    [FOTA] Install authorization granted
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    [FOTA INFO] fota.c:1346: Install authorization granted.
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 7
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA INFO] fota.c:804: Installing new version for component MAIN
    [FOTA INFO] fota_candidate.c:224: Found an encrypted image at address 0x0
    [FOTA INFO] fota_candidate.c:416: Validating image...
    [FOTA INFO] fota_candidate.c:461: Image is valid.
    [FOTA INFO] fota_component.c:189: Installing MAIN component
    [FOTA] Successfully installed MAIN component

    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 8
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA INFO] fota.c:725: Rebooting.
    !! new firmware !!
    !! new firmware !!
    !! new firmware !!
    !! new firmware !!
    !! new firmware !!
    In single-partition mode.
    Creating path ./pal
    Start Device Management Client
    Using hardcoded Root of Trust, not suitable for production use.
    Starting developer flow
    Developer credentials already exist, continuing..
    Generating random from /dev/random, this can take a long time!
    Application ready. Build at: Jan 19 2021 11:26:14
    Network initialized, registering...
    [FOTA DEBUG] fota.c:371: init start
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 1]
    [FOTA DEBUG] fota_source_profile_full.cpp:277: Announcing FOTA state is 1
    [FOTA INFO] fota.c:425: Registered MAIN component, version 0.3.0
    [FOTA DEBUG] fota.c:435: After upgrade, issuing post install actions
    [FOTA DEBUG] fota_block_device_linux.c:77: FOTA BlockDevice init file is fota_candidate
    [FOTA DEBUG] fota.c:327: install verify component name MAIN, version 196608
    [FOTA DEBUG] fota.c:467: init complete
    Commander initialized and waiting for cmds...
    Client registered
    Endpoint Name: 01771a7172b70000000000010016f8dc
    Device ID: 01771a7172b70000000000010016f8dc
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    Message status callback: (3313/0/5700) subscribed
    ```

## Delta updates (TBD)


1. Switch to the main program source directory:
    ```
    $ cd /build/mbed-cloud-client-example/
    ```
2. Assuming you've modifed `main.cpp` as suggested above, we can now proceed and produce the new firmware:
    ```
    $ make -C __x86_x64_NativeLinux_mbedtls/ mbedCloudClientExample.elf
    ```
3. Copy the new firmware to `firmwares/` directory:
    ```
    $ cp __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf firmwares/new_fw.bin
    ```
4. The `firmwares/` directory should now contain both the new firmware(`new_fw.bin`) and the current running one (`current_fw.bin`)following:
    ```
    $  ls -l firmwares/
    
    total 12824
    -rwxr-xr-x 1 1000 1000 6562072 Jan 19 15:29 current_fw.bin
    -rwxr-xr-x 1 root root 6562112 Jan 19 15:51 new_fw.bin
    ```
5. We are now ready to generate the delta firmware using the `manifest-delta-tool`:
    ```
    $ manifest-delta-tool -c firmwares/current_fw.bin -n firmwares/new_fw.bin -o firmwares/delta-patch.bin

    2021-01-19 13:36:30,437 INFO Current tool version PELION/BSDIFF001
    Wrote diff file firmwares/delta-patch.bin, size 166735. Max undeCompressBuffer frame size was 512, max deCompressBuffer frame size was 40.
    root@323939aac5da:/build/mbed-cloud-client
    ```
    
6. Start the update campaign
    ```
    $ manifest-dev-tool update -p firmwares/delta-patch.bin -w -n -v 0.3.0

    2021-01-19 15:51:53,622 INFO FW version: 0.3.0
    2021-01-19 15:51:53,624 INFO Uploading FW image firmwares/delta-patch.bin
    2021-01-19 15:51:54,571 INFO Uploaded FW image http://firmware-catalog-media-ca57.s3.dualstack.us-east-1.amazonaws.com/53ig3icEBJEa1QSXHiqG1n
    2021-01-19 15:51:54,572 INFO Vendor-ID: eb91fb344d3246359a56f6ae958a6b3e
    2021-01-19 15:51:54,572 INFO Class-ID: 0f2826c9586a4ea199400a34e1afc258
    2021-01-19 15:51:54,660 INFO Created manifest in v3 schema for delta update campaign
    2021-01-19 15:51:55,119 INFO Uploaded manifest ID: 01771b5869b80000000000010010035b
    2021-01-19 15:51:55,576 INFO Created Campaign ID: 01771b586b840000000000010010036d
    2021-01-19 15:51:57,335 INFO Started Campaign ID: 01771b586b840000000000010010036d
    2021-01-19 15:51:57,699 INFO Campaign state: publishing
    ```

    At the console prompt of the virtual demo, notice the firmware update being initiated, then downloaded and applied:
    ```
    [FOTA INFO] fota.c:596: Firmware update initiated.
    [FOTA DEBUG] fota.c:628: Pelion FOTA manifest is valid
    [FOTA DEBUG] fota.c:651: get manifest : curr version 0, new version 196608
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 3
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA DEBUG] fota.c:555: Download Authorization requested
    [FOTA] ---------------------------------------------------
    [FOTA] Updating component MAIN from version 0.0.0 to 0.3.0
    [FOTA] Update priority 0
    [FOTA] Delta update. Patch size 358023B full image size 6562112B
    [FOTA] ---------------------------------------------------
    [FOTA] Download authorization granted
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    [FOTA INFO] fota.c:1351: Download authorization granted.
    [FOTA DEBUG] fota_block_device_linux.c:77: FOTA BlockDevice init file is fota_candidate
    [FOTA DEBUG] fota.c:1128: FOTA BlockDevice initialized
    [FOTA DEBUG] fota.c:522: New FOTA key saved
    [FOTA DEBUG] fota.c:533: FOTA encryption engine initialized
    [FOTA DEBUG] fota.c:1216: Erasing storage at 0, size 4294967296
    [FOTA DEBUG] fota.c:1258: FOTA delta engine initialized
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 4
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA] Downloading firmware. 3%
    [FOTA] Downloading firmware. 5%
    [FOTA] Downloading firmware. 11%
    [FOTA] Downloading firmware. 19%
    [FOTA] Downloading firmware. 23%
    [FOTA] Downloading firmware. 27%
    [FOTA] Downloading firmware. 32%
    [FOTA] Downloading firmware. 36%
    [FOTA] Downloading firmware. 40%
    [FOTA] Downloading firmware. 45%
    [FOTA] Downloading firmware. 54%
    [FOTA] Downloading firmware. 59%
    [FOTA] Downloading firmware. 63%
    [FOTA] Downloading firmware. 68%
    [FOTA] Downloading firmware. 72%
    [FOTA] Downloading firmware. 77%
    [FOTA] Downloading firmware. 81%
    [FOTA] Downloading firmware. 86%
    [FOTA] Downloading firmware. 91%
    [FOTA] Downloading firmware. 95%
    [FOTA] Downloading firmware. 100%
    [FOTA ERROR] fota.c:158: Update aborted: (ret code -42) Failed on fragment event
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/3: value: 42
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 1
    ```

## Technical overview
The demo has been implemented to be run in 2 parts
1) an instance of the Pelion device management client built with your certificate that writes sensor values to the linux message queue running inside the docker container
2) a graphical representation (GUI) of a device that picks up the sensor values from the queue and displays values in a "fake device" so that conversations about managed devices can take place.

Our pre-existing docker image has a linux environment and a pre-built set of objects to run the demo. When you use the `docker run` command with your Access key as an argument the scripts inside the container use the Access key to retrieve a device certificate from your Pelion Device Management account and finalise the compliation of the client using your certificate. The client instance is then run, a `firstrun` file is written to the root of the docker container's linux environment, and the demo is running. The client passes values to the GUI using the linux message queue running inside the container, the the GUI is rendered on port 8888 of your local machine, the vibration values can be seen under the 3313/0/5700 resource for this device listed in the Pelion device management portal.

When you kill the demo with `CTRL-C` you are halting the docker container ´pelion-demo´ but not destroying it. When you start the demo with the supplied launch scripts you are resuming the previously halted container, this solution means that the pelion client is re-used and the same Pelion deviceID used over multiple demo sessions. If you don't use the restart command in the launch scripts and instead issue a `docker run` command then a fresh instance of the docker image will be created as a new container, which in turn means a fresh instance of the client will be executed, and a new deviceID will be issued by Pelion. This would create a growing list of stale devices in the device directory list of the Pelion portal webpage so we use the resume feature instead.

## Developing with the virtual demo:
The virtual demo docker image and the contents of this github repo can be used together as an environment to tweak and build your own modifications to the demo. Clone this demo to your local machine and use the following commands to mount the cloned directories inside the docker image, the result will be that any code changes you make to the repo's files on your machine can be built and executed inside the docker container.

1. Generate an Access key from Pelion Device management Portal

2. Start the `pelion/virtual-demo` container image from the root of the cloned repo replacing `CLOUD_SDK_API_KEY` with your key:

    ```
   docker run -it --name pelion-demo-dev -p 8888:8888 -v $(pwd):/build -e CLOUD_SDK_API_KEY=<YOUR_API_KEY> pelion/virtual-demo bash
    ```
This will create a container with the name tag "pelion-demo-dev" that is running the pelion/virtual-demo image with a bind mount folder on your local machine. You can use the pelion-demo-dev container name if you exit the running container and want to return to it with docker restart and resume commands.

3. You'll be at the bash shell inside the container, you now need to create the build environment to allow the demo to be rebuilt with your changes

```
cd ../mbed-cloud-client-example
mbed config root .
mbed deploy
python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
cd __x86_x64_NativeLinux_mbedtls
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake \
    -DEXTERNAL_DEFINE_FILE=./../define_linux_update.txt -DFOTA_ENABLE=ON -DFOTA_TRACE=ON
cd ../../sim-webapp
```

4. To build your changes you can use the sim-webapp.py python script. The first build will require all object files to be built, this can take 30+ minutes but subsequent builds will only rebuild your modifications. At the end of each build the script adds a marker `firstrun` file to the sim-webapp directory to ensure further executions of the script only cause the demo to be executed and not compiled again. Similarly a `certexists` file in the root of the docker container ensures that the certificate for your device is only pulled once. To kick a fresh compliation of your code changes use

``` 
rm -f firstrun && python3 sim-webapp.py
```

5. When the system has compiled your changes and the demo is running you'll see the console output at the bash shell, and you'll find the webapp running at the [localhost:8888](http://localhost:8888) URL on your host machine.
