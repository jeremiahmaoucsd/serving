/*add copyright information?*/

// APIs for the HTTP client.

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_

#include <cassert>

#include <functional>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/time/time.h"

#include "tensorflow_serving/util/net_http/client/public/client_request_interface.h"

//TODO: move EventExecutor to net_http/common
class EventExecutor {
 public:
  virtual ~EventExecutor() = default;

  EventExecutor(const EventExecutor& other) = delete;
  EventExecutor& operator=(const EventExecutor& other) = delete;

  // Schedule the specified 'fn' for execution in this executor.
  // Must be non-blocking
  virtual void Schedule(std::function<void()> fn) = 0;

 protected:
  EventExecutor() = default;
};

class HTTPClientInterface {
 public:
  virtual ~HTTPClientInterface() = default;

  HTTPClientInterface(const HTTPClientInterface& other) = delete;
  HTTPClientInterface& operator=(const HTTPClientInterface& other) = delete;

  virtual bool Connect(absl::string_view host, int port) = 0;

  virtual bool Connect(absl::string_view uri) = 0;
  
  virtual bool is_connected() const = 0;
  
  virtual bool SendRequest(ClientRequestInterface* request, bool asynchronous) = 0;

  // Starts the connection termination, and returns immediately.
  virtual void Terminate() = 0;

  // Returns true if Terminate() has been called.
  virtual bool is_terminating() const = 0;

  // Blocks the calling thread until the client is terminated and safe
  // to destroy.
  virtual void WaitForTermination() = 0;

  // Blocks the calling thread until the server is terminated and safe
  // to destroy, or until the specified timeout elapses.  Returns true
  // if safe termination completed within the timeout, and false otherwise.
  virtual bool WaitForTerminationWithTimeout(absl::Duration timeout) = 0;

  // Sets the executor for processing requests asynchronously.
  virtual void SetExecutor(std::unique_ptr<EventExecutor> executor) = 0;

 protected:
  HTTPClientInterface() = default;

};


#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_