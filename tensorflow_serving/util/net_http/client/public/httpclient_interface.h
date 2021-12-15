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

//Everything in this API is experimental and subject to change.

class ClientOptions{ //to mirror ServerOptions, though other than the event executor I am not sure what to add here.
 public: 
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

  void SetExecutor(std::unique_ptr<ClientOptions::EventExecutor> executor) {
    executor_ = std::move(executor);
  }
  ClientOptions::EventExecutor* executor() const { return executor_.get(); }

 private:
  std::unique_ptr<ClientOptions::EventExecutor> executor_;
};

class HTTPClientInterface {
 public:
  virtual ~HTTPClientInterface() = default;

  HTTPClientInterface(const HTTPClientInterface& other) = delete;
  HTTPClientInterface& operator=(const HTTPClientInterface& other) = delete;

  virtual void Connect(absl::string_view host, int port) = 0;

  virtual std::unique_ptr<ClientRequestInterface> MakeRequest(absl::string_view uri, absl::string_view method) = 0;
  
  virtual bool SendRequest(ClientRequestInterface* request) = 0; //There is currently a Send() method in the request class
                                                                //so maybe repetitive?

  // Closes connection
  virtual void CloseConnection() = 0;

  // Sets the executor for processing requests asynchronously. Brought over from serverreq
  virtual void SetExecutor(std::unique_ptr<EventExecutor> executor) = 0;

 protected:
  HTTPClientInterface() = default;

};


#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_
