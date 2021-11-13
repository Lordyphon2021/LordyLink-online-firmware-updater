# LordyLink

this QT-based GUI application connects a PC to an ATMega 644PA-based Eurorack Synth Controller via USB
for online firmware updating and user-data management.


USE CASE 1: FIRMWARE UPDATE:        

app loads all available firmware versions from manufacturer website and displays them in a QTableView.
selected firmware updates the microcontroller after successful checksum verifications.


USE CASE 2: LOAD AND SAVE:         

app loads eeprom data from controller and saves it to harddisk. files are displayed in another QTableView
where they can be renamed and/or deleted. 
             

USE CASE 3: SEND TO CONTROLLER:     

app sends stored user data back to the controller, where it is burned to eeprom.


LordyLink will only start if the controller is connected. hot plug detection scans the connection every 2 seconds.
GUI pushbuttons ( and dialogues ) kick off all described use cases in a seperate thread. pushbuttons are set to inactive, once
one of the threads starts to prevent erratic behaviour.

most of the business logic can be found in LordyLink.cpp and USBThread.cpp,
SerialHandler is a wrapper-class for QSerialPort to prevent code redundancy,
detailed description in the comments.

TODOs: 
       
       -display available firmware-versions before downloading them. 
       
       -implement "delete set" on mouse right-click instead of push button.
       
       -eliminate code redundancies and useless debugging branches
       
     
