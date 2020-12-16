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
    docker run --name pelion-demo -p 8888:8888 -e CLOUD_SDK_API_KEY=<YOUR_PELION_API_KEY> pelion/virtual-demo
    ```
    
3. Point your web browser to [http://localhost:8888](http://localhost:8888) to access the user interface of the simulator.

4. Note the deviceID and find the device in your device list on the Pelion Device Management Portal. You can now browse resources and see the data updates from the virtual device in real time.

Kill the demo with CTRL-C. The docker container running this instance of the demo will be suspended. You can choose to come back to this same container and resume it at a later date (the deviceID will be preserved), or you can issue a fresh `docker run` command to create a new device instance and a new device in your device directory.
```
docker restart pelion-demo
docker attach pelion-demo
```

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
   docker run -it --name pelion-demo-dev -p 8888:8888 -v $(pwd)/sim-webapp:/build/sim-webapp -v $(pwd)/mbed-cloud-client-example:/build/mbed-cloud-client-example -v $(pwd)/tools:/build/tools -e CLOUD_SDK_API_KEY=<YOUR_API_KEY> pelion/virtual-demo bash
    ```
This will create a container with the name tag "pelion-demo-dev" that is running the pelion/virtual-demo image with volume links to the sim-webapp, mbed-cloud-client-example, and tools folders on your local machine. You can use the pelion-demo-dev container name if you exit the running container and want to return to it with docker restart and resume commands.

3. You'll be at the bash shell inside the container, you now need to create the build environment to allow the demo to be rebuilt with your changes

```
cd ../mbed-cloud-client-example
mbed config root .
mbed deploy
python3 pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
cd __x86_x64_NativeLinux_mbedtls
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define.txt
cd ../../sim-webapp
```

4. To build your changes you can use the sim-webapp.py python script. The first build will require all object files to be built, this can take 30+ minutes but subsiquent builds will only rebuild your modifications. At the end of each build the script adds a `firstrun` file to the sim-webapp directory to ensure further executions of the script only cause the demo to be executed and not complied again. Similarly a `certexists` file in the root of the docker container ensures that the certificate for your device is only pulled once. To kick a fresh compliation of your code changes use

``` 
rm firstrun && python3 sim-webapp.py
```

5. When the system has complied your changes and the demo is running you'll see the console output at the bash shell, and you'll find the webapp running at the localhost:8888 URL on your host machine
