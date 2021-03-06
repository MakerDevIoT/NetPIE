// ----- 17 Feb 2018 ------- feed freeboard led
#include <ESP8266WiFi.h>
#include <MicroGear.h>              // library  ของ NETPIE คือ MicroGear
#include "DHT.h"                   // library สำหรับอ่านค่า DHT Sensor ต้องติดตั้ง DHT sensor library by Adafruit v1.2.3 ก่อน


// ---***** แก้ค่า config 7 ค่าข้างล่างนี้ *****-----------
const char* ssid     = "IOT KMUTNB_1";       // ชื่อ ssid
const char* password = "kmutnb2018";      // รหัสผ่าน wifi

#define APPID   "devtest01"                     // ให้แทนที่ด้วย AppID รวม
#define KEY     "PPhIEmSjeZ9fT9G"               // ให้แทนที่ด้วย Key รวม
#define SECRET  "kYFZJKihDo8YUmVGJ6Mr5X31W"     // ให้แทนที่ด้วย Secret รวม


#define ALIAS   "nodemcu_jirawin"              // แทนที่ด้วยหมายเลขของท่าน เช่น "A01" คือตัว node mcu ของเราเลย
#define NEIGHBOR "nodemcu_sraygraew"              // ชื่ออุปกรณ์ของเพื่อน เช่น "A02"

// Step 1 -------------------------------LINE-----------------------------------------------
#define LINE_TOKEN "LBvWsdNuoF7FdPgSQddQeLbRiXKj6pVmM90aaPFvMAk" //เอา token มาใส่
void Line_Notify(String message);

// End Step 1 -------------------------------LINE-------------------------------------------


#define TARGETNAME "display"

/* การส่งข้อมูลแบบ Json
*  จะส่งข้อมูลหัวข้อส่งแค่สถานะ
*/

#define LEDSTATETOPIC "/ledstate/" ALIAS      // topic ที่ต้องการ publish ส่งสถานะ led ในที่นี้จะเป็น /ledstate/{ชื่อ alias ตัวเอง} 
#define DHTDATATOPIC "/dht/" ALIAS            // topic ที่ต้องการ publish ส่งข้อมูล dht ในที่นี่จะเป็น /dht/{ชื่อ alias ตัวเอง}


#define BUTTONPIN  D5                         // pin ที่ต่อกับปุ่ม Flash บนบอร์ด NodeMCU
#define LEDPIN     D2                         // pin ที่ต่อกับไฟ LED บนบอร์ด NodeMCU


//------------***** FEEDID *****---------------
#define FEEDID   "FeedDev01"                          // ให้แทนที่ด้วย FeedID
#define FEEDAPI  "OWmRHPWCR9BNNt7N3KeA92nlTbpuIgeB"   // ให้แทนที่ด้วย FeedAPI เอามาจาก permition 


int currentLEDState = 0;      // ให้เริ่มต้นเป็น OFF
int lastLEDState = 1;
int currentButtonState = 1;   // หมายเหตุ ปุ่ม flash ต่อเข้ากับ GPIO0 แบบ pull-up
int lastButtonState = 0;

#define DHTPIN    D4          // GPIO2 ขาที่ต่อเข้ากับขา DATA (บางโมดูลใช้คำว่า OUT) ของ DHT
#define DHTTYPE   DHT22       // e.g. DHT11, DHT21, DHT22
DHT dht(DHTPIN, DHTTYPE);

float humid = 0;     // ค่าความชื้น
float temp  = 0;     // ค่าอุณหภูมิ

long lastDHTRead = 0;
long lastDHTPublish = 0;

long lastTimeWriteFeed = 0;

WiFiClient client;
MicroGear microgear(client);

void updateLED(int state) {
    currentLEDState = state;

    // ไฟ LED บน NodeMCU เป็น active-low จะติดก็ต่อเมื่อส่งค่า LOW ไปให้ LEDPIN
    if (currentLEDState == 1) 
         digitalWrite(LEDPIN, HIGH); // LED ON
    else  
          digitalWrite(LEDPIN, LOW); // LED OFF
}    

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg);

    if (*(char *)msg == '0') updateLED(0);
    else if (*(char *)msg == '1') updateLED(1);
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    microgear.setAlias(ALIAS);
}

