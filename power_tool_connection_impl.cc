#include "power_tool_connection_impl.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "base/logging.h"

namespace {
int Socket(int family, int type, int protocol) {
  int n;
  if ((n = socket(family, type, protocol)) < 0) {
    PLOG(ERROR) << "Socket creation error";
  }
  return n;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int n;
  if ((n = connect(sockfd, addr, addrlen)) < 0) {
    PLOG(ERROR) << "Connect error";
  }
  return n;
}

int ConnectTo(const std::string& ip_addr, uint32_t port) {
  VLOG(1) << "Connect to " << ip_addr << ":" << port;
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port); // must use htons as sin_port is short
  if (inet_pton(AF_INET, ip_addr.c_str(), &server_addr.sin_addr) <= 0) {
    PLOG(ERROR) << "inet_pton failed";
    return -1;
  }
  int server_socket = Socket(AF_INET, SOCK_STREAM, 0);
  Connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
  return server_socket;
}

ssize_t Send(int sockfd, const void *buf, size_t len, int flags) {
  ssize_t sent_bytes = send(sockfd, buf, len, flags);
  if (sent_bytes == -1) {
    PLOG(ERROR) << "send, socket gone";
  }
  return sent_bytes;
}

// Read until "\r\n\r\n" is encountered
// Return number of bytes read
ssize_t ReadLine(int fd, std::string *output) {
    ssize_t numRead = 0;                    /* # of bytes fetched by last read() */
    size_t totRead = 0;                     /* Total bytes read so far */
    char ch;
    int state = 0;

    for (;;) {
      numRead = read(fd, &ch, 1);

      if (numRead == -1) {
        if (errno == EINTR)         /* Interrupted --> restart read() */
          continue;
        else
          return -1;              /* Some other error */

      } else if (numRead == 0) {      /* EOF */
        if (totRead == 0)           /* No bytes read; return 0 */
          return 0;
        else                        /* Some bytes read; add '\0' */
          break;

      } else {                        /* 'numRead' must be 1 if we get here */
        *output += ch;

        if ((state == 0 || state == 2) && ch == '\r') {
          ++state;
        } else if ((state == 1 || state == 3) && ch == '\n') {
          ++state;
        } else {
          state = 0;
        }

        ++totRead;

        if (state == 4)
          break;
      }
  }

    return totRead;
}

}  // namespace


namespace browser_profiler {

PowerToolConnectionImpl::PowerToolConnectionImpl(const std::string& server_ip, uint32_t server_port)
  : PowerToolConnection(server_ip, server_port),
    socket_to_server_(0) {
}

PowerToolConnectionImpl::PowerToolConnectionImpl(
    const std::pair<std::string, uint32_t> server_ip_port)
  : PowerToolConnectionImpl(server_ip_port.first, server_ip_port.second) {
}

PowerToolConnectionImpl::~PowerToolConnectionImpl() {
  close(socket_to_server_);
  socket_to_server_ = 0;
}
  
bool PowerToolConnectionImpl::Connect() {
  int socket_to_server = ConnectTo(server_ip_, server_port_);
  if (socket_to_server < 0) {
    LOG(ERROR) << "Failed to connection server at " 
      << server_ip_ << ":" << server_port_;
    return false;
  }

  socket_to_server_ = socket_to_server;
  return true;
}

bool PowerToolConnectionImpl::SyncReceiveMessage(std::string* message) {
  if (socket_to_server_ == 0) {
    LOG(ERROR) << "Socket is not connected";
    return false;
  }

  if (ReadLine(socket_to_server_, message) == -1) {
    LOG(ERROR) << "Cannot read response";
    return false;
  }

  return true;
}

bool PowerToolConnectionImpl::SyncSendMessage(const std::string& message) {
  if (socket_to_server_ == 0) {
    LOG(ERROR) << "Socket is not connected";
    return false;
  }

  if (!Send(socket_to_server_, message.c_str(), message.length(), 0)) {
    LOG(ERROR) << "Fail to send message";
    return false;
  }
  return true;
}

}  // namespace browser_profiler
