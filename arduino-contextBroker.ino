/*
Arduino RFID ContextBroker 

*/
#include <SPI.h>
#include <SoftwareSerial.h>
#include <HttpClient.h>
#include <Wire.h>

//memory check
#include <MemoryFree.h>
#include <pgmStrToRAM.h>

//TIME
#include <DS1307RTC.h>
#include <Time.h>

//Ethernet shield
#include <Ethernet.h>

//Light meter
#include "TSL2561.h"

//RFID
#include <MFRC522.h>

//LSD display
#include <LiquidCrystal.h>

//Messages 
#define msgInit "Initializing.."
#define msgSend "Sending.."
#define msgErr "Fail"
#define msgReady "Ready"

//ContextBroker
#define url             "95.111.115.171"  
#define PORT            1026 
#define MSG_BUF_LEN     20
#define JSON_BUF_LEN    200

//TMP36 Pin Variables
int sensorPin = 0;
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
TSL2561 tsl(TSL2561_ADDR_FLOAT);

//RFID Reader
#define RST_PIN    9   //change defaults for RFID 
#define SS_PIN    8  //change defaults for RFID
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//Ethernet config
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;
HttpClient http(client);

void setup(){
    //Init serial bus
    Serial.begin(9600);
    
    //Init SPI bus
    SPI.begin();      
    
    //Init LCD
    lcd.begin(16, 2);
    
    //Init MFRC522
    mfrc522.PCD_Init();   
    
    Serial.println(msgInit);  
    lcd.print(msgInit);
    
    //Init Ethernet connection:
    if (Ethernet.begin(mac) == 0){
        Serial.println(msgErr);
        lcd.clear();
        lcd.print(msgErr);
        delay(1000000);
    }
    delay(1000);  //give the Ethernet shield a second to initialize:
    
    lcd.clear();
  
    //Serial.print("IP=");
    //lcd.print("IP=");
    
    Serial.println(Ethernet.localIP());
    lcd.print(Ethernet.localIP());
    
    //Init Light module    
    tsl.begin();
    //tsl.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
    tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
    // longer timelines are slower, but are good in very low light situtations!
    tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
     
    lcd.setCursor(0, 1);
    Serial.println(msgReady); 
    lcd.print(msgReady);
    delay(3000); 
}

void loop() {

  //TIME
  tmElements_t tm;
  RTC.read(tm);
 // /*
  char szResVal[4];
  String timestamp;
  Serial.print("Timestamp = ");
  lcd.clear();
  Serial.print(tmYearToCalendar(tm.Year));
  sprintf(szResVal,"%02u", tmYearToCalendar(tm.Year)) ;
  timestamp = szResVal;
  //Serial.println(timestamp);
  sprintf(szResVal,"%02u", tm.Month) ;
  Serial.print(szResVal);
  timestamp=timestamp+szResVal;
  //Serial.println(timestamp);
  sprintf(szResVal,"%02u", tm.Day) ;
  Serial.print(szResVal);
  lcd.setCursor(0, 1);
  timestamp=timestamp+szResVal;
  //Serial.println(timestamp);
  sprintf(szResVal,"%02u", tm.Hour) ;
  lcd.print(szResVal);
  Serial.print(szResVal);
  timestamp=timestamp+szResVal;
  lcd.print(":");
  sprintf(szResVal,"%02u", tm.Minute) ;
  lcd.print(szResVal);
  Serial.print(szResVal);
  timestamp=timestamp+szResVal;
  lcd.print(":");
  sprintf(szResVal,"%02u", tm.Second) ;
  lcd.print(szResVal);
  Serial.print(szResVal);
  timestamp=timestamp+szResVal;
  Serial.println();
  //Serial.println(timestamp);
 // */
  
  //temperature
  int reading = analogRead(sensorPin);  
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  Serial.print(voltage); Serial.println(" volts");
  float temperatureC = (voltage - 0.5) * 100 ;  
  Serial.print(temperatureC); Serial.println(" degrees C");
  

 // This can take 13-402 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2561_VISIBLE);     
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  //Serial.print("Visible: "); Serial.print(full - ir);   Serial.print("\t");
  //Serial.print("Lux: "); Serial.println(tsl.calculateLux(full, ir));
  lcd.setCursor(0, 0);
  lcd.print("T=");
 // lcd.setCursor(2, 0);
  lcd.print(temperatureC);
  lcd.setCursor(9, 0);
  lcd.print("L=");
  //lcd.setCursor(11, 0);
  lcd.print(full - ir);

  delay(1000);
        
        //RFID     
        int progress;
        // Look for new cards
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
                return;
        }
        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial()) 
        { return;  }
        
        Serial.print("Card UID:");
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Card UID:");
        
        Serial.print("  freeMemory()=");
        Serial.println(freeMemory());

        progress = sendCardId(dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size), timestamp,temperatureC );
        
        //lcd.clear();
        //lcd.setCursor(0, 0);
        //lcd.print(dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
        //Serial.println(dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
        //Serial.println("  freeMemory()=");
        //Serial.println(freeMemory());
        
        
        if(!progress){
            Serial.println(msgReady);
            lcd.setCursor(10, 0);
            lcd.print(msgReady);
            
        }else{
            Serial.println(msgErr);
            lcd.setCursor(10, 0);
            lcd.print(msgErr);
                      
        } 
        Serial.println();        
  
  delay(3000);
}