void setup() {
    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

    Serial.begin(115200);
    Serial.println("Starting...");
    dht.begin(); // initialize โมดูล DHT

    // กำหนดชนิดของ PIN (ขาI/O) เช่น INPUT, OUTPUT เป็นต้น
    pinMode(LEDPIN, OUTPUT);          // LED pin mode กำหนดค่า
    pinMode(BUTTONPIN, INPUT);        // Button pin mode รับค่า
    updateLED(currentLEDState);

    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    microgear.init(KEY,SECRET,ALIAS);   // กำหนดค่าตันแปรเริ่มต้นให้กับ microgear
    microgear.connect(APPID);           // ฟังก์ชั่นสำหรับเชื่อมต่อ NETPIE

//----- Step 2 Line setup
  Line_Notify("เริ่มต้นการใช้งาน อบรม IoT KMUTNB ปราจีนบุรี");  // เว็บใส่ภาษาไทย https://meyerweb.com/eric/tools/dencoder/
// End Step 2-----Line setup
    
}


void loop() {
    if (microgear.connected()) {
        microgear.loop();

        if(currentLEDState != lastLEDState){
          microgear.publish(LEDSTATETOPIC, currentLEDState);  // LEDSTATETOPIC ถูก define ไว้ข้างบน
          lastLEDState = currentLEDState;
        }

        if (digitalRead(BUTTONPIN)==HIGH) currentButtonState = 0;
        else currentButtonState = 1;

        if(currentButtonState != lastButtonState){
          microgear.chat(NEIGHBOR, currentButtonState);
          lastButtonState = currentButtonState;
        }

        // เซนเซอร์​ DHT อ่านถี่เกินไปไม่ได้ จะให้ค่า error เลยต้องเช็คเวลาครั้งสุดท้ายที่อ่านค่า
        // ว่าทิ้งช่วงนานพอหรือยัง ในที่นี้ตั้งไว้ 2 วินาที ก
        if(millis() - lastDHTRead > 2000){
          humid = dht.readHumidity();     // อ่านค่าความชื้น
          temp  = dht.readTemperature();  // อ่านค่าอุณหภูมิ
          lastDHTRead = millis();
          
          Serial.print("Humid: "); Serial.print(humid); Serial.print(" %, ");
          Serial.print("Temp: "); Serial.print(temp); Serial.println(" C ");
    
          // ตรวจสอบค่า humid และ temp เป็นตัวเลขหรือไม่
          if (isnan(humid) || isnan(temp)) {
            Serial.println("Failed to read from DHT sensor!");
          }
          else{
            // เตรียมสตริงในรูปแบบ "{humid},{temp}"
            String datastring = (String)humid+","+(String)temp;
            Serial.print("Sending --> ");
            Serial.println(datastring);
            microgear.publish(DHTDATATOPIC,datastring);   // DHTDATATOPIC ถูก define ไว้ข้างบน
          }
        }
        
        if(millis()-lastTimeWriteFeed > 15000){
          lastTimeWriteFeed = millis();
          if(humid!=0 && temp!=0){
            String 
            data = "{\"humid\":";     //************** เปลี่ยนชื่อให้ตรงกับชื่อในตารางของกราฟ feed
            data += humid ;           // ชื่อ data
            data += ", \"temp\":";   //**************  เปลี่ยนชื่อให้ตรงกับชื่อในตารางของกราฟ feed
            data += temp ;
            data += "}"; 
            Serial.print("Write Feed --> ");
            Serial.println(data);
           // microgear.writeFeed(FEEDID,data);
            microgear.writeFeed(FEEDID,data,FEEDAPI); ///////////// เพิ่ม

            // เพิ่มเพื่อให้ dashboard ทำงาน ได้ 
            String freeboard = (String)humid + "," + (String)temp;       // เพิ่มเพื่อให้ dashboard ทำงาน ได้ 
            microgear.chat(TARGETNAME,freeboard);

// Step 3 ------start Line---------------------
String message;
if (temp >28)  // ปรับเปลี่ยนในอุณหภูมิให้แจ้งเตือน
{
message = (String)humid+ "\n" +(String)temp;
Line_Notify(message);
}
// End Step 3 ------ end Line App---------------
            
          }
        }
    }
    else {
        Serial.println("connection lost, reconnect...");
        microgear.connect(APPID); 
    }
}

// Step 4-----------Line Funtion or Lib ----------------------//
void Line_Notify(String message) 
{
WiFiClientSecure client;
if (!client.connect("notify-api.line.me", 443)) {
Serial.println("connection failed");
return; 
}
String req = "";
req += "POST /api/notify HTTP/1.1\r\n";
req += "Host: notify-api.line.me\r\n";
req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
req += "Cache-Control: no-cache\r\n";
req += "User-Agent: ESP8266\r\n";
req += "Content-Type: application/x-www-form-urlencoded\r\n";
req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
req += "\r\n";
req += "message=" + message;
client.print(req);
delay(20);
while(client.connected()) {
String line = client.readStringUntil('\n');
if (line == "\r") {
break;
    }
  }
}

