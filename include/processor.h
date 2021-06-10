#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

class Processor {
 public:
  float Utilization();

 private:
  float prevIdleTime_{0.0}, prevTotalTime_{0.0};
};

#endif