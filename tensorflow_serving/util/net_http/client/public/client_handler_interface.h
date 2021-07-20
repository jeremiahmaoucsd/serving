/*add copyright information?*/

//response handler interface for net_http client

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_HANDLER_INTERFACE_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_HANDLER_INTERFACE_H_

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "tensorflow_serving/util/net_http/client/public/response_code_enum.h"

namespace tensorflow {
namespace serving {
namespace net_http {

class ClientHandlerInterface{
 public:
  virtual ~ClientHandlerInterface() = default;

  ClientHandlerInterface(const ClientHandlerInterface& other) = delete;
  ClientHandlerInterface& operator=(const ClientHandlerInterface& other) = delete;

  virtual void IfError() = 0;

  virtual void IfPartial() = 0;

  virtual void IfSuccess() = 0;
  


};

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow


#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_HANDLER_INTERFACE_H_