#define RTC_MCP 0x6F
#define RTCSEC 0x00
#define RTCMIN 0x01
#define RTCHOUR 0x02
#define RTCWKDAY 0x03
#define RTCDATE 0x04
#define RTCMTH 0x05
#define RTCYEAR 0x06
#define CONTROL 0x07
#define OSCTRIM 0x08
#define ALM0SEC 0x0A
#define ALM1SEC 0x11
#define ST 7
#define EXTOSC 3
#define OSCRUN 5
#define ALM0EN 4
#define ALM1EN 5
#define ALM0WKDAY 0x0D
#define ALM1WKDAY 0x14
#define ALM0SEC 0x0A
#define ALM1SEC 0x11
#define OUT 7
#define CRSTRIM 2
#define SQWEN 6
#define OSCTRIM 0x08

class RTC_MCP7940M{
  public:

  static void begin();
  static void timeSet(byte second, byte minute, byte hour, byte wkday, byte date, byte month, byte year);
  static byte stopClock();
  static void clockSelect(bool external_clock = false);
  static void alarmEnable(bool alarm0 = true, bool set_alarm = true, bool alarm_low = true, int alarm_mask_setting = 1);
  static void alarmAdjust(bool alarm0 = true, int second = 0, int minute = 0, int hour = 0, int wkday = 1, int date = 1, int month = 1);
  static void configure(int Mode = 1, bool out_pin = true, bool trim = false, int sqwave_set = 0);
  static void trimConfig(int cycles = 0, bool add = true);

  private:
  static byte decToBcd(byte val);
  
}
