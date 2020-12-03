# Virtual demo for Pelion 
The virtual demo for Pelion is a software only device that connects to the Pelion Device Management service so that the service can be used and demonstrated without the requirement for any device hardware. The project was specifically created to help discussion of high level device management topics with the constraints of travel due to the Covid-19 pandemic but can be used any time that a screen share demo would be useful when a face to face physical demo isn't possible.

An example demo walkthrough script is provided that gives an example of where the virtual demo can be used:
[Pelion-virtual-demo-script.md](Pelion-virtual-demo-script.md)

![Pelion Virtual Demo GUI](images/PDMvirtualDemo.jpg)

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
docker resume pelion-demo
docker attach pelion-demo
```

## Developing with the virtual device:
The virtual device docker image and the contents of this github repo can be used together as an environment to tweak and build your own modifications to the demo. Clone this demo to your local machine and use the following commands to mount the cloned directories inside the docker image, the result will be that any code changes you make to the repo's files on your machine can be built and executed inside the docker container.

1. Generate an API key from Pelion Device management Portal

2. Start the `pelion/device-simulator` container image from the root of the cloned repo replacing `CLOUD_SDK_API_KEY` with your key:

    ```
   docker run -it --name pelion-demo-dev -p 8888:8888 -v $(pwd)/sim-webapp:/build/sim-webapp -v $(pwd)/mbed-cloud-client-example:/build/mbed-cloud-client-example -v $(pwd)/tools:/build/tools -e CLOUD_SDK_API_KEY=<YOUR_API_KEY> pelion/virtual-demo bash
    ```
    
3. You'll be at the bash shell inside the container, you now need to create the build environment to allow the demo to be rebuilt with your changes

```
cd ../mbed-cloud-client-example
mbed config root .
mbed deploy
python pal-platform
cmake
cd ../sim-webapp
```

4. To build your changes you can use the sim-webapp.py python script. At the end of each build the script adds a `firstrun` file to the sim-webapp directory to ensure further executions of the script only cause the demo to be executed and not complied again. To kick a fresh compliation of your code changes use

``` 
rm firstrun && python3 sim-webapp.py
```

5. When the system has complied your changes and the demo is running you'll see the console output at the bash shell, and you'll find the webapp running at the localhost:8888 URL on your host machine
