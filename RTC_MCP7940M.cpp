/* Does not support 12-hr clock cycle*/
#include <Wire.h>

byte decToBcd(byte val){
  return ( ((val / 10) << 4) | (val % 10) );
}

void RTC_MCP7940M::begin(){
  Wire.begin();
}

void RTC_MCP7940M::timeSet(byte second, byte minute, byte hour, byte wkday, byte date, byte month, byte year){
/*year must be input as just the last two digits*/
  byte weekday_reg;
  bool oscillator_running;
  int counter;
  byte new_hour;
  byte new_month;
  byte register_val;
  

  // Stop clock
  counter = 0;
  do
  {
    if (counter == 0){ 
      byte clock_source = RTC_MCP7940M.stopClock(); // Stop clock (returns 0 for ext clock, 1 for crystal
    } else if (counter % 10 == 0){
      (void) RTC_MCP7940M.stopClock(); // After running 10 loops, restop clock (ignore func output)
    }
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCWKDAY);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    weekday_reg = Wire.read();
    oscillator_running = bitRead(weekday_reg, 5);
    counter++;
  } while (!oscillator_running)
  
  // Set seconds register (ignoring ST bit for now)
  Wire.beginTransmission(RTC_MCP);
  Wire.write(RTCSEC);
  Wire.write(decToBCD(second));

  // Set minutes register
  Wire.write(decToBCD(minute));

  // Set hour register (24 hour cycle)
  new_hour = dectoBcd(hour);
  new_hour |= B01000000; // Set 24 hour cycle bit
  Wire.write(new_hour);

  // Set weekday register
  Wire.write(wkday); // Weekday is user-defined

  // Set date register
  Wire.write(decToBcd(date));

  // Set month register
  new_month = decToBcd(month);
  
  if (year % 4 == 0){ // If leap year
    new_month |= B00100000;
  }
  
  Wire.write(new_month);

  // Set year register
  Wire.write(decToBcd(year));

  // Send writes to registers
  Wire.endTransmission();

  // Restart the oscillator

  Wire.beginTransmission(RTC_MCP);
  
  if (clock_source == 0) { // External clock
    Wire.write(CONTROL);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    register_val = Wire.read();
    register_val |= B00001000; // Turn on external oscillator bit
    Wire.beginTransmission(RTC_MCP);
    Wire.write(CONTROL);
    Wire.write(register_val);
  } else if (clock_source == 1){ // External crystal
    Wire.write(RTCSEC);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    register_val = Wire.read();
    register_val |= B10000000; // Turn on external crystal bit
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCSEC);
    Wire.write(register_val);
  }
  Wire.endTransmission();
}

byte RTC_MCP7940M::stopClock(){
/* This function should  be run after a clock source has been selected 
 *  Returns 0 if using external clock
 *  Returns 1 if using external crystal
 *  Returns -1 if neither
*/
    byte out;
    
    // Read control reg
    Wire.beginTransmission(RTC_MCP);
    Wire.write(CONTROL);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    byte control_reg = Wire.read();

    // Read seconds register
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCSEC);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    byte sec_reg = Wire.read();

    if (bitRead(control_reg, EXTOSC)){
      out = 0;
    } else if (bitRead(sec_reg, ST)){
      out = 1; 
    } else{
      out = -1;
    }

    bitClear(control_reg, EXTOSC);
    bitClear(sec_reg, ST);

    // Write control reg
    Wire.beginTransmission(RTC_MCP);
    Wire.write(CONTROL);
    Wire.write(control_reg);
    Wire.endTransmission();

    // Write seconds register
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCSEC);
    Wire.write(sec_reg)
    Wire.endTransmission();

    return out;
}

void RTC_MCP7940::clockSelect(bool external_clock){
  
    // Read control reg
    Wire.beginTransmission(RTC_MCP);
    Wire.write(CONTROL);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    byte control_reg = Wire.read();

    // Read seconds register
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCSEC);
    Wire.endTransmission();
    Wire.requestFrom(RTC_MCP, 1);
    byte sec_reg = Wire.read();
  
  if (external_clock) { // If using external clock
    bitSet(control_reg, EXTOSC);
    bitClear(sec_reg, ST);
  } else {
    bitClear(control_reg, EXTOSC);
    bitSet(sec_reg, ST);
  }
  
    // Write control reg
    Wire.beginTransmission(RTC_MCP);
    Wire.write(CONTROL);
    Wire.write(control_reg);
    Wire.endTransmission();

    // Write seconds register
    Wire.beginTransmission(RTC_MCP);
    Wire.write(RTCSEC);
    Wire.write(sec_reg)
    Wire.endTransmission();
}

