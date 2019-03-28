//-----------------INCLUDES--------------------
#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//--------------------------------------------
#define ONE_WIRE_BUS 2
#define MAX_CMD_LENGTH 1

//--------------------------- Thermometers setup ---------------------------
// You can find addresses with https://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress thermAddr0 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
              thermAddr1 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
              thermAddr2 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//---------------------------- Network setup -------------------------------
byte mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
IPAddress ip(10, 0, 0, 2);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server = EthernetServer(10050); //Zabbix Agent port
EthernetClient client;
boolean connected = false;
//--------------------------------------------
String cmd; //FOR ZABBIX COMMAND

void setup()
{
  Ethernet.begin(mac, ip, gateway, subnet);
  sensors.begin();
  server.begin();
}

void loop()
{
  client = server.available();
  if (client) {
    if (!connected) {
      client.flush();
      connected = true;
    }

    if (client.available() > 0) {
      readTelnetCommand(client.read());
    }

  }
}

//--------------------------------------------
void readTelnetCommand(char c)
{
  cmd += c;
  if (c == '\n' || cmd.length() == MAX_CMD_LENGTH) {
    parseCommand();
  }
}


//--------------------------------------------
float getTemperature(DeviceAddress deviceAddress)
{
  float tempC = 0.00;
  sensors.requestTemperatures();
  tempC = sensors.getTempC(deviceAddress);
  return tempC;
}

//--------------------------------------------
void parseCommand() 
{

  // AGENT.PING      
  if(cmd.equals("agent.ping")) {
     server.println("1");

  // AGENT.VERSION      
  } else if(cmd.equals("agent.version")) {
     server.println("Arduino Zabbix Agent 1.0");

  // AGENT.HOSTNAME      
  } else if(cmd.equals("agent.hostname")) {
     server.println("hostname.example.com");

  // TEMP0.READ
  } else if(cmd.equals("0")) {
    server.println(getTemperature(thermAddr0));

  // TEMP1.READ
  } else if(cmd.equals("1")) {
    server.println(getTemperature(thermAddr1));

  // TEMP2.READ
  } else if(cmd.equals("2")) {
    server.println(getTemperature(thermAddr2));
    
  // NOT SUPPORTED      
  } else {
    server.println("ZBXDZBX_NOTSUPPORTED");
  }
  delay(100);
  cmd = "";
  client.stop();
}
