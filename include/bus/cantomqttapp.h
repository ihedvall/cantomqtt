/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include <util/consoleapp.h>
#include <bus/cantomqtt.h>
namespace bus {

class CanToMqttApp : public util::ConsoleApp {
public:

  virtual ~CanToMqttApp() = default;
protected:
  bool OnInit() override;
  void OnRun() override;
  void OnExit() override;
private:
  CanToMqtt service_;
};

}  // namespace bus