void RTC_MCP7940::alarmEnable(bool alarm0, bool set_alarm, bool alarm_low, int alarm_mask_setting){
/* alarm0 = true for alarm 0, alarm0 = false for alarm 1
 * set_alarm = true to enable, set_alarm = false to disable
 * alarm_low = true means output pin goes low on alarm
 * alarm_mask_setting definitions:
 * 0 = seconds match
 * 1 = minutes match
 * 2 = hours match
 * 3 = day of week match
 * 4 = date match
 * 5 = seconds, minutes, hour, day of week, date, and month match
 * 
 * This function clears the alarm flag regardless of whether it is enabling or disabling an alarm.
 * 
 * CAUTION: Do not enable alarm that is currently true, will result in immediate alarm.
 * 
 */
 
  byte write_reg_addr;
  byte write_reg_value;
  
  // Set appropriate alarm polarity
  Wire.beginTransmission(RTC_MCP);
  
  if (alarm0){
    write_reg_addr = ALM0WKDAY;
  }else{
    write_reg_addr = ALM1WKDAY;
  }

  Wire.write(write_reg_addr);
  Wire.endTransmission();
  Wire.requestFrom(RTC_MCP, 1);
  write_reg_value = Wire.read();

  if (alarm_low){ // Alarm output is logic low
    bitClear(write_reg_value, 7);
  } else { // Alarm output is logic high
    bitSet(write_reg_value, 7);
  }
    
  // Set up alarm setting for appropriate alarm (alarm setting is the mask e.g. 000 = seconds)
  if (alarm_mask_setting == 0){ // Seconds match
    write_reg_value &= B10001111;
  } else if (alarm_mask_setting == 1){ // Minutes match
    write_reg_value &= B10011111;
  } else if (alarm_mask_setting == 2){ // Hours match
    write_reg_value &= B10101111;
  } else if (alarm_mask_setting == 3){ // Day of week match
    write_reg_value &= B10111111;
  } else if (alarm_mask_setting == 4){ // Date match
    write_reg_value &= B11001111;
  } else if (alarm_mask_setting == 5){ // All match
    write_reg_value &= B11111111;
  }
  
  // Send weekday register value
  Wire.beginTransmission(RTC_MCP);
  Wire.write(write_reg_addr);
  Wire.write(write_reg_value);
  Wire.endTransmission();

  // Enable/Disable alarm register
  // Get control register
  Wire.beginTransmission(RTC_MCP);
  Wire.write(CONTROL);
  Wire.endTransmission();
  Wire.requestFrom(RTC_MCP, 1);
  byte control_reg = Wire.read();
  
  if (alarm0) { // Setting/clearing alarm 0
    if (set_alarm){
      bitSet(control_reg, ALM0EN);
    } else {
      bitClear(control_reg, ALM0EN);
    }
  } else { // Setting/clearing alarm 1
    if (set_alarm){
      bitSet(control_reg, ALM1EN);
    } else{
      bitClear(control_reg, ALM1EN);
    }
  }

  // Send new register value
  Wire.beginTransmission(RTC_MCP);
  Wire.write(CONTROL);
  Wire.write(control_reg);
  Wire.endTransmission();
}

