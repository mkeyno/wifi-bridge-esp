#define STOR_DATA "/data/data.bin"
#define DeviceInfo "/data/device.inf"
#define UserData "/data/user.dat"

#define UpdateLink "http://iot.keyno-gc.com/update.php"  

String SerialPart= "KTE00017020701";

#define  ResetPin    16
#define  Coolant    5
#define  StartResume   4
#define  FeedHold    12
#define  ResetAbort    14
#define RestorePin 0
#define Wifi_Mode 15

byte PINS[]={ResetPin,Coolant,StartResume,FeedHold,ResetAbort};
byte PinLastStatus[sizeof(PINS)]={0};



bool EnabaleOTA=false; 
bool shouldReboot = false;
 
#include "param.h"
#include "html.h"
#include "func.h"
#include "aync.h"
#include "ws.h"

// void restore_setting(void);
//String compose(void);

  


void setup () {
   
  
  Serial.begin(115200);//, SERIAL_8N1,SERIAL_TX_ONLY);
  delay(500);
   for(byte i=0;i<sizeof(PINS);i++) pinMode(PINS[i],OUTPUT); 
 
 // Serial.setDebugOutput(true);
  Serial.print(F("\n\n\n Keyno Guidance & Control\nwww.Keyno-GC.com\nVersion="));  Serial.println(SerialPart);
 
 ////////////////////////////////////////////////////////////////////////////////////
  SPIFFS.begin();
 EEPROM.begin(32);
 delay(100);
  restore_setting();

  // pinMode(LED_BUILTIN, OUTPUT); 
   DIR(); 
   // print_chip_info();
  print_device_info();
   wifi_startup();
 
  if(EnabaleOTA) ConfigureOTA();

server.on("/exit",             [](AsyncWebServerRequest *request){ ACCESS=ADMIN=false; request->requestAuthentication(); request->send(401, "text/plain", "Thanks\n Exit");  });
server.on("/process", HTTP_GET,[](AsyncWebServerRequest *request){ /*if(Access) */handleProcess(request); });  
server.on("/config",           [](AsyncWebServerRequest *request){ handleConfig(request);  });
server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
              if(!check_Auth(request))   request->requestAuthentication(); 
              request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
              });
server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){                                                 
                      shouldReboot = !Update.hasError();
                        Serial.printf("Update was %s\n",shouldReboot?"OK":"FAIL");
                      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
                                              response->addHeader("Connection", "close");
                                request->send(response);
                                delay(10);
                             if(shouldReboot) ESP.restart();//reset();   
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)  {
    if(!index){
        Serial.printf("Update Start: %s\n", filename.c_str());
        Update.runAsync(true);
        if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) Update.printError(Serial);
       }
    if(!Update.hasError()){
              if(Update.write(data, len) != len)      Update.printError(Serial);
            }
    if(final){
        if(Update.end(true))  Serial.printf("Update Success: %uB\n", index+len);
        else               Update.printError(Serial);
      }
  }); 
server.on("/upload", HTTP_POST,[](AsyncWebServerRequest *request){ request->send(200, "text/plain", ""); }, handleFileUpload);  
server.onFileUpload([](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
if (!index)  Serial.printf("UploadStart: %s\n", filename.c_str());
Serial.printf("%s", (const char*)data);
if (final)  Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
});
server.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
if (!index)  Serial.printf("BodyStart: %u\n", total);
Serial.printf("%s", (const char*)data);
if (index + len == total)  Serial.printf("BodyEnd: %u\n", total);
}); 
//server.serveStatic("/", SPIFFS, "/Sindex.html.gz","max-age=86400").setFilter(check_Auth);//.setFilter(FilterStandalone);
/*server.serveStatic("/", SPIFFS, "/Mindex.html.gz","max-age=86400").setFilter(FilterMaster);*/
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){   
                              if(!check_Auth(request))  request->requestAuthentication();
                               else
                               {    
                                 {
                                  client_active=true;
                                   ///if(STANDALONE) {
                                      if(!handleFileRead("/Sindex.html", request)) request->send(404, "text/plain", "Sindex.html FileNotFound");
                                   }                                                                     
                               }   
                                             
                                });

//server.serveStatic("/Stand.js", SPIFFS, "/Stand.js.gz"); 
server.serveStatic("/images/logo.png", SPIFFS, "/images/logo.png" ); 
server.serveStatic("/images/logo1.png", SPIFFS, "/images/logo1.png" ); 
//server.serveStatic("/images/sun.png", SPIFFS, "/images/sun.png" );

// server.serveStatic("/images/ske.png", SPIFFS, "/images/ske.png" );
// server.serveStatic("/images/mob.png", SPIFFS, "/images/mob.png" );

//server.serveStatic("/images/tel.png", SPIFFS, "/images/tel.png" );

//server.serveStatic("/images/light.png", SPIFFS, "/images/light.png" );
//server.serveStatic("/images/cool.png", SPIFFS, "/images/cool.png" );
//server.serveStatic("/images/fan.png", SPIFFS, "/images/fan.png" );
//server.serveStatic("/images/app.png", SPIFFS, "/images/app.png" );

 
server.onNotFound([](AsyncWebServerRequest *request) {
Serial.printf("Not found: %s\r\n", request->url().c_str());
AsyncWebServerResponse *response = request->beginResponse(200);
          response->addHeader("Connection", "close");
          response->addHeader("Access-Control-Allow-Origin", "*");
if (!handleFileRead(request->url(), request))     request->send(404, "text/plain", "FileNotFound");
}); 
/*
events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()) Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId()); 
    client->send("hello!",NULL,millis(),1000);//send event with message "hello!", id current millis and set reconnect delay to 1 second
  });
 */ 
// server.addHandler(&events);
  
                     WS.onEvent(webSocketEvent);
  server.addHandler(&WS);
 server.begin();
//  Aserver.onClient(onClient, 0);
  // Aserver.begin();
  delay(500);

}

void loop ( void ) {if(!UPDATING){
  
if(EnabaleOTA) ArduinoOTA.handle();
check_restore_pin();
 SerialEvent();
 if(NewLine && (millis()-serialTS)>50){  WS.text(MainWSclinet,CODE+inputString); inputString = ""; serialTS=millis();NewLine=false;}    
if(INFO) {if( (millis()-Last_query)>210 ) {Serial.print('?'); Last_query=millis();}}
}}

