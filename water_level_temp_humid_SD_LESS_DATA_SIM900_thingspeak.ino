//Authors:Nayana Machado and Marina Ferrarez
//Email: {nayanamachado.02@gmail.com;maferrarezr@gmail.com}
//Date: 11/06/2020
//Revision: LESS DATA + CALIBRAÇÃO ULTRASONICO _ delay 60000 1 min#
//Project: {Data logger com sensor de temperatura (DHT22) e nível de água (HC-SR04)}

#include <NewPing.h>  // NewPing - Version: Latest 
#include <RTClib.h> // RTClib - Version: Latest 
#include <DHT.h> // DHT sensor library - Version: Latest 
#include <DHT_U.h> // DHT sensor library - Version: Latest 
//#include <SDConfig.h> // SDConfig - Version: Latest 
#include <SD.h>
#include <Adafruit_Sensor.h>

#include <gps.h>
#include <GSM.h>
#include <HWSerial.h>
#include <inetGSM.h>
#include <SIM900.h>
#include <SoftwareSerial.h>
#include <String.h>


//Pino CS do cartao SD
int Pino_CS = 10;

//Definicoes do sensor de temperatura DHT11
//#define DHTPIN A0 //Pino de conexao
//#define DHTTYPE DHT11 //Tipo de sensor

//Definicoes do sensor de temperatura DHT22 + HC-SR04
#define DHTPIN 6          // DHT-22 Output Pin connection
#define DHTTYPE DHT22     // DHT Type is DHT 22 (AM2302)
#define TRIGGER_PIN 4     //HC-SR04
#define ECHO_PIN  5       //HC-SR04
#define MAX_DISTANCE 400  //HC-SR04

//DHT dht(DHTPIN, DHT22);
SoftwareSerial SIM900(3, 2);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //inicialize the sensor - HC-SR04


//Le as informacoes da temperatura
// float t = dht.readTemperature();

float hum;    // Stores humidity value in percent
float temp;   // Stores temperature value in Celcius
float duration; // Stores HC-SR04 pulse duration value
float dist; // Stores calculated distance in cm
float soundsp;  // Stores calculated speed of sound in M/S
float soundcm;  // Stores calculated speed of sound in cm/ms
//float nivel_agua;  // stores calculated water level in cm
//float nivel_agua_m;  // stores calculated water level in cm
int iterations = 5;


DHT dht(DHTPIN, DHTTYPE); //Cria objeto DHT
RTC_DS1307 rtc;
File file;

void setup()
{

  Serial.println("Testando GSM shield...");
  //Inicia a configuracao do Shield
  SIM900.begin(9600);     // the GPRS baud rate
 // Serial.begin(9600);    // the GPRS baud rate

 //Inicializa o DHT22
  dht.begin();

  delay(1000);
  
  Serial.begin(9600); //speed parameter
  Serial.println("Data logger com sensor de temperatura DHT22 e sensor Ultrassônico");
  Serial.println();

  //Inicia o cartao SD
  Serial.println("Iniciando cartao SD...");
  if (!SD.begin(Pino_CS))
  {
    Serial.println("Falha na inicializacao do SD!");
    return;
  }
  Serial.println("Cartao SD iniciado. OK");
  Serial.println();

  //Verifica as condicoes do RTC
  if (! rtc.begin())
  {
    Serial.println("RTC nao encontrado!");
    while (1);
  }
  if (! rtc.isrunning())
  {
    Serial.println("RTC nao operante!");
    //A linha abaixo ajusta o RTC com a data e hora do momento da compilacao
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //A linha abaixo permite ajustar manualmente a data e hora do RTC
    //Formato: DateTime(ano, mes, dia, hora, minuto, segundo)
    //rtc.adjust(DateTime(2020, 6, 19, 14, 32, 0));
  }
  //Inicializa o DHT22
  //dht.begin();
}

