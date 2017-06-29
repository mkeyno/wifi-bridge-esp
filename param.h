#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
     AsyncWebServer server(80);
     AsyncWebSocket WS("/ws");
  //   AsyncClient  *client;
//     AsyncServer Aserver(80);
  //   AsyncClient  Aclient;     
 //      AsyncEventSource events("/events");
#include <EEPROM.h>
#include <AsyncMqttClient.h>
#include <ESP8266HTTPClient.h>
                HTTPClient http;
#include <StreamString.h>
#include <ESP8266httpUpdate.h>

 extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
 extern "C" void system_set_os_print(uint8 onoff);
#define TIME_ZONE    +3
 
String Device_info[30];

#define   Modes    0
#define   Hostname    1
#define   ST_ssid    2
#define   ST_pass    3
#define   AP_ssid    4
#define   AP_pass    5
#define   Static_IP    6
#define   AP_Server_IP    7
#define   Master_IP    8
#define   Last_store_date    9
#define   Last_store_time    10
#define   desc0    11
#define   desc1    12
#define   desc2    13
#define   desc3    14
#define   contl0    15
#define   contl1    16
#define   contl2    17
#define   contl3    18
#define   TelegramID    19
#define   telegrampass    20
#define   RemoteSensorIP    21
#define   MQTT_Server_IP    22
#define   MQTT_Topic    23
#define   MQTT_user    24
#define   MQTT_pass    25
#define   MQTT_port    26
#define   MQTT_clientid    27
#define   Zoon5    28
#define   TempZoon5    29







////////////EEPROM
#define STANDALONE_p      1
#define STATIC_p      2
#define Enable_log_p      3
#define COOLINGsys_p      4
#define ReciveRemoteSensor_p      5
#define Automatic_p      6
#define Coolant_Staus_p      7
#define ResetPin_Staus_p      8
#define StartResume_Staus_p      9
#define FeedHold_Staus_p      10
#define EnableTelegram_p      11
#define Scenario_index_p      12
#define onResetMode_p      13





byte reset_number=0;

byte  STANDALONE=1;
byte  STATIC=0;
byte  Enable_log=0;
byte  COOLINGsys=0;
byte  ReciveRemoteSensor=0;
byte  Automatic=1;
byte  Coolant_Staus=0;
byte  ResetPin_Staus=0;
byte  StartResume_Staus=0;
byte  FeedHold_Staus=0;
byte  EnableTelegram=0;
byte  Scenario_index=0;
byte  onResetMode=0;



 
bool EnablePreSetting=false;

bool DoRegisterTel =false;
bool  TelegramRegistered=false;

bool INFO=false;
bool STARTING=false;
String CODE="RAW@";
char WifiMode,Function_Modde;
 

String inputString;

///////////////////////////////

IPAddress apIP(192, 168, 4, 1),_ip;
File fsUploadFile;
unsigned long Ego_Seconds=27000;
uint32_t program_start,Last_StartCounting,serialTS;
uint32_t Last_query;
bool StartCounting=false;
byte  ZOON=0;
bool NewLine=false;
 
String MyIP,MyMAC;
String PerfixID="";
String preSetUser="";
String preSetPass="";

byte MainWSclinet;

bool UPDATING=false,PendingUpdating=false;;


bool client_active=false;
bool ADMIN=false;
bool ACCESS=false;
String USERS[5],PASS[5],LoginUser;
byte  IsAdmin[5];


uint32_t TimesSpans[5]={27000,45000,63000,75600,86400};

 /////////////////////////////////////////////////////////////////////////////////////////////////////////
