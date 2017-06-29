bool FilterHost (AsyncWebServerRequest *request){ if(request->url()==MyIP) return true; return false;} 
bool FilterStandalone (AsyncWebServerRequest *request){ if(STANDALONE) return true; return false;} 
bool FilterMaster (AsyncWebServerRequest *request)    { if(!STANDALONE) return true; return false;} 
bool check_Auth(AsyncWebServerRequest *request){ //if(!check_Auth(request)) request->send(404);  return request->requestAuthentication(); 
 // Serial.println(F("check_Auth"));
  if(ACCESS || ADMIN) { Serial.println(F("Granted"));return true;}
 for(byte i=0;i<5;i++) 
	 if(request->authenticate(USERS[i].c_str(), PASS[i].c_str())){
																 LoginUser=USERS[i]; 
																 Serial.println(F("Find user"));ACCESS=true;
																 if(IsAdmin[i])ADMIN=true; return true;
																 }
 Serial.println(F("No Auth user"));
  return false; 
}
void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  //Handle body
}
void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  //Handle upload
}
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
}
String getContentType(String filename, AsyncWebServerRequest *request) {
       if (request->hasArg("download")) return "application/octet-stream";
     
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
bool handleFileRead(String path, AsyncWebServerRequest *request) {
 Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))   path += "index.htm";
  String contentType = getContentType(path, request);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))    path += ".gz";  //"application/javascript"
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (path.endsWith(".gz"))      response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    return true;
  }
  Serial.printf("Cannot find %s\n", path.c_str()); 
  return false;
} 
 void ScanNetwork(AsyncWebServerRequest *request){  //this id of listbox defined here
  String records="<select name=\"Combobox2\" size=\"10\" id=\"_netList\"  onChange=\"selectssid_pass();\"   >";
    records+="<option value=\"100\"> Network Name  ,    Network Strength     </option>";
   int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED)   WiFi.scanNetworks(true);
  else if (n > 0) 
     {
      for (int i = 0; i < n; ++i)records+="<option value=\"" + String(n)+"\">" +WiFi.SSID(i) +" , " + WiFi.RSSI(i)+"</option>";      
    }
  records+="</select>";
  WiFi.scanDelete();
    request->send(200, "text/html", records); 
} 
void handleConfig(AsyncWebServerRequest *request){
 if(!request->authenticate(preSetUser.c_str(), preSetPass.c_str())) return request->requestAuthentication(); 
 ACCESS=ADMIN=true;
 request->send_P(200, "text/html", config_html);
}
void prinScanResult(int networksFound){
  Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++)
  {
    Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
  }
}
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) 
  { 
    Serial.printf("handleFileUpload Name: %s\n", filename.c_str());
    if (!filename.startsWith("/")) filename = "/" + filename;
    fsUploadFile = SPIFFS.open(filename, "w");
  }
  if (fsUploadFile)
  {
    if (fsUploadFile.write(data, len) != len) Serial.println("Write error during upload");
  }
  
  if (final) 
  { 
    if (fsUploadFile)   fsUploadFile.close();     
    Serial.printf("handleFileUpload Size: %u\n", len);
  }
}
// in setup() WiFi.scanNetworksAsync(prinScanResult);
void show_user_list(AsyncWebServerRequest *request,String path,String _ID,String parseFunction){ 
                     //"/data/user.dat","ListUserListBox","parse_line_and_put_inform"
  String records="<select name=\"Combobox2\" size=\"5\" id=\""+_ID+"\"  onChange=\""+parseFunction+"();\"   >";
  File f = SPIFFS.open(path, "a+");   ///a+ if not exist then create it 
  if (!f) return request->send(200, "text/plain", records); 
  byte line_num_count=0;
  String line="";
   while(f.available())
   { 
 char c=f.read();              
  if(c=='\n' || c=='\r' )          
                      {  if(line.length() >2)
                                            {
                                             // Serial.print("line="); Serial.print(line);     Serial.print("[number]"); Serial.println(line_num_count);  
                                              line_num_count++; 
                                               records+="<option value=\"" + String(line_num_count)+"\">" +line +"</option>";
                                              line="";  
                                            }        
                      }               
  else              line+=c;  
 } 
   f.close(); 
  
 records+="</select>";
  request->send(200, "text/plain", records); 
}
void AddUser(AsyncWebServerRequest *request,String user,String pass,String scop){
  request->send(404);
  String line=user+ "," + pass + "," + scop;
  //Serial.println(line); 
   File f = SPIFFS.open("/data/user.dat", "a+");  
  if (!f) return ; 
  f.println(line);
  f.close();
}
void RemoveUser(AsyncWebServerRequest *request,byte num){
  request->send(404);
  String temp="";
  File f = SPIFFS.open("/data/user.dat", "r");
  byte line_num_count=0;
    String line="";
  while(f.available())
  {//Lets read line by line from the file      
  char c=f.read();              
  if(c=='\n' || c=='\r' )          
                      {if(line.length() >2)
                                            {
                                           //Serial.print("line="); Serial.print(line);     Serial.print("[number]"); Serial.println(line_num_count);    
                                             if(num!=line_num_count)temp=temp+line+"\n\r";              
                                              line_num_count++;
                                             line="";     
                                            }  
                       }
 else              line+=c;                         
} 
  
  f.close();
  f = SPIFFS.open("/data/user.dat", "w+");
  f.println(temp);
   f.close();
}
void Createfile(String path, String filename,AsyncWebServerRequest *request){
  if(SPIFFS.exists(path+filename))  return request->send(200, "text/plain", "FILE EXISTS"); 
  File file = SPIFFS.open(path+filename, "w");
    if(file) file.close();   
  else                         return request->send(200, "text/plain", "CREATE FAILED");
}
void removefile(String path, String filename,AsyncWebServerRequest *request){
  if(!SPIFFS.exists(path+filename))    request->send(404, "text/plain", "FileNotFound");  
      SPIFFS.remove(path+filename);    request->send(200, "text/plain", "removed");
}
void RenameFile(String patch,String oldfilename,String newfilename,AsyncWebServerRequest *request){
 String fold=patch+oldfilename;
 String fnew=patch+newfilename;
if (SPIFFS.exists(fold))
          {
            SPIFFS.rename(fold,fnew);
            request->send(200, "text/plain", "Renamed");
          }
  
}
void show_listfile(AsyncWebServerRequest *request){ ///replace all item of list box
   String records="<select name=\"Combobox1\" width=\"400\" size=\"20\" id=\"listfile\"  onChange=\"parse_line();\"   >";
   byte i=0;
   Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
              String fileName = dir.fileName();
              size_t fileSize = dir.fileSize();
              records+="<option value=\"" + String(i)+"\">" +fileName +"</option>";
              i++;
            }
  records+="</select>";
 request->send(200, "text/plain", records);   
}
void show_file_content(String patch, String filename,AsyncWebServerRequest *request){ 
request->send(SPIFFS,patch+filename, String());
}
 
 bool RunUpdate(Stream& in, uint32_t size, String md5, int command){

    StreamString error;
int _lastError;
  HTTPUpdateResult ret = HTTP_UPDATE_FAILED;

    if(!Update.begin(size, command))
  {
        _lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
        Serial.printf("[httpUpdate] Update.begin failed! (%s)\n", error.c_str());
        return false;
    }

    if(md5.length())
  {
        if(!Update.setMD5(md5.c_str())) 
    {
            _lastError = HTTP_UE_SERVER_FAULTY_MD5;
            Serial.printf("[httpUpdate] Update.setMD5 failed! (%s)\n", md5.c_str());
            return false;
        }
    }

    if(Update.writeStream(in) != size) 
  {///------------------------------------------------------------>>>>>>
        _lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
       Serial.printf("[httpUpdate] Update.writeStream failed! (%s)\n", error.c_str());
        return false;
    }

    if(!Update.end()) 
  {
        _lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
        Serial.printf("[httpUpdate] Update.end failed! (%s)\n", error.c_str());
        return false;
    }

    return true;
}
String check_update(byte num){
 //     Result
 //Not Allow                  NA
 //Not Update  availablee     NU
 //Chip ID wring         CW
 // File Availablee           AF
 //Flash Availablee           AL
 //SPIFF Availablee           AS

 //  fail    FL
 // fail to open file  FF
 // malloc fail        MF
 // file update complat FC
 //file update error    FE
 // free sketch less     LF
 // free spiff less  LS
 // bad HTTP BH
String code="FL",Result,filename;
HTTPClient http;
http.begin(UpdateLink);
http.setReuse(true);
http.useHTTP10(true);
http.setTimeout(8000);
// setAuthorization(const char * user, const char * password);
http.setUserAgent(F("ESP8266-http-Update"));
http.addHeader("Serial",SerialPart);
const char * headerkeys[] = {"Result","FileName","x-MD5"};
 size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
http.collectHeaders(headerkeys,headerkeyssize);
int httpCode = http.GET();
if(httpCode > 0){
if(httpCode == HTTP_CODE_OK){
Result=http.header("Result");
filename=http.header("FileName");
Serial.printf("Result header : %s", Result.c_str());
 int len =http.getSize(); 
 
// if(SHOWpayload) Serial.printf(" payload size: %d\n->%s",len,http.getString().c_str());
 
     if(Result=="NA") {Serial.println(" You are band to update"); /*webSocket.sendTXT(num,"not allowed"); */code=Result;}
else if(Result=="NU") {Serial.println(" There is no update");/*webSocket.sendTXT(num,"there is no update"); */code=Result;}
else if(Result=="AF"){ Serial.println(" There is a "+filename+"  to update");
         if(!filename.startsWith("/")) filename = "/"+filename;       
      File f=SPIFFS.open(filename,"w+");   if(!f){      http.end();   Serial.printf("[file failed]\n"); return "FF";}    
    uint8_t *_buff=(uint8_t*) malloc(FLASH_SECTOR_SIZE);if(!_buff) { http.end();  Serial.printf("[malloc failed]\n"); return "MF";}  

     WiFiClient * stream = http.getStreamPtr();
    uint8_t *ptr =_buff;
            size_t space=FLASH_SECTOR_SIZE;
      int wlen=0;
            while(http.connected() && (len > 0 || len == -1)) 
      {
                // get available data size
              size_t size = stream->available();
                if(size) {
              int c = stream->readBytes(ptr, ((size > space) ? space : size));
               Serial.printf("->read %d bytes\n",c);
              ptr += c;
              space -= c;         
              if(len > 0) len -= c;             
              if(space == 0) {
                       Serial.printf("<-write %d bytes\n",FLASH_SECTOR_SIZE);
                      ESP.wdtDisable();
                      f.write(_buff,FLASH_SECTOR_SIZE);
                      ESP.wdtEnable(10);
                      ptr =_buff;
                      space=FLASH_SECTOR_SIZE;
                      wlen += space;
                    }
            }
                delay(1);
            }
            if(space < FLASH_SECTOR_SIZE){
                       Serial.printf("->write %d bytes\n",FLASH_SECTOR_SIZE-space);
                      ESP.wdtDisable();
                      f.write(_buff,FLASH_SECTOR_SIZE - space);
                      wlen += FLASH_SECTOR_SIZE-space;
                      ESP.wdtEnable(10);
                    }
            
      if(len == 0 || len == -1) code="FC";
      else                        code = "FE";    
             Serial.printf("[HTTP] connection closed. written %d.\n",wlen);
       f.close();
          free(_buff);  
      } 
else if(Result=="AL"){ Serial.println("There is a flash to update");
  
   if(len > (int) ESP.getFreeSketchSpace()) { http.end(); Serial.printf("[Free Sketch Space less\n]"); return "LF";}    
  
   WiFiClient * tcp = http.getStreamPtr();
   WiFiUDP::stopAll();//for (WiFiClient* it = _s_first; it; it = it->_next)
     WiFiClient::stopAllExcept(tcp);//for (WiFiClient* it = _s_first; it; it = it->_next)    
   delay(100);
 
  uint8_t buf[4];
  if(tcp->peekBytes(&buf[0], 4) != 4) { http.end(); Serial.printf("peekBytes missmatch\n"); return "ME";}                   
  if(buf[0] != 0xE9)                  { http.end(); Serial.printf("magic missmatch\n"); return "MM";} 
  uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4); 
        if(bin_flash_size > ESP.getFlashChipRealSize()) { http.end(); Serial.printf("not fits SPI\n"); return "[NSPIFIT]";}  
  
  
  if(RunUpdate(*tcp,  len, http.header("x-MD5"), U_FLASH)) { http.end(); Serial.printf("RunUpdate done\n"); return "[UDON]";}  
  else                                                     { http.end(); Serial.printf("RunUpdate fail\n"); return "[UFAIL]";}  
}
else if(Result=="AS"){ Serial.println("There is a SPIFF to update");
  
   size_t        spiffsSize = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
  if(len > (int) spiffsSize) {http.end(); Serial.printf("Free SPIFF Space less\n"); return "[SPIF]";}

  WiFiClient * tcp = http.getStreamPtr();
  WiFiUDP::stopAll();//for (WiFiClient* it = _s_first; it; it = it->_next)
  WiFiClient::stopAllExcept(tcp);//for (WiFiClient* it = _s_first; it; it = it->_next)
    delay(100);
   
   if(RunUpdate(*tcp,  len, http.header("x-MD5"), U_SPIFFS)) { http.end(); Serial.printf("SPIFF Update done\n"); return "[SDON]";}  
  else                                                     { http.end(); Serial.printf("SPIFF Update fail\n"); return "[SFAIL]";}   
}
}
code="BH";
Serial.println("Bad HTTP");
} 
else {code="FL"; Serial.println("Fail HTTP");/* webSocket.sendTXT(num,"update fail");*/}

