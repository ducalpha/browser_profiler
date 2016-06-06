// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)
#include "power_tool_controller.h"

#include <string>
#include <utility>
#include <vector>

#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/string_split.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
  
#include "power_tool_connection_impl.h"

namespace {

const char kCommandKey[] = "Command";
const char kStartSamplingCommand[] = "start_sampling";
const char kStopSamplingCommand[] = "stop_sampling";
const char kFinishAllExperimentsCommand[] =  "finish_all_experiments";

const char kResultKeysKey[] = "ResultKeys";
const char kResultValuesKey[] = "ResultValues";

const char kStatusKey[] = "Status";
const char kOkValue[] = "ok";

const char kKeyValueSeparator[] = ":";
const char kNewLine[] = "\r\n";
const char kBlankLine[] = "\r\n\r\n";

std::string ComposeKeyValue(const std::string& key, const std::string& value) {
  std::string result;
  result.append(key).append(" ").append(kKeyValueSeparator).append(" ").append(value);
  return result;
}

// Configuration file format, example:
// # Skip lines beginning with '#'
// server_ip : 10.172.96.40
// server_port : 3000
const char kServerIp[] = "server_ip";
const char kServerPort[] = "server_port";

bool ReadServerIpAndPort(const base::FilePath& server_config_filepath,
    std::pair<std::string, uint32_t> *server_ip_port) {
  std::string server_config;
  if (!base::ReadFileToString(server_config_filepath, &server_config)) {
    CHROMIUM_LOG(ERROR) << "Cannot read server config file at " << server_config_filepath.value();
    return false;
  }

  std::vector<std::string> lines =
      base::SplitString(server_config, "\n", base::WhitespaceHandling::TRIM_WHITESPACE,
                        base::SplitResult::SPLIT_WANT_NONEMPTY);
  for (size_t i = 0; i < lines.size(); ++i) {
    std::string line = lines[i];
    base::TrimWhitespaceASCII(line, base::TRIM_ALL, &line);
    if (line.empty() || StartsWith(line, "#", base::CompareCase::SENSITIVE))
      continue;
    std::vector<std::string> components =
        base::SplitString(line, ":", base::WhitespaceHandling::TRIM_WHITESPACE,
                          base::SplitResult::SPLIT_WANT_NONEMPTY);
    if (components.size() != 2) {
      CHROMIUM_LOG(ERROR) << "Error parsing line: " << line;
      continue;
    }
    for (size_t j = 0; j < components.size(); ++j) {
      base::TrimWhitespaceASCII(components[i], base::TRIM_ALL, &components[i]);
    }

    std::string key = components[0]; 
    std::string value = components[1]; 
    if (key.compare(kServerIp) == 0) {
      server_ip_port->first = value;
    } else if (key.compare(kServerPort) == 0) {
      if (!base::StringToUint(value, &server_ip_port->second)) {
        CHROMIUM_LOG(ERROR) << "Error parsing server port: " << value;
        return false;
      }
    } else {
      NOTREACHED();
    }
  }
  return true;
}

}  // namespace

namespace browser_profiler {

PowerToolController::Message::Message() {
}

PowerToolController::Message::Message(const std::string& key, const std::string& value) {
  Add(key, value);
}

void PowerToolController::Message::Add(
    const std::string& key, const std::string& value) {
  key_values_.push_back(std::pair<std::string, std::string>(key, value));
}

std::string PowerToolController::Message::ToString() const {
  std::string message;

  for (size_t i = 0; i < key_values_.size(); ++i) {
    message.append(
        ComposeKeyValue(key_values_[i].first, key_values_[i].second).append(kNewLine));
  }
  message.append(kNewLine);

  return message;
}

PowerToolController::PowerToolController(const base::FilePath& server_config_filepath) {
  std::pair<std::string, uint32_t> server_ip_port;
  if (!ReadServerIpAndPort(server_config_filepath, &server_ip_port)) {
    CHROMIUM_LOG(FATAL) << "Cannot read server config file at " << server_config_filepath.value();
  }

  VLOG(1) << "Server is at " << server_ip_port.first << ":" << server_ip_port.second;

  power_tool_connection_.reset(new PowerToolConnectionImpl(server_ip_port));
}


void PowerToolController::Connect() {
  if (!power_tool_connection_->Connect()) {
    CHROMIUM_LOG(FATAL) << "Fail to connection to server";
  }
}

bool PowerToolController::StartSampling() {
  Message message(kCommandKey, kStartSamplingCommand);

  if (!SyncSendAndCheckResponse(message.ToString())) {
    CHROMIUM_LOG(ERROR) << "Failed to send " << kCommandKey;
    return false;
  }

  // Wait some time for power tool to start
  // Fix warning: approximate navigationStart to 0
  base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(550));
  return true;
}


bool PowerToolController::SyncSendAndCheckResponse(const std::string& message) {
  power_tool_connection_->SyncSendMessage(message);

  std::string response;
  power_tool_connection_->SyncReceiveMessage(&response);

  std::string::size_type blank_line = response.find(kBlankLine);
  response = response.substr(0, blank_line);

  std::vector<std::string> key_values =
      base::SplitString(response, ":", base::WhitespaceHandling::TRIM_WHITESPACE,
                        base::SplitResult::SPLIT_WANT_NONEMPTY);

  if (!(key_values.size() == 2 && key_values[0].compare(kStatusKey) == 0 &&
        key_values[1].compare(kOkValue) == 0)) {
    CHROMIUM_LOG(ERROR) << "Response message is not OK but: " << response;
    // return false;
  }

  return true;
}

bool PowerToolController::StopSampling(const std::string& result_keys, const std::string& result_values) {
  Message stop_sampling_message;
  stop_sampling_message.Add(kCommandKey, kStopSamplingCommand);
  stop_sampling_message.Add(kResultKeysKey, result_keys);
  stop_sampling_message.Add(kResultValuesKey, result_values);

  if (!SyncSendAndCheckResponse(stop_sampling_message.ToString())) {
    CHROMIUM_LOG(ERROR) << "Failed to send stop sampling command";
    return false;
  }

  return true;
}

bool PowerToolController::FinishAllExp() {
  Message message(kCommandKey, kFinishAllExperimentsCommand);

  if (!SyncSendAndCheckResponse(message.ToString())) {
    CHROMIUM_LOG(ERROR) << "Failed to send " << kFinishAllExperimentsCommand;
    return false;
  }

  return true;
}

} // namespace browser_profiler 
