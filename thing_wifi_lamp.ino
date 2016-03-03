/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include "Wire.h"
#define TMP102_I2C_ADDRESS 72 /* This is the I2C address for our chip.
This value is correct if you tie the ADD0 pin to ground. See the datasheet for some other values. */

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN            4

#define NUMPIXELS      16

#define LAMP_OFF 0
#define LAMP_ON 1
#define LAMP_LOOP 2
#define LAMP_RGB 100

const int LED_PIN = 5; // Thing's onboard, green LED

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 100; // delay for half a second

const char* ssid = "...";
const char* password = "...";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

int red = 0;
int green = 0;
int blue = 0;
int num_leds = 8;

void updateLamp() {
  Serial.println("Turning off");
  for(int i=0;i<num_leds;i++){
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.println(i);
    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  for(int i=num_leds; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // off
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.println((String)i + " off");
    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  Serial.println("done");
}


float getTemp102() {
  byte firstbyte, secondbyte; //these are the bytes we read from the TMP102 temperature registers
  int val; /* an int is capable of storing two bytes, this is where we "chuck" the two bytes together. */
  float convertedtemp; /* We then need to multiply our two bytes by a scaling factor, mentioned in the datasheet. */
  float correctedtemp;
  // The sensor overreads (?)
 
 
  /* Reset the register pointer (by default it is ready to read temperatures)
You can alter it to a writeable register and alter some of the configuration -
the sensor is capable of alerting you if the temperature is above or below a specified threshold. */
 
  Wire.beginTransmission(TMP102_I2C_ADDRESS); //Say hi to the sensor.
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP102_I2C_ADDRESS, 2);
  Wire.endTransmission();
 
 
  firstbyte      = (Wire.read());
/*read the TMP102 datasheet - here we read one byte from
 each of the temperature registers on the TMP102*/
  secondbyte     = (Wire.read());
/*The first byte contains the most significant bits, and
 the second the less significant */
    val = ((firstbyte) << 4);  
 /* MSB */
    val |= (secondbyte >> 4);    
/* LSB is ORed into the second 4 bits of our byte.
Bitwise maths is a bit funky, but there's a good tutorial on the playground*/
    convertedtemp = val*0.0625;
    correctedtemp = convertedtemp - 5;
    /* See the above note on overreading */
 
  delay(10);
  Serial.print("firstbyte is ");
  Serial.print("\t");
  Serial.println(firstbyte, BIN);
  Serial.print("secondbyte is ");
  Serial.print("\t");
  Serial.println(secondbyte, BIN);
  Serial.print("Concatenated byte is ");
  Serial.print("\t");
  Serial.println(val, BIN);
  Serial.print("Converted temp is ");
  Serial.print("\t");
  Serial.println(val*0.0625);
  Serial.print("Corrected temp is ");
  Serial.print("\t");
  Serial.println(correctedtemp);
  Serial.println();

//  char buf[100];
//  sprintf(buf, "%f", correctedtemp);

  return correctedtemp;
}


void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  updateLamp();
}


void fixValuesRange() {
  if (red > 255)
    red = 255;
  if (green > 255)
    green = 255;
  if (blue > 255) 
    blue = 255;
   if (num_leds > NUMPIXELS)
    num_leds = NUMPIXELS;

  if (red < 0)
    red = 0;
   if (green < 0)
    green=0;
   if (blue < 0)
    blue=0;
   if(num_leds < 0)
    num_leds = 0;
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  String state;
  if (req.indexOf("/lamp/OFF") != -1) {
    red = 0; green = 0; blue = 0;
  }
  else if (req.indexOf("/lamp/ON") != -1) {
    red = 150; green = 150; blue = 150;
  }
  else if(req.indexOf("/lamp/brightness") != -1) {
    if (req.indexOf("UP") != -1)
      red+=15; green+=15; blue+=15;  
    if (req.indexOf("DOWN") != -1)
      red-=15; green-=15; blue-=15;
  } else if(req.indexOf("/lamp/leds") != -1) {
    if (req.indexOf("UP") != -1)
      num_leds++;
    if (req.indexOf("DOWN") != -1)
      num_leds--;
  }
  else if (req.indexOf("/lamp/red") != -1) {
    if (req.indexOf("UP") != -1)
      red=red+15;
    if(req.indexOf("/lamp/red/DOWN") != -1)
      red=red-15;
  }
  else if (req.indexOf("/lamp/green") != -1) {
    if (req.indexOf("UP") != -1)
      green=green+15;
    if(req.indexOf("DOWN") != -1)
      green=green-15;
  }  
  else if (req.indexOf("/lamp/blue") != -1) {
    if (req.indexOf("UP") != -1)
      blue=blue+15;
    if(req.indexOf("DOWN") != -1)
      blue=blue-15;
  }
  else if(req.indexOf("/lamp/rgb/") !=-1 ) {
    String string_to_look_for = "/lamp/rgb/";
    int rgb_location = req.indexOf(string_to_look_for) + string_to_look_for.length();
    String red_string = req.substring(rgb_location, rgb_location+1);
    String green_string = req.substring(rgb_location+2, rgb_location+3);
    String blue_string = req.substring(rgb_location+4, rgb_location+5);
  }
  else if(req.indexOf("/temp") !=-1 ) {
    client.flush();
    float temp = getTemp102();
    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
    s += "temp=" + (String)temp + "\n";
    s += "</html>\n";
  
    // Send the response to the client
    client.print(s);
    delay(10);
    return;
  }
  
  fixValuesRange();


  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "red=" + (String)red + "\n";
  s += "green=" + (String)green + "\n";
  s += "blue=" + (String)blue + "\n";
  s += "leds=" + (String)num_leds + "\n";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println(s);
  

  updateLamp();
  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed

  Serial.println("Client disconnected");
}



