
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiClient.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
//int solenoide = D2;
/*
In the ESP8266, D3 pin is RST_PIN and
D4 pin is SS_PIN
*/
#define RST_PIN 22
#define SS_PIN 21

MFRC522 reader = MFRC522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

// Credentials to connect to the wifi network
const char *ssid = "Galaxy A71B15A";
const char *password = "hpdm2015@@";
/*
The ip or server address. If you are on localhost, put your computer's IP (for example http://192.168.1.65)
If the server is online, put the server's domain for example https://parzibyte.me
*/
const String SERVER_ADDRESS = "http://192.168.37.155/iuget_rfid";
void setup()
{

  //Setup solenoide
 // pinMode(solenoide, OUTPUT);
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  SPI.begin();

  reader.PCD_Init();
  
 
  // Just wait some seconds...
  delay(4);
  // Prepare the security key for the read and write functions.
  // Normally it is 0xFFFFFFFFFFFF
  // Note: 6 comes from MF_KEY_SIZE in MFRC522.h
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF; //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
}

void loop()
{
  // If not connected, we don't need to read anything, that would be unnecessary
  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }
  // But, if there is a connection we check if there's a new card to read

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!reader.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards. This returns false if read is not successful; and if that happens, we stop the code
  if (!reader.PICC_ReadCardSerial())
  {
    return;
  }

  /*
    At this point we are sure that: there is a card that can be read, and there's a
    stable connection. So we read the id and send it to the server
  */

  String serial = "";
  for (int x = 0; x < reader.uid.size; x++)
  {
    // If it is less than 10, we add zero
    if (reader.uid.uidByte[x] < 0x10)
    {
      serial += "0";
    }
    // Transform the byte to hex
    serial += String(reader.uid.uidByte[x], HEX);
    // Add a hypen
    if (x + 1 != reader.uid.size)
    {
      serial += "-";
    }
  }
  // Transform to uppercase
  serial.toUpperCase();

  // Halt PICC
  reader.PICC_HaltA();
  // Stop encryption on PCD
  reader.PCD_StopCrypto1();
  
  WiFiClient client;
  HTTPClient http;

  // Send the tag id in a GET param
  const String full_url = SERVER_ADDRESS + "/rfid_register.php?serial=" + serial;
   
   Serial.println(full_url);
  http.begin(client,full_url);

  // Make request
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
       
       String userName = http.getString();
      
      // const String &payload = http.getString().c_str(); //Get the request respons e payload
      if (userName!="")
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(userName);
        lcd.setCursor(0,1);
        lcd.print("Present");
//         digitalWrite(solenoide, HIGH);
      }else{
        lcd.clear();
//        digitalWrite(solenoide, LOW);
        lcd.setCursor(0,0);
        lcd.print("Carte Non");
        lcd.setCursor(0,1);
        lcd.print("Enregistree");
        Serial.println("non enregistre ");
      }
    
    }
    else
    {
      
    }
  }
  else
  {
  }

  http.end(); //Close connection
}
