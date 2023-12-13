#include "FS.h"
#include "SPIFFS.h"


const int LIGHT_SENSOR_PIN = 36;

hw_timer_t *my_timer0 = NULL;
hw_timer_t *my_timer1 = NULL;

bool bool_timer0 = false;
bool bool_timer1 = false;

void ReadFile ();
void WriteFile();

void ConvertDouble8BitTo12Bit(unsigned char high_byte, unsigned char low_byte, int &analog_value);
void Convert12BitToDouble8Bit(int analog_value, unsigned char &high_byte, unsigned char &low_byte);

void SetupFile();

void FormatSPIFFS();
void BeginSPIFFS ();

void SetupMultipleTimer();

void SetupTimer1();
void SetupTimer0();

void IRAM_ATTR OnTimerReadFile ();
void IRAM_ATTR OnTimerWriteFile();

void setup()
{
  Serial.begin(115200);

  while(not Serial)
  {
    delay(100);
  }

  SetupFile();
  SetupMultipleTimer();
}

void loop()
{
  WriteFile();
  ReadFile();
}

void IRAM_ATTR OnTimerWriteFile()
{
  bool_timer0 = true;
}

void IRAM_ATTR OnTimerReadFile()
{
  bool_timer1 = true;
}

void SetupTimer0()
{
  my_timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(my_timer0, &OnTimerWriteFile, true);
  timerAlarmWrite(my_timer0, 1000000, true);
  timerAlarmEnable(my_timer0);
}

void SetupTimer1()
{
  my_timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(my_timer1, &OnTimerReadFile, true);
  timerAlarmWrite(my_timer1, 1000000, true);
  timerAlarmEnable(my_timer1);
}

void SetupMultipleTimer()
{
  SetupTimer0();
  SetupTimer1();
}

void BeginSPIFFS()
{
  if(SPIFFS.begin(true))
  {
    Serial.println("Successfully begin SPIFFS");
  }
  else
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void FormatSPIFFS()
{
  bool formatted = SPIFFS.format();
  if(formatted)
  {
    Serial.println("SPIFFS formatted successfully");
  }
  else
  {
    Serial.println("Error formatting");
    return;
  }
}

void SetupFile()
{
  BeginSPIFFS();
  FormatSPIFFS();
}

void Convert12BitToDouble8Bit(int analog_value, unsigned char &high_byte, unsigned char &low_byte)
{
  high_byte = (analog_value >> 8) & 0xFF;
  low_byte  = analog_value & 0xFF;
}

void ConvertDouble8BitTo12Bit(unsigned char high_byte, unsigned char low_byte, int &analog_value)
{
  analog_value = (high_byte << 8) | low_byte;
}

void WriteFile()
{
  if(bool_timer0)
  {
    File file_write = SPIFFS.open("/test.txt", "w");
    if(!file_write)
    {
      Serial.println("Failed to open file_write for reading");
      return;
    }
    else
    {
      Serial.println("Successfully to open file_write for reading");
    }

    const int analog_value = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Write: ");
    Serial.println(analog_value);

    unsigned char high_byte = 0U;
    unsigned char low_byte  = 0U;
    Convert12BitToDouble8Bit(analog_value, high_byte, low_byte);

    file_write.write(high_byte);
    file_write.write(low_byte);

    file_write.close();
    bool_timer0 = false;
  }
}

void ReadFile()
{
  if(bool_timer1)
  {
    File file_read = SPIFFS.open("/test.txt", "r");
    if(!file_read)
    {
      Serial.println("Failed to open file_read for reading");
      return;
    }
    else
    {
      Serial.println("Successfully to open file_read for reading");
    }

    const unsigned char high_byte = file_read.read();
    const unsigned char low_byte  = file_read.read();

    int analog_value = 0;
    ConvertDouble8BitTo12Bit(high_byte, low_byte, analog_value);

    Serial.print("Read: ");
    Serial.println(analog_value);

    file_read.close();
    bool_timer1 = false;
  }
}