void loop(void)
{

 delay(2000);  // Delay so DHT-22 sensor can stabalize - 2 segundos é o mín

  hum = dht.readHumidity();  // Get Humidity value
  temp = dht.readTemperature(); // Get Temperature value
  
    // Calculate the Speed of Sound in M/S
  soundsp = 331.4 + (0.606 * temp) + (0.0124 * hum);

  // Convert to cm/ms
  soundcm = soundsp / 10000;

  duration = sonar.ping_median(iterations);

  // Calculate the distance
  //Profundidade = 150 - ((duration/2)*soundcm); // 150 é a distância (cm) entre o sensor e o fundo do corrego (PEDREIRA)
  dist = (duration/2)*soundcm;

  if (SIM900.available())
    Serial.write(SIM900.read());
    SIM900.println("AT"); // check the starters
    delay(1000);
    SIM900.println("AT+CPIN?"); //
    delay(1000);
    SIM900.println("AT+CREG?");
    delay(1000);
    SIM900.println("AT+CGATT?");
    delay(1000);
    SIM900.println("AT+CIPSHUT");
    delay(1000);
    SIM900.println("AT+CIPSTATUS");
    delay(2000);
    SIM900.println("AT+CIPMUX=0");
    delay(2000);
    ShowSerialData();
    SIM900.println("AT+CSTT=\"internet.br\"");//start task and setting the APN,
    delay(1000);
    ShowSerialData();
    SIM900.println("AT+CIICR");//bring up wireless connection
    delay(3000);
    ShowSerialData();
    SIM900.println("AT+CIFSR");//get local IP adress
    delay(2000);
    ShowSerialData();
    SIM900.println("AT+CIPSPRT=0");
    delay(3000);
    ShowSerialData();

    SIM900.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
     delay(6000);
    ShowSerialData();
  
    SIM900.println("AT+CIPSEND");//begin send data to remote server
    delay(4000);
    ShowSerialData();

  String str = "GET https://api.thingspeak.com/update?api_key=KM8E2DGL38ZL5HUW&field1=0" + String(temp) + "&field2=" + String(hum);
  Serial.println(str);
  SIM900.println(str);//begin send data to remote server
  delay(4000);
  ShowSerialData();

  SIM900.println((char)26);//sending
  delay(7000);//waitting for reply, important! the time is base on the condition of internet
  SIM900.println();
  ShowSerialData();

  //SIM900.println("AT+CIPSHUT");//close the connection
  //delay(100);
  //ShowSerialData();
//}

  //Le as informacoes do RTC
  DateTime now = rtc.now();
  //Serial monitor informacoes de hora
  //Serial.print(now.day() < 10 ? "0" : "");
  //Serial.print(now.day(), DEC);
  //Serial.print('-');
  //Serial.print(now.month() < 10 ? "0" : "");
  //Serial.print(now.month(), DEC);
  //Serial.print('-');
  //Serial.print(now.year() < 10 ? "0" : "");
  //Serial.print(now.year(), DEC);
  //Serial.print(";");
  Serial.print(now.hour() < 10 ? "0" : "");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute() < 10 ? "0" : "");
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second() < 10 ? "0" : "");
  Serial.print(now.second(), DEC);
  Serial.print(';');
  Serial.print("Sound: ");
  Serial.print(soundsp);
  Serial.print(" m/s, ");
  Serial.print("Hum:");
  Serial.print(hum);
  Serial.print("%,Temp:");
  Serial.print(temp);
  Serial.print("C,Dist:");
  Serial.print(dist);
  Serial.println("cm");

  //Gravacao do cartao
  //Abre arquivo no SD para gravacao
  file = SD.open("mendler.csv", FILE_WRITE);
  //Grava os dados no cartao SD
  file.print(now.day() < 10 ? "0" : "");
  file.print(now.day(), DEC);
  file.print('-');
  file.print(now.month() < 10 ? "0" : "");
  file.print(now.month(), DEC);
  file.print('-');
  file.print(now.year() < 10 ? "0" : "");
  file.print(now.year(), DEC);
  file.print(";");
  file.print(now.hour() < 10 ? "0" : "");
  file.print(now.hour(), DEC);
  file.print(':');
  file.print(now.minute() < 10 ? "0" : "");
  file.print(now.minute(), DEC);
  file.print(':');
  file.print(now.second() < 10 ? "0" : "");
  file.print(now.second(), DEC);
  file.print(";");
  //file.print("Sound: ");
  //file.print(soundsp);
  //file.print(" m/s, ");
  file.print("Temp:");
  file.print(temp);
  file.print("C,Umid:");
  file.print(hum);
  file.print("%,Dist:");
  file.print(dist);
  file.println("cm");

  //Fecha arquivo
  file.close();
  //Delay ate a proxima leitura
  //delay(58000);
  delay(1000); //tempo de delay para ver o programa rodas
  
}
void ShowSerialData()
{
  while (SIM900.available() != 0)
    Serial.write(SIM900.read());
  delay(5000);
}
