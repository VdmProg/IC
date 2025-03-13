//Bibilotecas
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>
#include "RTClib.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include <BH1750.h>

//Pino do Esp32
#define Pinglobo 4 // Porta de dados do sensor de temperatura
#define Pinruido 34 // Porta de dados do sensor de Luminocência
#define Pinco2 35  // Porta de dados do sensor de co2


//Variaveis de controle de tempo
unsigned long tempsensor = 11000;
unsigned long templcd = 6000;

//Obejtos de Classes
File dataFile;                       //Obejto para o moedulo Micro_SD
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Obejto para o sensor de LCD
RTC_DS3231 rtc;                      //Obejto para o módulo de Tempo
OneWire oneWire(Pinglobo);           //Obejto para o sensor de Temperatura(Globo)
DallasTemperature sensors(&oneWire); //Obejto para o sensor de Temperatura(Globo)
BH1750 lightMeter;                   //Obejto para o sensor de Luz


//Variaveis Globais
int co2ppm = 0;
int estadoLCD = 0;
float tempC = 0;
uint16_t lux = 0;
int adc = 0;
int dB = 0;

void setup() {
  //Inicia a comunicação Serial
  Serial.begin(9600);

  //Inicializa o Display LCD
  lcd.init();
  lcd.backlight();

  //Módulo Micro-SD
  // Tenta Inicializar o cartão SD
  
  if (!SD.begin(5)) {
    Cursor(0, 0, 1);
    lcd.print("Falha no SD!");
    while (1);
  }
  dataFile = SD.open("/PLEM.csv", FILE_WRITE);
  
  //Tenta abrir/criar o arquivo
  if (dataFile) {
    //Imprime o Cabeçhalho da planilha
    dataFile.print("Ano;Mes;Dia;Hora;Minuto;Segundo;Latitude(graus);"
                   "Longitude(graus);Velocidade(km/h);Satelites(n);Temperatura do ar(C);"
                   "Umidade do ar(%);Temperatura de globo(C);Velocidade do vento(m/s);"
                   "Concentracao de CO2(ppm);Iluminancia(lux);Ruido Ambiente(db)\n");
  } else {
    Cursor(0, 0, 1);
    lcd.print("Erro no arquivo!");
    while (1);
  }

  //Módulo RTC

  if (!rtc.begin()) {
    Cursor(0,0,1);
    lcd.print("Erro no RTC!");
    while (1); // Trava o código se o RTC não for detectado
  }
  //Caso seja preciso redefinir a data
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // Inicializa os sensores de temperatura
  sensors.begin();
  
  //Inicializa o BH1750(Iluminância)
  lightMeter.begin();
}

void loop() {
  DateTime now = rtc.now();//Variavel que recebe valor atual de data e hora fornecido pelo RTC
  
  //controla o fluxo de informação dos sensores
  if (millis() - tempsensor >= 5000) {
    Co2();
    TempGlb();
    QueHajaLuz();
    Ruido();
    Salva_sd(now);
    tempsensor = millis();
  }

  //controla a taxa de tempo do Display
  if (millis() - templcd >= 5000) {
    Display();
    templcd = millis();
  }
}

/////////////////////////////////////////////////////////////////////
//Instanciandos as funções dos Módulos/Sensores

//Micro_SD
void Salva_sd(DateTime now) {
  dataFile.print(String(now.year()) + ";"); //Ano
  dataFile.print(String(now.month()) + ";"); //Mês
  dataFile.print(String(now.day()) + ";"); //Dia
  dataFile.print(String(now.hour()) + ";"); //Hora
  dataFile.print(String(now.minute()) + ";"); //Minuto
  dataFile.print(String(now.second()) + ";"); //Segundo
  dataFile.print("-;");//Latitude
  dataFile.print("-;");//Longitudo
  dataFile.print("-;");//Velocidade(km/h)
  dataFile.print("-;");//Satélite
  dataFile.print("-;");//Temperatura do ar
  dataFile.print("-;");//Umidade do ar
  dataFile.print(String(tempC) + ";");//Temperatura de globo(C)
  dataFile.print("-;");//Velocidade do vento(m/s)
  dataFile.print(String(co2ppm) + ";"); //Concentração de CO2
  dataFile.print(String(lux) + ";");//Iluminancia
  dataFile.print(String(dB) + "\n");//Ruido Ambiente

  dataFile.flush(); // Garante que os dados sejam gravados no cartão SD
}

//controla o Display LCD
void Display() { //verificar se é o método que ocupa menos linha
  if (estadoLCD == 0) {
    Cursor(0, 0, 1);
    lcd.print("CO2 ambiente: ");
    Cursor(0, 1, 0);
    lcd.print(String(co2ppm) + " ppm");
  }
  if (estadoLCD == 1) {
    Cursor(0, 0, 1);
    lcd.print("Temperatura: ");
    Cursor(0, 1, 0);
    lcd.print(String(tempC) + " C");
  }
  if (estadoLCD == 2) {
    Cursor(0, 0, 1);
    lcd.print("Iluminancia: ");
    Cursor(0, 1, 0);
    lcd.print(String(lux) + " lux");
  }
  if (estadoLCD == 3) {
    Cursor(0, 0, 1);
    lcd.print("Ruido: ");
    Cursor(0, 1, 0);
    lcd.print(String(dB) + " dB");
  }
  estadoLCD++;
  if (estadoLCD >= 4) {
    estadoLCD = 0;
  }
}

//Posiciona o Cursor do Diplay LCD
void Cursor(int linha, int coluna, bool limpar) {
  if (limpar) {lcd.clear();} //limpa o display se limpar for 1
  lcd.backlight();
  lcd.setCursor(linha, coluna);
}

//Sensor CO2
void Co2() {
  int co2soma = 0;
  //for (int x; x < 10; x++) {
  co2soma += analogRead(Pinco2);
  //delay(100);
  //}
  co2ppm = (co2soma / 10) + 350; //Verificar se é +/-
}

// Função para ler a temperatura do globo
void TempGlb() {
  sensors.requestTemperatures();     // Solicita a temperatura do sensor
  tempC = sensors.getTempCByIndex(0); // Obtém a temperatura (índice 0 para o primeiro sensor)
}

//Função para ler Iluminância
void QueHajaLuz() {
  lux = lightMeter.readLightLevel();
}

//Função para ler Ruído
void Ruido() {
  adc = analogRead(Pinruido); //Verificar se dá pra colocar junto com dB
  dB = (adc + 83.2073) / 11.003;

}