void RTC_MCP7940::alarmAdjust(bool alarm0, int second, int minute, int hour, int wkday, int date, int month){
  // Set the appropriate alarm registers to the input times (doublecheck that the initialized values are valid)
  // This function clears the alarm flag

  Wire.beginTransmission(RTC_MCP);
  if(alarm0){ // Alarm 0 registers
    Wire.write(ALM0SEC);
  } else { // Alarm 1 registers
    Wire.write(ALM1SEC);
  }

  // Write seconds register
  Wire.write(decToBcd(second));

  // Write minutes register
  Wire.write(decToBcd(minute));

  // Write hours register (24-hour clock only)
  byte hours_reg = decToBcd(hour);
  bitClear(hours_reg, 6); // Just to be sure
  Wire.write(hours_reg);

  // Send this data
  Wire.endTransmission();

  // Now need to read weekday register (it contains other data too)
  Wire.beginTransmission(RTC_MCP);
  if (alarm0){
    Wire.write(ALM0WKDAY);
  } else{
    Wire.write(ALM1WKDAY);
  }
  Wire.endTransmission()
  Wire.requestFrom(RTC_MCP, 1):
  byte weekday_reg = Wire.read();

  // Adjust weekday register to align with input
  weekday_reg |= B00000111;
  weekday_reg &= (B11111000 & wkday);

  Wire.beginTransmission(RTC_MCP);

  // Write weekday register
  if (alarm0){
    Wire.write(ALM0WKDAY);
  } else{
    Wire.write(ALM1WKDAY);
  }  
  Wire.write(weekday_reg);

  // Write date register
  Wire.write(decToBcd(date));

  // Write month register
  Wire.write(decToBcd(month));

  Wire.endTransmission();
}

void RTC_MCP7940M::configure(int Mode, bool out_pin, bool trim, int sqwave_set){
  /*
   * Mode == 1 is general purpose or alarm (must set alarms using alarmEnable func, note this method disables alarms currently set)
   * Mode != 1 is square wave clock output
   * 
   * out_pin = true sets out pin high (only used in general purpose mode)
   * 
   * trim == true enables coarse trimming of square wave output (so must be in Mode == 2)
   * 
   * sqwave_set is only used in Mode == 2 and trim == false
   * sqwave_set == 0 outputs squarewave @ 1 Hz freq (digitally trimmed)
   * sqwave_set == 1 outputs squarewave @ 4.096 Hz freq (digitally trimmed)
   * sqwave_set == 2 outputs squarewave @ 8.192 Hz freq (digitally trimmed)
   * sqwave_set == 3 outputs squarewave @ 32.768 Hz freq
   */

   // Get current control register
   Wire.beginTransmission(RTC_MCP);
   Wire.write(CONTROL);
   Wire.endTransmission();
   Wire.requestFrom(RTC_MCP, 1);
   byte current_control = Wire.read();

   // Set out pin
   if (out_pin){
    bitSet(current_control, OUT);
   } else {
    bitClear(current_control, OUT);
   }

   // Set mode
   if (Mode == 1) { // General purpose
    bitClear(current_control, SQWEN);
    bitClear(current_control, ALM1EN);
    bitClear(current_control, ALM0EN);
   } else { // Square wave
    bitSet(current_control, SQWEN);
   }

   // Trim on/off
   if (trim) {
    bitSet(current_control, CRSTRIM);
   } else {
    bitClear(current_control, CRSTRIM);
   }

   // Square wave settings 
   current_control &= B11111100;
   current_control |= (sqwave_set & B00000011); // Set with input (w/ protection against acidental inputs)

  
   Wire.beginTransmission(RTC_MCP);
   Wire.write(CONTROL);
   Wire.write(current_control);
   Wire.endTransmission();
}

void RTC_MCP7940M::trimConfig(int cycles, bool add){
  /* cycles is the number of cycles added or subtracted from the clock. 
   * cycles must be an even integer between 0 and 254 (if odd it will round down). 0 results in no trimming.
   * 
   * add == true results in cycles added (corrects for slow time)
   * add == false results in cycles subtracted (corrects for fast time)
   * 
   * This function is dependent on the trim set in RTC_MCP7940M.config. 
   * If trim == false, trimming is performed over a minute.
   * If trim == true, trimming is performed 128 times per second.
  */

  // Sign bit
  byte trim_reg_val = 0;
  if (add){ 
    trim_reg_val = 1 << 7; // Add clocks
  }

  // Trim value bits
  trim_reg_val |= (cycles / 2);
  
  // Write to oscillator digital trim register
  Wire.beginTransmission(RTC_MCP);
  Wire.write(OSCTRIM);
  Wire.write(trim_reg_val);
  Wire.endTransmission();
}