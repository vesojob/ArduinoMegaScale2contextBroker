
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

HX711 scale(A1, A0);

uint8_t buffer[14];
uint8_t* buffer_at;
uint8_t* buffer_end = buffer + sizeof(buffer);
 
String checksum;
boolean tagfound = false;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
 
void setup()
{
    Serial.begin(9600);
    Serial.println("Serial Ready");
 
    Serial1.begin(9600);
    Serial.println("RFID Ready");
 
 
 lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight
lcd.print("Hello, world!");
delay(1000);
lcd.clear(); 
lcd.print("8888888888888888888888888888888888888888888888888888888888888888888");

  delay(1000);
  
 Serial.println("HX711 Demo");

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided 
            // by the SCALE parameter (not set yet)  

  scale.set_scale(-431.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided 
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");



    
}
 
void loop()
{

Serial.print("one reading:\t");
  Serial.println(scale.get_units(), 1);
 // Serial.print("\t| average:\t");
 // Serial.println(scale.get_units(10), 1);

 // scale.power_down();              // put the ADC in sleep mode
  //delay(5000);
 // scale.power_up();


 // delay(1000);  
    if (Serial1.available()){
        lcd.clear();
        delay(20);
        float gramm; 
        gramm = scale.get_units() ;
         Serial.print(gramm);
        lcd.setCursor(0,0);
        lcd.print(gramm);
  Serial.println(" g");

  delay(2);
        buffer_at = buffer;
 
        while ( buffer_at < buffer_end )
        {
            *buffer_at++ = Serial1.read();
        }
        tagfound = true;
       Serial1.end();
       Serial1.begin(9600);
    }
 
    if (tagfound){
        buffer_at = buffer;
        uint32_t result = 0;
 
        // Skip the preamble
        ++buffer_at;
        // Accumulate the checksum, starting with the first value
        uint8_t checksum = rfid_get_next();
        // We are looking for 4 more values
        int i = 4;
        while(i--)
        {
            // Grab the next value
            uint8_t value = rfid_get_next();
            // Add it into the result
            result <<= 8;
            result |= value;
            // Xor it into the checksum
            checksum ^= value;
        }
        // Pull out the checksum from the data
        uint8_t data_checksum = rfid_get_next();
 
        // Print the result
        Serial.print("Tag: ");
        Serial.print(result);
        lcd.setCursor(0,1);
        lcd.print(result);
        if ( checksum == data_checksum )
            Serial.println(" OK");
        else
            Serial.println(" CHECKSUM FAILED");
        // We're done processing, so there is no current value
 
        tagfound = false;
    }
    //delay(1000);
 //    scale.tare();
}
 
uint8_t rfid_get_next(void)
{
    // sscanf needs a 2-byte space to put the result but we
    // only need one byte.
    uint16_t hexresult;
    // Working space to assemble each byte
    static char byte_chars[3];
    // Pull out one byte from this position in the stream
    snprintf(byte_chars,3,"%c%c",buffer_at[0],buffer_at[1]);
    sscanf(byte_chars,"%x",&hexresult);
    buffer_at += 2;
    return static_cast<uint8_t>(hexresult);
}
