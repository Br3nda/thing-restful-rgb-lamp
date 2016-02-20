/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            4

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

#define LAMP_OFF 0
#define LAMP_ON 1
#define LAMP_LOOP 2
#define LAMP_RGB 100

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

void updateLamp() {
  Serial.println("Turning off");
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(red, green, blue)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.println(i);
    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  Serial.println("done");
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
    state = "off";
    red = 0; green = 0; blue = 0;
  }
  else if (req.indexOf("/lamp/ON") != -1) {
    red = 255; green = 255; blue = 255;
    state = "on";
  }
  else if(req.indexOf("/lamp/brightness") != -1) {
    if (req.indexOf("/lamp/brightness/UP") != -1) {
      red = red+50; green =green+50; blue=blue+50;
    }
    else if (req.indexOf("/lamp/brightness/DOWN") != -1) {
      red = red-50; green =green-50; blue=blue-50;
    }
  }
  else if (req.indexOf("/lamp/red") != -1) {
    if (req.indexOf("/lamp/red/UP") != -1) {
      red=red+50;
    }
    else if(req.indexOf("/lamp/red/DOWN") != -1) {
      red=red-50;
    }
  }
  else if (req.indexOf("/lamp/green") != -1) {
    if (req.indexOf("/lamp/green/UP") != -1) {
      green=green+50;
    }
    else if(req.indexOf("/lamp/green/DOWN") != -1) {
      green=green-50;
    }
  }  
  else if (req.indexOf("/lamp/blue") != -1) {
    if (req.indexOf("/lamp/blue/UP") != -1) {
      blue=blue+50;
    }
    else if(req.indexOf("/lamp/blue/DOWN") != -1) {
      blue=blue-50;
    }
  }
  

  if (red > 255)
    red = 255;
  if (green > 255)
    green = 255;
  if (blue > 255) 
    blue = 255;

  if (red < 0)
    red = 0;
   if (green < 0)
    green=0;
   if (blue < 0)
    blue=0;

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "red=" + (String)red + "\n";
  s += "green=" + (String)green + "\n";
  s += "blue=" + (String)blue + "\n";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println(s);
  Serial.println("Client disconnected");

  updateLamp();
  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}



