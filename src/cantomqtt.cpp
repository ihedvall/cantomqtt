/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "bus/cantomqtt.h"

#include <dbc/dbcfile.h>

#include <metric/metricproperty.h>
#include <metric/metrictype.h>
#include <metric/metriclogstream.h>

#include <util/ixmlfile.h>
#include <util/logstream.h>

#include <chrono>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <sstream>

#include "bus/candataframe.h"
#include "bus/buslogstream.h"

using namespace std::filesystem;
using namespace util::log;
using namespace util::xml;
using namespace dbc;
using namespace metric;
using namespace std::chrono_literals;

namespace {
  void SetMetricDataType(const Signal& signal, Metric& metric) {

    // Many signals is enumerated values.
    // The signal value is an integer that responds to a string
    // scaled value.
    const auto& enum_list = signal.EnumList();

    const MetricProperty bits_prop("bits",
      std::to_string(signal.BitLength()) );
    metric.AddProperty(bits_prop);

    if (!enum_list.empty()) {
      std::ostringstream enum_str;
      for (const auto& [key, text] : enum_list) {
        if (!enum_str.str().empty()) {
          enum_str << ";";
        }
        std::string enum_text = text;
        std::ranges::replace(enum_text, ';', ' ');
        enum_str << key << ":" << text;
      }
      MetricProperty enum_prop("enumerate", enum_str.str());
      metric.AddProperty(enum_prop);
      metric.DataType(MetricType::String);
      return;
    }

    // Test if this is a value has data with more than 8 bytes.
    // It is either a string or a byte array.
    // Default to string value as the DBC file doesn't define
    // the data type.
    if (signal.IsArrayValue()) {
      metric.DataType(MetricType::String);
      return;
    }

    if (const bool no_scale = signal.Scale() == 1.0 && signal.Offset() == 0;
        no_scale) {
      switch (signal.DataType()) {
        case SignalDataType::SignedData:
          if (signal.BitLength() <= 8) {
            metric.DataType(MetricType::Int8);
          } else if (signal.BitLength() <= 16) {
            metric.DataType(MetricType::Int16);
          } else if (signal.BitLength() <= 32) {
            metric.DataType(MetricType::Int32);
          } else {
            metric.DataType(MetricType::Int64);
          }
          break;

        case SignalDataType::UnsignedData:
          if (signal.BitLength() <= 1) {
            metric.DataType(MetricType::Boolean);
          } else if (signal.BitLength() <= 8) {
            metric.DataType(MetricType::UInt8);
          } else if (signal.BitLength() <= 16) {
            metric.DataType(MetricType::UInt16);
          } else if (signal.BitLength() <= 32) {
            metric.DataType(MetricType::UInt32);
          } else {
            metric.DataType(MetricType::UInt64);
          }
          break;

        case SignalDataType::FloatData:
          metric.DataType(MetricType::Float);
          break;

        case SignalDataType::DoubleData:
          metric.DataType(MetricType::Double);
          break;

        default:
          metric.DataType(MetricType::Unknown);
          break;
      }
    } else {
      metric.DataType(MetricType::Double);
    }
  }

void LogMetricToUtil(std::source_location location,
               MetricLogSeverity severity,
               const std::string& message) {
    LogStream(location, static_cast<LogSeverity>(severity)) << message;
}

void LogBusToUtil(std::source_location location,
               bus::BusLogSeverity severity,
               const std::string& message) {
    LogStream(location, static_cast<LogSeverity>(severity)) << message;
}

}

