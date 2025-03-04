

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//************************************************************************
#define SS_PIN  D2  //D2
#define RST_PIN D1  //D1
#define Buzzer D0
#define RedLed D3
#define BlueLed D4
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "Student";
const char *password = "";
const char* device_token  = "9b61956cf7876cbc";
//************************************************************************
String URL = "http://172.31.99.169:8012/rfidattendance/getdata.php"; //computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
//************************************************************************
void setup() {
  delay(1000);
  Serial.begin(115200);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  pinMode(Buzzer, OUTPUT);
  pinMode(RedLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);
  connectToWiFi();
}
//************************************************************************
void loop() {
  //check if there's a connection to Wi-Fi or not
 
  if(!WiFi.isConnected()){
    connectToWiFi();    //Retry to connect to Wi-Fi
  }
  //---------------------------------------------
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID="";
  }
  delay(50);
  //---------------------------------------------
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
   tone(Buzzer, 700);
   delay(200);
   noTone(Buzzer);
   String CardID ="";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if( CardID == OldCardID ){
    return;
  }
  else{
    OldCardID = CardID;
  }
  //---------------------------------------------
//  Serial.println(CardID);
  SendCardID(CardID);
  delay(1000);
}
//************send the Card UID to the website*************
void SendCardID( String Card_uid ){
  
  Serial.println("Sending the Card ID");
  if(WiFi.isConnected()){
    WiFiClient client;
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token); // Add the Card ID to the GET array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request   //Specify content-type header
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload

//    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(Card_uid);     //Print Card ID
    Serial.println(payload);    //Print request response payload

    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
    //  Serial.println(user_name);
     digitalWrite(RedLed,HIGH);
        Serial.println("red on");
        delay(500);

      }
      else if (payload.substring(0, 6) == "logout") {
        String user_name = payload.substring(6);
    //  Serial.println(user_name);
    digitalWrite(BlueLed,HIGH);
        Serial.println("blue on");
        delay(500);
        
      }
    
      else if (payload == "succesful") {
        digitalWrite(BlueLed, HIGH);
        
        delay(500);

      }
      else if (payload == "available") {
        digitalWrite(RedLed, HIGH);
        
        delay(500);

      }
      delay(100);
      http.end();  //Close connection
      digitalWrite(RedLed,LOW);
      digitalWrite(BlueLed,LOW);
      

    }
  }
}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    
    delay(1000);
}
//=======================================================================
