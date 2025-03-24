#define RAIO_METROS 0.105f
#define CIRCUNFERENCIA(raio) 2.0*raio*PI
#define PIN 27

//a região que o sensor funciona é de 72° 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN, INPUT);
}

void loop() {
  velocidade();
}

void velocidade(){
  int sinal = digitalRead(PIN);
  int sinalAnterior = sinal;
  float tempoInicial = 0;
  float tempoFinal = 0;

  /*
  Primeira calibração para saber onde começa a circunferência
  */
  while (sinal == sinalAnterior){
    sinal = digitalRead(PIN);
  }
  tempoInicial = millis();
  /*
  O anemometro possui duas áreas em sua cirncunferência, uma de output 0
  e outra de output 1, para contar quanto tempo demora para fazer toda a circunferência
  é preciso antes 
  */
  for (short nMudancas = 0, sinalAnterior = sinal; nMudancas < 2;){
    sinal = digitalRead(PIN);
    sinal != sinalAnterior ? ++nMudancas : false;
  }
  tempoFinal = millis();

  float tempo = (tempoFinal - tempoInicial)/1000;
  float velocidade = CIRCUNFERENCIA(RAIO_METROS) / tempo;

  Serial.print("Velocidade:");
  Serial.println(velocidade);
}
