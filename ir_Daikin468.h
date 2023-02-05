// Daikin class for ARC468A3
// https://diysmartmatter.com/wp-content/uploads/2023/02/daikinremo-scaled.jpg
// based on the IRremoteESP8266 library: https://github.com/crankyoldgit/IRremoteESP8266
// Supported functions are limitted to those of Apple HomeKit Heater/Cooler Accessory
// i.e. power(on,off)/mode(heater,cooler)/swing(on,off)/fanspeed/temp are supported.

//#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include <stdint.h>
#include <string> //for max()


// Daikin Constants
const uint8_t kDaikinAuto = 0b000;  // auto mode
const uint8_t kDaikinDry =  0b010;  // dry mode
const uint8_t kDaikinCool = 0b011;  // cool mode
const uint8_t kDaikinFan =  0b110;  // fan mode
const uint8_t kDaikinHeat = 0b100;  // heat mode
const uint8_t kDaikinSwingOn =  0b1111;
const uint8_t kDaikinSwingOff = 0b0000;

const uint8_t kDaikinMinTemp = 10;  // Celsius
const uint8_t kDaikinMaxTemp = 32;  // Celsius

// Daikin ARC468 Constants
const uint16_t kDaikin468StateLength = 39;
const uint16_t kDaikin468DefaultRepeat = 0;

const uint16_t kDaikin468Freq = 36700;  // Modulation Frequency in Hz.
const uint16_t kDaikin468LeaderMark = 10024;
const uint16_t kDaikin468LeaderSpace = 25180;
const uint16_t kDaikin468Gap = kDaikin468LeaderMark + kDaikin468LeaderSpace;
const uint16_t kDaikin468HdrMark = 3500;
const uint16_t kDaikin468HdrSpace = 1728;
const uint16_t kDaikin468BitMark = 460;
const uint16_t kDaikin468OneSpace = 1270;
const uint16_t kDaikin468ZeroSpace = 420;
const uint16_t kDaikin468Section1Length = 20;
const uint16_t kDaikin468Section2Length = 19;
const uint8_t kDaikin468Tolerance = 5;  // Extra percentage tolerance
const uint8_t kDaikin468LeaderLength=10;
const uint16_t kDaikin468Leader[kDaikin468LeaderLength]= { 1260, 420, 420, 420, 420, 420, 420, 420, 420, 25300 };

const uint8_t kDaikin468Fan1 = 3;
const uint8_t kDaikin468Fan2 = 4;
const uint8_t kDaikin468Fan3 = 5;
const uint8_t kDaikin468Fan4 = 6;
const uint8_t kDaikin468Fan5 = 7;
const uint8_t kDaikin468FanAuto = 0xA;
const uint8_t kDaikin468FanQuiet = 0xB;

const uint8_t kDaikin468MinCoolTemp = 18;  // Min temp (in C) when in Cool mode.



/// Native representation of a Daikin ARC468 A/C message.
union Daikin468Protocol{
  struct{
    uint8_t raw[kDaikin468StateLength];  ///< The state of the IR remote.
  };
  struct {
  //frame1
    // Byte 0~18
    uint8_t NA1[19];
    // Byte 19
    uint8_t Sum1     :8; //sum of bytes 0 ~ 18
  //frame2
    // Byte 20~24
    uint8_t NA2[5];
    // Byte 25
    uint8_t Power    :1; //on=1, off=0
    uint8_t          :3;
    uint8_t Mode     :3; //cool=3, heat=4
    uint8_t          :1;
    // Byte 26
    uint8_t Temp_half :1; //half degree, 0.5 = 1, 0 = 0
    uint8_t Temp      :6; //degree
    uint8_t           :1;
    // Byte 27
    uint8_t           :8;
    // Byte 28
    uint8_t SwingV    :4; //off=0, on=0xF
    uint8_t Fan       :4; // {1, 2, 3, 4, 5, auto, quiet} = {0x30, 0x40, 0x50, 0x60, 0x70, 0xA0, 0xB0} 
    // Byte 29
    uint8_t SwingH    :4; //off=0, on=0xF, not used in ACR468
    uint8_t           :4;
    // Byte 30~37
    uint8_t NA3[8];
    // Byte 38
    uint8_t Sum2      :8; //sum of bytes 20 ~37
  };
};





/// Class for handling detailed Daikin ARC468A3 A/C remote messages
/// @note Code by diySmartMatter, Reverse engineering analysis by diySmartMatter
class IRDaikin468 {
 public:
  explicit IRDaikin468(const uint16_t pin, const bool inverted = false,
                     const bool use_modulation = true);

  void send(const uint16_t repeat = kDaikin468DefaultRepeat);
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool state);
  bool getPower(void) const;
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void) const;
  void setFan(const uint8_t fan);
  uint8_t getFan(void) const;
  uint8_t getMode(void) const;
  void setMode(const uint8_t mode);
  void setSwingVertical(const uint8_t position);
  uint8_t getSwingVertical(void) const;
  void setSwingHorizontal(const uint8_t position);
  uint8_t getSwingHorizontal(void) const;
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[]);  
  char* toChars(void);


 private:
  IRsend _irsend;  ///< instance of the IR send class
  Daikin468Protocol _;
  void stateReset(void);
  void checksum(void);
//  void clearOnTimerFlag(void);
//  void clearSleepTimerFlag(void);
};