namespace bus {

CanToMqtt::CanToMqtt() {
  // Attach all logger interface to the util logger system
  MetricLogStream::SetLogFunction(LogMetricToUtil);
  BusLogStream::UserLogFunction = LogBusToUtil;
}
CanToMqtt::~CanToMqtt() {
  CanToMqtt::Stop();
  MetricLogStream::SetLogFunction(nullptr);
  BusLogStream::UserLogFunction = nullptr;
}

bool CanToMqtt::SaveConfigFile() const {
  // Verify the path
  try {
    if (config_file_.empty()) {
      throw std::runtime_error("No config file have been set.");
    }
    path filename(config_file_);
    if (!exists(filename)) {
      if (filename.has_parent_path()) {
        path parent_path = filename.parent_path();
        create_directories(parent_path);
      }
    }
    std::unique_ptr<IXmlFile> xml_file = CreateXmlFile("FileWriter");
    if (!xml_file) {
      throw std::runtime_error("Failed to create the XML file.");
    }
    auto& root_node = xml_file->RootName("CanToMqtt");
    xml_file->FileName(config_file_);
    SaveGeneral(root_node);
    SaveDbcFiles(root_node);
    SaveSelectedItems(root_node);
    const bool write = xml_file->WriteFile();
    if (!write) {
      throw std::runtime_error("Failed to write XML file.");
    }
  } catch (std::exception &err) {
    LOG_ERROR() << "Can't save config file. File: " << config_file_
      << ", Error: " << err.what();
    return false;
  }
  return true;
}

bool CanToMqtt::ReadConfigFile() {
  try {
    if (config_file_.empty()) {
      throw std::runtime_error("No config file have been set.");
    }
    if (!exists(config_file_)) {
      throw std::runtime_error("The config file doesn't exist.");
    }
    auto xml_file = CreateXmlFile();
    if (!xml_file) {
      throw std::runtime_error("Failed to create the XML file.");
    }
    xml_file->FileName(config_file_);
    const bool parse = xml_file->ParseFile();
    if (!parse) {
      throw std::runtime_error("Failed to parse the XML file.");
    }
    const auto* root_node = xml_file->RootNode();
    if (root_node == nullptr) {
      throw std::runtime_error("Failed to get the root node.");
    }
    ReadGeneral(*root_node);
    ReadDbcFiles(*root_node);
    ReadSelectedItems(*root_node);

  } catch (std::exception &err) {
    LOG_ERROR() << "Can't read config file. File: " << config_file_
      << ", Error: " << err.what();
    return false;
  }
  return true;
}

bool CanToMqtt::Start() {
  Stop();
  // Connect to the CAN bus. It's either TCP/IP or shared memory
  try {
    if (!shared_mem_name_.empty()) {
      bus_broker_ = BusInterfaceFactory::CreateBroker(
        BrokerType::SharedMemoryBrokerType);
    } else {
      bus_broker_ = BusInterfaceFactory::CreateBroker(
        BrokerType::TcpBrokerType);
    }
    if (!bus_broker_) {
      throw std::runtime_error("Failed to create the bus broker.");
    }

    bus_subscriber_ = bus_broker_->CreateSubscriber();
    if (!bus_broker_) {
      throw std::runtime_error("Failed to create the subscriber.");
    }
    bus_subscriber_->Start();

    // Enable the MQTT client
    std::ostringstream name;
    name << broker_host_ << ":" << broker_port_;
    mqtt_node_.Name(name.str());
    mqtt_node_.Description("Connection to the MQTT broker.");
    mqtt_node_.Transport(transport_layer_);
    mqtt_node_.Host(broker_host_);
    mqtt_node_.Port(broker_port_);
    mqtt_node_.ClientId(broker_client_id_);
    mqtt_node_.UserName(broker_user_);
    mqtt_node_.Password(broker_password_);
    mqtt_node_.Version(ProtocolVersion::Mqtt5);
    mqtt_node_.InService();

    if (const bool broker_init = mqtt_node_.Init(); !broker_init ) {
      throw std::runtime_error("Failed to initialize the MQTT broker.");
    }

    stop_thread_ = false;
    work_thread_ = std::thread(&CanToMqtt::WorkingThread, this);

  } catch (const std::exception &err) {
    LOG_ERROR() << "Can't start the service. Error: " << err.what();
    return false;
  }
  return true;
}

void CanToMqtt::Stop() {
  stop_thread_ = true;
  LOG_TRACE() << "Trying to stop the working thread.";
  if (work_thread_.joinable()) {
    work_thread_.join();
  }
  LOG_TRACE() << "Stopped the working thread.";

  mqtt_node_.OutOfService();
  if (const bool exit = mqtt_node_.Exit(); !exit ) {
    LOG_TRACE() << "Failed to stop the MQTT broker.";
  }

  if (bus_subscriber_) {
    bus_subscriber_->Stop();
    bus_subscriber_.reset();
  }
  if (bus_broker_) {
    bus_broker_.reset();
  }

}

void CanToMqtt::SaveGeneral(IXmlNode& root_node) const {
  if (!shared_mem_name_.empty()) {
    root_node.SetProperty("SharedMem", shared_mem_name_);
  }
  if (!bus_host_.empty()) {
    root_node.SetProperty("BusHost", bus_host_);
  }
  if (bus_port_ > 0) {
    root_node.SetProperty("BusPort", bus_port_);
  }
  root_node.SetProperty("BrokerHost", broker_host_);
  root_node.SetProperty("BrokerPort", broker_port_);

}

void CanToMqtt::ReadGeneral(const IXmlNode& root_node) {
  shared_mem_name_ =  root_node.Property<std::string>("SharedMem");
  bus_host_ = root_node.Property<std::string>("BusHost");
  bus_port_ =  root_node.Property<uint16_t>("BusPort");
  broker_host_  = root_node.Property<std::string>("BrokerHost",
    "127.0.0.1");
  broker_port_ = root_node.Property<uint16_t>("BrokerPort", 1883);
}

void CanToMqtt::SaveDbcFiles(IXmlNode& root_node) const {
  auto& node = root_node.AddNode("DbcFiles");
  for (const auto& dbc_file : dbc_files_) {
    if (dbc_file.Filename().empty()) {
      continue;
    }
    auto& dbc_node = node.AddNode("DbcFile");
    dbc_node.SetAttribute("name", dbc_file.Filename());
    dbc_node.SetProperty("FileName", dbc_file.Filename());
  }
}

void CanToMqtt::ReadDbcFiles(const IXmlNode& root_node) {
  dbc_files_.clear();
  const auto* node = root_node.GetNode("DbcFiles");
  if (node == nullptr) {
    return;
  }
  IXmlNode::ChildList dbc_nodes;
  node->GetChildList(dbc_nodes);

  for (const auto* dbc_node : dbc_nodes) {
    if (dbc_node == nullptr || !dbc_node->IsTagName("DbcFiles") ) {
      continue;
    }
    auto file_name = dbc_node->Attribute<std::string>("name");
    if (file_name.empty()) {
      file_name = dbc_node->Property<std::string>("FileName");
    }

    try {
      if (file_name.empty()) {
        throw std::runtime_error("File name is empty.");
      }
      if (!exists(file_name)) {
        throw std::runtime_error("File doesn't exist.");
      }
      DbcFile dbc_file;
      const bool parse = ParseDbcFile(dbc_file);
      if (!parse) {
        throw std::runtime_error("Failed to parse the DbcFile.");
      }
      dbc_files_.emplace_back(std::move(dbc_file));
    } catch (const std::exception &err) {
      LOG_ERROR() << "Can't parse the DBC file. File: " << file_name
        << ", Error: " << err.what();
    }

  }
}

void CanToMqtt::SaveSelectedItems(IXmlNode& root_node) const {
  auto& node = root_node.AddNode("SelectedItems");
  for (const auto& metric : metric_db_.Metrics()) {
    if (!metric || !metric->IsSelected()) {
      continue;
    }
    auto& metric_node = node.AddNode("Metric");
    metric_node.SetAttribute("name", metric->Name());
    metric_node.SetAttribute("msg_id", metric->GroupIdentity());
    metric_node.SetAttribute("msg_name", metric->GroupName());
  }

}

void CanToMqtt::ReadSelectedItems(const IXmlNode& root_node) {
  // Clear any selected metrics in the database.
  for (auto& metric : metric_db_.Metrics()) {
    if (!metric) {
      continue;
    }
    metric->Selected(false);
  }

  const auto* node = root_node.GetNode("SelectedItems");
  if (node == nullptr) {
    return;
  }
  IXmlNode::ChildList metric_nodes;
  node->GetChildList(metric_nodes);

  for (const auto* metric_node : metric_nodes) {
    if (metric_node == nullptr || !metric_node->IsTagName("Metric") ) {
      continue;
    }
    const auto name = metric_node->Attribute<std::string>("name");
    const auto msg_id = metric_node->Attribute<int64_t>("msg_id");
    const auto msg_name = metric_node->Attribute<std::string>("msg_name");

    auto metric_group = metric_db_.CreateGroup(msg_name,
      msg_id);
    if (!metric_group) {
      LOG_ERROR() << "Can't create metric group. Group: " << msg_id << ":"
        << msg_name;
      continue;
    }

    auto metric = metric_db_.CreateMetric(*metric_group,
      name);
    if (!metric) {
      LOG_ERROR() << "The selected metric not found in the DB. Metric: "
        << name << " (" << msg_id << ":" << msg_name << ")";
      continue;
    }
    metric->Selected(true);
  }
}

bool CanToMqtt::ParseDbcFile(DbcFile& dbc_file) const {
  try {
    const bool parse = dbc_file.ParseFile();
    if (!parse) {
      std::ostringstream err;
      err << "Didn't parse the DBC file. Error: " << dbc_file.LastError();
      throw std::runtime_error(err.str());
    }

    const auto* network = dbc_file.GetNetwork();
    if (network == nullptr) {
      throw std::runtime_error("No network in the DBC file.");
    }
    for (const auto& [msg_id, msg] :
      network->Messages()) {

      auto group = metric_db_.GetGroupByIdentity(
        static_cast<int64_t>(msg_id));
      if (!group) {
        continue;
      }
      // If the CAN message is defined in multiple DBC file, use the first
      // occurannce
      if (group->Context() == nullptr) {
        group->Context(&dbc_file);
        group->Description(msg.Comment());
      }


      for (const auto& [signal_name, signal] : msg.Signals()) {
        auto metric = metric_db_.GetMetricByGroupIdentity(
          group->Identity(), signal_name);
        if (!metric) {
          continue;
        }
        if (metric->Context() == nullptr) {
          metric->Context(const_cast<Signal*>(&signal));
          metric->Description(signal.Comment());
          metric->Unit(signal.Unit());
          SetMetricDataType(signal, *metric);
          if (signal.Min() < signal.Max()) {
            MetricProperty min("min", std::to_string(signal.Min()));
            metric->AddProperty(min);
            MetricProperty max("max", std::to_string(signal.Max()));
            metric->AddProperty(max);
          }
        }
      }
    }
  } catch (const std::exception& err) {
    LOG_ERROR() << "DBC parsing error. DBC File: " << dbc_file.Filename()
      << ", Error: " << err.what();
    return false;
  }
  return true;
}

void CanToMqtt::WorkingThread() {
  while (!stop_thread_) {
    if (!bus_subscriber_) {
      LOG_ERROR() << "The bus subscriber is not craeted. Invalid use.";
      break;
    }
    std::shared_ptr<IBusMessage> msg = bus_subscriber_->PopWait(1s);
    if (!msg || msg->Type() != BusMessageType::CAN_DataFrame) {
      continue;
    }

    CanDataFrame can_msg(msg);
    if (const bool updated = UpdateMetrics(can_msg); updated ) {
      // Todo: Update MQTT with changes
    }

  }
}

bool CanToMqtt::UpdateMetrics(const CanDataFrame& can_msg) {
  const uint32_t msg_id = can_msg.MessageId();
  auto metric_group = metric_db_.GetGroupByIdentity(msg_id);
  if (!metric_group || metric_group->Context() == nullptr) {
    return false;
  }

  const DbcMessage dbc_msg(can_msg.Timestamp(), can_msg.CanId(),
    can_msg.DataBytes());
  auto* dbc_file = static_cast<DbcFile*>(metric_group->Context());
  dbc_file->ParseMessage(dbc_msg);

  // The DBC signals are updated. Update the metric value as well
  auto metric_list =
    metric_db_.MetricsByGroupIdentity(msg_id);
  bool updated = false;
  for (auto& metric : metric_list) {
    if (!metric || metric->Context() == nullptr) {
      continue;
    }

    const auto* dbc_signal = static_cast<Signal*>(metric->Context());
    switch (metric->DataType()) {
      case MetricType::Int8:
      case MetricType::Int16:
      case MetricType::Int32:
      case MetricType::Int64: {
        int64_t value;
        const bool valid = dbc_signal->EngValue(value);
        metric->Valid(valid);
        metric->Value(value);
        break;
      }

      case MetricType::UInt8:
      case MetricType::UInt16:
      case MetricType::UInt32:
      case MetricType::UInt64: {
        uint64_t value;
        const bool valid = dbc_signal->EngValue(value);
        metric->Valid(valid);
        metric->Value(value);
        break;
      }

      case MetricType::Float:
      case MetricType::Double: {
        double value;
        const bool valid = dbc_signal->EngValue(value);
        metric->Valid(valid);
        metric->Value(value);
        break;
      }

      case MetricType::Boolean: {
        bool value;
        const bool valid = dbc_signal->EngValue(value);
        metric->Valid(valid);
        metric->Value(value);
        break;
      }

      default: {
        std::string value;
        const bool valid = dbc_signal->EngValue(value);
        metric->Valid(valid);
        metric->Value(value);
        break;
      }
    }
    if (metric->IsUpdated()) {
      updated = true;
    }
  }
  return updated;
}

bool CanToMqtt::StartMqtt() {
  std::ostringstream name;
  name << broker_host_ << ":" << broker_port_;

  try {
    mqtt_node_.Name(name.str());
    mqtt_node_.Description("Connection to the MQTT broker.");
    mqtt_node_.Transport(transport_layer_);
    mqtt_node_.Host(broker_host_);
    mqtt_node_.Port(broker_port_);
    mqtt_node_.ClientId(broker_client_id_);
    mqtt_node_.UserName(broker_user_);
    mqtt_node_.Password(broker_password_);
    mqtt_node_.Version(ProtocolVersion::Mqtt5);
    mqtt_node_.InService();

    // Todo: Create a better or user defined topic name
    for (const auto& group :metric_db_.Groups()) {
      if (!group) {
        continue;
      }
      const auto metric_list =
        metric_db_.MetricsByGroupIdentity(group->Identity());
      if (metric_list.empty()) {
        continue;
      }
      std::ostringstream topic_name;
      topic_name << "CanMetrics/";
      if (group->Name().empty()) {
        topic_name << group->Identity();
      } else {
        topic_name << group->Name();
      }
      auto mqtt_topic = mqtt_node_.CreateTopic(
         topic_name.str());
      if (!mqtt_topic) {
        throw std::runtime_error("Failed to create the MQTT topic.");
      }
      mqtt_topic->Description("JSON coded CAN signal values.");
      mqtt_topic->ContentType("application/json");
      for (const auto& metric : metric_list) {
        if (!metric || metric->Name().empty()) {
          continue;
        }
        mqtt_topic->AddMetric(metric);
      }
    }
    if (const bool broker_init = mqtt_node_.Init(); !broker_init ) {
      throw std::runtime_error("Failed to initialize the MQTT broker.");
    }
  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to start the MQTT broker. Name: " << name.str()
      << ", Error: " << err.what();
    return false;
  }
  return true;
}

}  // namespace bus