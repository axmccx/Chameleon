# Chameleon

Chameleon, by Alex McCulloch. 

A multipayload rubber ducky clone. 

Started off as a modification of the Peensy project, turned into more of its own thing. 
https://github.com/offensive-security/hid-backdoor-peensy, 
https://www.offensive-security.com/offsec/advanced-teensy-penetration-testing-payloads/

Ported some ducky scripts from Hak5. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payloads. 
And ported some output code from the Kautilya project. https://github.com/samratashok/Kautilya

## This project is very much a work in progress

Once loaded with the arduino code, the Chameleon USB will execute the specified "script" upon being connected to a PC or Mac. The dip switches are used to chose which one it will perform. 
Once it's finished it will blick the LED quickly for a few seconds to let the user know. 

Some of these scripts are dependent on files stored on the SD card, and I've generated them by metasploit or powershell Empire. 
I will not be posting them here, but I will post my server's configuration so you can generate your own.

This has only been tested on a Teensy 2.0, with the SD card adapter and a 3 position dip switch. The Windows scripts have been tested on Windows 10, and the Mac scripts have been tested on a Mac VM running 10.12

### Current Dip switch configuration: 

000	- No script is executed, the SD Card is mounted. 

001	- Windows. Configures scheduled task to start a meterpreter and PowerShell Empire reverse shell every 12 hours. Requires the user to be part of the Administrator's group. 

010	- Windows. Disables Windows defender and run Mimikatz to dump in memery passwords to a file, and copies this file to the SD card. 

011	- Windows. Dump all saved Wifi passwords to a file on the SD card. 

100	- Mac. Configures Root reverse shell task. Must be run after mac is booted into single user mode. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payload---OSX-Root-Backdoor

101	- Mac. Configures user reverse shell task. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payload---OSX-User-Backdoor

110	- Same as 000

111	- Same as 000


### To Do List

- [DONE] Separate unsed functions from the Peensy project.
- [DONE] Work on Planned section.
- Add 2 more scripts, not sure which yet...
- Write clean up scripts
- Rewrite the code for how the dip switch setting is read.
- Write How-Tos for generating the msf.bat and PE.bat scripts. 
