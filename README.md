# Virtual demo for Pelion 
[![katacoda-button](images/katacoda-button.png)](https://www.katacoda.com/cvasilak/scenarios/pelion-device-simulator)

![Screenshot of demo and browser](images/BrowserScreenShot.png)

## Introduction

The virtual demo for Pelion is a software only device that connects to the Pelion Device Management service so that the service can be used and demonstrated without the requirement for any device hardware. The project was specifically created to help discussion of high-level device management topics with the constraints of travel due to the Covid-19 pandemic but can be used any time that a screen share demo would be useful when a face to face physical demo isn't possible.

An example demo walkthrough script is provided that gives an example of where the virtual demo can be used:
[Pelion-virtual-demo-script.md](Pelion-virtual-demo-script.md)

To use this demo you'll need to have a Pelion Device Management account, visit the [Pelion Device Management](portal.mbedcloud.com) site to sign up for an account if you don't already have one.

Running the demo will register a device to your account and allow you to see and write values to and from the device management service in real-time via your Internet link.

**Note** This demo is great for:

* Demoing a connected device on a computer so that you can explain IoT device management without the overhead of configuring and running up physical hardware
* Running a real Pelion Device Management demo in a video call screen share
* Quickly and easily connecting a device so that you can experiment with the Pelion Device Management Portal in real-time
* Testing out the FOTA firmware update tools that Pelion Device Management provides
* Connecting a device as part of your work to implement and test the REST API for Pelion Device Management. Pelion doesn't know that this is a virtual demo device

The demo is *not intended* to be used as a:

* Starter template for a constrained, low power IoT device. The Linux platform that we use to be able to demo the device on a computer is too heavyweight for a constrained hardware device. We suggest you apply the concepts explained here to an application running on an mbed enabled board.
* Method of running firmware update management for Linux IoT devices. The firmware update method demoed here is for single-application updates. When you're working with real Linux IoT devices you should implement update methods for Linux packages and any application libraries that you use. We suggest that you take a look at Pelion Edge if you're implementing that type of product.

## Updates
* **V1.2 Oct 2021**
    * Support for [factory-provisioning mode](#factory-provisioning)
	* Mbed cloud client updated to v4.11.1
* **V1.1 Feb 2021**
	* Mbed cloud client updated to v4.7.0
	* Firmware update added including delta update generation
	* Multi-sensor type support. Inclusion of an option to build a counter-type device with the same functionality as mbed cloud client example

## Quickstart:
The virtual demo can be run using a docker container without any code changes. You can run the environment locally on your machine or you can use our walkthrough on [Katakoda](https://www.katacoda.com/cvasilak/scenarios/pelion-device-simulator). 

If you're running the demo locally then docker needs to be installed on your host machine [https://www.docker.com/products/docker-desktop](https://www.docker.com/products/docker-desktop). A docker image has been prepared and uploaded to the docker hub site, the commands below will pull the pelion/virtual-demo image from docker hub and run an instance on your machine

1. Generate an Access key from [Pelion Device management Portal](https://portal.mbedcloud.com/). You'll find the Access keys in the Access Management section.

2. Start the `pelion/virtual-demo` container image replacing `CLOUD_SDK_API_KEY` with your key:

    ```
    docker run --name pelion-demo -p 8888:8888 -e CLOUD_SDK_API_KEY=<YOUR_PELION_API_KEY> pelion/virtual-demo
    ```
    
    > NOTE: You can also specify the default Pelion Cloud address to use using `CLOUD_URL` environment variable, if omitted the default '`api.us-east-1.mbedcloud.com`' production address is used.

3. Point your web browser to [http://localhost:8888](http://localhost:8888) to access the user interface of the virtual device.

4. Note the deviceID and find the device in your device list on the Pelion Device Management Portal. You can now browse resources and see the data updates from the virtual device in real-time.

Kill the demo with CTRL-C. The docker container running this instance of the demo will be suspended. You can choose to come back to this same container and resume it at a later date (the deviceID will be preserved), or you can issue a fresh `docker run` command to create a new device instance and a new device in your device directory.
```
docker restart pelion-demo
docker attach pelion-demo
```

## Specifying the device type and sensor update interval
The default device type simulated when running the virtual-demo is a vibration sensor(_/3313/0/5700_). If you want to build a counter device(_/3200/0/5501_) or temperature(_/3303/0/5700_), then append either `-e SENSOR=counter` or `-e SENSOR=temperature` respectively when invoking the docker run instruction. Further, the default sensor update interval is set to **5 seconds**, but you can override it by appending the `-e INTERVAL=secs` parameter.

```
docker run --name pelion-demo -p 8888:8888 -e CLOUD_SDK_API_KEY=<YOUR_PELION_API_KEY> -e SENSOR=counter pelion/virtual-demo
```

## Performing firmware update
The docker image comes already pre-configured with the necessary tools to perform a firmware and delta update. In particular, build tools and the suite of tools from [manifest-tool](https://github.com/PelionIoT/manifest-tool) are included to a) create a delta firmware image and b) sign and produce a firmware manifest ready to be uploaded to Pelion portal to start an update campaign. 

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


4. Let's alter the emitted simulated value sent by the virtual demo to be multiplied by 1000. Using `vi` editor, open `source/blinky.cpp`, navigate to line `:308` and add the following line:

    ```
    _sensed_count = _sensed_count * 1000;
    ```

    Save and exit.


5.  Bootstrap a new development container of the virtual demo image to use it for building our new firmware. Notice that we local mount the credential sources and the manifest configuration we copied in step 3 above so that they are available from inside the new container: 
    ```
    $ docker run -it --name pelion-demo-dev \
     -v $(pwd)/source/blinky.cpp:/build/mbed-cloud-client-example/source/blinky.cpp \
     -v $(pwd)/mbed_cloud_dev_credentials.c:/build/mbed-cloud-client-example/mbed_cloud_dev_credentials.c \
     -v $(pwd)/update_default_resources.c:/build/mbed-cloud-client-example/update_default_resources.c \
     -v $(pwd)/.manifest-dev-tool/:/build/mbed-cloud-client-example/.manifest-dev-tool/ \
     pelion/virtual-demo bash
    ```
    > NOTE: If planning to perform delta patch update, we need to local mount the 'firmwares/' directory too. Simply append `-v $(pwd)/firmwares/:/build/mbed-cloud-client-example/firmwares` in the command above.


You can now choose either to perform a full firmware image update or a delta patch. Follow the appropriate section below.

### Option 1 - Full Image Update

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
    
    -rwxr-xr-x. 1 root root 7115152 Oct 11 07:12 mbedCloudClientExample.elf
    ```
4. We now need to generate the firmware manifest describing the update, upload it to the portal and start an update campaign. The `manifest-tool` can conveniently perform all this in one step. Simple execute:

    ```
    $ manifest-dev-tool update -p __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf -w -n -v 0.2.0

    INFO FW version: 0.2.0
    INFO Uploading FW image __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf
    INFO Uploaded FW image http://firmware-catalog-media-ca57.s3.dualstack.us-east-1.amazonaws.com/uPHX9cwDMtxNeLKde4bJW4
    INFO Vendor-ID: da64439cf2d3433c8a19d844b8e56dc1
    INFO Class-ID: 67ebb37962224e30a0ed38e5ab5f54d4
    INFO Attention: When updating Mbed OS devices, candidate features must match the device's bootloader features. Incompatibility may result in damaged devices.
    INFO Created manifest in v3 schema for full update campaign
    INFO Uploaded manifest ID: 017c6e213ce900000000000100100153
    INFO Created Campaign ID: 017c6e213fae00000000000100100363
    INFO Started Campaign ID: 017c6e213fae00000000000100100363
    INFO Campaign state: publishing
    INFO Campaign is finished in state: autostopped
    INFO ----------------------------
    INFO     Campaign Summary 
    INFO ----------------------------
    INFO  Successfully updated:   1
    INFO  Failed to update:       0
    INFO  Skipped:                0
    INFO  Pending:                0
    INFO  Total in this campaign: 1
    ```

    At the console prompt of the virtual demo, notice the firmware update being initiated, then downloaded and applied. Upon completion the device reboots with the new firmware:
    ```
    Firmware download requested (priority=0)
    Updating component MAIN from version 0.0.0 to 0.2.0
    Update priority 0
    Update size 7115144B
    Downloading firmware. 0%
    Downloading firmware. 5%
    Downloading firmware. 10%
    Downloading firmware. 15%
    Downloading firmware. 20%
    Downloading firmware. 25%
    Downloading firmware. 30%
    Downloading firmware. 35%
    Downloading firmware. 40%
    Downloading firmware. 45%
    Downloading firmware. 50%
    Downloading firmware. 55%
    Downloading firmware. 60%
    Downloading firmware. 65%
    Downloading firmware. 70%
    Downloading firmware. 75%
    Downloading firmware. 80%
    Downloading firmware. 85%
    Downloading firmware. 90%
    Downloading firmware. 95%
    Downloading firmware. 100%
    Resource(3313/0/5700) automatically updated. Value 5
    Firmware install authorized
    [FOTA] Successfully installed MAIN component
    In single-partition mode.
    Creating path ./pal
    Start Device Management Client
    Using hardcoded Root of Trust, not suitable for production use.
    Starting developer flow
    Developer credentials already exist, continuing..
    Generating random from /dev/random, this can take a long time!
    Finished generating random from /dev/random.
    Commander initialized and waiting for cmds...
    Application ready. Build at: Oct  8 2021 11:20:59
    PDMC version 4.11.0
    Network initialized, registering...
    ```

## Option 2 - Delta updates


1. Switch to the main program source directory:
    ```
    $ cd /build/mbed-cloud-client-example/
    ```
2. Assuming you've modified `source/blinky.cpp` as suggested above, we can now proceed and produce the new firmware:
    ```
    $ make -C __x86_x64_NativeLinux_mbedtls/ mbedCloudClientExample.elf
    ```
3. Copy the new firmware to `firmwares/` directory:
    ```
    $ cp __x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf firmwares/new_fw.bin
    ```
4. The `firmwares/` directory should now contain both the new firmware(`new_fw.bin`) and the currently running one(`current_fw.bin`):
    ```
    $  ls -l firmwares/
    
    total 13904
    -rwxr-xr-x. 1 1000 1000 7115152 Oct 11 07:11 current_fw.bin
    -rwxr-xr-x. 1 root root 7115152 Oct 11 07:12 new_fw.bin
    ```
5. We are now ready to generate a delta firmware using the `manifest-delta-tool`:
    ```
    $ manifest-delta-tool -c firmwares/current_fw.bin -n firmwares/new_fw.bin -o firmwares/delta-patch.bin

    2021-10-11 07:13:21,572 INFO Current tool version PELION/BSDIFF001
    Wrote diff file firmwares/delta-patch.bin, size 180745. Max undeCompressBuffer frame size was 512, max deCompressBuffer frame size was 37.
    ```
    
    If we list the directory contents, we can verify the producing of the `delta-patch.bin` firmware. Notice the significant shrinkage in size, from 6.8MB of a full firmware image, down to a delta of 177K!

    ```
    ls -l firmwares/delta-patch.bin

    -rw-r--r--. 1 root root  180745 Oct 11 07:13 delta-patch.bin
    ```

6. Start the update campaign
    ```
    $ manifest-dev-tool update -p firmwares/delta-patch.bin -w -n -v 0.2.0

    INFO FW version: 0.2.0
    INFO Uploading FW image firmwares/delta-patch.bin
    INFO Uploaded FW image http://firmware-catalog-media-ca57.s3.dualstack.us-east-1.amazonaws.com/iXHsjUj5zJbZ9KTMS1HSXi
    INFO Vendor-ID: da64439cf2d3433c8a19d844b8e56dc1
    INFO Class-ID: 67ebb37962224e30a0ed38e5ab5f54d4
    INFO Attention: When updating Mbed OS devices, candidate features must match the device's bootloader features. Incompatibility may result in damaged devices.
    INFO Created manifest in v3 schema for delta update campaign
    INFO Uploaded manifest ID: 017c6e36a8aa00000000000100100161
    INFO Created Campaign ID: 017c6e36ab5d0000000000010010036a
    INFO Started Campaign ID: 017c6e36ab5d0000000000010010036a
    INFO Campaign state: publishing
    INFO Campaign is finished in state: autostopped
    2021-10-11 07:17:35,404 INFO ----------------------------
    2021-10-11 07:17:35,414 INFO     Campaign Summary 
    2021-10-11 07:17:35,414 INFO ----------------------------
    2021-10-11 07:17:35,415 INFO  Successfully updated:   1
    2021-10-11 07:17:35,415 INFO  Failed to update:       0
    2021-10-11 07:17:35,416 INFO  Skipped:                0
    2021-10-11 07:17:35,416 INFO  Pending:                0
    2021-10-11 07:17:35,416 INFO  Total in this campaign: 1
    ```

    At the console prompt of the virtual demo, notice that the device logs the downloading of the new firmware and the successful delta update:
    ```
    Firmware download requested (priority=0)
    Updating component MAIN from version 0.0.0 to 0.2.0
    Update priority 0
    Delta update. Patch size 180745B full image size 7115152B
    Downloading firmware. 0%
    Downloading firmware. 7%
    Downloading firmware. 17%
    Downloading firmware. 24%
    Downloading firmware. 25%
    Downloading firmware. 32%
    Downloading firmware. 41%
    Downloading firmware. 50%
    Downloading firmware. 55%
    Downloading firmware. 62%
    Downloading firmware. 68%
    Downloading firmware. 77%
    Downloading firmware. 87%
    Downloading firmware. 96%
    Downloading firmware. 100%
    [FOTA] Successfully installed MAIN component
    In single-partition mode.
    Creating path ./pal
    Start Device Management Client
    Using hardcoded Root of Trust, not suitable for production use.
    Starting developer flow
    Developer credentials already exist, continuing..
    Generating random from /dev/random, this can take a long time!
    Finished generating random from /dev/random.
    Commander initialized and waiting for cmds...
    Application ready. Build at: Oct  8 2021 11:20:59
    PDMC version 4.11.0
    Network initialized, registering...
    ```

## Verifying Firmware Update

Once the firmware update is completed, the Pelion update campaign dashboard should display the successful completion:

![Campaign Dashboard](images/portal-campaign-dashboard.png)

and the device should display the new firmware version (0.2.0 in our case):

![Firmware Version](images/device-fw-version.png)

Notice now that the vibration sensor values sent by the device are indeed multiplied by 1000 marking the successful firmware update!

**Congratulations !**

## Factory Provisioning

All previous examples showcase virtual-demo running in development mode where development certificates are generated and used to connect and authenticate to Pelion device management platform. For production deployments though, the factory provisioning process should be followed. To aid in learning, virtual-demo also supports this mode too. The container image conveniently includes all the required artifacts and services required ([simulated TPM](https://sourceforge.net/projects/ibmswtpm2/), [PARSEC](https://parallaxsecond.github.io/parsec-book/)), assembled together to support a 'simulated' factory provisioning process. 

> NOTE: This section assume you have basic knowledge of how the factory provisioning process works and want to apply this knowledge with the virtual-demo. If not, head over to our [documentation](https://developer.pelion.com/docs/device-management/current/provisioning-process/index.html) page for a brief introduction. Also, worth looking is our end-to-end [factory provisioning tutorial](https://developer.pelion.com/docs/device-management/current/connecting/factory-provisioning-to-firmware-update.html).

### Quickstart

1. Start virtual-demo in 'factory-provisiong' mode.
    ```
    docker run -it --name pelion-demo -p 8888:8888 -e FACTORY_PROVISIONING_MODE=ON pelion/virtual-demo
    ```
    
    You should see the [factory-configurator-client](https://developer.pelion.com/docs/device-management-provision/latest/ft-demo/building-demo.html) starting up, then blocks waiting for connections from the [factory-configurator-utility](https://developer.pelion.com/docs/device-management-provision/latest/fcu/index.html):
    ```
    INFO:launching in [factory-provisioning] mode..

    INFO:starting 'tpm_server'..
    INFO:setting up authentication to 'tpm_server'..
    INFO:starting 'parsec'..
    Application ready. Build at: Oct  8 2021 11:17:07
    In single-partition mode.
    Creating path psa/
    [INFO][fcc ]: factory_configurator_client.c:84:fcc_init:===> 
    [INFO][fcc ]: fcc_output_info_handler.c:472:fcc_init_output_info_handler:===> 
    [INFO][fcc ]: fcc_output_info_handler.c:479:fcc_init_output_info_handler:<=== 
    [INFO][fcc ]: factory_configurator_client.c:106:fcc_init:<=== 

    Trying receive interface ...

    Client IP Address and Port :  172.17.0.2:8888

    Client is waiting for incoming connection...
    Factory flow begins...
    ```
2. Assumming you have correctly [setup factory-configurator utility](https://developer.pelion.com/docs/device-management-provision/latest/fcu/index.html) in your local machine, you can proceed to inject the configuration to the virtual-demo adjusting accordingly the parameters with your details:
    ```
    python3 ft_demo/sources/ft_demo.py inject --endpoint-name=virtual-demo --serial-number=10101 tcp --ip=localhost --port=8888
    ```

    Once done, the 'factory-configurator-client' should report a successfull provisioning and exit.

    ```
    Factory flow begins...
    [INFO][fcc ]: factory_configurator_client.c:152:fcc_storage_delete:===> 
    [INFO][fcc ]: key_config_manager.c:75:kcm_finalize:===> 
    [INFO][fcc ]: key_config_manager.c:96:kcm_finalize:<=== 
    [INFO][fcc ]: storage_psa.cpp:822:storage_factory_item_delete:===> item name = mbed.BootstrapDevicePrivateKey len = 30
    [INFO][fcc ]: key_config_manager.c:37:kcm_init:===> 
    [INFO][fcc ]: storage_psa.cpp:256:storage_build_item_name:<=== 
    [INFO][fcc ]: storage_psa.cpp:520:storage_rbp_read:===> item name =  saved_time
    [INFO][fcc ]: storage_psa.cpp:256:storage_build_item_name:<=== 
    [INFO][fcc ]: storage_psa.cpp:520:storage_rbp_read:===> item name =  last_time_back
    [INFO][fcc ]: storage_psa.cpp:256:storage_build_item_name:<=== 
    [INFO][fcc ]: key_config_manager.c:66:kcm_init:<=== 
    [INFO][fcc ]: storage_psa.cpp:762:storage_get_item_name_for_deleting:===> item name = mbed.BootstrapDevicePrivateKey len = 30
    [INFO][fcc ]: storage_psa.cpp:256:storage_build_item_name:<=== 
    [INFO][fcc ]: factory_configurator_client.c:163:fcc_storage_delete:<=== 
    Storage is erased
    [INFO][fcc ]: fcc_bundle_handler.c:279:fcc_bundle_handler:===> encoded_blob_size = 2134
    [INFO][fcc ]: fcc_output_info_handler.c:484:fcc_clean_output_info_handler:===> 
    [INFO][fcc ]: fcc_output_info_handler.c:496:fcc_clean_output_info_handler:<=== 
    [INFO][fcc ]: key_config_manager.c:37:kcm_init:===> 
    [INFO][fcc ]: key_config_manager.c:66:kcm_init:<=== 
    [INFO][fcc ]: factory_configurator_client.c:289:fcc_is_factory_disabled:===> 
    [INFO][fcc ]: storage_psa.cpp:520:storage_rbp_read:===> item name =  factory_done
    [INFO][fcc ]: storage_psa.cpp:256:storage_build_item_name:<=== 
    [INFO][fcc ]: factory_configurator_client.c:294:fcc_is_factory_disabled:pal_status:-63, factory_disable_flag:0

    [INFO][fcc ]: factory_configurator_client.c:300:fcc_is_factory_disabled:<=== 
    [INFO][fcc ]: fcc_bundle_handler.c:360:fcc_bundle_handler: key name SchemeVersion
    [INFO][fcc ]: fcc_bundle_handler.c:360:fcc_bundle_handler: key name Keys
    [INFO][fcc ]: key_config_manager.c:111:kcm_item_store:===> 
    ...
    ...
    ...
    [INFO][fcc ]: factory_configurator_client.c:143:fcc_finalize:<=== 
    Successfully completed factory flow
    INFO:[factory-provisioning] completed successfully, please start virtual-demo with 'docker start -a pelion-demo'..
    ```

3. You can now start virtual-demo in production-mode:
    ```
    docker start -a pelion-demo
    ```

    You should see virtual-demo starting up, connecting to the platform:

    ```
    INFO:launching in [factory-provisioning] mode..

    INFO:starting 'tpm_server'..
    INFO:starting 'parsec'..
    INFO:[factory-provisioning] has already occurred, starting 'virtual-demo' in production mode..
    DEBUG:Using selector: EpollSelector
    DEBUG:Using selector: EpollSelector
    In single-partition mode.
    Creating path psa/
    Start Device Management Client
    Commander initialized and waiting for cmds...
    Application ready. Build at: Oct  6 2021 12:30:07
    PDMC version 4.11.0
    Network initialized, registering...
    Add sub components
    Client registered
    Endpoint Name: virtual-demo
    Device ID: 017be3e8875cd285fba5223800000000
    ```

### Notes on firmware update

The process of firmware updates in production-mode is similar to development-mode with some subtle differences:

1. Prior to compiling the new firmware you should:
    
    a) Comment `DMBED_CONF_APP_DEVELOPER_MODE` in [define.txt](https://github.com/PelionIoT/virtual-demo-for-pelion/blob/master/mbed-cloud-client-example/define.txt#L39), so the firmware is built in production-mode.

    b) Pass the `PARSEC_TPM_SUPPORT=ON` and use [define_linux_psa.txt](https://github.com/PelionIoT/virtual-demo-for-pelion/blob/master/mbed-cloud-client-example/define_linux_psa.txt) as parameter to `DEXTERNAL_DEFINE_FILE`

    In a nutshell, the following series of commands should be used:
    ```
    $ # remove the existing (dev) firmware build artifacts inside the container
    $ rm -rf __x86_x64_NativeLinux_mbedtls/Debug/

    $ # deploy dependencies
    $ python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
    
    $ # generate cmake files
    $ cd __x86_x64_NativeLinux_mbedtls
    $ cmake -G "Unix Makefiles" -DPARSEC_TPM_SE_SUPPORT=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_linux_psa.txt
    
    $ # start the build
    $ make mbedCloudClientExample.elf
    
2. Since we are not using the `manifest-dev-tool` in production-mode to auto-create the underlying dev certs and config files, we need to manually create and sign the manifest, upload the firmware and manifest to Pelion portal and start the update campaign. Follow the instructions in our [documentation](https://developer.pelion.com/docs/device-management/current/connecting/update-your-device-s-firmware-image.html) page for more information on applying firmware updates in production-mode.

3. The Linux firmware binary generated by the build is rather large(40+ MB), so we suggest to opt for a Delta update directly. The precompiled production running firmware is located in `/build/mbed-cloud-client-example-production/` inside the docker image to use it as the parameter ('`-c`') with the `manifest-delta-tool`.  


## Technical overview
The demo has been implemented to be run in 2 parts:
1) an instance of the Pelion device management client built with your certificate that writes sensor values to the Linux message queue running inside the docker container
2) a graphical representation (GUI) of a device that picks up the sensor values from the queue and displays values in a "fake device" so that conversations about managed devices can take place.

Our pre-existing docker image has a Linux environment and a pre-built set of objects to run the demo. When you use the `docker run` command with your Access key as an argument the scripts inside the container use the Access key to retrieve a device certificate from your Pelion Device Management account and finalise the compilation of the client using your certificate. The client instance is then run, a `firstrun` file is written to the root of the docker container's Linux environment and the demo starts running. The client passes values to the GUI using the message queue running inside the container, the GUI is rendered on port 8888 of your local machine, the vibration values can be seen under the 3313/0/5700 resource for this device listed in the Pelion device management portal.

When you kill the demo with `CTRL-C` you are halting the docker container ´pelion-demo´ but not destroying it. When you start the demo with the supplied launch scripts you are resuming the previously halted container, this solution means that the pelion client is re-used and the same Pelion deviceID used over multiple demo sessions. If you don't use the restart command in the launch scripts and instead issue a `docker run` command then a fresh instance of the docker image will be created as a new container, which in turn means a fresh instance of the client will be executed, and a new deviceID will be issued by Pelion. This would create a growing list of stale devices in the device directory list of the Pelion portal webpage so we use the resume feature instead.

## Developing with the virtual demo:
The virtual demo docker image and the contents of this github repo can be used together as an environment to tweak and build your own modifications to the demo. Clone this demo to your local machine and use the following commands to mount the cloned directories inside the docker image, the result will be that any code changes you make to the repo's files on your machine can be built and executed inside the docker container.

1. Generate an Access key from Pelion Device management Portal

2. Start the `pelion/virtual-demo` container image from the root of the cloned repo replacing `CLOUD_SDK_API_KEY` with your key and add `-e SENSOR=counter` if you want to build a counter device instead of the default vibration sensor:

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

4. To build your changes you can use the sim-webapp.py python script. The first build will require all object files to be built, this can take 30+ minutes but subsequent builds will only rebuild your modifications. At the end of each build the script adds a marker `firstrun` file to the sim-webapp directory to ensure further executions of the script only cause the demo to be executed and not compiled again. Similarly, a `certexists` file in the root of the docker container ensures that the certificate for your device is only pulled once. To kick a fresh compilation of your code changes execute the following:
``` 
rm -f firstrun && python3 sim-webapp.py
```

5. When the system has compiled your changes and the demo is running you'll see the console output at the bash shell, and you'll find the webapp running at the [localhost:8888](http://localhost:8888) URL on your host machine.
