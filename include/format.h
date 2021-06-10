#ifndef FORMAT_H
#define FORMAT_H

#define HOUR 3600
#define MINUTE 60

#include <string>

namespace Format {
std::string ElapsedTime(long elapsedSeconds);
};  // namespace Format

#endif