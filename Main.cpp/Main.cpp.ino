//***************************************
// PLEMS - Portable Low-cost Environment Monitoring System
//***************************************
//--------------------------------------------------------------------------------------------------------------------------------------
// Inclusão de bibliotecas
//--------------------------------------------------------------------------------------------------------------------------------------
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <Adafruit_AHTX0.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750FVI.h>
#include <MQ135.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "RTClib.h"
#include <string>
using namespace std;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_AHTX0 aht;
RTC_DS3231 rtc;
File myFile;
//--------------------------------------------------------------------------------------------------------------------------------------
// Versões das bibliotecas utilizadas
//--------------------------------------------------------------------------------------------------------------------------------------
// LiquidCrystal (1.0.7)
// SD (1.2.4)
// 107-Arduino-Sensor (1.0.4)
// Adafruit AHT10 (0.1.0)
// Adafruit AHTX0 (2.0.3)
// Adafruit BME280 Library (2.2.2)
// Adafruit BMP085 Library (1.2.1)
// Adafruit BMP280 Library (2.6.3)
// Adafruit BusIO (1.14.1)
// Adafruit Unified Sensor (1.1.11)
// BH1750 (1.3.0)
// BH1750FVI (1.1.1)
// Ch376msc (1.4.5)
// DallasTemperature (3.9.0)
// DHT Sensor Library (1.4.4
// DHT Stable (1.1.1)
// LiquidCrystal I2C (1.1.2)
// MQ135 (1.1.0)
// OneWire (2.3.7)
// RTClib (1.2.0)
// Time (1.6.1)
// TinyDHT sensor library (1.1.0)
// TinyGPS (13.0.0)
// TinyGPSPlus (1.0.3)
// TinyGPSPlus – ESP32 (0.0.2)
// TinyLiquidCrystal (1.2.0)
// TinyWireM (1.2.0)
// U8glib (1.19.1)
//--------------------------------------------------------------------------------------------------------------------------------------
// Definição de portas do ESP32
//--------------------------------------------------------------------------------------------------------------------------------------
#define RXD2 16
#define TXD2 17
#define MIC 34
#define potPin1 14
#define potPin2 12
#define potPin3 13
#define myPeriodic 15
#define anInput 35
#define co2Zero -350
// ajuste CO2
//--------------------------------------------------------------------------------------------------------------------------------------
// Definição de variáveis
//--------------------------------------------------------------------------------------------------------------------------------------
const int oneWireBus = 4;
const int intPin = 27;
const int bot1 = 32;
const int bot2 = 14;
const float pi = 3.14159265;
int novadata = 1;
int novomes = 1;
int novoano = 2023;
int novahrs = 0;
int novomin = 0;
int adc;
int dB, PdB;
int potValue1 = 0;
int potValue2 = 0;
int potValue3 = 0;
int potValue1ok = 0;
int potValue2ok = 0;
int potValue3ok = 0;
int luxok = 0;
int dBok = 0;
int PPMok = 0;
int period = 3000;
int radius = 147;
int time_gps = 0;
int time_aht = 4000;
int time_globo = 8000;
int time_vento = 12000;
int time_co2 = 16000;
int time_luz = 20000;
int time_db = 24000;
int extra = 28000;
//--------------------------------------------------------------------------------------------------------------------------------------
// Calibração simples
//--------------------------------------------------------------------------------------------------------------------------------------
float calibratemp = 1;
float calibraumid = 1;
float calibraglobo = 1;
float calibravento = 1;
float calibraco2 = 1;
float calibralux = 1;
float calibrasom = 1;
//--------------------------------------------------------------------------------------------------------------------------------------
// Inicialização dos sensores
//--------------------------------------------------------------------------------------------------------------------------------------
volatile uint8_t counter;
float RPM = 0;
float windspeed = 0;
float vm = 0;
float vmd = 0;
float vmax = 0;
unsigned long startTime = 0;
boolean tx_thing = false;
HardwareSerial neogps(1);
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
TinyGPSPlus gps;
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
//--------------------------------------------------------------------------------------------------------------------------------------
// Setup - configuração geral do código
//--------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(bot1, INPUT_PULLUP);
  pinMode(anInput, INPUT);
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(10000);

  Serial.println(Wire.getClock());

  sensors.begin();
  LightSensor.begin();

  //--------------------------------------------------------------------------------------------------------------------------------------
  // Exibe mensagem de erro do RTC
  //--------------------------------------------------------------------------------------------------------------------------------------
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
  }
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Ajusta data e hora do RTC caso acabe a bateria
  //--------------------------------------------------------------------------------------------------------------------------------------
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(_DATE_), F(_TIME_)));
  }

  //--------------------------------------------------------------------------------------------------------------------------------------
  // Exibe mensagem de erro do cartão microSD
  //--------------------------------------------------------------------------------------------------------------------------------------
  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Inserir micro SD");
    lcd.setCursor(2, 1);
    lcd.print("e reiniciar");
    {
    }
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Inserir micro SD");
    lcd.setCursor(2, 1);
    lcd.print("e reiniciar");
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("Adafruit AHT10/AHT20 demo!");
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Exibe mensagem de erro do sensor de temperatura e umidade do ar
  //--------------------------------------------------------------------------------------------------------------------------------------
  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(intPin, addcount, RISING);
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Inicialização do GPS
  //--------------------------------------------------------------------------------------------------------------------------------------
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Ajuste da data e hora
  //--------------------------------------------------------------------------------------------------------------------------------------
  DateTime now = rtc.now();
  String theyear = String(now.year(), DEC);  //"integer" converting integer into a string
  String mon = String(now.month(), DEC);     //"integer" converting integer into a string
  String theday = String(now.day(), DEC);    //"integer" converting integer into a string
  String thehour = String(now.hour(), DEC);  //"integer" converting integer into a string
  String themin = String(now.minute(), DEC); //"integer" converting integer into a string
  String thesec = String(now.second(), DEC); //"integer" converting integer into a string
  String data = String(theday) + "/" + String(mon) + "/" + String(theyear);
  String horario = String(thehour) + ":" + String(themin) + ":" + String(thesec);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data: " + data);
  lcd.setCursor(0, 1);
  lcd.print("Hora: " + horario);
  delay(2000);
  if (digitalRead(bot1) == LOW)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Solte o botao");
    lcd.setCursor(0, 1);
    lcd.print("para ajustar");
    delay(1000);
    while (true)
    {
      delay(100);
      if (digitalRead(bot2) == LOW)
      {
        novadata++;
        if (novadata == 32)
        {
          novadata = 1;
        }
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selecione o dia: ");
      lcd.setCursor(7, 1);
      lcd.print(novadata);
      delay(100);
      if (digitalRead(bot1) == LOW)
      {
        break;
      }
    }
    if (digitalRead(bot1) == LOW)
    {
      delay(1000);
      while (true)
      {
        delay(100);
        if (digitalRead(bot2) == LOW)
        {
          novomes++;
          if (novomes == 13)
          {
            novomes = 1;
          }
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selecione o mes: ");
        lcd.setCursor(7, 1);
        lcd.print(novomes);
        delay(100);
        if (digitalRead(bot1) == LOW)
        {
          break;
        }
      }
      if (digitalRead(bot1) == LOW)
      {
        delay(1000);
        while (true)
        {
          delay(100);
          if (digitalRead(bot2) == LOW)
          {
            novoano++;
            if (novoano == 2099)
            {
              novoano = 2000;
            }
          }
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Selecione o ano: ");
          lcd.setCursor(7, 1);
          lcd.print(novoano);
          delay(100);
          if (digitalRead(bot1) == LOW)
          {
            break;
          }
        }
        if (digitalRead(bot1) == LOW)
        {
          delay(1000);
          while (true)
          {
            delay(100);
            if (digitalRead(bot2) == LOW)
            {
              novahrs++;
              if (novahrs == 25)
              {
                novahrs = 00;
              }
            }
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Selecione as hrs:");
            lcd.setCursor(7, 1);
            lcd.print(novahrs);
            delay(100);
            if (digitalRead(bot1) == LOW)
            {
              break;
            }
          }
          if (digitalRead(bot1) == LOW)
          {
            delay(1000);
            while (true)
            {
              delay(100);
              if (digitalRead(bot2) == LOW)
              {
                novomin++;
                if (novomin == 60)
                {
                  novomin = 00;
                }
              }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Selecione os min:");
              lcd.setCursor(7, 1);
              lcd.print(novomin);
              delay(100);
              if (digitalRead(bot1) == LOW)
              {
                rtc.adjust(DateTime(novoano, novomes, novadata, novahrs, novomin, 0));

                String datajustada = String(novadata) + "/" + String(novomes) + "/" + String(novoano);
                String horajustada = String(novahrs) + ":" + String(novomin);

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Data: " + datajustada);
                // lcd.print("Data: "+ data);
                lcd.setCursor(0, 1);
                lcd.print("Hora: " + horajustada);
                // lcd.print("Hora: "+ horario);
                delay(2000);

                break;
              }
            }
          }
        }
      }
    }
  }
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Imprime mensagens iniciais
  //--------------------------------------------------------------------------------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("P.L.E.M.S");
  lcd.setCursor(3, 1);
  lcd.print("10.3.2024");
  delay(2000);
  /*
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Versao");
    lcd.setCursor(3, 1);
    lcd.print("19.01.2024");
    delay(3000);
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Aluno:");
    lcd.setCursor(0, 1);
    lcd.print("Walter Ihlenfeld");
    delay(3000);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Mestrando");
    lcd.setCursor(0, 1);
    lcd.print("PPGEC UTFPR 2023");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Orientador:");
    lcd.setCursor(0, 1);
    lcd.print("Eduardo L Kruger");
    delay(1500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Co-orientadora:");
    lcd.setCursor(0, 1);
    lcd.print("Solange M Leder");
    delay(1500);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Iniciando");
    for (int aux=0; aux<=15; aux++)
    {
    lcd.setCursor(aux, 1);
    lcd.print(".");
    delay(300);
    }
  */
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Imprime cabeçalho na planilha .CSV cada vez que inicia
  //--------------------------------------------------------------------------------------------------------------------------------------

  appendFile(SD, "/mochila.csv", "Dia;Mes;Ano;Hora;Minuto;Segundo;Latitude (graus);"
             "Longitude (graus);Velocidade (km/h);Satelites (n);"
             "Temperatura do ar (C);Umidade do ar (%);Temperatura de globo (C);"
             "Velocidade do vento (m/s);Concentracao de CO2 (ppm);"
             "Iluminancia (lux);Ruido Ambiente (db)\n");
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Loop de leituras das variáveis ambientais
//--------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  delay(300);
  printa_gps();
  delay(50);
  printa_aht();
  delay(50);
  printa_globo();
  delay(50);
  printa_vento();
  delay(50);
  printa_co2();
  delay(50);
  printa_luz();
  delay(50);
  printa_db();
  delay(50);
  tela_salva_SD();
  delay(50);
  salva_SD();
  delay(50);
}
void tela_salva_SD()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Salvando dados");
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Salva os dados no cartão microSD
//--------------------------------------------------------------------------------------------------------------------------------------

