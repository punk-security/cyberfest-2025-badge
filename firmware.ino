
// BUILD SETTINGS:
// ATTINY412
// disable millis
// run at 16mhz

#define WAKE_TIME_MS 2160000 // Roughly 60 minutes

#include <avr/sleep.h>
#include <avr/io.h>-
#include <avr/pgmspace.h>
#include <tinyNeoPixel_Static.h>
#define NUMLEDS 8
#define BUTTON_PIN PIN_PA7
byte pixels[NUMLEDS * 3];
tinyNeoPixel strip = tinyNeoPixel(NUMLEDS, PIN_PA6, NEO_GRB, pixels);

#define OFF 0,0,0
#define RED 255,0,0
#define GREEN 0,255,0
#define BLUE 0,0,255
#define ORANGE 255,80,0
#define PINK 255,0,180
#define PURPLE 20,0,255
#define OLIVE 2,2,0
#define YELLOW 180,120,0
#define YELLOWGREEN 20,30,20

bool color_switch = false;

uint16_t time_pin_low(uint16_t max_ms)
{
  // blocking for up to max_ms
  if (digitalRead(BUTTON_PIN) == HIGH)
  {
    return(0);
  }
  delay(50); //debounce-
  uint16_t t = 50;
  while(digitalRead(BUTTON_PIN) == LOW)
  {
    delay(5);
    t = t + 5;
    if ( t > max_ms )
      return(max_ms);
  }
  return(t);
}

void setAllPixels(int r, int g, int b, bool show = false)
{
  for (int i = 0; i < NUMLEDS; i++) 
  {
    strip.setPixelColor(i,r,g,b);
  }
  if(show)
    strip.show();
}

void RTC_init()
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;    /* 1kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */
}

void mini_sleep( uint8_t period = RTC_PERIOD_CYC16_gc)
{
  // default timer is about 16ms
  RTC.PITINTCTRL = RTC_PI_bm;  // Enable RTC interrupt
  RTC.PITCTRLA = period | RTC_PITEN_bm;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  RTC.PITINTCTRL = ~(RTC_PI_bm); // Disable RTC interrupt
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;  // Clear RTC interrupt flag otherwise keep coming back here
}

ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS = PORT_INT7_bm; // Clear Pin 7 interrupt flag otherwise keep coming back here
}


void sleep()
{
  writeReg(0x60, 0x93); // 10010000 TEMP + LOW POWER + 10HZ + Idle
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  PORTA.PIN7CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc; // enable pullup and interrupt
  digitalWrite(PIN_PA3, LOW); // turn off LED power rail
  sleep_enable();
  sleep_cpu();
  // sleep resumes here
  PORTA.PIN7CTRL = PORT_PULLUPEN_bm; // renable pullup but no interrupt
  digitalWrite(PIN_PA3, HIGH);  // turn on LED power rail
  lis2mdl_begin();
}

int police(int n)
{
  setAllPixels(40,0,0);
  if (n & 1)
  {
    strip.setPixelColor(0,0,0,255);
    strip.setPixelColor(2,0,0,255);
    strip.setPixelColor(4,0,0,255);
    strip.setPixelColor(6,0,0,255);
  }
  else
  {
    strip.setPixelColor(1,0,0,255);
    strip.setPixelColor(3,0,0,255);
    strip.setPixelColor(5,0,0,255);
    strip.setPixelColor(7,0,0,255);
  }
  strip.show();
  return 200;
}

int phase(int n)
{
  setAllPixels(0,0,0);
  switch(n % 8)
  {
    case 0:
      strip.setPixelColor(1,0,255,0);
      break;
    case 1:
      strip.setPixelColor(0,0,255,0);
      strip.setPixelColor(2,0,255,0);
      break;
    case 2:
      strip.setPixelColor(7,0,255,0);
      strip.setPixelColor(3,0,255,0);
      break;
    case 3:
      strip.setPixelColor(6,0,255,0);
      strip.setPixelColor(4,0,255,0);
      break;
    case 4:
      strip.setPixelColor(5,0,255,0);
      break;
    case 5:
      strip.setPixelColor(6,0,255,0);
      strip.setPixelColor(4,0,255,0);
      break;
    case 6:
      strip.setPixelColor(7,0,255,0);
      strip.setPixelColor(3,0,255,0);
      break;
    case 7:
      strip.setPixelColor(0,0,255,0);
      strip.setPixelColor(2,0,255,0);
      break;
  }
  strip.show();
  return 150;
}

