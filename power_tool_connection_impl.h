// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#ifndef BROWSER_PROFILER_POWER_TOOL_CONNECTION_IMPL_H_
#define BROWSER_PROFILER_POWER_TOOL_CONNECTION_IMPL_H_

#include "public/power_tool_connection.h"

#include "base/compiler_specific.h"

namespace browser_profiler {

// Use simple protocol
// Send and receive messages per line because no value contains the new line character
// Send lines containing tab-separated values because no value has the tab character
//  Server split the line and interpret 
// This way has the following advantage:
//  Synchronously sending/receiving message
//  Have no network library (REST client, curl) dependency
//  Have no format library (json) dependency
//  Any error will abort the experiment (and will restart the experiment)
//  Result header and log line is done on client
// Disadvantage:
//  Custom protocol is less flexible as using general-purpose protocols (HTTP), and format (Json)
// client, server
// Command: start_sampling, OK
// Command: stop_sampling, OK
// Header: ..., OK
// Result: ..., OK
//
// KEY <space> : <space> VALUE
class PowerToolConnectionImpl : public PowerToolConnection {
 public:
  PowerToolConnectionImpl(const std::string& server_ip, uint32_t server_port);

  PowerToolConnectionImpl(const std::pair<std::string, uint32_t> server_ip_port);

  ~PowerToolConnectionImpl();

  virtual bool Connect() OVERRIDE;

  virtual bool SyncReceiveMessage(std::string* message) OVERRIDE;

  virtual bool SyncSendMessage(const std::string& message) OVERRIDE;
 
 private:
  int socket_to_server_;
};

} // namespace browser_profiler 
#endif // BROWSER_PROFILER_POWER_TOOL_CONNECTION_IMPL_H_
