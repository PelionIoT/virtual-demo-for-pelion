# Docker support for Windows & Mac demos

Docker can be used as a method to build and run the Pelion client functionality of this demo on a Windows or Mac machine without having to fully install Linux. The original demo was built to run in a Linux environment, the steps outlined below will help you use Docker if you prefer.

## Top level summary

* Install Docker on your machine
* Clone this repo or download as a zip and extract on your machine
* Download a Pelion device management certificte
* Use a build script file to kick off the Docker build process
* Install NPM and the Electron library to allow the GUI to run on your machine
* Use the correct demo run scripts for your environment so that the Docker-ised client is used whenever the demo is started
* See the [Pelion-virtual-demo-script.md](../Pelion-virtual-demo-script.md) file in the repo for an explaination on how the demo can be used to explain the key features of Pelion Device Management.

## Prep - all environments

1. Install docker - [https://www.docker.com/products/docker-desktop](https://www.npmjs.com/get-npm)
2. Install NPM and node.js - [https://www.npmjs.com/get-npm](https://www.npmjs.com/get-npm)
3. Clone this repo from github or download a zip of the repo using the "Code" down arrow at the top of the main repo page [https://github.com/ARMmbed/virtual-demo-for-pelion](https://github.com/ARMmbed/virtual-demo-for-pelion)
4. Download a certificate from your Pelion Device Management account by creating a new certificate or downloading an existing one from the device management portal [https://portal.mbedcloud.com/identity/certificates/list](https://portal.mbedcloud.com/identity/certificates/list)
5. Copy the `mbed_cloud_dev_credentials.c` file to the `mbed-cloud-client-example` directory in the copy of the repo on your machine
6. Use a terminal or command prompt to navigate to the root directory of this repo where `Dockerfile` exists and use the command `docker build -t peliondemo .` to build a docker image with the Pelion client using your certificate.
7. Use the command `npm install electron` at the command prompt to install the Electron NPM library required by the GUI.

### Windows

1. Run the file `init_pelion_client.docker.bat` from the `docker_scripts` directory to launch the docker container that will run your demo instance of the client.
2. Use `CTRL-C` to kill the docker container that is running. This first init process is only required once
3. For each demo session move to the webapp directory of the repo, use `launch_pelion_client.docker.bat` to resume the docker container running your client, and use `npm start` from the webapp directory running in a second command line window to start the GUI.
4. At the end of each demo session use `CTRL-C` to kill the instance of the docker container. It will still be available for the next demo session using the launch batch file.

### Mac

1. Run the file `init_pelion_client.docker.sh` from the `docker_scripts` directory to launch the docker container that will run your demo instance of the client.
2. Use `CTRL-C` to kill the docker container that is running. This first init process is only required once
3. Modify the start command in the package.json file in the webapp directory to call `./launch_pelion_client.docker.sh`
4. For each demo session run the `npm start` command from the `webapp` directory. The docker instance of your client will be run as well as the GUI.
5. At the end of each demo session use `CTRL-C` to end the npm environment. The client will be halted but will be available for the next demo session by using `npm start` again.

## Technical overview
The demo has been implemented to be run  in 2 parts
1) an instance of the Pelion device management client built with your certificate that writes sensor values to a file
2) a graphical representation (GUI) of a device that picks up the sensor values from the file and displays values in a "fake device" so that conversations about managed devices can take place.

The sensor value files are stored in the `data` directory of this repo, the docker container is configured in the init scripts to mount the data directory on your machine as a shared docker volume so the the client can write data values to your computer's file system. The GUI is configured to read the data files from your local file system, and so the demo displays the generated sensor values correctly. The application inside the docker container is writing the updated values to the pelion device client as a normal IoT device would, you'll see the values under the 3313/0/5700 resource for this device listed in the Pelion device management portal.

When you kill the demo with `CTRL-C` you are halting the docker container but not destroying it. When you start the demo with the supplied launch scripts you are resuming the previously halted container, this solution means that the pelion client is re-used and the same Pelion deviceID used over multiple demo sessions. If you don't use the resume command in the launch scripts and instead issue a `docker run` command then a fresh instance of the docker image will be created, which in turn means a fresh instance of the client will be executed, and a new deviceID will be issued by Pelion. This would create a growing list of stale devices in the device directory list of the Pelion portal webpage so we use the resume feature instead.
