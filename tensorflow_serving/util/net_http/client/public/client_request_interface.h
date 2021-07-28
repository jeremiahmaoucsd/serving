/*add copyright information?*/

// net_http::ClientRequestInterface defines a pure interface class for handling 
// an HTTP request/response on the client-side. It is designed...

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "tensorflow_serving/util/net_http/client/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/client/public/client_handler_interface.h"

namespace tensorflow {
namespace serving {
namespace net_http {

// To be used with memory blocks returned via std::unique_ptr<char[]>
struct BlockDeleter {
 public:
  BlockDeleter() : size_(0) {}  // nullptr
  explicit BlockDeleter(int64_t size) : size_(size) {}
  inline void operator()(char* ptr) const {
    // TODO: c++14 ::operator delete[](ptr, size_t)
    std::allocator<char>().deallocate(ptr, static_cast<std::size_t>(size_));
  }

 private:
  int64_t size_;
};

class ClientRequestInterface {
 public:
  virtual ~ClientRequestInterface() = default;

  ClientRequestInterface(const ClientRequestInterface& other) = delete;
  ClientRequestInterface& operator=(const ClientRequestInterface& other) = 
     delete;
 
  // The portion of the request URI after the host and port.
  // E.g. "/path/to/resource?param=value&param=value#fragment".
  // Doesn't unescape the contents; returns "/" at least.

  virtual void SetUriPath(absl::string_view path) = 0;
  virtual absl::string_view uri_path() const = 0;

  // HTTP request method.
  // Must be in Upper Case.

  virtual void SetHTTPMethod(absl::string_view method) = 0;
  virtual absl::string_view http_method() const = 0;

  virtual bool SetResponseHandler(const ClientResponseHandler* handler) = 0;
 
  // Appends the data block of the specified size to the request body.
  // This request object takes the ownership of the data block.
  virtual void WriteRequestBytes(const char* data, int64_t size) = 0;

  // Appends (by coping) the data of string_view to the end of
  // the response body.
  virtual void WriteRequestString(absl::string_view data) = 0;

  virtual void OverwriteRequestHeader(absl::string_view header,
                                       absl::string_view value) = 0;
  virtual void AppendRequestHeader(absl::string_view header,
                                    absl::string_view value) = 0;

  // Returns all the request header names.
  // This is not an efficient way to access headers, mainly for debugging uses.
  virtual std::vector<absl::string_view> request_headers() const = 0;
  
  virtual std::unique_ptr<char[], BlockDeleter> ReadResponseBytes(
      int64_t* size) = 0;

  virtual absl::string_view GetResponseHeader(
       absl::string_view header) const = 0;

  // Returns all the response header names.
  // This is not an efficient way to access headers, mainly for debugging uses.
  virtual std::vector<absl::string_view> response_headers() const = 0;

  // Aborts the current request forcibly.
  // Once Abort() is called, the request object will be owned and destructed
  // by the server runtime.
  virtual void Abort() = 0;

 protected:
  ServerRequestInterface() = default;

 private:
  // Do not add any data members to this class.
}

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_