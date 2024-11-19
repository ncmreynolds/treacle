#if defined LED_BUILTIN

#if defined ARDUINO_ESP8266_WEMOS_D1MINI || defined ARDUINO_ESP8266_WEMOS_D1MINIPRO
  ezLED led(LED_BUILTIN, CTRL_CATHODE);  //Create an 'inverted' LED object
#else
  ezLED led(LED_BUILTIN);  //Create an LED object
#endif

void setupLed()
{
  led.turnOFF(); //Turn the LED off at boot
  led.loop(); //Manage the LED to set initial state
}

void manageLed()
{
  if(treacle.online())
  {
    if(led.getState() == LED_BLINKING)
    {
      led.turnON(); //Turn the LED on once online
    }
  }
  else
  {
    if(led.getState() != LED_BLINKING)
    {
      led.blink(50, 950); //Blink while offline
    }
  }
  led.loop(); //Manage the LED
}
#endif
