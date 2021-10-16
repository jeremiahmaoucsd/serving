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
#include "tensorflow_serving/util/net_http/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/public/block_deleter.h"

enum class ResponseStatus{
  COMPLETE = 0,
  FAILED = 1,
};

class ClientHandlerInterface{
 public:
  virtual ~ClientHandlerInterface() = default;

  ClientHandlerInterface(const ClientHandlerInterface& other) = delete;
  ClientHandlerInterface& operator=(const ClientHandlerInterface& other) = delete;

  virtual void OnResponse(ResponseStatus status) = 0;

  virtual void OnError() = 0;
};

namespace tensorflow {
namespace serving {
namespace net_http {

class ClientRequestInterface {
 public:
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

  virtual ~ClientRequestInterface() = default;

  ClientRequestInterface(const ClientRequestInterface& other) = delete;
  ClientRequestInterface& operator=(const ClientRequestInterface& other) = 
     delete;
 
  // The portion of the request URI after the host and port.
  // E.g. "/path/to/resource?param=value&param=value#fragment".
  // Doesn't unescape the contents; returns "/" at least.

  virtual void SetUriPath(absl::string_view path) = 0; //new setter for uri, does not exist in serverreq
  virtual absl::string_view uri_path() const = 0; //brought over from serverreq

  // HTTP request method.
  // Must be in Upper Case.

  virtual void SetHTTPMethod(absl::string_view method) = 0;//new setter for uri
  virtual absl::string_view http_method() const = 0;//brought over from serverreq

  virtual void SetResponseHandler(const ClientHandlerInterface* handler) = 0;//new method needed to set response handler
 
  // Appends the data block of the specified size to the request body.
  // This request object takes the ownership of the data block.
  virtual void WriteRequestBytes(const char* data, int64_t size) = 0;//from serverreq

  // Appends (by coping) the data of string_view to the end of
  // the response body.
  virtual void WriteRequestString(absl::string_view data) = 0; //from serverreq
  
  virtual std::unique_ptr<char[], ClientRequestInterface::BlockDeleter> ReadResponseBytes( //from serverreq
      int64_t* size) = 0;

  virtual absl::string_view GetResponseHeader( //from serverreq
       absl::string_view header) const = 0;

  // Returns all the response header names.
  // This is not an efficient way to access headers, mainly for debugging uses.
  virtual std::vector<absl::string_view> response_headers() const = 0; //from server req

  virtual void OverwriteRequestHeader(absl::string_view header,
                                       absl::string_view value) = 0;//from serverreq
  virtual void AppendRequestHeader(absl::string_view header,
                                    absl::string_view value) = 0;//from serverreq

  //Shuts down request processes, request can be safely deleted after calling
  virtual void Shutdown() = 0; //new method for request internal shutdown

  virtual void Send() = 0; //method sends the request

 protected:
  ServerRequestInterface() = default;

 private:
  // Do not add any data members to this class.
}

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_