// Daikin class for ARC468A3
// https://diysmartmatter.com/wp-content/uploads/2023/02/daikinremo-scaled.jpg
// based on the IRremoteESP8266 library: https://github.com/crankyoldgit/IRremoteESP8266
// Supported functions are limitted to those of Apple HomeKit Heater/Cooler Accessory
// i.e. power(on,off)/mode(heater,cooler)/swing(on,off)/fanspeed/temp are supported.

#include "ir_Daikin468.h"
#include <cstring> //for memcpy
#include "IRremoteESP8266.h"


/// Class constructor.
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin468::IRDaikin468(const uint16_t pin, const bool inverted,
                     const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin468::begin(void) { _irsend.begin(); }

/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin468::send(const uint16_t repeat) {
  checksum();
  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    _irsend.sendRaw(kDaikin468Leader, kDaikin468LeaderLength, kDaikin468Freq);
    // Section #1
    _irsend.sendGeneric(kDaikin468HdrMark, kDaikin468HdrSpace, kDaikin468BitMark,
                kDaikin468OneSpace, kDaikin468BitMark, kDaikin468ZeroSpace,
                kDaikin468BitMark, kDaikin468Gap, &_.raw[0], kDaikin468Section1Length,
                kDaikin468Freq, false, 0, 50);
    // Section #2
    _irsend.sendGeneric(kDaikin468HdrMark, kDaikin468HdrSpace, kDaikin468BitMark,
                kDaikin468OneSpace, kDaikin468BitMark, kDaikin468ZeroSpace,
                kDaikin468BitMark, kDaikin468Gap, &_.raw[kDaikin468Section1Length], kDaikin468Section2Length,
                kDaikin468Freq, false, 0, 50);
  }
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin468::checksum(void) {
  _.Sum1 = 0;
  for(int i=0; i<kDaikin468Section1Length - 1;i++) _.Sum1 += _.raw[i];
  _.Sum2 = 0;
  for(int i=kDaikin468Section1Length; i<kDaikin468StateLength - 1;i++) _.Sum2 += _.raw[i];

}

/// Prepare hex (0-9 and A-F) chars of the current internal state.
/// @return PTR to the char.
char* IRDaikin468::toChars(void) {
  static char result[kDaikin468StateLength * 2 + 1];
  checksum();   // Ensure correct settings jic.
  for(int i=0;i<kDaikin468StateLength;i++){
    uint8_t byte,hex1,hex2;
    byte=_.raw[i];
    hex1=byte >> 4;
    hex2=byte & 0xF;
    if(hex1 < 10) hex1 += '0'; else hex1 += 'A' - 10;
    if(hex2 < 10) hex2 += '0'; else hex2 += 'A' - 10;
    result[i * 2] = (char)hex1;
    result[i * 2 + 1] = (char)hex2;
  }
  result[kDaikin468StateLength * 2]='\0';
  return result;
}


/// Reset the internal state to a fixed known good state.
// the state is: temp=28deg. power=off, mode=cool, fan=max, swing=no
void IRDaikin468::stateReset(void) {
  for (uint8_t i = 0; i < kDaikin468StateLength; i++) _.raw[i] = 0x0;
  _.raw[0] = 0x11; // fixed
  _.raw[1] = 0xDA; // fixed
  _.raw[2] = 0x27; // fixed
  _.raw[4] = 0x01; // unknown
  // _.raw[19] is a checksum byte, it will be set by checksum().
  _.raw[20] = 0x11; // fixed
  _.raw[21] = 0xDA; // fixed
  _.raw[22] = 0x27; // fixed
  _.raw[25] = 0x38; // mode=cool, power=off
  _.raw[26] = 0x38; // 0x38 / 2 = 0x1c (0d28)
  _.raw[28] = 0x70; // fan=max, swing virtical=off
  _.raw[31] = 0x06; // unknown
  _.raw[32] = 0x60; // unknown
  _.raw[35] = 0xC1; // unknown
  _.raw[36] = 0x90; // unknoan
  // _.raw[38] is a checksum byte, it will be set by checksum().
  checksum();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin468::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin468::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin468StateLength);
}

/// Change the power setting to On.
void IRDaikin468::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRDaikin468::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin468::setPower(const bool on) { _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin468::getPower(void) const { return _.Power;}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin468::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] desired_mode The desired operating mode.
void IRDaikin468::setMode(const uint8_t desired_mode) {
  uint8_t mode = desired_mode;
  switch (mode) {
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry: break;
    default: mode = kDaikinAuto;
  }
  _.Mode = mode;
  // Redo the temp setting as Cool mode has a different min temp.
  if (mode == kDaikinCool) setTemp(getTemp());
}

/// Set the temperature.
/// @param[in] desired The temperature in degrees celsius.
void IRDaikin468::setTemp(const uint8_t desired) {
  // The A/C has a different min temp if in cool mode.
  uint8_t temp = std::max(
      (_.Mode == kDaikinCool) ? kDaikin468MinCoolTemp : kDaikinMinTemp,
      desired);
  _.Temp = std::min(kDaikinMaxTemp, temp);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin468::getTemp(void) const { return _.Temp; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or Auto or Quiet
void IRDaikin468::setFan(const uint8_t desired_fan) {
  uint8_t fan = desired_fan;
  switch (fan) {
    case kDaikin468Fan1:
    case kDaikin468Fan2:
    case kDaikin468Fan3:
    case kDaikin468Fan4:
    case kDaikin468Fan5:
    case kDaikin468FanAuto:
    case kDaikin468FanQuiet: break;
    default: fan = kDaikin468FanAuto;
  }
  _.Fan = fan;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRDaikin468::getFan(void) const { return _.Fan; }

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void IRDaikin468::setSwingVertical(const uint8_t position) {
  switch (position) {
    case kDaikinSwingOn:
    case kDaikinSwingOff: _.SwingV = position;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t IRDaikin468::getSwingVertical(void) const { return _.SwingV; }

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void IRDaikin468::setSwingHorizontal(const uint8_t position) {
  switch (position) {
    case kDaikinSwingOn:
    case kDaikinSwingOff: _.SwingH = position;
  }
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t IRDaikin468::getSwingHorizontal(void) const { return _.SwingH; }
