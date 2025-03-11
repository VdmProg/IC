#define potPin2 12
#define MIC 34

int saidaRuido = 0;
int saidaRuido_ajust;
int PdB, dB;
int adc;
int dBok = 0;

void ruido()
{
  // saidaRuido = analogRead(potPin2);
  // saidaRuido_ajust = ((saidaRuido * 100) / 4095) - 50;

  PdB = dB;
  adc = analogRead(MIC);
  dB = (adc + 83.2073) / 11.003;

  dBok = (dB);
}
