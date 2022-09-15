#include "ldr.h"
#include "Arduino.h"
#include "main.h"

void init_ldr(void)
{
   pinMode(GARDEN_LIGHT, OUTPUT);
}

void brightness_control(void)
{
  unsigned short input;
  
  input = analogRead(LDR_SENSOR);
  input = input/4;
  input = 255 - input;
  analogWrite(GARDEN_LIGHT,input);
}
