# LordyLink

This Qt-based GUI application connects a PC/Mac to an ATMega 644PA-based Eurorack Synth Controller via USB
for online firmware updating and user-data management.
It is intended as a free download for every "Lordyphon" owner.


USE CASE 1: FIRMWARE UPDATE:        

  App loads all available firmware versions from manufacturer website and displays them in a QTableView.
  Selected firmware updates the microcontroller after successful checksum verifications.

  ( I am intending to provide access to all firmware releases in case something goes wrong on the controller.
  I have had issues with a well known synth manufacturer's firmware going weird every now and then and 
  would have wanted such a feature very badly )


USE CASE 2: LOAD AND SAVE:         

  App loads eeprom data from controller and saves it to harddisk. Files are displayed in another QTableView
  where they can be renamed and/or deleted. 
             

USE CASE 3: SEND TO CONTROLLER:     

  App sends stored user data back to the controller, where it is burned to eeprom.


LordyLink will only start if the controller is connected. Hotplug detection scans the connection every 2 seconds.
GUI pushbuttons ( and dialogues ) kick off all described use cases in seperate threads. 

Most of the business logic can be found in LordyLink.cpp and USBThread.cpp,
SerialHandler is a wrapper-class for QSerialPort to prevent code redundancy,
detailed description in the comments.

TODOs: 

- error handling: 

    unsuccessful controller identification after update process 
    (hardware is in default - MIDI mode after startup and will not respond to USB )
    solution: automatically resume usb-mode on controller after restart if LordyLink is connected.
  
- implement "lordylink disconnected" message on controller

- use secure server connection and figure out how to HIDE server credentials in a public repository!!!!

- enable support for more than one hardware unit

- alert user when new release is available.

- implement "delete set" on mouse right-click instead of push button.

- display github link and mandatory qt licence text in menu bar

- newer macOS builds, it only works on catalina

- create application shortcut to desktop (dialog??)


       
     
