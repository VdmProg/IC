#include <BH1750.h>

BH1750 lightMeter;                   //Obejto para o sensor de Luz

uint16_t lux = 0;

void Setup()
{
   //Inicializa o BH1750(Iluminância)
  lightMeter.begin();
}

void QueHajaLuz() {
    lux = lightMeter.readLightLevel();
  }