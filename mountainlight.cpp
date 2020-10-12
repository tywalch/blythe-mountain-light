#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

volatile int f_wdt = 1;
int relayPowerPin = 3;                // I supply the relay with 5v power using this pin
int lightSensorPowerPin = 4;          // I supply the photoresistor with 5v power using this pin
int relayDataPin = 7;                 // Output pin for switching relay
int lightDataPin = A2;                // Input pin for reading photoresistance
int clockAdjustment;                  // I use this to manage my delays so things work smoother
bool lastReading;                     // Used to detect change from day-time to night-time

/* ==== ISR is run when after a the WDT interrupt ==== */
// Good info found here: http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
ISR(WDT_vect) {
  if(f_wdt == 0) 
  {
    f_wdt = 1;
  } 
}

/* ==== Configures my Watchdog timer to be a nonresetting interrupt every 8 seconds ==== */
void configureWatchdog() {
  cli();                              // disable system interrupts during watchdog configuration
  wdt_reset();                        // reset the watchdog timer
  WDTCSR |= (1<<WDCE) | (1<<WDE);     // follow unlocking procedure at the bottom of page 51 on the datasheet
  WDTCSR = 1<<WDP0 | 1<<WDP3;         // 8 seconds - Page 55 of the datasheet
  WDTCSR |= _BV(WDIE);                // Enable the WD interrupt (note no reset)
  sei();                              // enable interrupts again, it's cool now
}

void configureRelay(int relayPowerPin, int relayDataPin, int lightDataPin) {
  // creates starting values for the relay, makes the relay closed
  digitalWrite(relayPowerPin, HIGH);
  digitalWrite(relayDataPin, HIGH);
  digitalWrite(relayDataPin, LOW);
  digitalWrite(relayPowerPin, LOW);
  digitalWrite(lightDataPin, LOW);
  lastReading = true;
}

/* ==== setup Sets up my baud rate, listens on A1 for UV readings, writes to the relay on 13, sets my clock speed to 1MHz, and runs my watchdog configurating ==== */
void setup() {
  configureWatchdog();                 // I put this above the pre-scaler in hopes it would help with the 8 second issue
  CLKPR = 0x80;                        // unlock prescaler
  CLKPR = 0x04;                        // set prescaler
  clockAdjustment = 16;                // I change this to 16 if I uncomment the prescaler above for dev purposes
  pinMode(relayPowerPin,OUTPUT);
  pinMode(lightSensorPowerPin,OUTPUT);
  pinMode(relayDataPin,OUTPUT);
  pinMode(lightDataPin,INPUT);
  digitalWrite(relayDataPin,LOW);
  configureRelay(relayPowerPin, relayDataPin, lightDataPin);
}

/* ==== sleepControl puts the processor to sleep ==== */
void sleepControl() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // using power save sleep mode
  sleep_enable();                      // enabling the possibility of sleeping
  sleep_mode();                        // executing the set sleep mode
                                       // this is exactly where the code continues after waking up
  sleep_disable();                     // disabling the possibility of sleeping
  power_all_enable(); 
}

/* ==== resets our watchdog flag so we don't go to sleep when we're not supposed to ==== */
void setFlagAndSleep() {
  if(f_wdt == 1) 
  {
    f_wdt = 0;
    sleepControl();
  }
}

/* ==== reads our UV Sensor ==== */
unsigned int getLightReading(int lightSensorPowerPin) {
  digitalWrite(lightSensorPowerPin, HIGH);
  byte numberOfReadings = 10;
  unsigned int runningValue = 0; 

  for (int x = 0 ; x < numberOfReadings ; x++) {
    runningValue += analogRead(lightDataPin);
  }
  runningValue /= numberOfReadings;
  digitalWrite(lightSensorPowerPin,LOW);
  return runningValue;
  // reviewLight(runningValue);
}

/* ==== Evaluates last UV reading ==== */
bool isDaytime(int lightValue) {
  return lightValue > 75;
}

/* ==== Sends appropriate signal to the relay based on our last UV reading ==== */
void setLight(int clockAdjustment, int relayPowerPin, int lightDataPin, bool isDay) {
  if ((isDay) && (!lastReading)) 
  {     
    digitalWrite(relayPowerPin, HIGH);
    delay(2000/clockAdjustment);          // I have these delays to give the arduino enough time to ramp up the pin to 5v
    digitalWrite(relayDataPin, HIGH);
    digitalWrite(relayDataPin, LOW);
    delay(2000/clockAdjustment);
    digitalWrite(relayPowerPin, LOW);
    digitalWrite(lightDataPin, LOW);
  } 
  else if ((!isDay) && (lastReading)) 
  {
    digitalWrite(relayPowerPin, HIGH);
    delay(2000/clockAdjustment);
    digitalWrite(relayDataPin, LOW);
    digitalWrite(relayDataPin, HIGH);
    delay(2000/clockAdjustment);
    digitalWrite(relayPowerPin, LOW);
    digitalWrite(lightDataPin, LOW);
  } 
  else 
  {
    digitalWrite(relayPowerPin, LOW);
    digitalWrite(lightDataPin, LOW);
  }
  lastReading = isDay;
}

void loop() {
  unsigned int lightReading = getLightReading(lightSensorPowerPin);
  bool isDay = isDaytime(lightReading);
  setLight(clockAdjustment, relayPowerPin, lightDataPin, isDay);
  setFlagAndSleep();
}
