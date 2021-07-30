/*copyright stuff*/

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_H_

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/synchronization/notification.h"

#include "libevent/include/event2/buffer.h"
#include "libevent/include/event2/bufferevent.h"
#include "libevent/include/event2/event.h"
#include "libevent/include/event2/http.h"
#include "libevent/include/event2/keyvalq_struct.h"
#include "libevent/include/event2/util.h"

#include "tensorflow_serving/util/net_http/client/public/httpclient_interface.h"
#include "tensorflow_serving/util/net_http/client/internal/client_evhttp_request.h"

namespace tensorflow {
namespace serving {
namespace net_http {

struct evbuffer;
struct evhttp_request;
struct evhttp_uri;
struct evkeyvalq;
enum evhttp_cmd_type;

class EvHTTPConnection : public HTTPClientInterface {
  public:
    ~HTTPClientInterface() = default;//change?

    EvHTTPConnection(const EvHTTPConnection& other) = delete;
    EvHTTPConnection& operator=(const EvHTTPConnection& other) = delete;

    EvHTTPConnection() = default;

    bool Connect(absl::string_view host, int port) override;

    bool Connect(absl::string_view uri) override;
  
    bool is_connected() const override;
  
    bool SendRequest(ClientEvHTTPRequest* request, bool asynchronous) override;

    // Starts the connection termination, and returns immediately.
    void Terminate() override;

    // Returns true if Terminate() has been called.
    bool is_terminating() const override;

    // Blocks the calling thread until the client is terminated and safe
    // to destroy.
    void WaitForTermination() override;

    // Blocks the calling thread until the server is terminated and safe
    // to destroy, or until the specified timeout elapses.  Returns true
    // if safe termination completed within the timeout, and false otherwise.
    bool WaitForTerminationWithTimeout(absl::Duration timeout) override;

    // Sets the executor for processing requests asynchronously.
    void SetExecutor(std::unique_ptr<EventExecutor> executor) override;

  private:

    struct event_base* ev_base_;
    struct evhttp_connection* evcon_;

    std::unique_ptr<EventExecutor> executor_;

    std::unique_ptr<absl::Notification> loop_exit_;

};

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_H_