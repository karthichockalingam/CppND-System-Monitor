
#include "processor.h"
#include <fstream>
#include <iostream>
#include "linux_parser.h"

float Processor::Utilization() {
 
  return (float) (LinuxParser::Jiffies() - LinuxParser::IdleJiffies())/(LinuxParser::Jiffies());
}
