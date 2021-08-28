/*add copyright information?*/

// The entry point to access different HTTP server implementations.

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_

#include <memory>

#include "absl/memory/memory.h"

#include "tensorflow_serving/util/net_http/client/public/evhttp_client.h"
#include "tensorflow_serving/util/net_http/client/public/httpclient_interface.h"

namespace tensorflow {
namespace serving {
namespace net_http {

// Creates a client implemented based on the libevents library.
// Returns nullptr if there is any error??????
//
// Must call WaitForTermination() or WaitForTerminationWithTimeout() before
// the client is to be destructed?????

inline std::unique_ptr<HTTPClientInterface> CreateEvHTTPClient() {
  auto client = absl::make_unique<EvHTTPClient>(std::move(options));

  return std::move(client);
}

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_