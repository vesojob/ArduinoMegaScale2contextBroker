/*
Arduino RFID ContextBroker 

*/

#include <MFRC522.h>
#include <b64.h>
#include <HttpClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

#define LOCK A0                       //mechanical LOCK power out pin
#define RED_LED A1                    //access denied LED
#define LOCK_OPEN_TIME 2000           //lock open time in milliseconds
 
//LCD messages
#define msgInit "Initializing.."
#define msgErrDhcp "DHCP Fail"
#define msgCon "Connecting.."
#define msgChkUsr "Validating User."
#define msgReady "Ready.."
#define msgNxt "Try Next card!"
#define msgChkFail "Check: Failed"

#define PORT  81              //web request port
char rfidChar;
String userName ="";


//REST API settings
#define apiKey                "aa1bf85f3d7f43d9abd104df9ea16b4e"
#define thingId               "12fc649716e511e49d127f574f86c83b"
#define methodId              "12a638de16e611e49d127f574f86c83b"
#define url                   "192.168.2.97"  //"api.gadgetkeeper.com"


#define RST_PIN    7   // 
#define SS_PIN    6   //

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

EthernetClient client;
HttpClient http(client);
EthernetServer server(80);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

void setup(){
  
    Serial.begin(9600);
    SPI.begin();      // Init SPI bus

    mfrc522.PCD_Init();   // Init MFRC522
    ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
    Serial.println("Scan PICC to see UID, type, and data blocks...");   
    Serial.println(msgInit);  //start the Ethernet connection:
    if (Ethernet.begin(mac) == 0){
        Serial.println(F("Failed to configure Ethernet using DHCP"));
        Ethernet.begin(mac, ip);
    }
      delay(1000);  //give the Ethernet shield a second to initialize:
    
    server.begin(); //start server
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());

    Serial.println(msgReady); 
    
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
 }

void loop() {
 // when characters arrive over the serial port...
    lcd.setCursor(0, 0);
    // clear the screen
    //lcd.clear();
     lcd.print("hello, world!");
   // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

  
        // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
        
    //check     
        int progress;
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
        MFRC522::MIFARE_Key key;
        for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
        
        // Look for new cards
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
                return;
        }

        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial())   {    Serial.println("Deneme1");    return;  }
        
        Serial.print("Card UID:");
        progress = sendCardId(dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
        Serial.print("Validating user: ");
        if(!progress){
            Serial.println("OK ");
        }else{
            Serial.println(F("Fail"));          
        } 
        Serial.println();        
  
  
}

String dump_byte_array(byte *buffer, byte bufferSize) {
    String uid;
    String uid1;
    
    for (byte i = 0; i < bufferSize; i++) {
       /* Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);      */        
        uid += buffer[i] < 0x10 ? " 0" : " ";
        uid += buffer[i];
    }

    Serial.println();    
    Serial.print("uid:");    
    Serial.print(uid);
    
    return uid;
}

int sendCardId(String tag) {
        userName = "";
        tag = "\"uid=" + tag + "\"";
        char msgBuffer[150], valueBuffer[18], msgContentLength = 0;
        int result;           
        memset(msgBuffer , 0 , 150);
        sprintf(msgBuffer , "/rfid.php");  
        http.beginRequest();
        result = http.startRequest(url, PORT, msgBuffer, HTTP_METHOD_POST, "Arduino");
        Serial.println(result);        
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
            memset( msgBuffer , 0 , 80 );
            sprintf( msgBuffer , "X-ApiKey: %s" , apiKey );
            http.sendHeader(msgBuffer);
            http.sendHeader("Content-Type: text/json; charset=UTF-8");
            tag.toCharArray(valueBuffer, 18);
            valueBuffer[strlen(valueBuffer)] = '\r';
            valueBuffer[strlen(valueBuffer)+1] = '\n';
            valueBuffer[strlen(valueBuffer)+2] = '\r';
            valueBuffer[strlen(valueBuffer)+3] = '\n';
            msgContentLength = strlen(valueBuffer) - 1 ;       
       
            //Serial.print(valueBuffer);
            
            memset(msgBuffer , 0 , 80);
            sprintf( msgBuffer , "Content-Length: %d" , msgContentLength );
            http.sendHeader(msgBuffer);
            http.write((const uint8_t*) valueBuffer , msgContentLength + 2 );
            http.endRequest();
            int err =0;
            err = http.responseStatusCode();
            if(err < 0){
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
              Serial.print(c);              
              if(c == '"'){
                saveNow = true;
                count++;    
             }else if(saveNow && count < 2){
                userName = userName + c;          
              }else if(count >= 2){
                break;
              }  
            }                                                                 
            Serial.println();
            Serial.println(F("Connection success"));  
            Serial.print(F("User : "));
            Serial.println(userName);   
            http.stop();     
            return 0;            
        }else{
            http.endRequest();   
            http.stop();                                   
           return 1;
        }         
        
  
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("MFRC522 Software Version: 0x");
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(" = v1.0");
  else if (v == 0x92)
    Serial.print(" = v2.0");
  else
    Serial.print(" (unknown)");
          Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println("WARNING: Communication failure, is the MFRC522 properly connected?");
  }
}
