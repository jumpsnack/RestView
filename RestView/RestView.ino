#include "DHT.h"
#include <Console.h>
#include <Process.h>

//define xively connect info
#define APIKEY        "pzRV0MjPrEFfOT4zHAdSEkJwWvt9xjiOJvCJZkvYSoruMvgk"   // replace your xively api key here
#define FEEDID        "46267484"                   // replace your xively feed ID

/**
   DHT SET
*/
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/**
   IR SET
*/
#define IRPIN 8
int ir_state = 0, last_state = 0;

/**
 * Button SET
*/
#define BTNPIN 9
int btn_state = 0;


// set up net client info:
const unsigned long postingInterval = 6000;  //delay between updates to xively.com
unsigned long lastRequest = 0;      // when you last made a request
String dataString = "";

static char tbuffer[6];
static char hbuffer[6];
static char irbuffer[2];
static char btnbuffer[2];

void setup()
{
  Bridge.begin();
  Console.begin();

  /**
     IR SET
  */
  pinMode(IRPIN, INPUT);
  digitalWrite(IRPIN, HIGH);

  /**
   * Button SET
  */
  pinMode(BTNPIN, INPUT);

  while (!Console) {
    ; // wait for Console port to connect.
  }

}

void loop()
{
  // get a timestamp so you can calculate reading and sending intervals:
  long now = millis();

  // if the sending interval has passed since your
  // last connection, then connect again and send data:
  if (now - lastRequest >= postingInterval) {
    updateData();
    sendData();
    lastRequest = now;
  }
}

void updateData() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  ir_state = digitalRead(IRPIN);
  btn_state = digitalRead(BTNPIN);

  if (isnan(t) || isnan(h) || isnan(ir_state))
  {
    Console.println("\nFailed to read from DHT");
  }
  else
  {
    // print the sensor data in serial monitor
    Console.print("Humidity: ");
    Console.print(h);
    Console.print(" %\t");
    Console.print("Temperature: ");
    Console.print(t);
    Console.println(" *C");
    Console.print("IR: ");
    Console.print(ir_state);
    Console.print("\t");
    Console.print("BTN: ");
    Console.print(btn_state);
    Console.print("\n");

    //convert the sensor data from float to string
   // sprintf(irbuffer, "%d", ir_state);
   //sprintf(btnbuffer, "%d", btn_state);
   itoa(ir_state, irbuffer, 10);
   itoa(btn_state, btnbuffer, 10);

    dtostrf(t, 5, 2, tbuffer);
    dtostrf(h, 5, 2, hbuffer);


    //add Humidity:
    dataString = "Humidity,";
    dataString += hbuffer;

    // convert the readings to a String to send it:
    dataString += "\nTemperature,";
    dataString += tbuffer;

    //add IR, if ir == 0 : Full, ir == 1 : empty
    dataString += "\nIR,";
    dataString += irbuffer;

    //add BTN
    dataString += "\nBTN,";
    dataString += btnbuffer;
  }
}

// this method makes a HTTP connection to the server:
void sendData() {
  // form the string for the API header parameter:
  String apiString = "X-ApiKey: ";
  apiString += APIKEY;

  // form the string for the URL parameter:
  String url = "https://api.xively.com/v2/feeds/";
  url += FEEDID;
  url += ".csv";

  // Send the HTTP PUT request

  // Call a command in MS14.
  // Is better to declare the Process here, so when the
  // sendData function finishes the resources are immediately
  // released. Declaring it global works too, BTW.
  Process xively;
  Console.print("Sending data... ");
  xively.begin("curl");
  xively.addParameter("-k");
  xively.addParameter("--request");
  xively.addParameter("PUT");
  xively.addParameter("--data");
  xively.addParameter(dataString);
  xively.addParameter("--header");
  xively.addParameter(apiString);
  xively.addParameter(url);
  xively.run();
  Console.println("done!\n\n");

  // If there's incoming data from the net connection,
  // send it out the Serial:
  while (xively.available() > 0) {
    char c = xively.read();
    Console.write(c);
  }
}

