<div align="right">

[![english](https://raw.githubusercontent.com/stevenrskelton/flag-icon/master/png/16/country-4x3/gb.png)](./README.md) | [![spanish](https://raw.githubusercontent.com/stevenrskelton/flag-icon/master/png/16/country-4x3/es.png)](./README_ES.md)

</div>

# Emotion cube (Health-Cube CuboLab)

This repository contains the code and schematics for an **Emotions cube** project, which uses an **Accelerometer** to meassure which face it is placed in. Allowing patients with difficulty when expresing their feelings to communicate thier degree of confort/disconfort.

<img src="https://github.com/user-attachments/assets/4acb5369-0473-4307-a8ee-d033f2667c8e" alt="Cubo Cerrado" width="400">

## Features

- **3-D printed faces and structure**
- **ESP32-WROOM-32**
- **1850 Bateries**
- **IMU and other electronic components**

<img src="https://github.com/user-attachments/assets/579a614f-7863-444f-83a4-882f781875a8" alt="Cubo Abierto" width="400">

## Installation

1. **Clone the repository**:

   Clone this repository to your local machine using:

   ```bash
   git clone https://github.com/MedialabUniovi/Healthcare_CuboLab/Proto-v2.2.2.git
   ```

2. **Set up the environment**:

   Open the project in **ArduinoIDE** or **Platform.io** and ensure you have installed the required libraries.
   
3. **Upload the code**:

   Compile and upload the code to the **ESP32-WROOM-32** microcontroller.

4. **Assemble the hardware**:

   Connect the components to the PCB, and attach embed it into the 3-D printed structure.

## Usage

1. **Charge the device**: 
   It has a "DC barrel jack" conector. It should be charged with a 10V charger.

2. **Data reception**:
   The generated data will be saved on a DataBase and accesible via http.
   
3. **Configuration**
   Once it starts charging it allows the user to acces the configuration of the cube via WiFi
   
4. **Customization**:
   Modify the code to suit your needs by adding additional sensors or improving the data logging system.

## General flowchart

The systems toggles between three states goberned which indicate whether the cube is in motion detection, charging and configuration, and only charging. Indicating the level of cahrge after disconnection.

<img src="https://github.com/user-attachments/assets/72a10f76-19d9-4fb1-bb84-207e6a488bdb" alt="General" width="400">

## Pins

| Number | Name  | Description |
|--------------|--------------|--------------|
| G33     | Vcharge       | Pin used to detect if the charger is connected       |
| G34       | Vbat       | Pin used to meassure the battery voltage       |
| G21 | SDA | Pin used by I2C data transmission |
| G22 | SCL | Pin used by I2C clock |
| G14 | PIN14 | Pin used for the wakeup IMU interrupt |
| G18 | PIN18 | Pin used to activate the buzzer |
| G25 | PIN25 | LED red Pin |
| G26 | PIN26 | LED green Pin |
| G27 | PIN27 | LED blue Pin |

## Power supply

As for the power supply of the product: The values of the resistances shown in the schematic are calculated taking into account the use of two batteries with a BMS controlling them. Any modification in the power system should take this into account.
The recommended voltage supply is 10V; any lower may cause the BMS not to open and more than 12V could damage the BMS.

It should be noted that for the battery reading to be succesful there should be a bridge made between pins ¿? and ¿? in the PCB. Unless the system is supplied with only one battery, in that case thse pins are used for separate purposes.
