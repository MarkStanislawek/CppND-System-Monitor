#include <dirent.h>
#include <unistd.h>
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (auto const& p : std::filesystem::directory_iterator(kProcDirectory))
    if (p.is_directory()) {
      std::string name = p.path().filename();
      if (std::all_of(name.begin(), name.end(), isdigit))
        pids.emplace_back(stoi(name));
    }
  return pids;
}

float LinuxParser::MemoryUtilization() {
  vector<long> memInfo = GetMemoryInfo();
  float totalMem = memInfo.at(0);
  float usedMem = totalMem - memInfo.at(1);
  return usedMem / totalMem;
}

long LinuxParser::UpTime() {
  string line = GetLineNumber(kProcDirectory + kUptimeFilename, 1);
  std::istringstream linestream(line);
  string utime_str;
  linestream >> utime_str;
  return std::stol(utime_str);
}

long LinuxParser::Jiffies() { return sysconf(_SC_CLK_TCK); }

long LinuxParser::ActiveJiffies(int pid) {
  vector<string> stats = GetPidStats(pid);
  long total_time = stol(stats.at(13)) + stol(stats.at(14)) +
                    stol(stats.at(15)) + stol(stats.at(16));
  return total_time;
}

long LinuxParser::ActiveJiffies() {
  vector<long> cpuTimes = CpuUtilization();
  long totalTime = std::accumulate(cpuTimes.begin(), cpuTimes.end(), 0);
  long activeTime = totalTime - cpuTimes.at(3);
  return activeTime;
}

long LinuxParser::IdleJiffies() {
  vector<long> cpuTimes = CpuUtilization();
  return cpuTimes.at(3);
}

vector<long> LinuxParser::CpuUtilization() {
  string cpuLine = FindLine(kProcDirectory + kStatFilename, "cpu");
  return GetCpuTimes(cpuLine);
}

int LinuxParser::TotalProcesses() {
  string line = FindLine(kProcDirectory + kStatFilename, "processes");
  vector<string> values = GetValues(line);
  assert(values.size() == 1);
  return stoi(values.at(0));
}

int LinuxParser::RunningProcesses() {
  string line = FindLine(kProcDirectory + kStatFilename, "procs_running");
  vector<string> values = GetValues(line);
  assert(values.size() == 1);
  return stoi(values.at(0));
}

string LinuxParser::Command(int pid) {
  return GetLineNumber(kProcDirectory + to_string(pid) + "/cmdline", 1);
}

long LinuxParser::Ram(int pid) {
  string memLine =
      FindLine(kProcDirectory + to_string(pid) + "/status", "VmSize:");
  vector<string> values = GetValues(memLine);
  return values.size() == 2 ? stol(values.at(0)) : 0;
}

string LinuxParser::Uid(int pid) {
  string uidLine =
      FindLine(kProcDirectory + to_string(pid) + "/status", "Uid:");
  return GetValues(uidLine).at(0);
}

string LinuxParser::User(int pid) { return GetUserName(Uid(pid)); }

long LinuxParser::UpTime(int pid) {
  vector<string> stats = GetPidStats(pid);
  long seconds = UpTime() - (stol(stats.at(21)) / Jiffies());
  return seconds;
}

string LinuxParser::FindLine(const string& path, const string& startsWith) {
  string line;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.rfind(startsWith, 0) == 0) {
        return line;
      }
    }
  }
  return string();
}

string LinuxParser::GetLineNumber(const string& path, int lineNbr) {
  string line;
  std::ifstream stream(path);
  if (stream.is_open()) {
    for (int i = 1; std::getline(stream, line); i++) {
      if (i == lineNbr) {
        return line;
      }
    }
  }
  return string();
}

vector<string> LinuxParser::GetValues(const string& keyAndValues) {
  string word;
  std::istringstream linestream(keyAndValues);
  linestream >> word;
  vector<string> values;
  while (linestream >> word) values.push_back(word);
  return values;
}

vector<long> LinuxParser::GetMemoryInfo() {
  const vector<string> headers{
      "MemTotal:", "MemFree:", "MemAvailable:", "Buffers:"};
  vector<long> memInfo;
  string line;
  for (string header : headers) {
    line = FindLine(kProcDirectory + kMeminfoFilename, header);
    memInfo.push_back(stol(GetValues(line).at(0)));
  }
  return memInfo;
}

vector<long> LinuxParser::GetCpuTimes(const string& cpuLine) {
  std::istringstream linestream(cpuLine);
  string item;
  linestream >> item;
  vector<long> times;
  long time;
  while (linestream >> time) times.push_back(time);
  return times;
}

string LinuxParser::GetUserName(const string& uid) {
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    string line;
    while (std::getline(stream, line)) {
      vector<string> split = Split(line);
      if (split.at(2) == uid) return split.at(0);
    }
  }
  return string();
}

vector<string> LinuxParser::Split(const string& line, const char delim) {
  std::istringstream linestream(line);
  string token;
  vector<string> tokenized;
  while (getline(linestream, token, delim)) tokenized.push_back(token);
  return tokenized;
}

vector<string> LinuxParser::GetPidStats(int pid) {
  vector<string> stats =
      Split(GetLineNumber(kProcDirectory + to_string(pid) + "/stat"), ' ');
  if (stats.size() != 52) stats = vector<string>(52, "0");
  return stats;
}
