/*copyright stuff*/
#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_

#include <cstdint>
#include <memory>
#include <string>

#include "tensorflow_serving/util/net_http/client/public/client_handler_interface.h"
#include "tensorflow_serving/util/net_http/client/public/client_request_interface.h"

struct evbuffer;
struct evhttp_request;
struct evhttp_uri;
struct evkeyvalq;
enum evhttp_cmd_type;

namespace tensorflow {
namespace serving {
namespace net_http {

// Copied from server/internal/evhttp_request.h and modified to fit response
//Headers only
struct ParsedEvResponse {
  public:
    // Doesn't take the ownership
    ParsedEvResponse(evhttp_request* request_out);
    ~ParsedEvResponse();

    evhttp_request* request;  // raw request (also the target of request related method)
    absl::string_view method; //request method
    absl::string_view path; //request uri

    // Decode and cache the result; or return false if any parsing error
    bool decode_response();

    absl::string_view status = HTTPStatusCode::UNDEFINED;

    evkeyvalq* headers = nullptr;
};

class ClientEvHTTPRequest : public ClientRequestInterface {
  public:
    ~ClientEvHTTPRequest() = default; //needs to be changed for sure

    ClientEvHTTPRequest(const ClientEvHTTPRequest& other) = delete;
    ClientEvHTTPRequest& operator=(const ClientEvHTTPRequest& other) = 
      delete;

    explicit ClientEvHTTPRequest(std::unique_ptr<ParsedEvResponse> parsed_response);

    //added but not sure about

    bool SetResponseHandler(const ClientResponseHandler* handler) override;
 
    void SetUriPath(absl::string_view path) override;
    absl::string_view uri_path() const override;

    void SetHTTPMethod(absl::string_view method) override;
    absl::string_view http_method() const override;

    void WriteRequestBytes(const char* data, int64_t size) override;

    void WriteRequestString(absl::string_view data) override;

    void OverwriteRequestHeader(absl::string_view header,
                                absl::string_view value) override;
    void AppendRequestHeader(absl::string_view header,
                             absl::string_view value) override;


    std::vector<absl::string_view> request_headers() const override;
  
    std::unique_ptr<char[], BlockDeleter> ReadResponseBytes(
      int64_t* size) override;

    absl::string_view GetResponseHeader(absl::string_view header) const override;

    std::vector<absl::string_view> response_headers() const override;

    void Abort() override;

    // Initializes the resource and returns false if any error.
    bool Initialize();

  private:

    const ClientResponseHandler* response_handler_;

    std::unique_ptr<ParsedEvResponse> parsed_response_;

    evbuffer* output_buf;  // owned by this
}

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_