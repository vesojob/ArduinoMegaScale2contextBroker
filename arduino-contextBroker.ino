#include <SPI.h>
#include <SoftwareSerial.h>

//memory check
#include <MemoryFree.h>
#include <pgmStrToRAM.h>

//Ethernet shield
#include <Ethernet.h>

//Light meter
#include "TSL2561.h"
#include <Wire.h>
#include <RTClib.h>

//RFID
#include <MFRC522.h>

//LSD display
#include <LiquidCrystal.h>

//==============================================================================================  
//
//  MACROS
//
//==============================================================================================  
//Messages 
#define msgInit "Initializing.."
#define msgSend "Sending.."
#define msgErr "Fail"
#define msgReady "Ready"

//SENSOR ID
#define sid       10606353

//ContextBroker
#define url             "95.111.115.171"  
#define PORT            1026 


#define MAX_ATTR_COUNT  7

//5 Seconds wait before refresh
#define LCD_WAIT_TIME_REFRESH 5

//RFID Reader
#define RST_PIN    9   //change defaults for RFID 
#define SS_PIN    8  //change defaults for RFID

//==============================================================================================  
//
//  Global Variables
//
//==============================================================================================  
//TIME
RTC_DS1307 RTC;

//TMP36 Pin Variables
int sensorPin = 0;
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
TSL2561 tsl(TSL2561_ADDR_FLOAT);


MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//Ethernet config
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Client
EthernetClient client;

unsigned long nLCDLastTimeRefresh= 0;
//==============================================================================================  
//
// Namespaces
//
//==============================================================================================  

namespace CHTTPRequest
{
  typedef struct _AttrData
  {
      String sAttrName;
      String sAttrType;
      String sAttrValue;
  } CAttrData ;
  
  
//----------------------------------------------------------------------------------------------    
//
//
//----------------------------------------------------------------------------------------------  
int sendContext(const char * szTypeID
                  , unsigned long nElementID
                  ,  CAttrData   arrAttributes[]
                  ,  int nArrElemCount
                  , unsigned long timestamp )
  {
       if (nArrElemCount > MAX_ATTR_COUNT)
       {
          // Serial.println("Too many elements in 'arrAttributes' array.");
          // Serial.println("If you want to pass more elements then increase count of arrCBStrings.");
          
          return -1; // fail
       }
       else
       {
          lcd.setCursor(0, 1);
          lcd.print(nElementID);
          if (client.connect(url, PORT)) 
          {
            long conLength = 0;
            
            String arrCBStrings[MAX_ATTR_COUNT + 3];
            int nStrIndex = 0;
            
            arrCBStrings[nStrIndex] = "{\"contextElements\":[{\"type\":\"";
            arrCBStrings[nStrIndex]+=  szTypeID;
            arrCBStrings[nStrIndex]+=  "\",\"isPattern\":\"false\",\"id\":\"";
            arrCBStrings[nStrIndex]+=  nElementID;
            
            nStrIndex++;
            
            arrCBStrings[nStrIndex]= "\",\"attributes\":[{\"name\":\"ts\",\"type\":\"string\",\"value\":\"";
            arrCBStrings[nStrIndex] += timestamp;
   
            nStrIndex++;
    
            for (int nCurrAttrIndex = 0; nCurrAttrIndex < nArrElemCount; nCurrAttrIndex++)
            {
              arrCBStrings[nStrIndex]= "\"},{\"name\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrName;
              arrCBStrings[nStrIndex]+= "\",\"type\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrType;
              arrCBStrings[nStrIndex]+= "\",\"value\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrValue;
    
              nStrIndex++;  
            }
            
            nStrIndex++;
            arrCBStrings[nStrIndex] = "\"}]}],\"updateAction\":\"APPEND\"}";
            
            //Check Context Length: 
            
            for (int nCurrLenIndex = 0; nCurrLenIndex <= nStrIndex; nCurrLenIndex++)
            {
            //  Serial.println(arrCBStrings[nCurrLenIndex]);
              conLength+= arrCBStrings[nCurrLenIndex].length();
            }

            Serial.println("connected");
            Serial.println(conLength);
            
            // Make a HTTP request:
            client.println("POST  /ngsi10/updateContext");
            //client.println("Connection: keep-alive");
            client.println("Accept: application/json");
            client.println("Content-Type: application/json");
            //client.println("Content-Type:   text/plain; charset=UTF-8");
            //client.print("X-Auth-Token: "); 
            //client.println("VMfS1db5lyV3mtJ7YSqAqPFHR9cu9m");
          
            //client.println("Connection: close");
            client.print("Content-Length: ");
            client.println(conLength);
            client.println(); 
            for (int nCurrStrIndex = 0; nCurrStrIndex <= nStrIndex; nCurrStrIndex++)
            {
              client.print(arrCBStrings[nCurrStrIndex]);
            }
            client.println();
            //client.println("Connection: close");
            //Serial.println(data);
            client.stop();
            //delay (1000);
            return 0;
         }
         else 
         {
          // if you didn't get a connection to the server:
             Serial.println(msgErr);
                  lcd.setCursor(10, 0);
                  lcd.print(msgErr);
         }  
     }
  }
//----------------------------------------------------------------------------------------------    
//
//
//----------------------------------------------------------------------------------------------    
};

