#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const {

  float uptimeprocess = LinuxParser::UpTime() - LinuxParser::UpTime(pid_);
  float totalTime = LinuxParser::ActiveJiffies(pid_)/sysconf(_SC_CLK_TCK);
  
  return totalTime / uptimeprocess;
}

string Process::Command() { return LinuxParser::Command(pid_); }

string Process::Ram() { return LinuxParser::Ram(pid_); }

string Process::User() { return LinuxParser::User(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() < a.CpuUtilization();
}
