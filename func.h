String formatBytes(size_t bytes){
       if (bytes < 1024)                 return String(bytes)+"B"; 
  else if(bytes < (1024 * 1024))         return String(bytes/1024.0)+"KB";
  else if(bytes < (1024 * 1024 * 1024))  return String(bytes/1024.0/1024.0)+"MB";
  else                                   return String(bytes/1024.0/1024.0/1024.0)+"GB";
}
void print_chip_info(void){
Serial.print("Slected FlashChipSize: ");Serial.println(formatBytes(ESP.getFlashChipSize()));
 Serial.print("FlashChipRealSize: ");Serial.println(formatBytes(ESP.getFlashChipRealSize()));
Serial.print("SketchSize:");Serial.println(formatBytes(ESP.getSketchSize()));
Serial.print("FreeSketchSpace:");Serial.println(formatBytes(ESP.getFreeSketchSpace()));
Serial.print("FreeHeap:");Serial.println(formatBytes(ESP.getFreeHeap()));

Serial.print("\nChipId: ");Serial.println(ESP.getChipId());
Serial.print("CpuFreqMHz: ");Serial.println(ESP.getCpuFreqMHz());  
Serial.print("SdkVersion: ");Serial.println(ESP.getSdkVersion());
Serial.print("FlashChipSpeed: ");Serial.println(ESP.getFlashChipSpeed());
Serial.print("FlashChipMode: ");Serial.println(ESP.getFlashChipMode());	

}
void print_device_info(void){
   Serial.println("\n\nDevice ID=" + preSetPass +"(" + preSetUser + ")");/*
  Serial.print(F("Mode="));      Serial.println(Device_info[MyMode]);
  Serial.print(F("Hostname="));      Serial.println(Device_info[myHostname]);
  Serial.print(F("SSID="));           Serial.println(Device_info[_SSID]);
  Serial.print(F("Password ="));      Serial.println(Device_info[SSIDPASS]);
  Serial.print(F("AP SSID="));        Serial.println(Device_info[AP_SSID]);
  Serial.print(F("AP Password ="));   Serial.println(Device_info[AP_SSIDPASS]);
  Serial.print(F("reset number ="));  Serial.println(reset_number);
  Serial.print(F("Server IP Address ="));  Serial.println(Device_info[AP_Server_IP]);
 // Serial.print(F("remote Actuator  IP ="));  Serial.println(Device_info[Remote_Sensor_IP]);
  

  Serial.print(F("static IP ="));  Serial.println(Device_info[Static_IP]);  
 // Serial.print(F("Sensor Des ="));  Serial.println(Device_info[Sensor_DES]);
 // Serial.print(F("Sensor ID ="));  Serial.println(Device_info[Remote_Sensor_IP]);
  Serial.print(F("Ego Seconds ="));  Serial.println(Ego_Seconds);
 
  
  Serial.print(F("Store sensor data ="));  Serial.println(STORE);
 Serial.print(F("Sending Mode ="));  Serial.println(SendingMode);
 Serial.print(F("static mode ="));  Serial.println(STATIC);
 Serial.print(F("Mode="));          Serial.println((Device_info[MyMode][0]=='A')?"Access Point":"Station");
 Serial.print(F("Network Mode="));  Serial.println((STANDALONE)?"Slave Mode":"Server Mode");
 Serial.print(F("System Mode="));  Serial.println((COOLING)?"Cooling":"Heating");
 Serial.print(F("Control Mode="));  Serial.println((AUTOMATIC)?"Automatic":"Manual");
 Serial.print(F("last store Mac="));  Serial.println(Device_info[SensorMAC]);
*/
}
 bool initTime(){
  time_t rawtime;
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  uint8_t i = 0;
  system_set_os_print(0);
  while(time(&rawtime) == 0 && i++ < 100) delay(10);
  system_set_os_print(1);
  if(i==100){
    return false;
  }
  return true;
}
String getIP(){
  IPAddress ip = (WiFi.getMode() & WIFI_STA) ? WiFi.localIP() : WiFi.softAPIP();
  return ip.toString();
}
void Station_blink(byte i){	
	digitalWrite(PINS[0],LOW);digitalWrite(PINS[1],LOW);digitalWrite(PINS[2],LOW);digitalWrite(PINS[i],HIGH);
}
IPAddress str2ip(String sip){
   IPAddress result;
   String s="";
   byte j=0;
   for (byte i = 0; i < sip.length() ; i++)
   {
    if(sip[i]!='.') s+=sip[i];
    else     {result[j]=s.toInt(); j++; s="";}
   } 
  result[3]=s.toInt(); 
return result;  
}
void DIR(void){
  uint32_t total=0;
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
              String fileName = dir.fileName();
              size_t fileSize = dir.fileSize();
                    total+=fileSize;
              Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
            }
    Serial.print("\t\t\tTotal:");Serial.println(total);
  
}
void Append_Data(String data){ // general file   store data in file
 
 File f = SPIFFS.open(STOR_DATA, "a+");
 if (!f) {   
   Serial.println("[F-a+/store/data.bin]");//
 return ;}
 f.println(data);
  f.close();
}
void wifi_startup(void){
	
  if(WifiMode=='A' || reset_number>4 || Device_info[ST_ssid].length()<3 ){
                               Serial.println(F("\t\t\t Entering the Access Mode"));                               
                               STANDALONE=true; 
							   EnableTelegram=false;
                               EEPROM.write(STANDALONE_p,'E');EEPROM.commit();                             
                               WiFi.mode(WIFI_AP);
                               WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
                               String temp_name="SmartTouch_" +String(ESP.getChipId(), HEX);                               
                               if(Device_info[AP_ssid].length()<3) {WiFi.softAP(temp_name.c_str(),"12345678"); Device_info[AP_ssid]=temp_name; }
                               else                                 WiFi.softAP(Device_info[AP_ssid].c_str(),Device_info[AP_pass].c_str());                                
                               delay(500); // Without delay I've seen the IP address blank
                               EEPROM.write(0, 0);EEPROM.commit();  
                               MyIP=WiFi.softAPIP().toString();
                               MyMAC=WiFi.softAPmacAddress();
                               Serial.print("AP IP address: ");Serial.println(MyIP);
                               Serial.print("My MAC: ");Serial.println(MyMAC);
                              // dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
                              // dnsServer.start(DNS_PORT, "*", apIP); 
                              digitalWrite(Wifi_Mode,HIGH);
                            }
else if(reset_number<5){
						Serial.println(F("\t\t\t Entering the Station Mode"));
						WiFi.mode(WIFI_STA);
						if(STATIC)   WiFi.config(str2ip(Device_info[Static_IP]),str2ip(Device_info[AP_Server_IP]), IPAddress(255,255,255,0));   
						WiFi.begin(Device_info[ST_ssid].c_str(), Device_info[ST_pass].c_str());             
						unsigned long startTime = millis();
						while (WiFi.status() != WL_CONNECTED && millis() - startTime < 7000) { byte j=0;
																								Serial.write('%');delay(500); Station_blink((j++)%3);
																								} 
						if(WiFi.status() != WL_CONNECTED) {
															  reset_number++;                                               
														   EEPROM.write(0, reset_number);EEPROM.commit();                                               
														  ESP.restart();
														  }            
						 MyIP=WiFi.localIP().toString();
						 MyMAC=WiFi.macAddress();
						 Serial.print("AP IP address: ");Serial.println(MyIP);
						 Serial.print("My MAC: ");Serial.println(MyMAC);
						if (!MDNS.begin(Device_info[Hostname].c_str())) {
															Serial.println(F("Error setting up MDNS responder!"));
															while(1) {  delay(1000); }
														  }
						Serial.println(F("mDNS responder started"));
		//				if(_MQTT) Mqtt_setting();
					  }
	
}
void ConfigureOTA(void) {
  // ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(("OTA_"+preSetUser).c_str());
  ArduinoOTA.setPassword(preSetPass.c_str());  
  ArduinoOTA.onStart([]() {SPIFFS.end(); /* Disable client connections */WS.enable(false);  WS.textAll("OTA Update Started");WS.closeAll(); Serial.println("StartOTA \n");  });
  ArduinoOTA.onEnd([]()   {SPIFFS.end(); Serial.println("\nEnd OTA\n");  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { Serial.printf("OTA Progress: %u%%\n", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
         if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  Serial.println(F("\nOTA Ready"));
  ArduinoOTA.begin();
}
void SerialEvent(){ 
 while (Serial.available() > 0) // if (Serial.available())
 {
    char inChar = (char)Serial.read();
    inputString += inChar; 
    if (inChar == '\n'  ) {
                            NewLine=true;
                                     
                             }
  }
   
 }