void salva_SD()
{
  DateTime now = rtc.now();

  Serial.println("/n");
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(",");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  int ano = 0;
  int mes = 0;
  int dia = 0;
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Leitura dos sensores
  //--------------------------------------------------------------------------------------------------------------------------------------
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  sensors.requestTemperatures();
  windvelocity();

  int co2now[10];
  int co2raw = 0;
  int co2ppm = 0;
  int zzz = 0;
  for (int x = 0; x < 10; x++)
  {
    co2now[x] = analogRead(35);
    delay(100);
  }
  for (int x = 0; x < 10; x++)
  {
    zzz = zzz + co2now[x];
  }
  co2raw = zzz / 10;
  co2ppm = co2raw - co2Zero;

  potValue1 = analogRead(potPin1);
  potValue1ok = ((potValue1 * 2000) / 4095) - 1000;
  uint16_t lux = LightSensor.GetLightIntensity();
  luxok = (lux);
  potValue2 = analogRead(potPin2);
  potValue2ok = ((potValue2 * 100) / 4095) - 50;
  PdB = dB;
  adc = analogRead(MIC);
  dB = (adc + 83.2073) / 11.003;
  dBok = (dB);
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Salva data e hora
  //--------------------------------------------------------------------------------------------------------------------------------------
  String theyear = String(now.year(), DEC); //"integer" converting integer into a string
  String mon = String(now.month(), DEC);    //"integer" converting integer into a string
  String theday = String(now.day(), DEC);   //"integer" converting integer into a string
  String thehour = String(now.hour(), DEC); //"integer" converting integer into a string
  // String themin = String(now.minute(), DEC); //"integer" converting integer into a string
  String themin = "0";
  int ominuto = 0;
  if (now.minute() == 0)
  {
    themin = "59";
  }
  else
  {
    ominuto = (now.minute() - 1);
    themin = String(ominuto, DEC);
  }
  String thesec = String(now.second(), DEC); //"integer" converting integer into a string
  dia = (now.day(), DEC);
  mes = (now.month(), DEC);
  ano = (now.year(), DEC);

  String data = String(dia) + "/" + String(mes) + "/" + String(ano) + ";";
  Serial.print("testeaqui");
  Serial.print(data);
  float hora = 0;
  float minuto = 0;
  hora = (now.hour(), DEC);
  minuto = (now.minute(), DEC);
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Transforma a leitura dos sensores em numeral
  //--------------------------------------------------------------------------------------------------------------------------------------
  String horario = String(hora) + ":" + String(minuto) + ";";
  String latitude = String(gps.location.lat(), 6) + ";";
  String longitude = String(gps.location.lng(), 6) + ";";
  String Altitude = String(gps.speed.kmph(), 6) + ";";
  String Nsat = String(gps.satellites.value()) + ";";

  String temperatura = String(temp.temperature * calibratemp) + ";";
  String umidade = String(humidity.relative_humidity * calibraumid) + ";";
  String temperatura_globo = String(sensors.getTempCByIndex(0) * calibraglobo) + ";";
  String anemometro = String(windspeed * calibravento) + ";";
  String mq135 = String(co2ppm * calibraco2) + ";";
  String intensidadeluminosa = String(luxok * calibralux) + ";";
  String intensidadesonora = String(dBok * calibrasom) + "\n";
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Adiciona as leituras na planilha
  //--------------------------------------------------------------------------------------------------------------------------------------
  appendFile(SD, "/mochila.csv", theday.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", mon.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", theyear.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", thehour.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", themin.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", thesec.c_str());
  appendFile(SD, "/mochila.csv", ";");
  appendFile(SD, "/mochila.csv", latitude.c_str());
  appendFile(SD, "/mochila.csv", longitude.c_str());
  appendFile(SD, "/mochila.csv", Altitude.c_str());
  appendFile(SD, "/mochila.csv", Nsat.c_str());
  appendFile(SD, "/mochila.csv", temperatura.c_str());
  appendFile(SD, "/mochila.csv", umidade.c_str());
  appendFile(SD, "/mochila.csv", temperatura_globo.c_str());
  appendFile(SD, "/mochila.csv", anemometro.c_str());
  appendFile(SD, "/mochila.csv", mq135.c_str());
  appendFile(SD, "/mochila.csv", intensidadeluminosa.c_str());
  appendFile(SD, "/mochila.csv", intensidadesonora.c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do GPS (GPS GY-NEO6MV2 com antena / u-blox®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_gps()
{
  boolean newData = false; // Declaração de uma variável booleana para verificar se há novos dados
  for (unsigned long start = millis(); millis() - start < 1000;)
  { // Loop que executa por 1 segundo
    while (neogps.available())
    { // Enquanto houver dados disponíveis no GPS
      if (gps.encode(neogps.read()))
      { // Se os dados puderem ser codificados pelo objeto 'gps'
        newData = true; // Define newData como true, indicando que novos dados foram recebidos
      }
    }
  }
  // Se newData for true
  if (newData == true)
  {
    newData = false;                        // Redefine newData para false
    Serial.println(gps.satellites.value()); // Imprime o número de satélites conectados no serial monitor
    print_speed();                          // Chama a função print_speed() (presumivelmente para imprimir a velocidade)
  }

  // Se newData for false
  if (newData == false)
  {
    lcd.clear();                  // Limpa a tela LCD
    lcd.setCursor(0, 0);          // Define o cursor para a posição inicial (linha 0, coluna 0)
    lcd.print("Atualizando GPS"); // Imprime a mensagem "Atualizando GPS" na tela LCD
    delay(250);                   // Pausa por 250 milissegundos
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de temperatura e umidade do ar (AHT10 / ASAIR®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_aht()
{
  sensors_event_t humidity, temp; // Declaração de variáveis para armazenar os eventos de temperatura e umidade
  aht.getEvent(&humidity, &temp); // Obtém os dados do sensor e os armazena nas variáveis 'humidity' e 'temp'
  if (millis() > time_aht)
  {
    lcd.clear();                               // Limpa a tela do LCD
    lcd.setCursor(0, 0);                       // Define o cursor para a posição inicial (linha 0, coluna 0)
    lcd.print("Medindo");                      // Exibe a mensagem "Medindo" na primeira linha do LCD
    lcd.setCursor(0, 1);                       // Define o cursor para a segunda linha do LCD
    lcd.print("Temp. e Umid.");                // Exibe a mensagem "Temp. e Umid." na segunda linha do LCD
    delay(2000);                              // Aguarda 30.75 segundos
    lcd.clear();                               // Limpa a tela do LCD
    lcd.setCursor(0, 0);                       // Define o cursor para a posição inicial (linha 0, coluna 0)
    lcd.print("Temp: ");                       // Exibe "Temp: " na primeira linha do LCD
    lcd.setCursor(6, 0);                       // Move o cursor para a posição onde a temperatura será exibida
    lcd.print(temp.temperature * calibratemp); // Exibe a temperatura ajustada pelo fator de calibração 'calibratemp'

    lcd.setCursor(12, 0);                                // Move o cursor para a posição onde será exibida a unidade de temperatura
    lcd.print("C");                                      // Exibe a unidade "C" (Celsius)
    lcd.setCursor(0, 1);                                 // Define o cursor para a posição inicial da segunda linha
    lcd.print("Umid: ");                                 // Exibe "Umid: " na segunda linha do LCD
    lcd.setCursor(6, 1);                                 // Move o cursor para a posição onde a umidade será exibida
    lcd.print(humidity.relative_humidity * calibraumid); // Exibe a umidade ajustada pelo fator de calibração 'calibraumid'

    lcd.setCursor(12, 1); // Move o cursor para a posição onde será exibida a unidade de umidade
    lcd.print("%");       // Exibe a unidade "%" (percentual)
    time_aht = millis() + extra;
  }
}
//-------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de temperatura de globo (DS18B20 / Maxim Integrated Products®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_globo() {
  if (millis() > time_globo) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Medindo");
    lcd.setCursor(0, 1);
    lcd.print("Temp. de globo");
    delay(2000);

    sensors.requestTemperatures();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp. de globo:");
    lcd.setCursor(0, 1);
    lcd.print(sensors.getTempCByIndex(0) * calibraglobo);
    lcd.setCursor(6, 1);
    lcd.print("C");
    time_globo = millis() + extra;
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de lux (BH1750-FVI GY-30 / ROHM Semicondutor®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_luz() {
  if (millis() > time_luz) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Medindo");
    lcd.setCursor(0, 1);
    lcd.print("Iluminancia");
    delay(1000);

    potValue1 = analogRead(potPin1);
    potValue1ok = ((potValue1 * 2000) / 4095) - 1000;
    uint16_t lux = LightSensor.GetLightIntensity();
    luxok = (lux);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Iluminancia: ");
    lcd.setCursor(0, 1);
    lcd.print(luxok);
    lcd.setCursor(6, 1);
    lcd.print("lux");
    time_luz = millis() + extra;
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de ruído (KY-038 / JOY-IT®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_db() {
  if (millis() > time_db) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Medindo");
    lcd.setCursor(0, 1);
    lcd.print("Ruido Ambiente");
    delay(1000);

    potValue2 = analogRead(potPin2);
    potValue2ok = ((potValue2 * 100) / 4095) - 50;
    PdB = dB;
    adc = analogRead(MIC);
    dB = (adc + 83.2073) / 11.003;

    dBok = (dB);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ruido Ambiente: ");
    lcd.setCursor(0, 1);
    lcd.print(dBok);
    lcd.setCursor(4, 1);
    lcd.print("dB");
    time_db = millis() + extra;
  }

}
//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de concentração de CO2 (MQ-135 / Winsen®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_co2() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Medindo");
  lcd.setCursor(0, 1);
  lcd.print("CO2 ambiente");
  delay(1000);

  int co2now[10];
  int co2raw = 0;
  int co2ppm = 0;
  int zzz = 0;
  for (int x = 0; x < 10; x++)
  {
    co2now[x] = analogRead(35);
    delay(100);
  }
  for (int x = 0; x < 10; x++)
  {
    zzz = zzz + co2now[x];
  }
  co2raw = zzz / 10;
  co2ppm = co2raw - co2Zero;

  if (millis() > time_co2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CO2 ambiente: ");
    lcd.setCursor(0, 1);
    lcd.print(co2ppm);
    lcd.setCursor(6, 1);
    lcd.print("ppm");
    time_co2 = millis() + extra;
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Programação do sensor de ventilação (Anemômetro AN-1 / WRF COMERCIAL®)
//--------------------------------------------------------------------------------------------------------------------------------------
void printa_vento() {
  if (millis() > time_vento) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Medindo");
    lcd.setCursor(0, 1);
    lcd.print("Vel. do vento");
    delay(500); // 500
    windvelocity();
    vmd = vm / 20;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vel. do vento: ");
    lcd.setCursor(0, 1);
    lcd.print(windspeed * calibravento);
    lcd.setCursor(6, 1);
    lcd.print("m/s");
    time_vento = millis() + extra;
  }
}
// 4000-3100
// Delay mínimo de 3,1 segundos, adotado 4s para cada leitura para fim de arredondamento
//--------------------------------------------------------------------------------------------------------------------------------------
// Função adicional do sensor de GPS
//--------------------------------------------------------------------------------------------------------------------------------------
void print_speed()
{
  lcd.clear();

  if (gps.location.isValid() == 1)
  {
    // String gps_speed = String(gps.speed.kmph());

    lcd.setCursor(0, 0);
    lcd.print("Lat: ");
    lcd.setCursor(5, 0);
    lcd.print(gps.location.lat(), 6);
    lcd.setCursor(0, 1);
    lcd.print("Lng: ");
    lcd.setCursor(5, 1);
    lcd.print(gps.location.lng(), 6);
    delay(1000); // Teste
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Vel: ");
    lcd.setCursor(5, 0);
    lcd.print(gps.speed.kmph(), 2);
    lcd.setCursor(10, 0);
    lcd.print("km/h");

    lcd.setCursor(0, 1);
    lcd.print("Sat Conect: ");
    lcd.setCursor(12, 1);
    lcd.print(gps.satellites.value(), 1);
    delay(1000); // Teste
    lcd.clear();
  }
  if (gps.location.isValid() == 0)
  {
    lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("Atualizando GPS");
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Função adicional do sensor de ventilação
//--------------------------------------------------------------------------------------------------------------------------------------
void windvelocity()
{
  for (int i = 0; i < 2; i++)
  {
    Serial.print(" Leitura: ");
    Serial.println(i);

    if (millis() - startTime >= period)
    {

      detachInterrupt(intPin); // Desabilita interrupcao
      Serial.print("Pulsos :");
      Serial.print(counter);
      RPM = ((counter) * 60) / (period / 1000); // Calculate revolutions per minute (RPM) 60
      Serial.print(" - RPM :");
      Serial.print(RPM);
      windspeed = 0;
      counter = 0; // Zera cont pulsos
      unsigned long millis();
      startTime = millis();

      attachInterrupt(intPin, addcount, RISING); // Habilita interrupcao
    }
    windspeed = (((4 * pi * radius * RPM) / 60) / 3600); // Calculate wind speed on m/s

    // windspeed = (((4 * pi * radius * RPM)/60) / 1000)/3.60 ; // Calculate wind speed on m/s
    Serial.print(" - Veloc : ");
    Serial.println(windspeed);
    vm = vm + windspeed;
    if (windspeed > vmax)
    {
      vmax = windspeed;
    }

    delay(3100); // Tem que ser maior que o periodo
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Gravação do arquivo do cartão microSD
//--------------------------------------------------------------------------------------------------------------------------------------
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Função para adicionar leituras no cartão microSD
//--------------------------------------------------------------------------------------------------------------------------------------
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Função do contador do sensor de ventilação
//--------------------------------------------------------------------------------------------------------------------------------------
void addcount()
{
  counter++;
}
//***************************************
// PLEMS - Portable Low-cost Environment Monitoring System