int chase(int n, int r, int g, int b)
{
  uint8_t i = n % 8;
  setAllPixels(OFF);
  strip.setPixelColor(i,r,g,b);
  strip.show();
  return 80;
}

int chase2(int n, int r, int g, int b)
{
  uint8_t i = n % 8;
  uint8_t i2 = (n + 4) % 8;
  setAllPixels(OFF);
  strip.setPixelColor(i,r,g,b);
  strip.setPixelColor(i2,r,g,b);
  strip.show();
  return 200;
}

int cycle(int n)
{
  uint8_t i = n % 5;
  switch(i) {
   case 0:
    setAllPixels(RED,true);
    break;
   case 1:
    setAllPixels(GREEN,true);
    break;
   case 2:
    setAllPixels(YELLOW,true);
    break;
   case 3:
    setAllPixels(PURPLE,true);
    break;
   case 4:
    setAllPixels(BLUE,true);
    break;
  }
  return 100;
}

#include <Wire.h>

static const uint8_t LIS2MDL_ADDR = 0x1E; 

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(LIS2MDL_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t readReg(uint8_t reg) {
  uint8_t pins = Wire.checkPinLevels();
  if ((pins & 0x03))
  {
    strip.setPixelColor(3,1,0,0);strip.show();
  }
  Wire.beginTransmission(LIS2MDL_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(LIS2MDL_ADDR, (uint8_t)1);
  return Wire.read();
}

void readBytes(uint8_t startReg, uint8_t *buf, uint8_t len) {
  // Set MSB of subaddress for auto-increment (ST convention)
  Wire.beginTransmission(LIS2MDL_ADDR);
  Wire.write(startReg | 0x80);
  Wire.endTransmission(false);
  Wire.requestFrom(LIS2MDL_ADDR, len);
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
}

bool lis2mdl_begin() {
  // Setup the 3-axis magnometer for low power continous mode
  writeReg(0x60, 0x20);  // SOFT_RST=1
  delay(10);
  writeReg(0x60, 0x00);  // clear reset
  delay(10);
  writeReg(0x60, 0x90); // 10010000 TEMP + LOW POWER = 10HZ
  return true;
}

void setup()
{
  // Power save
  // http://www.technoblogy.com/show?2RA3
  // https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PowerSave.md
  ADC0.CTRLA &= ~ADC_ENABLE_bm; // Disable ADC
  pinMode(PIN_PA6, OUTPUT);
  // UPDI does not need setting (PA0)

  // Pin setup
  pinMode(PIN_PA3, OUTPUT);
  pinMode(PIN_PA7, INPUT_PULLUP);
  digitalWrite(PIN_PA3, HIGH);

  Wire.usePullups();
  Wire.begin();
  Wire.setClock(100000L);
  if (!lis2mdl_begin()) {
    strip.setPixelColor(3,255,0,0);
  }
  else
  {
    strip.setPixelColor(3,0,255,0);
  }
  strip.show();
  RTC_init();
}

int16_t offX=0, offY=0;
int16_t minX=32767, maxX=-32768, minY=32767, maxY=-32768;

void accumulateCal(int16_t x, int16_t y){
  if (x<minX) minX=x; if (x>maxX) maxX=x;
  if (y<minY) minY=y; if (y>maxY) maxY=y;
}

void finishCal() {
  offX = (int16_t)((maxX + minX)/2);
  offY = (int16_t)((maxY + minY)/2);
}

void lis2mdl_readRaw(int16_t &mx, int16_t &my) {
  uint8_t buf[4];
  readBytes(0x68, buf, 4);  // OUTX_L .. OUTZ_H
  int16_t x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  int16_t y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  mx = x;
  my = y;
}

// Integer-only compass heading in whole degrees [0..359]
uint16_t computeHeading(int16_t mx, int16_t my) {
  
  int32_t x = mx, y = my;

  // Guard: undefined when both are zero
  if (x == 0 && y == 0) return 0;

  int32_t ay = (y < 0) ? -y : y;   // |y|
  int32_t angle_cdeg;              // angle in centi-degrees (deg*100)

  if (x >= 0) {
    int32_t denom = x + ay;        // >= 0
    if (denom == 0) {
      angle_cdeg = 0;              // x==0 & y==0 already handled; pick 0°
    } else {
      // ≈ 45° − 45° * (x − |y|)/(x + |y|)
      angle_cdeg = 4500 - (4500 * (x - ay)) / denom;
    }
  } else {
    // x < 0
    int32_t denom = ay - x;        // > 0
    // ≈ 135° − 45° * (x + |y|)/( |y| − x )
    angle_cdeg = 13500 - (4500 * (x + ay)) / denom;
  }

  if (y < 0) angle_cdeg = -angle_cdeg;

  // Normalize to [0, 36000)
  angle_cdeg %= 36000;
  if (angle_cdeg < 0) angle_cdeg += 36000;

  // Convert to whole degrees. Important: guard 360 after rounding.
  uint16_t deg = (uint16_t)((angle_cdeg + 50) / 100); // round to nearest
  if (deg >= 360) deg -= 360;                         // avoid 360 wrap
  return deg;
}

int display_north()
{
    setAllPixels(OFF,true);
    int16_t rx, ry;
    int heading;
    int pixel;
    lis2mdl_readRaw(rx, ry);
    accumulateCal(rx, ry);
    finishCal();
    rx -= offX;
    ry -= offY;
    heading = computeHeading(rx >> 3, ry >> 3 );

    uint8_t sector = (heading + 22) / 45;
    if (sector >= 8) sector = 0;

    const uint8_t sectorToPixel[8] = {
      1, // sector 0: 0°–22°
      0, // sector 1: 23°–67°
      7, // sector 2: 68°–112°
      6, // sector 3: 113°–157°
      5, // sector 4: 158°–202°
      4, // sector 5: 203°–247°
      3, // sector 6: 248°–292°
      2  // sector 7: 293°–337°, 338–359 wraps to sector 0 again (→ pixel 1)
    };

    pixel = sectorToPixel[sector];

    if (color_switch)
    {
      strip.setPixelColor(pixel, GREEN);
    }
    else
    {
      strip.setPixelColor(pixel, YELLOW);
    }
    
    strip.show();
    return(20);
}

void loop()
{
  uint8_t mode = 0;
  uint8_t interval;
  uint16_t button_low_time = 0;
  uint32_t total_interval = 0;
  strip.begin();
  uint8_t i = 0;
  while(true)
  {
    if ( mode == 0)
    {
      interval = chase(i,0,100,255);
    }
    else if ( mode == 1 )
    {
      interval = chase2(i, YELLOW);
    }
    else if ( mode == 2)
    {
      interval = police(i);
    }
    else if ( mode == 3)
    {
      interval = cycle(i);
    }
    else if ( mode == 4)
    {
      interval = phase(i);
    }
    else if ( mode == 5)
    {
      interval = chase(i,RED);
    }
    else if ( mode == 9)
    {
      mode = 0;
      continue;
    }
    else if ( mode == 10)
    {
      // COMPASS MODE
      interval = display_north();
    }
    else
    {
      mode++;
    }
    i++;
    // This section breaks down the sleep interval to catch button presses
    total_interval = total_interval + interval;
    while(interval > 0)
    {
      mini_sleep();
      interval = interval - 10;
      button_low_time = time_pin_low(2000);
      if (button_low_time > 100)
      {
       /*
       * MAIN MENU
       * NO PRESS = CONTINUE
       * SHORT PRESS = CHANGE FLASHY MODE
       * LONG PRESS = SLEEP
       */
        if (button_low_time > 1200)
        {
          setAllPixels(RED,true);
          while(digitalRead(BUTTON_PIN) == LOW)
          {
            // WAIT FOR RELEASE BEFORE SLEEPING otherwise we wake back up!
            delay(5);
          }
          delay(50); //DEBOUNCE
          sleep();
        }
        else if (button_low_time > 400)
        {
          // SWITCH MODE
          if (mode == 10)
          {
            // If we are in compass mode, leave it
            mode = 0;
            continue;
          }
          // ELSE< setup and enter compass mode
          mode = 10;
          minX=32767;
          maxX=-32768;
          minY=32767;
          maxY=-32768;
          if (color_switch)
          {
            setAllPixels(GREEN,true);
          }
          else
          {
            setAllPixels(YELLOW,true);
          }
          
          for (uint8_t i=0; i<250; i++) {
            int16_t rx, ry; lis2mdl_readRaw(rx, ry);
            accumulateCal(rx, ry);
            mini_sleep();
          }
          finishCal();
          continue;
        }
        else
        {
          if (mode != 10) mode++;
          color_switch = !color_switch;
        }
        // reset timer
        i = 0;
        total_interval = 0;
      }

    }
    // At the end of each interval, see if we need to sleep
    if (total_interval > WAKE_TIME_MS)
    {
      total_interval = 0;
      i = 0;
      sleep();
      if (mode == 10) mode = 0; // If in compass mode, leave it
    }
  }
}
