/*
* Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "bus/cantomqttapp.h"

#include <util/logstream.h>
#include <util/stringutil.h>

#include <filesystem>
#include <string>
using namespace std::filesystem;
using namespace util;
using namespace util::log;
using namespace util::string;
namespace bus {

bool CanToMqttApp::OnInit() {
  try {

    if (const bool init = ConsoleApp::OnInit(); !init) {
      throw std::runtime_error("Can't initialize console application");
    }
    std::string config_file;
    for (const std::string& args : Args()) {
      config_file = args;
    }
    if (config_file.empty()) {
      throw std::runtime_error("No config file as input argument");
    }
    service_.ConfigFile(config_file);
    if (const bool read = service_.ReadConfigFile(); !read ) {
      throw std::runtime_error("Can't parse the config file");
    }
    if (const bool start = service_.Start(); ! start) {
      throw std::runtime_error("Can't start the service");
    }
  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to start the CAN to MQTT service. Error: "
      << err.what();
    return false;
  }
  return true;
}

void CanToMqttApp::OnRun() {
  ConsoleApp::OnRun();
}

void CanToMqttApp::OnExit() {
  service_.Stop();
  ConsoleApp::OnExit();
}

}  // namespace bus