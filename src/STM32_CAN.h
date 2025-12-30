#ifndef STM32_CAN_H
#define STM32_CAN_H

#include <Arduino.h>

// CAN message 
struct CAN_Message {
  uint32_t id;
  uint8_t length;
  uint8_t data[8];
  bool extended;
};

class STM32_CAN {
public:
  STM32_CAN();
  bool begin(uint32_t bitrate);
  void write(CAN_Message &msg);

private:
  void configurePins();
  bool setBitTiming(uint32_t bitrate);
};

#endif // STM32_CAN_H
