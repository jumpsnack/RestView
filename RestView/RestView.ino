#include <Bridge.h>
#include <Console.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>
#include <DHT.h>
#include <SPI.h>

void updateData();
void sendData();

const char* server = "210.125.31.34";
//const char* server = "192.168.0.104";
char buffer[64];

YunClient client;
String parametri = "";

/**
   DHT SET
*/
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/**
   IR SET
*/
#define IRPIN_1 4
int ir_1_state = 0, last_1_state = 0;

#define IRPIN_2 6
int ir_2_state = 0, last_2_state = 0;

/**
    PIR SET
*/
#define PIRPIN_1 8
int pir_1_state = 0;

#define PIRPIN_2 10
int pir_2_state = 0;

/**
   Button SET
*/
#define BTNPIN 9
int btn_state = 0;

static char tbuffer[6];
static char hbuffer[6];
static char ir1buffer[2];
static char ir2buffer[2];
static char pir1buffer[2];
static char pir2buffer[2];
static char btnbuffer[2];

void setup()
{
  Bridge.begin();
  Console.begin();


  /**
      IR SET
  */
  pinMode(IRPIN_1, INPUT);
  pinMode(IRPIN_2, INPUT);
  pinMode(PIRPIN_1, INPUT);
  pinMode(PIRPIN_2, INPUT);
  digitalWrite(IRPIN_1, HIGH);
  digitalWrite(IRPIN_2, HIGH);

  /**
     Button SET
  */
  pinMode(BTNPIN, INPUT);


  while (!Console);
  parametri = "cmd=clear";
  sendData();
}

void loop()
{
  updateData();
  sendData();
}

void updateData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  ir_1_state = digitalRead(IRPIN_1);
  ir_2_state = digitalRead(IRPIN_2);
  pir_1_state = digitalRead(PIRPIN_1);
  pir_2_state = digitalRead(PIRPIN_2);
  btn_state = digitalRead(BTNPIN);

  if (isnan(t) || isnan(h) || isnan(ir_1_state) || isnan(ir_2_state) || isnan(pir_1_state) || isnan(pir_2_state))
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
    Console.print("IR 1 : ");
    Console.print(ir_1_state);
    Console.print("\t");
    Console.print("IR 2 : ");
    Console.println(ir_2_state);
    Console.print("PIR 1 : ");
    Console.print(pir_1_state);
    Console.print("\t");
    Console.print("PIR 2 : ");
    Console.println(pir_2_state);
    //Console.print("BTN: ");
    //Console.print(btn_state);
    Console.print("\n");

    //convert the sensor data from float to string
    // sprintf(irbuffer, "%d", ir_state);
    //sprintf(btnbuffer, "%d", btn_state);
    itoa(ir_1_state, ir1buffer, 10);
    itoa(ir_2_state, ir2buffer, 10);
    itoa(pir_1_state, pir1buffer, 10);
    itoa(pir_2_state, pir2buffer, 10);
    itoa(btn_state, btnbuffer, 10);
    dtostrf(t, 5, 2, tbuffer);
    dtostrf(h, 5, 2, hbuffer);


    //add Humidity:
    parametri = "1=";
    parametri += "Humidity/";
    parametri += hbuffer;

    // convert the readings to a String to send it:
    parametri += ",Temperature/";
    parametri += tbuffer;

    //add IR, if ir == 0 : Full, ir == 1 : empty
    parametri += ",IR/";
    parametri += ir1buffer;

    parametri += ",PIR/";
    parametri += pir1buffer;

    //add BTN
    parametri += ",BTN/";
    parametri += btnbuffer;

    //add Humidity:
    parametri += "&2=";
    parametri += "Humidity/";
    parametri += hbuffer;

    // convert the readings to a String to send it:
    parametri += ",Temperature/";
    parametri += tbuffer;

    //add IR, if ir == 0 : Full, ir == 1 : empty
    parametri += ",IR/";
    parametri += ir2buffer;

    parametri += ",PIR/";
    parametri += pir2buffer;

    //add BTN
    parametri += ",BTN/";
    parametri += btnbuffer;
  }
}

void sendData() {
  if (client.connect(server, 443)) {
    Console.println("Sending data...");
    delay(2500);
    client.print("POST ");
    client.print("/?");
    client.print(parametri);
    client.println(" HTTP / 1.1");
    client.println("Connection: Close");
    client.println();
    Console.println("done!\n\n");
  } else {
    Console.println("connection failed");
    //digitalWrite(led_rosso, HIGH);
    delay(1000);
  }
  if (client.connected()) {
    client.stop();   //disconnect from server
  }
}
