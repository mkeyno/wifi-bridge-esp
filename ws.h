#define MAX_WS_CLIENT  5
#define CLIENT_NONE     0
#define CLIENT_ACTIVE   1

String compose_js(){

 
String s=LoginUser+"@"+MyIP+"@"+SerialPart+"@"+Device_info[Hostname]+"@"+Device_info[ST_ssid]+"@"+Device_info[AP_ssid]+"@"+Device_info[Static_IP]+"@"+Device_info[AP_Server_IP]+"@"+Device_info[Master_IP]+"@"+
Device_info[RemoteSensorIP]+"@"+
Device_info[TelegramID]+"@"+
Device_info[telegrampass]+ "@"+
preSetPass+ "@"+
Device_info[desc0]+ "@"+
Device_info[desc1]+ "@"+
Device_info[desc2]+ "@"+
Device_info[desc3]+ "@"+
Device_info[contl0]+ "@"+
Device_info[contl1]+ "@"+
Device_info[contl2]+ "@"+
Device_info[contl3]+ "@"+
	 "";
return s;		 
} 
 
typedef struct {
  uint32_t  id;
  uint8_t   state;
} _ws_client; 
  _ws_client   ws_client[MAX_WS_CLIENT]; 
  
void parse_webSocket(AsyncWebSocketClient * client, String income,uint32_t num){
     byte eqI1 = income.indexOf('=');
 String cammand = income.substring(0, eqI1);  //Serial.print("Cammand=");Serial.print(cammand);
 String value   = income.substring(eqI1+1); // Serial.print(" value=");Serial.println(value);
 
 if(cammand=="SGT")  { // [-Standalone-]
	Device_info[Last_store_date]=value.substring(2,8);  // 
	Device_info[Last_store_time]=value.substring(8);                           
	Ego_Seconds=Device_info[Last_store_time].toInt();   Serial.print(" Base second=");Serial.println(Ego_Seconds);   //77666000  
	String respond= "RST@"+compose_js();		
	
	//  Serial.println(respond);                                     
	client->text(respond); 
 } 
 else if(cammand=="RST")  client->text("RST@"+compose_js()); 

 //'''''''''''''''''''''''''''''''''''''''''   page setting     ''''''''''''''''''''''''''''''''''''''''''''''''''''''''

 else if(cammand=="UDM")  { UPDATING=true; check_update(num); UPDATING=false; } 
 else if(cammand=="BIF")  {if(!STARTING) {Serial.println("$$");  CODE="BIF@"; serialTS=millis(); }}//digitalWrite(LED_BUILTIN, HIGH);

 //'''''''''''''''''''''''''''''''''''''''''   wifi setting     ''''''''''''''''''''''''''''''''''''''''''''''''''''''''
 else if(cammand=="WFM")  {if(value[0]=='S') WifiMode='S'; else WifiMode='A'; }//Serial.print("WifiMode="); Serial.println(WifiMode);} 
 else if(cammand=="SIP")  Device_info[Master_IP]=value;
 else if(cammand=="STI")  Device_info[Static_IP]=value; 
 else if(cammand=="SHN")  Device_info[Hostname]=value;  
 else if(cammand=="APN")  Device_info[AP_ssid]=value; 
 else if(cammand=="APP")  Device_info[AP_pass]=value;
 else if(cammand=="SNW") {  // SSID SSIDPASS Select Network user&pass        [-Standalone-]     
						       eqI1 = value.indexOf('&');
						 Device_info[ST_ssid]= value.substring(0, eqI1);// Serial.print(" SSID=");Serial.println(_ssid);
						 Device_info[ST_pass]= value.substring(eqI1+1); //Serial.print(" pass=");Serial.println(pass);						    
						}
 else if(cammand=="SNT") { if(value[0]=='S') STANDALONE=1; else   STANDALONE=0; EEPROM.write(STANDALONE_p, STANDALONE);EEPROM.commit();}
 else if(cammand=="SVE")  { client->text("SVE@"); UpdateContent("/data/device.inf",compose()); }
 //'''''''''''''''''''''''''''''''''''''''''   Light & app key control     ''''''''''''''''''''''''''''''''''''''''''''''''''''''''

 else if(cammand=="RAW") {if(!STARTING) {Serial.print(value);  CODE="CNC@"; serialTS=millis(); }} 
 
 
}
void  webSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *payload, size_t length) {
     if (type == WS_EVT_DISCONNECT) {
									  Serial.printf("WS URL=%s ID=%u Disconnected: %u\n", server->url(), client->id());
									  client_active=false;
                  //  client->close();
									 // client->ping();
									  /*
									for (uint8_t i=0; i<MAX_WS_CLIENT ; i++) {
																			  if (ws_client[i].id == client->id() )
																				{
                                       
																				ws_client[i].id = 0;
																				ws_client[i].state = CLIENT_NONE;												
																				break; // Exit for loop
																			  }
																			}
	 */
								}
else if (type == WS_EVT_CONNECT) { 
								 	Serial.printf("Client %u added with ip=", client->id());	Serial.println(client->remoteIP());	
									client->ping();
									MainWSclinet=client->id();
                 // client->keepAlivePeriod(2);
									/*byte index=0;
								  for (  index=0; index<MAX_WS_CLIENT ; index++) 
								  {
									  if (ws_client[index].id == 0 )  
												   {
													ws_client[index].id = MainWSclinet=client->id();
													ws_client[index].state = CLIENT_ACTIVE;
													Serial.printf("Client %u added  at index[%d] with ip=", client->id(), index);	Serial.println(client->remoteIP());												 
													client_active=true;
													//client->ping();
													break;  
												   }
									}
									if(index>=MAX_WS_CLIENT) { Serial.println("Client request rejected ");}//client->ping();
									*/						
                                }
else if (type == WS_EVT_DATA)  {
					AwsFrameInfo * info = (AwsFrameInfo*)arg;
					String msg = "";
					if (info->final && info->index == 0 && info->len == length){
														  if (info->opcode == WS_TEXT) { for (size_t i = 0; i < info->len; i++) msg += (char)payload[i]; }
														  else { // Binary
															char buff[3];
															for (size_t i = 0; i < info->len; i++) 
															{
															  sprintf(buff, "%02x ", (uint8_t)payload[i]);
															  msg += buff;
															}
														  }
														   // Serial.printf("GOT[%s][%u] %s-message[%lu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
															//  Serial.printf("%s\r\n", msg.c_str());
															//Serial.print("[[WS]]"); Serial.println(msg);
														}
					else 			       {
											  if (info->index == 0) 
											  { // Message start
												//if (info->num == 0) //Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
															//		Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
											  }
											  // Continue message
											//  Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + length);
											  if (info->opcode == WS_TEXT) 
											  { // Text
												for (size_t i = 0; i < info->len; i++)  msg += (char)payload[i];
											  }
											  else 
											  { // Binary
												char buff[3];
												for (size_t i = 0; i < info->len; i++) 
												{
												  sprintf(buff, "%02x ", (uint8_t)payload[i]);
												  msg += buff;
												}
											  }
											//  Serial.printf("%s\r\n", msg.c_str());
											  if ((info->index + length) == info->len) 
											  { // Message end
										//		Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
												if (info->final) 
												{
											//	  Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
												}
											  }
											}
					
					parse_webSocket(client, msg,client->id()); 
							  msg.~String();  /*
					for (byte index=0; index<MAX_WS_CLIENT ; index++){
					  if (ws_client[index].id == client->id() ) // if client registered before parse its message
						  {
							parse_webSocket(client, msg,client->id()); 
							  msg.~String();   
						  }  
					}  
					*/
				  }
}
