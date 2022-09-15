/*************************************************************
  Title         :   Home automation using blynk
  Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
  Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Comment this out to disable prints
#define BLYNK_PRINT Serial

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPL6Ih8_OiP"
//#define BLYNK_DEVICE_NAME "Home Automation Project"
#define BLYNK_AUTH_TOKEN "AJkRtuovtmFc83NmZ3NSO2ag2zDjgJzC"

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char auth[] = "AJkRtuovtmFc83NmZ3NSO2ag2zDjgJzC";

#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

bool heater_sw, inlet_sw, outlet_sw, cooler_sw;
unsigned int tank_volume;
String temp;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
/*To turn ON and OFF cooler based virtual PIN value*/
BLYNK_WRITE(COOLER_V_PIN)
{
  bool cooler_sw = param.asInt();
  if (cooler_sw) {
    cooler_control(ON);
    lcd.setCursor(7, 0);
    lcd.print("CO_LR ON");
  }
  else {
    cooler_control(OFF);
    lcd.setCursor(7, 0);
    lcd.print("CO_LR OFF");
  }
}

/*To turn ON and OFF heater based virtual PIN value*/
BLYNK_WRITE(HEATER_V_PIN )
{
  heater_sw = param.asInt();
  if (heater_sw) {
    heater_control(ON);
    lcd.setCursor(7, 0);
    lcd.print("HE_TR ON");
  }
  else {
    heater_control(OFF);
    lcd.setCursor(7, 0);
    lcd.print("HE_TR OFF");
  }
}

/*To turn ON and OFF inlet vale based virtual PIN value*/
BLYNK_WRITE(INLET_V_PIN)
{
  inlet_sw = param.asInt();
  if (inlet_sw) {
    enable_inlet();
    lcd.setCursor(7, 1);
    lcd.print("IN_LT ON");
  }
  else {
    disable_inlet();
    lcd.setCursor(7, 1);
    lcd.print("IN_LT OFF");
  }
}

/*To turn ON and OFF outlet value based virtual switch value*/
BLYNK_WRITE(OUTLET_V_PIN)
{
  outlet_sw = param.asInt();
  if (outlet_sw) {
    enable_outlet();
    lcd.setCursor(7, 1);
    lcd.print("OU_LT ON");
  }
  else {
    disable_outlet();
    lcd.setCursor(7, 1);
    lcd.print("OU_LT OFF");
  }
}

/* To display temperature and water volume as gauge on the Blynk App*/
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  
  // displaying temperature on gauge
  Blynk.virtualWrite(TEMPERATURE_GAUGE, read_temperature());

  // displaying water volume on gauge
  Blynk.virtualWrite(WATER_VOL_GAUGE, volume());
}

/*To turn off the heater if the temperature raises above 35 deg C*/
void handle_temp(void)
{
  if ((read_temperature() > float(35)) && heater_sw)
  {
    heater_sw = OFF;
    heater_control(OFF);
    
    lcd.setCursor(7, 0);
    lcd.print("HE_TR OFF");
    
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning OFF the Heater, as Temp is above 35 degrees \n\n");
    Blynk.virtualWrite(HEATER_V_PIN, OFF);
  }
}

/*To control water volume above 2000ltrs*/
void handle_tank(void)
{
  if((tank_volume < 2000) && (inlet_sw == OFF))
  {
    enable_inlet();
    inlet_sw = ON;

    lcd.setCursor(7, 1);
    lcd.print("IN_LT ON");

    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Water level is less than 2000, water in-flow enabled \n\n");
    Blynk.virtualWrite(INLET_V_PIN, ON);
  }

  if((tank_volume == 3000) && (inlet_sw == ON))
  {
    disable_inlet();
    inlet_sw = OFF;

    lcd.setCursor(7, 1);
    lcd.print("IN_LT OFF");

    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Water level is equal to 3000, water in-flow disabled \n\n");
    Blynk.virtualWrite(INLET_V_PIN, OFF);
  }
}


void setup(void)
{
  Serial.begin(19200);
  /*Serial.write(0xFF); //sincroniza comunicação
  Serial.write(0xFF);
  Serial.write(0xFF);*/
  
  Blynk.begin(auth);

  lcd.init();                     
  lcd.backlight();
  lcd.clear();
  lcd.home();
  
  // To print temperature in first line
  lcd.setCursor(0, 0);
  lcd.print("T=");

  // To print volum of water in second line
  lcd.setCursor(0, 1);
  lcd.print("V=");

  // Initializing Temperature System
  init_temperature_system();

  // Initializing Garden light System
  init_ldr();

  init_serial_tank();

  timer.setInterval(1000L, update_temperature_reading);
}

void loop(void)
{
  Blynk.run();
  timer.run();

  temp = String(read_temperature(), 2);
  lcd.setCursor(2, 0);
  lcd.print(temp);

  tank_volume = volume();
  lcd.setCursor(2, 1);
  lcd.print(tank_volume);

  brightness_control();
  handle_temp();
  handle_tank();
}
