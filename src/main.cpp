#include <Arduino.h>
#include "ESP8266WiFi.h"


char ssid[] = "BL_TEST";   //  your network SSID (name)
char pass[] = "uzuuav503";    // your network password (use for WPA, or use as key for WEP)
bool alreadyConnected = false; // whether or not the client was connected previously
int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
char ClientData[2]={0};
char SerialData[2]={0};
char ControlData[2]={0};
char BinDate[256]={0};
char flag=0;
char Bin_trans_flag=1;
IPAddress localIP;
WiFiClient client;
WiFiServer server(8086);
//    function declaration
void wait_ack();
void wait_wifi_ack();
void Write_to_Vehicle(char *binfile,uint16_t length);

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial1.begin(115200);
  while (!Serial1) { ; }
  Serial.flush();
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  localIP = WiFi.softAPIP();
  server.begin();
  Serial1.println("Server started");
  Serial1.println(localIP);

  client = server.available();
  while (!client) {
    client=server.available();
    delay(100);
  }
  Serial1.println("new client");
 }

 void loop() {

    Serial.flush();
    while(!client.available())
    {
      Serial1.print(".");
      delay(500);
    }
    while(flag==0)
     {
       client.readBytes(ClientData, 2);
       Serial1.print(ClientData);        //Serial1 is just a debug monitor
       client.flush();                 //clean the cache buffer
       if ((ClientData[0]==0x01)&&(ClientData[1]==0x20))
       {
         Serial1.println("The GCS is ready");
         flag=1;
       }
       ClientData[0] = 0;
       ClientData[1] = 0;
     }
     flag=0;
     Serial.flush();
     while(!Serial.availableForWrite());
     Serial.write(0x21);
     while(!Serial.availableForWrite());
     Serial.write(0x20);
     wait_ack();
     Serial1.println("got the Insync ack");
     delay(10000);

     while(!Serial.availableForWrite());
     Serial.write(0x23);
     while(!Serial.availableForWrite());
     Serial.write(0x20);
     wait_ack();
     Serial1.println("got the Erase ack");


       client.write(0x01);    //tell the GCS ,I am ready to receive your data  .
       client.write(0x20);
       wait_wifi_ack();
       Serial1.println("got the wifi start ack");

       client.write(0x01);    //tell the GCS ,send  your data  .
       client.write(0x21);    //0x01  0x21
     while(Bin_trans_flag==1)    // 此处逻辑有问题
     {
       while(!client.available());
       client.readBytes(ControlData, 2);  //the first bit is status of trans 0x01 :continual   0x02 : last one
       client.readBytes(BinDate, ControlData[1]);
       client.flush();
       Serial1.print(ControlData[0]);
       Serial1.print(ControlData[1]);
       Serial1.print(BinDate[0]);
       Write_to_Vehicle(BinDate,ControlData[1]);
       wait_ack();
       if(ControlData[0]==0x02)
       {
         Bin_trans_flag=0;  //if the first bit equal 0x02  ,after this trans jump out.
       }
       else if(ControlData[0]==0x01)
       {
         client.write(0x01);    //tell the GCS ,send  your data  .
         client.write(0x21);    //0x01  0x21
       }
     }

     while(!Serial.availableForWrite());
     Serial.write(0x30);
     while(!Serial.availableForWrite());
     Serial.write(0x20);
     wait_ack();
      Serial1.println("got the Boot ack");

     client.write(0x02);          //tell the GCS the trans finish, turn the mode of GCS
     client.write(0x20);
     wait_wifi_ack();

 }
void Write_to_Vehicle(char *binfile,uint16_t length)
{
  Serial.write(0x27);
  Serial.write(length);
  for(uint16_t i=0; i<length; i++)
   Serial.write(binfile[i]);
  Serial.write(0x20);
}

// This function is wait for the TCP client echo
//
void wait_wifi_ack()
{
  while(flag==0){
    if(client.available()>0){
      client.readBytes(ClientData, 2);
      Serial1.println(ClientData[0]);
      Serial1.println(ClientData[1]);
      client.flush();
        if((ClientData[0]==0x12)&&(ClientData[1]==0x10))
        {
            flag=1;
            ClientData[0]=0;
            ClientData[1]=0;
            delay(100);
        }
        else
        {
          flag=0;
          SerialData[0]=0;
          SerialData[1]=0;
          delay(100);
        }
    }
    else
    {
      Serial1.print(".");
      delay(1000);
    }
  }
  flag=0;
}

void wait_ack(){
  while(flag==0){
    if(Serial.available()>0){
      Serial.readBytes(SerialData, 2);
      Serial1.println(SerialData[0]);
      Serial1.println(SerialData[1]);
      Serial.flush();
        if((SerialData[0]==0x12)&&(SerialData[1]==0x10))
        {
            flag=1;
            SerialData[0]=0;
            SerialData[1]=0;
            delay(100);
        }
        else
        {
          flag=0;
          SerialData[0]=0;
          SerialData[1]=0;
          delay(100);
        }
    }
    else
    {
      Serial1.print(".");
      delay(1000);
    }
  }
  flag=0;
}
