#include <Adafruit_AHTX0.h>
#include <OneWire.h>
#include <DallasTemperature.h>

Adafruit_AHTX0 aht;

const int oneWireBus = 4;

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void tempUmid()
{
    sensors_event_t humidity, temp;                     // Declaração de variáveis para armazenar os eventos de temperatura e umidade
    aht.getEvent(&humidity, &temp);                     // Obtém os dados do sensor e os armazena nas variáveis 'humidity' e 'temp'
}