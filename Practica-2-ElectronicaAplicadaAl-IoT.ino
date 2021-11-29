#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <strings_en.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <arduino-timer.h>


#include <math.h>

#include <ArduinoJson.h>

// Sensor temperatura v1.2 Grove
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0

const int pinLed = D2;

const int timeDelayForPostTemperatura = 300000;

auto timer = timer_create_default();

// Para utilizar ESP.deepSleep()
// se debe de conectar el rst al D0 del nodemcu esp8266

// URL del Endpoint que postea la temperatura 
//String url="http://192.168.1.88:9000/api/temperatura"; 
//String urlGet="http://192.168.1.88:9000/api/dispositivo/1";
String url="http://iot-rudy.softpak.com.mx/api/temperatura";
String urlGet="http://iot-rudy.softpak.com.mx/api/dispositivo/1";
// Nombre del Dispositivo
String device="Sensor Temperatura 1";
// Variable para la temperatura
int temp;


void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.begin(115200);
    
    // WiFi.mode(WiFi_STA); // it is a good practice to make sure your code sets wifi mode how you want it.

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    //reset settings - wipe credentials for testing
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...");
    }

    // Aquí ya se conecto el esp8266

    
   // Se declara el Pin D2 como de salida
   pinMode(pinLed, OUTPUT);

   // Timer para enviar al srver la lectura de la temperatura
   timer.every(timeDelayForPostTemperatura, setPostTemperatura);
}

void loop() {
    
  WiFiClient client;
  // Se crea la instacia de la clase HTTPClient
  HTTPClient http; //Creamos el objeto del tipo HTTPClient
  http.useHTTP10(true);
  // Se inicia el objeto con la URL del EndPoint
  http.begin(client,urlGet);
    
  // Header del Post en este caso es Json
  http.addHeader("Content-Type", "application/json");
    
    int httpCode=http.GET();
    //Respuesta del Servidor
    String respuesta=http.getString(); 
    if(httpCode == 200){
      
      DynamicJsonDocument resp(1024);
      deserializeJson(resp, respuesta );
      Serial.println("Encendido: "+ resp["encendido"].as<String>());
      digitalWrite(pinLed, resp["encendido"].as<boolean>());
      
    }


    //Se imprimen las respuestas
    Serial.println("httpCode: "+String(httpCode)); 
    Serial.println("respuesta: "+respuesta);

    //Se termina la comunicación
    http.end(); 

    timer.tick();
    // Se duerme un minuto
    //ESP.deepSleep(60e6);   
}

// Función que calcula la temperatura del sensor Grove v1.2 NTC
// https://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
int temperatura() {
    int a = analogRead(pinTempSensor);
 
    float R = 1023.0/a-1.0;
    R = R0*R;
 
    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
 
    Serial.print("temperature = ");
    Serial.println(temperature);    
    return temperature;
}

bool setPostTemperatura(void *){
  WiFiClient client;
  HTTPClient httpB; //Creamos el objeto del tipo HTTPClient
  httpB.useHTTP10(true);
  // Se inicia el objeto con la URL del EndPoint
  httpB.begin(client,url); 
  // Header del Post en este caso es Json
  httpB.addHeader("Content-Type", "application/json");
    
  // Se asigna el valor que se postea
  temp = int(temperatura());
    
  // Se arama la cadena del Json "payload"
  String postData="{\"dispositivo\":\""+device+"\",\"temperatura\":"+String(temp)+"}";
    
  //Se envia por método POST y se guarda la respuesta 200=OK -1=ERROR
  int httpCode=httpB.POST(postData);
  //Respuesta del Servidor
  String respuesta=httpB.getString();
  //Se imprimen las respuestas
  Serial.println("httpCode: "+String(httpCode)); 
  Serial.println("respuesta: "+respuesta);
  //Se termina la comunicación
  httpB.end();
  return true; 
}