//==============================================================================================  
//
// Functions
//
//==============================================================================================    
void setup(){
    //Init serial bus
    Serial.begin(9600);
    
    //Init SPI bus
    SPI.begin();      
    
    //Init Time
    Wire.begin();
    RTC.begin();
 
    
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
    
    delay(2000); 
}
//----------------------------------------------------------------------------------------------  
//
//
//----------------------------------------------------------------------------------------------  
void UpdateLCD(const DateTime & currTimeDate,  float temperatureC)
{
    Serial.print("Timestamp= ");
    Serial.println(currTimeDate.unixtime());
    Serial.print(currTimeDate.year());
    Serial.print('/');
    Serial.print(currTimeDate.month());
    Serial.print('/');
    Serial.print(currTimeDate.day());
    Serial.print(' ');
    Serial.print(currTimeDate.hour());
    Serial.print(':');
    Serial.print(currTimeDate.minute());
    Serial.print(':');
    Serial.println(currTimeDate.second());
    
    lcd.setCursor(0, 1);
    lcd.print(currTimeDate.hour());
    lcd.print(":");
    lcd.print(currTimeDate.minute());
    lcd.print(":");
     lcd.print(currTimeDate.second());
      //temperature
  
 /* int reading = analogRead(sensorPin);  
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  Serial.print(voltage); Serial.println(" volts");
  float temperatureC = (voltage - 0.5) * 100 ;  */
  Serial.print(temperatureC); Serial.println(" degrees C");
   
   char str_temp[6];
   /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
     dtostrf(temperatureC, 4, 2, str_temp);
     //sprintf(temperature,"%s F", str_temp);

    Serial.println(str_temp);

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



}

//----------------------------------------------------------------------------------------------  
//
//
//----------------------------------------------------------------------------------------------  
void loop() 
{

  lcd.clear();
  DateTime now = RTC.now();
  
 

  if ((0 == nLCDLastTimeRefresh)
     || (nLCDLastTimeRefresh + LCD_WAIT_TIME_REFRESH < now.unixtime())
     )
  {

    int reading = analogRead(sensorPin);  
    float voltage = reading * 5.0;
    voltage /= 1024.0; 
    Serial.print(voltage); Serial.println(" volts");
    float temperatureC = (voltage - 0.5) * 100 ;  
    
    UpdateLCD(now, temperatureC);
    
    nLCDLastTimeRefresh = now.unixtime();
  }
  //delay(1000);
        
        //RFID card operation     
        int progress;
        // Look for new cards
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
                return;
        }
        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial()) 
        { return;  }
        
        
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sending..");
        
        Serial.print("  freeMemory()=");
        Serial.println(freeMemory());
        
        //Genereting Card serial niumber

        CHTTPRequest::CAttrData  attrItem[2];       

        attrItem[0].sAttrName = "sid";
        attrItem[0].sAttrType = "string";
        attrItem[0].sAttrValue+= sid;

        attrItem[1].sAttrName = "kg";
        attrItem[1].sAttrType = "integer";
        attrItem[1].sAttrValue+= 1300;
        
        progress = CHTTPRequest::sendContext("b"
                                ,dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size)
                                ,attrItem
                                ,2
                                , now.unixtime());   

        CHTTPRequest::CAttrData  attrSensorItems[4];  
        char szTemperature[6];
   /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
         dtostrf(26.6, 4, 2, szTemperature);
     
        attrSensorItems[0].sAttrName = "T";
        attrSensorItems[0].sAttrType = "float";
        attrSensorItems[0].sAttrValue = "";
        attrSensorItems[0].sAttrValue += szTemperature;

        attrSensorItems[1].sAttrName = "H";
        attrSensorItems[1].sAttrType = "integer";
        attrSensorItems[1].sAttrValue = "";
        attrSensorItems[1].sAttrValue+= 78;

        attrSensorItems[2].sAttrName = "M";
        attrSensorItems[2].sAttrType = "integer";
        attrSensorItems[2].sAttrValue = "";
        attrSensorItems[2].sAttrValue+= 78;

        attrSensorItems[3].sAttrName = "F";
        attrSensorItems[3].sAttrType = "integer";
        attrSensorItems[3].sAttrValue = "";
        attrSensorItems[3].sAttrValue+= 78;
        
        progress = CHTTPRequest::sendContext("s"
                                          , (unsigned long)sid
                                          ,attrSensorItems
                                          ,4
                                          , now.unixtime());                                   
        //Serial.println(progress);
        
        
       
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
            
        }
           
  delay(3000);
}
//----------------------------------------------------------------------------------------------    
//
//
//----------------------------------------------------------------------------------------------  
long dump_byte_array(byte *buffer, byte bufferSize) {
   
    String sID;
        if (mfrc522.uid.size <= 4)
        {
           unsigned long nNumber = 0 ;
            for (int nCurrByte = 0; nCurrByte < mfrc522.uid.size; nCurrByte++ )
              {
                  unsigned long nTempVal = mfrc522.uid.uidByte[nCurrByte]; //left bytes side read
                  //unsigned long nTempVal = mfrc522.uid.uidByte[((mfrc522.uid.size-1) -   nCurrByte)]; //right bytes side read
              
                  nTempVal<<= (((mfrc522.uid.size-1) -   nCurrByte) * 8);
                  nNumber += nTempVal;
              }
              Serial.println(nNumber);
              return  nNumber;
        }
        else
        {
          Serial.println("Error! source buffer is larger than 4 bytes!");
        }
}
//----------------------------------------------------------------------------------------------    
//
//
//----------------------------------------------------------------------------------------------  



 