http.end(); 
Serial.println(http.errorToString(httpCode));  
WS.text(MainWSclinet,"HTP@"+http.errorToString(httpCode));//typecontrol 
return code;
}
 

void handleProcess(AsyncWebServerRequest *request){
 Serial.println("[HTML]"+request->arg(0));
 if(!ACCESS) return request->send(404); 
       if(request->arg(0) =="CRF") Createfile(request->arg(1),request->arg(2),request) ; //http://192.168.4.1/process?code=RCF&patch=patch&file=file   [-CONFIG-]
  else if(request->arg(0) =="RRF") removefile(request->arg(1),request->arg(2),request) ; //http://192.168.4.1/process?code=RRF&patch=patch&file=file [-CONFIG-]
  else if(request->arg(0) =="RNF") RenameFile(request->arg(1),request->arg(2),request->arg(3),request);//http://192.168.4.1/process?code=RRN&patch=patch& [-CONFIG-]
  else if(request->arg(0) =="DIR") show_listfile(request) ;//http://192.168.4.1/process?code=RDH  CONFIG
  else if(request->arg(0) =="SFC") show_file_content(request->arg(1),request->arg(2),request) ;

  else if(request->arg(0) =="UDT") {request->send(200, "text/plain", "Pending"); check_update(0);} ////-------)()()()()()()
  //  if(!ACCESS) return; 
  else if(request->arg(0) =="NET") ScanNetwork(request); //[-Standalone-]
  else if(request->arg(0) =="RST") {request->send(200, "text/plain", ""); delay(500); ESP.restart();} //[-Standalone-]
  else if(request->arg(0) =="USL") show_user_list(request,"/data/user.dat","ListUserListBox","parse_line_and_put_inform");//http://192.168.4.1/process?code=RUL [-Standalone-]
  else if(request->arg(0) =="AUS") AddUser(request,request->arg(1),request->arg(2),request->arg(3));//http://192.168.4.1/process?code=RAU&user=user&pass=pass&scop=scop [-Standalone-]
  else if(request->arg(0) =="RUS") RemoveUser(request,request->arg(1)[0]-'0');//http://192.168.4.1/process?code=RRU&index=index  [-Standalone-]

  }
