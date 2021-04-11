#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
  long time(seconds);

  long hour = time / 3600;
  time = time % 3600;
  long min = time / 60;
  time = time % 60;
  long sec = time;

  // display data to user
  string colon = ":";

  return std::to_string(hour) + colon + std::to_string(min) + colon +
         std::to_string(sec);
}
