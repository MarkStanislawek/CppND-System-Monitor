#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid)
    : pid_(pid),
      cmd_(LinuxParser::Command(pid)),
      user_(LinuxParser::User(pid)) {
  Reload();
}

int Process::Pid() const { return pid_; }

float Process::CpuUtilization() { return cpuUtilization_; }

string Process::Command() const { return cmd_; }

string Process::Ram() const { return ramMb_; }

string Process::User() const { return user_; }

long int Process::UpTime() const { return upTime_; }

bool Process::operator<(Process const& a) const {
  return cpuUtilization_ < a.cpuUtilization_;
}

bool Process::operator>(Process const& a) const {
  return cpuUtilization_ > a.cpuUtilization_;
}

long Process::GetUpTime() const { return LinuxParser::UpTime(pid_); }

float Process::CalcCpuUtil() {
  long jiffyDiff = LinuxParser::ActiveJiffies(pid_) - prevJiffies;
  float activeSeconds = static_cast<float>(jiffyDiff) / LinuxParser::Jiffies();
  long upTimeDiff = upTime_ - prevUpTime;
  prevJiffies += jiffyDiff;
  prevUpTime += upTimeDiff;
  return activeSeconds / upTimeDiff;
}

string Process::GetRamInMb() const {
  return to_string(LinuxParser::Ram(pid_) / 1000);
}

void Process::Reload() {
  upTime_ = GetUpTime();
  cpuUtilization_ = CalcCpuUtil();
  ramMb_ = GetRamInMb();
}