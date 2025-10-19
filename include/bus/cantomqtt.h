/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <util/ixmlnode.h>

#include <dbc/dbcfile.h>

#include <metric/metricdatabase.h>
#include <mqtt/mqttnode.h>

#include <bus/interface/businterfacefactory.h>
#include <bus/ibusmessagequeue.h>
#include <bus/candataframe.h>

namespace bus {


class CanToMqtt {
public:
  CanToMqtt();
  virtual ~CanToMqtt();

  void ConfigFile(std::string config_file) {
    config_file_ = std::move(config_file);
  }

  [[nodiscard]] const std::string& ConfigFile() const {
    return config_file_;
  }

  bool SaveConfigFile() const;
  bool ReadConfigFile();

  bool Start();
  void Stop();
private:
  std::string config_file_;
  std::vector<dbc::DbcFile> dbc_files_;


  std::string shared_mem_name_;
  std::string bus_host_;
  uint16_t bus_port_ = 0;

  std::string broker_host_ = "127.0.0.1";
  uint16_t broker_port_ = 1883;
  metric::TransportLayer transport_layer_ = metric::TransportLayer::MqttTcp;
  std::string broker_client_id_;
  std::string broker_user_;
  std::string broker_password_;

  metric::MetricDatabase metric_db_;

  std::unique_ptr<IBusMessageBroker> bus_broker_;
  std::shared_ptr<IBusMessageQueue> bus_subscriber_;

  mqtt::MqttNode mqtt_node_;
  std::thread work_thread_;
  std::atomic<bool> stop_thread_ = true;

  void SaveGeneral(util::xml::IXmlNode& root_node) const;
  void ReadGeneral(const util::xml::IXmlNode& root_node);
  void SaveDbcFiles(util::xml::IXmlNode& root_node) const;
  void ReadDbcFiles(const util::xml::IXmlNode& root_node);
  void SaveSelectedItems(util::xml::IXmlNode& root_node) const;
  void ReadSelectedItems(const util::xml::IXmlNode& root_node);
  bool ParseDbcFile(dbc::DbcFile& dbc_file) const;
  void WorkingThread();
  bool UpdateMetrics(const CanDataFrame& can_msg);
  bool StartMqtt();
};

}  // namespace bus


