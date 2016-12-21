# Chameleon
Inspired by Offensive Security's Peensy project, I've change it to how I want to see it work. You can find more details on that here https://www.offensive-security.com/offsec/advanced-teensy-penetration-testing-payloads/ and here https://github.com/offensive-security/hid-backdoor-peensy. I'm also taking from the list of ducky scripts found here https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payloads. 

## This project is very much a work in progress

Once loaded with the arduino code, the Chameleon USB will execute the specified "script" upon being connected to a PC or Mac. The dip switches are used to chose which one it will perform. Once it's finished it will blick the LED quickly for a few seconds to let the user know. 

Some of these scripts are dependent on files stored on the SD card, and I've generated them by metasploit or powershell Empire. I will not be posting them here, but I will post my server's configuration so you can generate your own.

This has only been tested on a Teensy 2.0, with the SD card adapter and a 3 position dip switch. The Windows scripts have been tested on Windows 10. 

### Current Dip switch configuration: 

000	- No script is executed, the SD Card is mounted. 

001	- Windows. Configures scheduled task to start a meterpreter and PowerShell Empire reverse shell every 12 hours. Requires the user to be part of the Administrator's group. 

010	- Windows. Disables Windows defender and run Mimikatz to dump in memery passwords to a file, and copies this file to the SD card. 


### Planned Dip switch configuration:

011	- Windows. Dump all saved Wifi passwords to a file on the SD card. 

100	- Mac. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payload---OSX-Root-Backdoor

101	- Mac. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payload---OSX-User-Backdoor

110	- 

111	- 
