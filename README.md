# Condition Monitoring Profile on STM32 (STM32F103RET6)
To develop a stable embedded system, We use Real Time Operating System(RTOS) in our Projects. The type of RTOS is SEGGER’S embOS. The most important concept of embOS, as a RTOS, is TASKs. TASKs are separated program sections. Each TASK has a  Priority level due to its importance for system and multiple tasks running at the same time. 
**Note:** You can download the related embOS sample project for STM32 (Cortex-M3 core) chips and also see the related examples from [here[DOWNLOAD_LINK]].
We use two concepts for our projects, DAS and Gateway.
## What is DAS?
in our project, every board is a DAS (Data Acquisition System). it has I/O and can connect to Sensors and Actuators. The main job of DAS is to collect environmental data through sensors and apply necessary changes using Actuators.  
## What is Gateway?
In our project, a Gateway is a higher-level DAS. The only difference is that It has a network module to communicate with different servers (e.g., MQTT, FTP). Gateway’s main job is to collect system data and sends them to the cloud, and receive commands from server to apply them to the system.
## How do they communicate?
Gateway and DAS nodes communicate with each other using RS485/LoRa protocols. 
> [!NOTE]
> In this project, we only have one of these concepts: **Gateway**.

![stm32_condition_monitoring_sina](https://github.com/user-attachments/assets/893edc87-c7d4-45c5-8f14-f1cce4951842)


## Platform Layers
1-	Perception Layer
  -	communicate with environment using Sensors & Actuators

2-	Network Layer
  -	update Perception Layer’s data to Server, and Vice Versa

3-	Application Layer
  -	Store the system data into cloud database, represent it to the client through web application or any other software.
## Platform Limitations
For Each Gateway we have at most:

1-	256 Digital Inputs (512 ASCII Characters)

2-	256 Digital Outputs (512 ASCII Characters)

3-	256 Sensors which could be ASCII/RTU type
  - each Sensor has at least 256 ASCII Characters for commands
  - each Sensor has at least 256 ASCII Characters for responses
   
4-	32 Analog Inputs (64 ASCII Characters)

5-	32 Analog Outputs (64 ASCII Characters)

6-	32 PWMs (256 ASCII Characters)

![sht3x](https://github.com/user-attachments/assets/f69d8439-ea3c-4746-a91b-c494a8cc5a95)

## Current Project Configuration
-	Main Chip: STM32 F103RET6 (Cortex-M3 core)
-	Network Module: Quectel M65/M66
-	Debugging: RS232
-	IDE: STM32CubeIDE	
-	Programming language: C 
## Current project Hardware Limitations
1-	3 I/O Expanders
  -	in our project, “I/O Expander” is a board which has 24 Inputs and 24 Outputs. It can be connected to Gateway to expand I/O. (what is I/O Expander? more information [here[DOWNLOAD_LINK]])

2-	8 Digital Inputs + 24 I/O Expander Digital Input (64 ASCII Characters)

3-	8 Digital Outputs + 24 I/O Expander Digital Outputs + 1 Relay (66 ASCII Characters)

4-	2 default Sensors which are ASCII type
  -	each Sensor has at least 48 ASCII Characters for commands
  -	each Sensor has at least 48 ASCII Characters for responses


## System TASKs
**1.	IO (Digital Input/Output + Sensor Modbus Controlling)**

**2.	Operator**
   
**3.	Quectel**
   
**4.	Separator**

Each task explanation (in every single interval):

1-	IO TASK:
  -	writes/reads system I/O
  -	reads Chip’s temperature
  -	creates a buffer which holds I/O variables.
  -	updates new ASCII/RTU Sensors data ( if there is any available )
  -	sends commands to sensors and receives the responses.
  -	updates a buffer which holds the system variables.

2-	Operator TASK:
  -	As a GATEWAY:
    -	parse received commands from MQTT server
    -	apply changes to the system
    -	collect system data.

3-	Quectel TASK:
  -	parse Quectel responses
  -	apply changes to the system
  -	in the first time running:
    -	Configures the module
    -	Configures the MQTT Server
    -	Configures the NTP Server for datetime
    -	Opens the Connection of the MQTT Server then connects and subscribes to the related topics.
  -	communicates with the MQTT Server (publish/receive data).

4-	Separator TASK:
  -	separates the received data from Quectel.

## How do we update our nodes firmware?
**FOTA (Firmware Over The Air)** is the process of updating IoT devices without calling them back to garage. In this process, Gateway connects to server and receives the latest updates, then writes the received data on FOTA section of its own flash memory (second-half of internal flash). Next, Gateway starts programming process through Bootloader Application. This application reads data from mentioned flash memory and writes on application section of flash of the main chip(first-half of internal flash). Finally, Bootloader finishes and jumps to new Main Application. 
You can see the related sample codes for STM32 (Cortex-M3 core) from [here[DOWNLOAD_LINK]].
