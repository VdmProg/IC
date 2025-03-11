#define potPin1 14

int saidaLuz = 0;
int saidaLuz_ajust = 0;
int luxok = 0;

volatile uint8_t counter;
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

void Setup()
{
    LightSensor.begin();
}

void luz()
{
    saidaLuz = analogRead(potPin1);
    saidaLuz_ajust = ((saidaLuz * 2000) / 4095) - 1000;
    uint16_t lux = LightSensor.GetLightIntensity();
    luxok = (lux);
}