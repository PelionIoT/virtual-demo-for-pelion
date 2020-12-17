# Virtual demo for Pelion 
The virtual demo for Pelion is a software only device that connects to the Pelion Device Management service so that the service can be used and demonstrated without the requirement for any device hardware. The project was specifically created to help discussion of high level device management topics with the constraints of travel due to the Covid-19 pandemic but can be used any time that a screen share demo would be useful when a face to face physical demo isn't possible.

An example demo walkthrough script is provided that gives an example of where the virtual demo can be used:
[Pelion-virtual-demo-script.md](Pelion-virtual-demo-script.md)

![Pelion Virtual Demo GUI](images/PDMvirtualDemo.jpg)

To use this demo you'll need to have a Pelion Device Management account, visit the [Pelion Device Management](portal.mbedcloud.com) site to sign up for an account if you don't already have one.

Running the demo will register a device to your account and allow you to see and write values to and from the device from the device management service in real time via your Internet link. You can view the Pelion Device Management portal in a browser locally on the same machine that is running the demo, or you can split the demo and the browser accross real and virtual machines, run the portal on one machine and the portal on a second machine, etc.

![Screenshot of demo and browser](images/BrowserScreenShot.png)

## Quick start:
The demo can be run as a docker container without any code changes or compilation. Docker needs to be installed on the host machine [https://www.docker.com/products/docker-desktop](https://www.docker.com/products/docker-desktop).
A docker image has been prepared and uploaded to the docker hub site, the commands below will pull the pelion/virtual-demo image from docker hub and run and instance on your machine

1. Generate an API key from [Pelion Device management Portal](https://portal.mbedcloud.com/). You'll find the API keys in the Access Management section.

2. Start the `pelion/device-simulator` container image replacing `CLOUD_SDK_API_KEY` with your key:

    ```
    docker run --name pelion-demo -p 8888:8888 -e CLOUD_SDK_API_KEY=<YOUR_PELION_API_KEY> pelion/virtual-demo:1.1
    ```
    
3. Point your web browser to [http://localhost:8888](http://localhost:8888) to access the user interface of the simulator.

4. Note the deviceID and find the device in your device list on the Pelion Device Management Portal. You can now browse resources and see the data updates from the virtual device in real time.

Kill the demo with CTRL-C. The docker container running this instance of the demo will be suspended. You can choose to come back to this same container and resume it at a later date (the deviceID will be preserved), or you can issue a fresh `docker run` command to create a new device instance and a new device in your device directory.
```
docker restart pelion-demo
docker attach pelion-demo
```

## Performing firmware update
The docker image comes already pre-configured with the necessary tools to perform a firmware update. In particular, [manifest-tool](https://github.com/PelionIoT/manifest-tool) is included, to sign and produce a firmware manifest, ready to be uploaded to Pelion portal to start an update campaign. 

> For more information about Pelion Device Management update, please consult our [documentation page.](https://developer.pelion.com/docs/device-management/current/updating-firmware/index.html)

Let's change the firmware code running on the simulator, to simulate a code change and subsequent firmware update:

1. Make sure the simulator container is running and note the `'Container ID'` column:

    ```
    $ docker ps
    CONTAINER ID   IMAGE                     COMMAND                  CREATED          STATUS          PORTS                    NAMES
    b7b30f49982c   pelion/virtual-demo:1.1   "python3 sim-webapp.…"   23 minutes ago   Up 23 minutes   0.0.0.0:8888->8888/tcp   recur
    sing_jennings
    ```

2. Access a shell inside the container:
    ```
    $ docker exec -it b7b30f49982c bash
    ```
3. Switch to the main program source directory:
    ```
    $ cd /build/mbed-cloud-client-example/
    ```
4. Use '`vi main.cpp`', then go to `line :273` and insert the following code to simulate a code change:
    ```
    for (int i=0; i<5; i++) {
        printf("!! new firmware !! \n");
    }
    ```
    Save and exit.
5.  Compile and build the new firmware:
    ```
    $ make -C __x86_x64_NativeLinux_mbedtls/ mbedCloudClientExample
    ```
6. Upon completion a new firmware binary is generated:
    ```
    $ ls -l /build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf

    -rwxr-xr-x 1 root root 6549288 Dec 17 13:31 mbedCloudClientExample.elf
    ```
7. We now need to genarate the firmware manifest describing the update, upload it to portal to start an update campaign. The `manifest-tool` can perform all this in one step:

    > NOTE: Replace <device_id> with the ID of your device.

    ```
    root@f1428894995e:/build/mbed-cloud-client-example# manifest-dev-tool update -i <device_id> -p __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf -w
    2020-12-17 15:33:46,713 INFO FW version: 0.0.2
    2020-12-17 15:33:46,714 INFO Uploading FW image __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf
    [==================================================] 100.00%
    [==================================================] 100.00%
    2020-12-17 15:35:23,901 INFO Uploaded FW image http://firmware-catalog-media-ca57.s3.dualstack.us-east-1.amazonaws.com/0158778da56002420a014c1100000000/01767155f4d00000000000010010030f
    2020-12-17 15:35:24,758 INFO Vendor-ID: 93aac9091a8d4d8f87ce8823bd4055f1
    2020-12-17 15:35:24,759 INFO Class-ID: 9d51cd5d9f3f4d2eb2d437394c491955
    2020-12-17 15:35:25,867 INFO Uploaded manifest ID: 01767157745700000000000100100315
    2020-12-17 15:35:26,971 INFO Created Campaign ID: 017671577806000000000001001000f8
    2020-12-17 15:35:29,244 INFO Started Campaign ID: 017671577806000000000001001000f8
    2020-12-17 15:35:30,118 INFO Campaign state: publishing
    2020-12-17 15:35:36,233 INFO Campaign state: autostopped
    2020-12-17 15:35:36,234 INFO Campaign is finished in state: autostopped
    2020-12-17 15:35:36,845 INFO 1 devices successfully updated
    2020-12-17 15:35:36,846 INFO Cleaning up resources.
    2020-12-17 15:35:38,457 INFO Deleted campaign 017671577806000000000001001000f8
    2020-12-17 15:35:39,556 INFO Deleted manifest ID: 01767157745700000000000100100315
    2020-12-17 15:35:40,582 INFO Deleted FW image 0176715767b400000000000100100314
    ```

    At the console prompt of the simulator, notice the firmware update being initiated then downloaded and applied:
    ```
    [FOTA INFO] fota.c:596: Firmware update initiated.
    [FOTA DEBUG] fota.c:628: Pelion FOTA manifest is valid
    [FOTA DEBUG] fota.c:651: get manifest : curr version 0, new version 2
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 3
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA DEBUG] fota.c:555: Download Authorization requested
    [FOTA] ---------------------------------------------------
    [FOTA] Updating component MAIN from version 0.0.0 to 0.0.2
    [FOTA] Update priority 0
    [FOTA] Update size 6549336B
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
    --> (qd_resp) : {"cmd":"observe","data":"3","params":"3313/0/5700"}
    --> (qd_resp) sent.
    Resource(3313/0/5700) automatically updated. Value 3
    [I 201217 15:35:33 sim-webapp:75] got (qd_resp) msg: '{"cmd":"observe","data":"3","params":"3313/0/5700"}'
    [I 201217 15:35:33 sim-webapp:107] sent (ws) msg to 1 waiters
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 4 type: 0
    [FOTA DEBUG] fota.c:1460: Install Authorization requested
    [FOTA] Install authorization granted
    Message status callback: (3313/0/5700) Message sent to server
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    [FOTA INFO] fota.c:1346: Install authorization granted.
    [FOTA DEBUG] fota_source_profile_full.cpp:444: Reporting resource: /10252/0/2: value: 7
    Message status callback: (3313/0/5700) Message delivered
    [FOTA DEBUG] fota_source_profile_full.cpp:153: Callback for resource: /10252/0/2 status: 3 type: 0
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
    Application ready. Build at: Dec 16 2020 16:07:33
    Network initialized, registering...
    [FOTA DEBUG] fota.c:371: init start
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 1]
    [FOTA DEBUG] fota_source_profile_full.cpp:277: Announcing FOTA state is 1
    [FOTA INFO] fota.c:425: Registered MAIN component, version 0.0.2
    [FOTA DEBUG] fota.c:435: After upgrade, issuing post install actions
    [FOTA DEBUG] fota_block_device_linux.c:77: FOTA BlockDevice init file is fota_candidate
    [FOTA DEBUG] fota.c:327: install verify component name MAIN, version 2
    [FOTA DEBUG] fota.c:467: init complete
    Commander initialized and waiting for cmds...
    Client registered
    Endpoint Name: 0176715249f60000000000010016ba3a
    Device ID: 0176715249f60000000000010016ba3a
    [FOTA DEBUG] fota_event_handler.c:61: FOTA event-handler got event [type= 4]
    Message status callback: (3313/0/5700) subscribed
    ```

## Delta updates (TBD)

1. Switch to the main program source directory:
    ```
    $ cd /build/mbed-cloud-client-example/
    ```
2. Create a `firmwares/` directory to store fw artifacts:
    ```
    $ mkdir firmwares
    ```
3. Copy the current running firmware to `firmwares/` directory:
    ```
    $ cp __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf firmwares/current_fw.bin
    ```
4. Modify `main.cpp` as suggested above and produce new firmware:
    ```
    $ make -C __x86_x64_NativeLinux_mbedtls/ mbedCloudClientExample.elf
    ```
5. Copy the new firmware to `firmwares/` directory:
    ```
    $ cp __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf firmwares/new_fw.bin
    ```
6. The `firmwares/` directory should now contain the following:
    ```
    $ ls -l
    total 12792
    -rwxr-xr-x 1 root root 6549288 Dec 17 15:56 current_fw.bin
    -rwxr-xr-x 1 root root 6549336 Dec 17 15:57 new_fw.bin
    ```
7. We are now ready to generate the delta firmware using the `manifest-delta-tool`:
    ```
    $ manifest-delta-tool -c firmwares/current_fw.bin -n firmwares/new_fw.bin -o firmwares/delta-patch.bin
    
    2020-12-17 15:59:06,998 INFO Current tool version PELION/BSDIFF001
    Wrote diff file firmwares/delta-patch.bin, size 352599. Max undeCompressBuffer frame size was 512, max deCompressBuffer frame size was 222.
    ```
    
8. Upload `firmwares/delta-patch.bin` image to Pelion portal and note the firmware URL.
9. Generate the delta manifest:
    ```    
    $ manifest-dev-tool create \
    --payload-url <firmware_url> \
    --payload-path firmwares/delta-patch.yaml \
    --fw-version 0.3.0 \
    --output firmwares/delta-update-manifest.bin
    ```
10. Upload `firmwares/delta-update-manifest.bin` to Pelion portal and start an update campaign.

## Technical overview
The demo has been implemented to be run in 2 parts
1) an instance of the Pelion device management client built with your certificate that writes sensor values to the linux message queue running inside the docker container
2) a graphical representation (GUI) of a device that picks up the sensor values from the queue and displays values in a "fake device" so that conversations about managed devices can take place.

Our pre-existing docker image has a linux environment and a pre-built set of objects to run the demo. When you use the `docker run` command with your API key as an argument the scripts inside the container use the API key to retrieve a device certificate from your Pelion Device Management account and finalise the compliation of the client using your certificate. The client instance is then run, a `firstrun` file is written to the root of the docker container's linux environment, and the demo is running. The client passes values to the GUI using the linux message queue running inside the container, the the GUI is rendered on port 8888 of your local machine, the vibration values can be seen under the 3313/0/5700 resource for this device listed in the Pelion device management portal.

When you kill the demo with `CTRL-C` you are halting the docker container ´pelion-demo´ but not destroying it. When you start the demo with the supplied launch scripts you are resuming the previously halted container, this solution means that the pelion client is re-used and the same Pelion deviceID used over multiple demo sessions. If you don't use the restart command in the launch scripts and instead issue a `docker run` command then a fresh instance of the docker image will be created as a new container, which in turn means a fresh instance of the client will be executed, and a new deviceID will be issued by Pelion. This would create a growing list of stale devices in the device directory list of the Pelion portal webpage so we use the resume feature instead.

## Developing with the virtual device:
The virtual device docker image and the contents of this github repo can be used together as an environment to tweak and build your own modifications to the demo. Clone this demo to your local machine and use the following commands to mount the cloned directories inside the docker image, the result will be that any code changes you make to the repo's files on your machine can be built and executed inside the docker container.

1. Generate an API key from Pelion Device management Portal

2. Start the `pelion/device-simulator` container image from the root of the cloned repo replacing `CLOUD_SDK_API_KEY` with your key:

    ```
   docker run -it --name pelion-demo-dev -p 8888:8888 -v $(pwd):/build -e CLOUD_SDK_API_KEY=<YOUR_API_KEY> pelion/virtual-demo:1.1 bash
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

4. To build your changes you can use the sim-webapp.py python script. The first build will require all object files to be built, this can take 30+ minutes but subsiquent builds will only rebuild your modifications. At the end of each build the script adds a `firstrun` file to the sim-webapp directory to ensure further executions of the script only cause the demo to be executed and not complied again. Similarly a `certexists` file in the root of the docker container ensures that the certificate for your device is only pulled once. To kick a fresh compliation of your code changes use

``` 
rm firstrun && python3 sim-webapp.py
```

5. When the system has complied your changes and the demo is running you'll see the console output at the bash shell, and you'll find the webapp running at the localhost:8888 URL on your host machine
