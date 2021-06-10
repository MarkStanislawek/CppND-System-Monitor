#include "processor.h"
#include <cassert>
#include <numeric>
#include <string>
#include "linux_parser.h"

float Processor::Utilization() {
  std::vector<long> cpuTimes = LinuxParser::CpuUtilization();
  assert(cpuTimes.size() == 10);
  long totalTime = std::accumulate(cpuTimes.begin(), cpuTimes.end(), 0);
  totalTime -=
      cpuTimes.at(8) +
      cpuTimes.at(
          9);  // remove guest, guest_nice; they're included in user, nice
  long iowait = cpuTimes.at(4);
  long idle = cpuTimes.at(3) + iowait;
  float timeDelta = totalTime - prevTotalTime_;
  float idleDelta = idle - prevIdleTime_;
  prevTotalTime_ += timeDelta;
  prevIdleTime_ += idleDelta;
  return 1 - idleDelta / timeDelta;
}