// Arduino and KY-038 module
void setup ()
{
  Serial.begin(9600); // initialize serial
}
void loop ()
{
  // display analog and digital values to serial

  int saida = analogRead(A0);
  Serial.print("\nAnalog pin: ");
  if(saida > 100){
    Serial.print(saida);
    Serial.println("SOM MUITO ALTO");
  }
  Serial.print(saida);
}
