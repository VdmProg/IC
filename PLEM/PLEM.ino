//Bibilotecas
#include <Wire.h>                 //Biblioteca para I2C
#include <LiquidCrystal_I2C.h>    //Biblioteca da LCD
#include <SD.h>                   //Biblioteca do SD
#include <SPI.h>                  //


//Variaveis de controle de tempo
unsigned long tempsensor = 11000;
unsigned long templcd = 6000; 

//Variaveis Globais
File dataFile;
LiquidCrystal_I2C lcd(0x27,16,2);
int co2ppm = 0; //verificar se coloca no tipo float
int estadoLCD = 0;

void setup() {
  
//Inicializa o Display LCD
lcd.init();
 
//Módulo Micro-SD

// Tenta Inicializar o cartão SD
if(!SD.begin(5)) {
    Cursor(0,0,1);
    lcd.print("Falha ao inicializar o cartão SD!");
    while(1);
  } 

dataFile = SD.open("/PLEM.csv", FILE_WRITE);

if (dataFile) {
   //Imprime o Cabeçhalho da planilha
    dataFile.print("Dia;Mes;Ano;Hora;Minuto;Segundo;Latitude(graus);"
    "Longitude(graus);Velocidade(km/h);Satelites(n);Temperatura do ar(C);"
    "Umidade do ar(%);Temperatura de globo(C);Velocidade do vento(m/s);"
    "Concentracao de CO2(ppm);Iluminancia(lux);Ruido Ambiente(db)\n");
} else {
    Cursor(0,0,1);
    lcd.print("Erro no arquivo!");
}
 /////////////////////////////////////////////////////////////////////

}

void loop() {
 
//controla o fluxo de informação dos sensores
 if (millis() - tempsensor >= 10000) {
    Co2(); 
    Salva_sd();
    tempsensor = millis();
  }
  
//controla a taxa de tempo do Display
 if (millis() - templcd >= 5000) { 
    Display();
    templcd = millis();     
  }
 
}

//Micro_SD
void Salva_sd(){
dataFile.println(String(co2ppm)+";");
dataFile.flush(); // Garante que os dados sejam gravados no cartão SD
}

//controla o Display LCD
void Display(){//verificar se é o método que ocupa menos linha
if(estadoLCD == 0){
    Cursor(0,0,1);
    lcd.print("CO2 ambiente: "); 
    Cursor(0,1,0); 
    lcd.print(String(co2ppm)+"ppm");  
}
estadoLCD++;
if(estadoLCD>=0){
    estadoLCD = 0;
}

}

//Posiciona o Cursor do Diplay LCD
void Cursor(int linha,int coluna,bool limpar){
if (limpar){ 
    lcd.clear(); //limpa o display se limpar for 1
}
lcd.setCursor(linha,coluna);

}

//Sensor CO2
void Co2(){
int co2soma = 0;
for(int x; x<10;x++){
    co2soma += analogRead(35);
    delay(100); //Verificar como que vai afetar o código
  }
co2ppm = (co2soma/10) - 350;

}