String read_line(String _path,byte line_num){
File f = SPIFFS.open(_path, "r");
if (!f) return "" ; 
byte counter=0; 
String line="";
while(f.available())//1AAA@THAT54545646|1$15:
{ 
 line = f.readStringUntil('\n'); 
 line.trim();
 //Serial.print("read line="); Serial.println(line);                          
 if(line.length() >2 && counter==line_num) break;
 counter++;
}
 f.close();  
 return line;
}
void restor_user_pass(void){
  File f = SPIFFS.open(UserData, "r");
 if(!f) 
   {
Serial.println("\t\t[Open User Pass Failed]");
  return;
  } 
    int numLine=1;
 String line="";
   while(f.available())
   { 
 char c=f.read();              
  if(c=='\n' || c=='\r' )          
                      {  if(line.length() >4)
                                            {
                                              byte commaIndex = line.indexOf(',');
                                              byte secondCommaIndex = line.indexOf(',', commaIndex+1);
                                              USERS[numLine]= line.substring(0, commaIndex); USERS[numLine].trim();
                                              PASS[numLine] = line.substring(  commaIndex+1, secondCommaIndex); PASS[numLine].trim();
                                              String S=line.substring(secondCommaIndex+1);
                                              if(S[0]=='A') IsAdmin[numLine]=1; else IsAdmin[numLine]=0; 
                                               Serial.println(USERS[numLine] +":"+PASS[numLine]+":"+String(IsAdmin[numLine]));              
                                              numLine++;  
                                            }        
                      }               
  else              line+=c;  
 } 
   f.close(); 
}
void UpdateContent(String path, String income){
          File f = SPIFFS.open(path, "w");   
  if (!f) {
    Serial.println("[F-w-"+path+"]");// now write two lines in key/value style with  end-of-line characters  
    return;
      }   
  else {   f.println(income); f.close();  }        
}
String compose(void){
Device_info[Modes]=String(WifiMode)+String(Function_Modde);
String s=
Device_info[Modes]+ "@"+
Device_info[Hostname]+ "@"+
Device_info[ST_ssid]+ "@"+
Device_info[ST_pass]+ "@"+
Device_info[AP_ssid]+ "@"+
Device_info[AP_pass]+ "@"+
Device_info[Static_IP]+ "@"+
Device_info[AP_Server_IP]+ "@"+
Device_info[Master_IP]+ "@"+
Device_info[Last_store_date]+ "@"+
Device_info[Last_store_time]+ "@"+
Device_info[desc0]+ "@"+
Device_info[desc1]+ "@"+
Device_info[desc2]+ "@"+
Device_info[desc3]+ "@"+
Device_info[contl0]+ "@"+
Device_info[contl1]+ "@"+
Device_info[contl2]+ "@"+
Device_info[contl3]+ "@"+
Device_info[TelegramID]+ "@"+
Device_info[telegrampass]+ "@"+
Device_info[RemoteSensorIP]+ "@"+
Device_info[MQTT_Server_IP]+ "@"+
Device_info[MQTT_Topic]+ "@"+
Device_info[MQTT_user]+ "@"+
Device_info[MQTT_pass]+ "@"+
Device_info[MQTT_port]+ "@"+
Device_info[MQTT_clientid]+ "@"+
Device_info[Zoon5]+ "@"+
Device_info[TempZoon5]+ "@"+




" ";
Serial.println(s);
/*
SA@
SmartIOT@
keyno@
09127163464@
SmartKeys@
12345678@
192.168.1.13@
192.168.4.1@
192.168.0.0@
170524@  Last_store_date
74992@  Last_store_time
light home@
L050@
refigirator@
M00#0@
room light@
M1@
toster@
T18000#0%N43812@
dinning room light@
L270@
machin mow@
T24000#10%F43932@
192.8.8.8@   RemoteSensorIP
192.168.0.1@
top@
user@
pass@
55@
1@
NON@  TelegramID
2700-4500-6300-7560-8640-@
27-27-27-27-27-@ 
*/
return s;
}
void decompose(String s){
byte As=0; //Modes = WifiMode,Function_Modde,Scenario
 WifiMode=s[0];           if(WifiMode!='S') WifiMode='A'; 
 Function_Modde=s[1];    if(Function_Modde!='L' || Function_Modde!='F' || Function_Modde!='C' ) Function_Modde='A'; 
 
As=s.indexOf('@'); s=s.substring(As+1);
 
As=s.indexOf('@');  Device_info[Hostname]=s.substring(0,As);     if(Device_info[Hostname].length()<3) Device_info[Hostname]="SmartIOT"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[ST_ssid]=s.substring(0,As);     if(Device_info[ST_ssid].length()<3) Device_info[ST_ssid]="NS"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[ST_pass]=s.substring(0,As);     if(Device_info[ST_pass].length()<3) Device_info[ST_pass]="NS"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[AP_ssid]=s.substring(0,As);     if(Device_info[AP_ssid].length()<3) Device_info[AP_ssid]="SmartKeys" +String(ESP.getChipId(), HEX); s=s.substring(As+1);
As=s.indexOf('@');  Device_info[AP_pass]=s.substring(0,As);     if(Device_info[AP_pass].length()<3) Device_info[AP_pass]="12345678"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[Static_IP]=s.substring(0,As);     if(Device_info[Static_IP].length()<3) Device_info[Static_IP]="192.168.4.0"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[AP_Server_IP]=s.substring(0,As);     if(Device_info[AP_Server_IP].length()<3) Device_info[AP_Server_IP]="192.168.4.1"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[Master_IP]=s.substring(0,As);     if(Device_info[Master_IP].length()<3) Device_info[Master_IP]="192.168.0.0"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[Last_store_date]=s.substring(0,As);     if(Device_info[Last_store_date].length()<3) Device_info[Last_store_date]="20160510"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[Last_store_time]=s.substring(0,As);     if(Device_info[Last_store_time].length()<3) Device_info[Last_store_time]="77660"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[desc0]=s.substring(0,As);     if(Device_info[desc0].length()<3) Device_info[desc0]="desc0"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[desc1]=s.substring(0,As);     if(Device_info[desc1].length()<3) Device_info[desc1]="desc1"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[desc2]=s.substring(0,As);     if(Device_info[desc2].length()<3) Device_info[desc2]="desc2"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[desc3]=s.substring(0,As);     if(Device_info[desc3].length()<3) Device_info[desc3]="desc3"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[contl0]=s.substring(0,As);     if(Device_info[contl0].length()<3) Device_info[contl0]="M0"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[contl1]=s.substring(0,As);     if(Device_info[contl1].length()<3) Device_info[contl1]="M1"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[contl2]=s.substring(0,As);     if(Device_info[contl2].length()<3) Device_info[contl2]="M2"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[contl3]=s.substring(0,As);     if(Device_info[contl3].length()<3) Device_info[contl3]="M3"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[TelegramID]=s.substring(0,As);     if(Device_info[TelegramID].length()<3) Device_info[TelegramID]="NON"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[telegrampass]=s.substring(0,As);     if(Device_info[telegrampass].length()<3) Device_info[telegrampass]="NON"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[RemoteSensorIP]=s.substring(0,As);     if(Device_info[RemoteSensorIP].length()<3) Device_info[RemoteSensorIP]="192.168.0.0"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_Server_IP]=s.substring(0,As);     if(Device_info[MQTT_Server_IP].length()<3) Device_info[MQTT_Server_IP]="192.168.0.1"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_Topic]=s.substring(0,As);     if(Device_info[MQTT_Topic].length()<3) Device_info[MQTT_Topic]="top"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_user]=s.substring(0,As);     if(Device_info[MQTT_user].length()<3) Device_info[MQTT_user]="user"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_pass]=s.substring(0,As);     if(Device_info[MQTT_pass].length()<3) Device_info[MQTT_pass]="pass"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_port]=s.substring(0,As);     if(Device_info[MQTT_port].length()<3) Device_info[MQTT_port]="55"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[MQTT_clientid]=s.substring(0,As);     if(Device_info[MQTT_clientid].length()<3) Device_info[MQTT_clientid]="1"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[Zoon5]=s.substring(0,As);     if(Device_info[Zoon5].length()<3) Device_info[Zoon5]="2700-4500-6300-7560-8640-"; s=s.substring(As+1);
As=s.indexOf('@');  Device_info[TempZoon5]=s.substring(0,As);     if(Device_info[TempZoon5].length()<3) Device_info[TempZoon5]="27-27-27-27-27-";

 
}
void restore_EEPROM(void){
 
reset_number=EEPROM.read(0); if(reset_number>10) reset_number=0;	

STANDALONE=EEPROM.read(STANDALONE_p); 
STATIC=EEPROM.read(STATIC_p);
Enable_log=EEPROM.read(Enable_log_p);
COOLINGsys=EEPROM.read(COOLINGsys_p);
ReciveRemoteSensor=EEPROM.read(ReciveRemoteSensor_p);
Automatic=EEPROM.read(Automatic_p); if(Automatic>1)  Automatic=1;
Coolant_Staus=EEPROM.read(Coolant_Staus_p);
ResetPin_Staus=EEPROM.read(ResetPin_Staus_p);
StartResume_Staus=EEPROM.read(StartResume_Staus_p);
Coolant_Staus=EEPROM.read(FeedHold_Staus_p);
EnableTelegram=EEPROM.read(EnableTelegram_p);
Scenario_index=EEPROM.read(Scenario_index_p); if(Scenario_index>3)  Scenario_index=3;
onResetMode=EEPROM.read(onResetMode_p); if(onResetMode>3)  onResetMode=3;

//Serial.println("Automatic="+String(Automatic));
  
}
void restore_setting(void)  { 
 restore_EEPROM();
 String STRING=read_line(DeviceInfo,0);
 decompose(STRING);

 Ego_Seconds=Device_info[Last_store_time].toInt();   
 String dummy=Device_info[Zoon5];
 for(byte i=0;i<5;i++)
  {
                         byte Sa=dummy.indexOf('-');
                   TimesSpans[i]=dummy.substring(0,Sa).toInt()*10;//                   
  if( Ego_Seconds> TimesSpans[i]) ZOON++;
              dummy=dummy.substring(Sa+1);                
  } 

 preSetUser=String(ESP.getChipId());//
 
             preSetPass=String(ESP.getChipId(), HEX);
             preSetPass.toUpperCase();
			 if(preSetPass.length()<5) preSetPass="X"+preSetPass;
 SerialPart+=preSetPass;
 USERS[0]=preSetUser;
 PASS[0]=preSetPass;
 IsAdmin[0]=1;
 restor_user_pass();
 Serial.println(USERS[0]);
  Serial.println(PASS[0]);
 delay(100);
}
void store_setting(void){
String STRING=compose();
Serial.print("save STRING:");Serial.println(STRING);
UpdateContent(DeviceInfo,STRING);	
Serial.println("[Stored]");
}
 void Fail_Safe(void){
 Serial.print(F("ENTER SAFE MODE(Restarting.....................)"));
 EEPROM.write(STANDALONE_p, 1);
 EEPROM.write(STATIC_p,0);
 EEPROM.commit();
 String ss="AM@SmartIOT@NS@NS@SmartTouch" +String(ESP.getChipId(), HEX)+"@12345678@192.168.4.0@192.168.4.1@192.168.0.0@20160510@77660@device 0@device 1@device 2@device 3@0M00#0@1M10#0@2M20#0@3M30#0@telegram@telpass@192.168.0.0@192.168.0.1@top@user@pass@55@1@2700-4500-6300-7560-8640-@27-27-27-27-27-@";

 //Serial.println("RemoteSensorIP="+Device_info[RemoteSensorIP]);
 UpdateContent(DeviceInfo,ss);  
delay(1000);
 ESP.restart();              
}
void check_restore_pin(void){
 if(!digitalRead(RestorePin)){ if(!StartCounting) {Last_StartCounting=millis(); StartCounting=true; Serial.println(F("Counting to restor"));}
                              else if( (millis()-Last_StartCounting)>3000 ) Fail_Safe(); 
							  }		
}
 
