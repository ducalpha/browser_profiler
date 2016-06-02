// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "public/power_tool_connection.h"

namespace browser_profiler {

PowerToolConnection::PowerToolConnection(const std::string& server_ip, uint32_t server_port)
  : server_ip_(server_ip),
    server_port_(server_port) {
}

PowerToolConnection::~PowerToolConnection() {
}

} // namespace browser_profiler 
