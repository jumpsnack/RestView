#include <Bridge.h>
#include <Console.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>
#include <DHT.h>
#include <SPI.h>
#include <IRremote.h>

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
#define IR1LED_SU 11
#define IR1LED_IN 5

#define IRPIN_2 6
int ir_2_state = 0, last_2_state = 0;
#define IR2LED_SU 12
#define IR2LED_IN 7

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

/**
  IR Remote
*/
#define REMOTE_PIN 13
#define IR_NEED_PAPER 0xFFA25D
#define IR_NEED_REPAIR 0xFFE21D
IRrecv irrcv(REMOTE_PIN);
decode_results results;
int need_1_paper = 0;
int need_2_paper = 0;
int need_1_repair = 0;
int need_2_repair = 0;

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

  pinMode(IR1LED_SU, OUTPUT);
  pinMode(IR1LED_IN, OUTPUT);
  digitalWrite(IR1LED_SU, LOW);
  digitalWrite(IR1LED_IN, LOW);

  pinMode(IR2LED_SU, OUTPUT);
  pinMode(IR2LED_IN, OUTPUT);
  digitalWrite(IR2LED_SU, LOW);
  digitalWrite(IR2LED_IN, LOW);

  /**
     Button SET
  */
  pinMode(BTNPIN, INPUT);

  /**
    Remote SET
  */
  irrcv.enableIRIn();

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

  if (ir_1_state == 1) {//휴지가 소모됨
    digitalWrite(IR1LED_IN, HIGH);
    digitalWrite(IR1LED_SU, LOW);
  } else if (ir_1_state == 0) { //휴지가 충분함
    digitalWrite(IR1LED_SU, HIGH);
    digitalWrite(IR1LED_IN, LOW);
  }

  if (ir_2_state == 1) {//휴지가 소모됨
    digitalWrite(IR2LED_IN, HIGH);
    digitalWrite(IR2LED_SU, LOW);
  } else if (ir_2_state == 0) { //휴지가 충분함
    digitalWrite(IR2LED_SU, HIGH);
    digitalWrite(IR2LED_IN, LOW);
  }
need_1_paper = need_1_repair = 0;
  if (irrcv.decode(&results)) {
    Console.println(results.value, HEX);

    if (results.value == IR_NEED_PAPER) {
      need_1_paper = 1;
    } else if (results.value == IR_NEED_REPAIR) {
      need_1_repair = 1;
    } else {
      need_1_paper = need_1_repair = 0;
    }
    
    irrcv.resume();
  }

  if (isnan(t) || isnan(h) || isnan(ir_1_state) || isnan(ir_2_state) || isnan(pir_1_state) || isnan(pir_2_state))
  {
    Console.println("\nFailed to read from Sensor");
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
    Console.print("PAPER 1 : ");
    Console.print(need_1_paper);
    Console.print("\t");
    Console.print("PAPER 2 : ");
    Console.println(need_2_paper);
    Console.print("REPAIR 1 : ");
    Console.print(need_1_repair);
    Console.print("\t");
    Console.print("REPAIR 2 : ");
    Console.println(need_2_repair);
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

    //add PAPER
    parametri += ",PAPER/";
    parametri += need_1_paper;

    //add REPAIR
    parametri += ",REPAIR/";
    parametri += need_1_repair;

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

    //add PAPER
    parametri += ",PAPER/";
    parametri += need_2_paper;

    //add REPAIR
    parametri += ",REPAIR/";
    parametri += need_2_repair;
  }
}

void sendData() {
  if (client.connect(server, 443)) {
    Console.println("Sending data...");
    delay(500);
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
    delay(500);
  }
  if (client.connected()) {
    client.stop();   //disconnect from server
  }
}
