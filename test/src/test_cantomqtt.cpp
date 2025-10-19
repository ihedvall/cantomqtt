/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include <gtest/gtest.h>

#include <util/logconfig.h>
#include <util/logstream.h>
#include "bus/cantomqtt.h"
#include "bus/buslogstream.h"
#include <metric/metriclogstream.h>

using namespace util::log;
using namespace metric;

namespace bus::test {

TEST(TestCanToMqtt, TestLogger) {
  auto& log_config = LogConfig::Instance();
  log_config.Type(LogType::LogToConsole);

  log_config.CreateDefaultLogger();

  CanToMqtt server;

  LOG_TRACE() << "Util system message";
  BUS_TRACE() << "Bus system message";
  METRIC_TRACE() << "Metric system message";

  log_config.DeleteLogChain();
}

}