String dump_byte_array(byte *buffer, byte bufferSize) {
    String uid;
    
    for (byte i = 0; i < bufferSize; i++) {
       
        //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        //Serial.print(buffer[i], HEX); 

        //if (i >0)
        //{
        //  uid += buffer[i] < 0x10 ? "0" : " ";
        //}
        uid += buffer[i];
    }

    //Serial.println();    
    //Serial.print("uid:");    
    Serial.print(uid);
    Serial.println();
    lcd.setCursor(0, 1);
    lcd.print(uid);
    return uid;
}

int sendCardId(String tag, String tstamp, float temp) {

        Serial.print(msgSend);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(msgSend);
        lcd.setCursor(0, 1);
        lcd.print(tag);
    
    Serial.print("  freeMemory()=");
    Serial.println(freeMemory());
    //Serial.println(tag);
char str_temp[6];
      
      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(temp, 4, 2, str_temp);
     // sprintf(temperature,"%s F", str_temp);
  
        char msgBuffer[MSG_BUF_LEN], valueBuffer[JSON_BUF_LEN];
        int  msgContentLength = 0;
        char tagValue[32];
        char tstampValue[32];
        tag.toCharArray(tagValue, 32);
        tstamp.toCharArray(tstampValue, 32);
        sprintf(valueBuffer
                , "%s%s%s%s%s%s%s%s%s%s%s%s"
                ,"{\"contextElements\":[{\"type\":\"f\",\"isPattern\":\"false\",\"id\":\""
                , tagValue
                ,"\",\"attributes\":["
                ,"{\"name\":\""
                ,"ts\",\"type\":\"string\",\"value\":\""
                , tstampValue
                , "\"},"
                ,"{\"name\":\""
                ,"t2\",\"type\":\"float\",\"value\":\""
                , str_temp
                , "\"}"
                ,"]}],\"updateAction\":\"APPEND\"}\r\n\r\n\r\n");
        
        int result;           
        memset(msgBuffer , 0 , MSG_BUF_LEN);
        http.beginRequest();
        result = http.startRequest(url, PORT, "/ngsi10/updateContext", HTTP_METHOD_POST, "Arduino");
        
         Serial.println(valueBuffer);  
              
        if(result == HTTP_ERROR_API){
          Serial.println(F("API error"));
          http.endRequest();       
          http.stop();
          return 1;     
        }else if(result == HTTP_ERROR_CONNECTION_FAILED){
            Serial.println(F("Connection failed"));   
            http.endRequest();   
            http.stop();
            return 1;                 
        }else if( result == HTTP_SUCCESS ){     
            
            //Serial.println("begin HTTP request");         
            
            memset( msgBuffer , 0 , MSG_BUF_LEN );
            //http.sendHeader(msgBuffer);
            http.sendHeader("Accept: application/json");
            http.sendHeader("Content-Type: application/json");
            msgContentLength = strlen(valueBuffer) - 1 ;  
            
            //Serial.print(F("  freeMemory()="));
            //Serial.println(freeMemory());
            //Serial.print(valueBuffer);
            
            memset(msgBuffer , 0 , MSG_BUF_LEN);
            sprintf( msgBuffer , "Content-Length: %d" , msgContentLength );
            http.sendHeader(msgBuffer);
            http.write((const uint8_t*) valueBuffer , msgContentLength + 2 );          
            http.endRequest();
            
            //Serial.println(F("HTTP request end"));
            
            int err =0;
            err = http.responseStatusCode();
            if(err < 0){
              Serial.println(err);
              http.stop();
              return 1;
            }
            
            err = http.skipResponseHeaders();
            
            //Serial.println(err);                       
            
            char c;
            int count = 0;
            boolean saveNow =false;
            while(1){
              c = http.read();
            //Print responce from server
              Serial.print(c);              
            
              if(c == '"'){
                saveNow = true;
                count++;    
             }else if(saveNow && count < 2){
                       
              }else if(count >= 2){
                break;
              }  
            }                                                                 
            Serial.println();
            
            http.stop();     
            return 0;            
        }else{
            http.endRequest();   
            http.stop();                                   
           return 1;
        }           
        
  }
