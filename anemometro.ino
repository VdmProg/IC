const int intPin = 27; // Pino digital conectado ao sensor de vento para interrupção.

const float pi = 3.14159265; // Valor de Pi usado nos cálculos.
int radius = 147;            // Raio em milímetros do sensor de vento (anemômetro).
int period = 3000;           // Período em milissegundos para calcular a velocidade do vento.

volatile uint8_t counter;    // Variável contadora usada em interrupções.
float RPM = 0;               // Armazena rotações por minuto do anemômetro.
float windspeed = 0;         // Velocidade atual do vento (m/s).
float vm = 0;                // Velocidade média do vento.
float vmd = 0;               // Variável não utilizada no código atual.
float vmax = 0;              // Velocidade máxima do vento registrada.
unsigned long startTime = 0; // Tempo inicial para medir o período.

void Setup(){
    Serial.begin(9600);
}

void loop(){
    windspeed();
    Serial.print(windspeed);
}

void windvelocity()
{
    if (millis() - startTime >= period)
    {
        detachInterrupt(intPin);                  // Desabilita interrupcao
        RPM = ((counter) * 60) / (period / 1000); // Calculate revolutions per minute (RPM) 60
        windspeed = 0;
        counter = 0; // Zera cont pulsos
        unsigned long millis();
        startTime = millis();

        attachInterrupt(intPin, addcount, RISING); // Habilita interrupcao
    }
    // Calcula a velocidade do vento (m/s) com base no RPM, raio e tempo.
    windspeed = (((4 * pi * radius * RPM) / 60) / 3600);
    vm = vm + windspeed;  // Soma a velocidade atual à velocidade média acumulada.
    if (windspeed > vmax) // Verifica se a velocidade atual é a maior já registrada.
    {
        vmax = windspeed; // Atualiza a velocidade máxima.
    }
}

// Função chamada pela interrupção para incrementar o contador.
void addcount()
{
    counter++; // Incrementa o contador a cada pulso detectado.
}
