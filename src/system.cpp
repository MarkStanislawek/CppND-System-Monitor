#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include <algorithm>
#include "linux_parser.h"
#include "process.h"

#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

System::System()
    : kernel_(LinuxParser::Kernel()),
      operatingSystem_(LinuxParser::OperatingSystem()) {}

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  // 1. Remove Process instances from processes_ which are no longer running.
  // 2. Find the difference between pids and processes_
  // 3. For Process instances still running, reload their resource snapshot
  // 4. Add new Process instances to processes_ for each new pid.
  // 5. Sort processes_ for display
  vector<int> runningPids = LinuxParser::Pids();
  RemoveProcessesNotRunning(runningPids);
  vector<int> newProcesses = FindProcessDifference(runningPids);
  std::for_each(processes_.begin(), processes_.end(),
                [](Process& p) { p.Reload(); });
  std::for_each(newProcesses.begin(), newProcesses.end(),
                [&](int& pid) { processes_.emplace_back(pid); });
  std::sort(processes_.begin(), processes_.end(), std::greater<Process>());
  return processes_;
}

std::string System::Kernel() { return kernel_; }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return operatingSystem_; }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }

void System::RemoveProcessesNotRunning(vector<int>& runningPids) {
  std::sort(runningPids.begin(), runningPids.end());
  processes_.erase(std::remove_if(processes_.begin(), processes_.end(),
                                  [runningPids](Process& p) {
                                    return std::binary_search(
                                               runningPids.begin(),
                                               runningPids.end(),
                                               p.Pid()) == false;
                                  }),
                   processes_.end());
}

vector<int> System::FindProcessDifference(vector<int>& runningPids) {
  vector<int> intersection;
  std::transform(processes_.begin(), processes_.end(),
                 std::inserter(intersection, intersection.begin()),
                 [](Process& p) -> int { return p.Pid(); });
  std::sort(intersection.begin(), intersection.end());
  vector<int> difference;
  std::set_difference(runningPids.begin(), runningPids.end(),
                      intersection.begin(), intersection.end(),
                      std::inserter(difference, difference.begin()));
  return difference;
}