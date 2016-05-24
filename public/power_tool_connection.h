// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)

#ifndef BROWSER_PROFILER_PUBLIC_POWER_TOOL_CONNECTION_H_
#define BROWSER_PROFILER_PUBLIC_POWER_TOOL_CONNECTION_H_

#include <stdint.h>
#include <string>
#include <utility>

namespace browser_profiler {

// Provide a connection to power tool server
// Subclasses can use any protocol to provides the following services
class PowerToolConnection {
 public:
  PowerToolConnection(const std::string& server_ip, uint32_t server_port);

  // required by scoped_ptr
  virtual ~PowerToolConnection();

  // Connect to server
  virtual bool Connect() = 0;

  // Receive a message synchronously
  virtual bool SyncReceiveMessage(std::string* message) = 0;

  // Send a message synchronously
  virtual bool SyncSendMessage(const std::string& message) = 0;

 protected:
  std::string server_ip_;

  uint32_t server_port_;
};

}  // namespace browser_profiler

#endif // BROWSER_PROFILER_PUBLIC_POWER_TOOL_CONNECTION_H_
