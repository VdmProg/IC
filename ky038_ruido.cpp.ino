#define potPin2 12
#define MIC 34

int dB;
int adc;

void ruido()
{
  adc = analogRead(MIC);
  dB = (adc + 83.2073) / 11.003;
}
