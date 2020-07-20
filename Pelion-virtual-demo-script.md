# Pelion Virtual demo script

## Introduction 

This script has been written to give an example of using the Pelion Virtual Demo as part of a demo or explaination on Pelion Device Management.
The script covers more topics that the virtual demo can show to give context and some scene-setting about features that the virtual demo can support and where it can't be used. 
Sections in **bold text** highlight where the virtual demo can used to demonstrate or support the conversation. 


## Demo script 

### Welcome 

Welcome to our Pelion device management demo. In this demo we’ll cover the basic device lifecycle functions that make up the core of device lifecycle management, these are the backbone of pelion device management and are the critical services that you need to have in place to ensure stable, secure, and scalable management of your (estate of, fleet of) devices in the field. We have a whole suite of services to add more functionality, more control, more insight and management that we can run through later on, but lets focus on the core device management functions to begin with (on premises, SDA, sentry, echo)  

### Security 

PDM uses a certificate based system to ensure that only authorised devices connect to your account and that all messaging between the device and the service are encrypted with a key that only your account has access to. I’ll use a virtual device here to take the place of a physical sensor, this device is a basic vibration sensor that takes a reading every 5 seconds.  


**At the bottom of the sensor you can see 3 tick boxes for the provisioning of the device to Pelion and the encryption of all messaging between the device and the service. This virtual demo has a device that I complied in from my Pelion account, when the device starts up it get server information from the certificate and uses that to make an initial bootstrap connection, when the first connection is make the device presents the certificate to the service which is then validated and validated back to the device so that there is mutual trust between the device and the service. All messaging is encrypted so its not possible to inject false certificates or monitor the connection - we prevent other, unauthorised devices from pushing untrusted data into your IoT system.**


### Device ID 

**When the device first connects to Pelion it is assigned a DeviceID, we’ve shown the last 6 digits of the ID on the device so that I can easily find it in my list of devices. This ID is sticky, it will remain with the device throughout its life so that you have consistent device management. If you want to decommission and re-use the device you can simply reload a fresh certificate from your account and the device will re-bootstrap on its next start up and receive a fresh ID.** 

### Pelion device management portal 

You can see a set of stats on the Pelion device management dashboard that give you a brief overview on the current status and recent history of your devices. On my dashboard you can see that I’ve got a handful of demo devices associated with my account, I’ve used my certificates to bootstrap the devices. When I click into the Device Directory view I can see each of my devices listed - you can see the device ID for my virtual device here and clicking on that entry allows me to see the device’s current status. Its registered and active, it first connected on X, and when I click on the resources tab I can see all of the information that the device makes available through pelion.  

### LWM2M 

We use the lightweight machine to machine LWM2M protocol in pelion, the protocol specifies how information should be grouped and shared so its really easy to work with devices that use the protocol. All device specific information like the manufacturer, the model number, the serial number is here in the 3/ section, I’m using a development certificate for this virtual device so these fields aren’t populated, this is where your details would appear. LWM2M has predefined methods for describing all of the sensor values or actuator controls that you need for your device.   

**My demo device is a vibration sensor so sensor values will appear in the accelerometer section 3313. You can see 3313/0/5700 has the values that the virtual device is reporting. When I click the SHAKE button on the device it puts my demo into a high vibration mode and so you can see the numbers coming through to Pelion. LWM2M has all of the hooks to allow alerts and thresholds to be set so that you can get automatic notifications if out of range values are detected so that you IoT system can react urgently.** 

### REST API and the admin portal

All of the device status and management control functionality that you can see in this website portal is being performed over Pelion’s REST API, the REST API is the method that server side applications use to access Pelion Device Management and the devices in your IoT system. Our admin portal provides everything you need to manage your network of devices, where you want to use PDM to integrate IoT functionality directly into your systems you can call the REST API directly and receive update notifications from the service. Message flow over the REST API is secured with an API key just as the messaging between Pelion and the device is secured with a certificate so you can be confident that your data remains secure throughout the flow of messages.  

### Firmware update 

On the device management menu bar you can see firmware updates, access management, and team configuration. Firmware update is where you can build campaigns to roll new software out to your devices giving you all of the enterprise software update tools that you’ll want to have available when you’re managing devices at scale. All of the firmware updates are validated down at the device end both for integrity and non corruption of the update before its installed, but also for validity of the sender to insure fake firmware updates can’t be injected or spoofed onto your devices that might create a security vulnerability.  

### Final summary 

Pelion sits between your IoT application and the end device, it enables you to run a trusted, secure, management fleet of devices where the tools are in place to deal with the realities of bringing devices on board, keeping them up to date and patched with latest security fixes, secure the transmission right through from the end device to your IoT application and back again when control is needed. 

 
