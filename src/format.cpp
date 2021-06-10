#include <sstream>
#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long elapsedSeconds) {
  int hour = elapsedSeconds / HOUR;
  int second = elapsedSeconds % HOUR;
  int minute = second / MINUTE;
  second %= MINUTE;
  char buffer[9];
  std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hour, minute, second);
  return string(buffer);
}