// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)
#ifndef BROWSER_PROFILER_POWER_TOOL_CONTROLLER_H_
#define BROWSER_PROFILER_POWER_TOOL_CONTROLLER_H_

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
#include <memory>
#else
#include "base/memory/scoped_ptr.h"
#endif

#include <string>

#include "public/power_tool_connection.h"
#include "base/files/file_path.h"

namespace browser_profiler {

// Use directly the socket interface
// Why?
//  Need control
//  Need portability: don't want to introduce dependency on external libraries
class PowerToolController {
 public:
  // init server ip and port from a file containing 1 line, server_ip:port
  // 10.172.96.40:3000
  PowerToolController(const base::FilePath& server_config_filepath);

  // Connect to the server
  void Connect();

  bool StartSampling();

  // Stop sampling with experiment result
  // exp_result_fields: tab-separated field names
  // exp_result_values: corresponding tab-separated values
  bool StopSampling(const std::string& exp_result_fields,
        const std::string& exp_result_values);

  bool FinishAllExp();

 private:
  // Represents a message to send
  // Contains pairs of key-value of strings
  class Message {
   public:
    Message();
    Message(const std::string& key, const std::string& value);

    // Add a pair of key-value to message
    void Add(const std::string& key, const std::string& value);

    // Return a message: newline-separated key-value
    // Ending by a blank line
    std::string ToString() const;
   private:
    std::vector<std::pair<std::string, std::string>> key_values_;
  };

  // Send a message which is appended a blank line
  bool SyncSendAndCheckResponse(const std::string& message);

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
  std::unique_ptr<PowerToolConnection> power_tool_connection_;
#else
  scoped_ptr<PowerToolConnection> power_tool_connection_;
#endif
};

}  // namespace browser_profiler

#endif // BROWSER_PROFILER_POWER_TOOL_CONTROLLER_H_
