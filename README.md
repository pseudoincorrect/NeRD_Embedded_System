# Neural activity transmission device 

This repository describe the code of a neural activity transmission device embedded on a rodent.
The purpose of this system is to log the the electrical activity of a rat's neurones onto a computer.

## Global view of the acquisition system 

![alt text][global_system]

[global_system]: https://github.com/pseudoincorrect/Electrophy_Base_System/pictures/global_system.png "Global data acquisition system"


### THis repository contain the code and the eagle file necessary to create a the transmitting system embedded on the rodent (see above)


### Features (Embedded Part)

1. Omnetics adapter
2. Transmit 4 channels of 16 bits at 20 kHz 
3. Or 8 channels compressed (16 bits after decompression) at 20 kHz
4. Consume 18 mA at 3.7 V at full transmission load
5. Weight 7 grams 
6. Volume : 2cm *2cm * 2cm 


### Features (Interface Part)

1. USB audio protocol interface to display and log on PC
2. 8 channels DAC to connect to an acquisition card
3. USB as power supply

#### [This file]( https://github.com/pseudoincorrect/Electrophy_Base_System/UserManual.txt)  contain the directions to program the Embedded system through Keil Uvision or directly by programming through the ST-link-V2 by STMicroelectronics

#### [This file]( https://github.com/pseudoincorrect/Electrophy_Base_System/Eagle/AssemblyDirection.txt) contain the directions to assemble the PCB


## Technical part

On this version, we use the Microcontroler [STM32f051C8T7](http://www.st.com/web/en/catalog/mmc/SC1169/SS1574/LN7/PF251889) to interface the Intantech [RHD2132](http://www.intantech.com/products_RHD2000.html) ADC device, and the Nordic Semiconductor [Nrf24l01+](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24L01) RF transciever.

### Short description of the working process of the system

