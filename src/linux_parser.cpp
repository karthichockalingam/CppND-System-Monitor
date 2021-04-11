#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
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
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string info, meminfo;
  string line;
  float memTotal = 0;
  float memFree = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> info >> meminfo;
      if (info == "MemTotal:") memTotal = std::stoi(meminfo);
      if (info == "MemFree:") memFree = std::stoi(meminfo);
    }
  }
  stream.close();
  
  return (memTotal - memFree) / memTotal;
}

long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  stream.close();
 
  return std::stol(uptime);
}


vector<string> LinuxParser::CpuUtilization() {
  string word;
  string line;
  vector<string> vec;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    int count = 0;

    while (linestream >> word) {
      if (count > 0) vec.push_back(word);

      count = count + 1;
    }
  }
  stream.close();

  return vec;
}

long LinuxParser::ActiveJiffies() {
  long sum = 0.0;

  vector<string> vec = CpuUtilization();

  for (int i = 0; i < 3; i++) sum += std::stol(vec[i]);
  for (int i = 5; i < 8; i++) sum += std::stol(vec[i]);
  
  return sum;
}


long LinuxParser::IdleJiffies() {
  long sum = 0.0;

  vector<string> vec = CpuUtilization();

  for (int i = 3; i < 5; i++) sum += std::stol(vec[i]);

  return sum;
}


long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}


long LinuxParser::ActiveJiffies(int pid) {
  string temp;
  string line;
  long utime, stime, cutime, cstime;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    int i = 1;

    while (linestream >> temp) {
      if (i == 14) utime = std::stol(temp);
      if (i == 15) stime = std::stol(temp);
      if (i == 16) cutime = std::stol(temp);
      if (i == 17) cstime = std::stol(temp);

      i = i + 1;
    }
  }
  stream.close();
  return (utime + stime + cutime + cstime);
}


int LinuxParser::TotalProcesses() {
  string info, proc;
  string line;
  int i = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> info >> proc;
      if (info == "processes") i = std::stoi(proc);
    }
  }
  stream.close();
  return i;
}

int LinuxParser::RunningProcesses() {
  string info, proc;
  string line;
  int i = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> info >> proc;
      if (info == "procs_running") i = std::stoi(proc);
    }
  }
  stream.close();
  return i;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  stream.close();
  return line;
}

string LinuxParser::Ram(int pid) {
  string info, value;
  string line;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> info >> value;
      if (info == "VmSize:") {
	long val = stol(value);
	val /= 1000;
	return to_string(val); 
      }
    }
  }
  stream.close();
  return 0;
}

string LinuxParser::Uid(int pid) {
  string info, id;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> info >> id;
      if (info == "Uid:") return id;
    }
  }
  stream.close();
  return 0;
}

string LinuxParser::User(int /* pid */) {
  string line;
  string key_one, key_two, key_three;
  string value;
  std::ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> value >> key_one >> key_two >> key_three) {
        if (key_one == "x" && key_two == "1000" && key_three == "1000")
          return value;
      }
    }
  }
  filestream.close();
  return 0;
}

long LinuxParser::UpTime(int pid) {
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    int count = 0;

    while (linestream >> uptime) {
      if (count == 21)
        return std::stol(uptime) / sysconf(_SC_CLK_TCK);

      count = count + 1;
    }
  }
  stream.close();
  return 0;
}
