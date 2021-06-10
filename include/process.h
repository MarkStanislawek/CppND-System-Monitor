#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization();
  std::string Ram() const;
  long int UpTime() const;
  void Reload();
  bool operator<(Process const& a) const;
  bool operator>(Process const& a) const;

 private:
  int pid_;
  std::string cmd_;
  std::string user_;
  std::string ramMb_;
  long upTime_{0};
  float cpuUtilization_{0};
  long prevJiffies{0};
  long prevUpTime{0};
  long GetUpTime() const;
  float CalcCpuUtil();
  std::string GetRamInMb() const;
};

#endif