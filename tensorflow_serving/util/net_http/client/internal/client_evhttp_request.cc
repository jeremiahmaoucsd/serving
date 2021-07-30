/*copyright stuff*/

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_CC_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_CC_

#include "tensorflow_serving/util/net_http/client/internal/client_evhttp_request.h"

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

namespace tensorflow {
namespace serving {
namespace net_http {

ParsedEvReponse::~ParsedEvResponse() {
  if (request && evhttp_request_is_owned(request)) {
    evhttp_request_free(request);
  }
}

ParsedEvRequest::ParsedEvRequest(evhttp_request* request_out)
    : request(request_out) {}

//not sure how the bool works here
bool ParsedEvRequest::decode_response(){
  this->status =
      static_cast<HTTPStatusCode>(evhttp_request_get_response_code(req));

  struct evkeyvalq* headers = evhttp_request_get_input_headers(request);
 
  return true;
}

//ClientEvHTTPRequest::~ClientEvHTTPRequest()

ClientEvHTTPRequest::ClientEvHTTPRequest(std::unique_ptr<ParsedEvRequest> request)
    : parsed_response_(std::move(request),
      response_handler_(nullptr)) {}

ClientEvHTTPRequest::ClientEvHTTPRequest(const ClientResponseHandler* handler)
    : parsed_response_(std::move(request),
      response_handler_(nullptr)) {}
      
bool ClientEvHTTPRequest::SetResponseHandler(const ClientResponseHandler* handler){
  if(handler == nullptr){
    return false;
  }
  this->response_handler_ = handler;
  return true;
}

void ClientEvHTTPRequest::SetUriPath(absl::string_view path){
  parsed_response_->path = path;
}

void ClientEvHTTPRequest::SetHTTPMethod(absl::string_view method){
  parsed_response_->method = method;
}

bool EvHTTPRequest::Initialize() {
  output_buf = evhttp_request_get_output_headers(parsed_response_->request);
  return output_buf != nullptr;
}

void ClientEvHTTPRequest::WriteRequestBytes(const char* data, int64_t size){
  assert(size >= 0);
  if (output_buf == nullptr) {
    NET_LOG(FATAL, "Request not initialized.");
    return;
  }

  int ret = evbuffer_add(output_buf, data, static_cast<size_t>(size));
  if (ret == -1) {
    NET_LOG(ERROR, "Failed to write %zu bytes data to output buffer",
            static_cast<size_t>(size));
  }
}

void ClientEvHTTPRequest::WriteRequestString(absl::string_view data){
 WriteRequestBytes(data.data(), static_cast<int64_t>(data.size()));
}

void ClientEvHTTPRequest::OverwriteRequestHeader(absl::string_view header,
                                                 absl::string_view value){
  evkeyvalq* ev_headers =
    evhttp_request_get_output_headers(parsed_response_->request);

  std::string header_str = std::string(header.data(), header.size());
  const char* header_cstr = header_str.c_str();

  evhttp_remove_header(ev_headers, header_cstr);
  evhttp_add_header(ev_headers, header_cstr,
                    std::string(value.data(), value.size()).c_str());
}

void ClientEvHTTPRequest::AppendRequestHeader(absl::string_view header,
                         absl::string_view value){
  evkeyvalq* ev_headers =
      evhttp_request_get_output_headers(parsed_response_->request);

  int ret = evhttp_add_header(ev_headers,
                              std::string(header.data(), header.size()).c_str(),
                              std::string(value.data(), value.size()).c_str());

  if (ret != 0) {
    NET_LOG(ERROR,
            "Unexpected: failed to set the request header"
            " %.*s: %.*s",
            static_cast<int>(header.size()), header.data(),
            static_cast<int>(value.size()), value.data());
  }
}

std::vector<absl::string_view> ClientEvHTTPRequest::request_headers(){
  auto result = std::vector<absl::string_view>();
  const auto ev_headers = evhttp_request_get_output_headers(parsed_response_->request);

  for (evkeyval* header = ev_headers->tqh_first; header;
       header = header->next.tqe_next) {
    result.emplace_back(header->key);
  }

  return result;
}

std::unique_ptr<char[], BlockDeleter> ClientEvHTTPRequest::ReadResponseBytes(
      int64_t* size){
  evbuffer* input_buf =
      evhttp_request_get_input_buffer(parsed_response_->request);
  if (input_buf == nullptr) {
    *size = 0;
    return nullptr;  // no body
  }

  // possible a reentry after gzip uncompression
  if (evbuffer_get_length(input_buf) == 0) {
    *size = 0;
    return nullptr;  // EOF
  }

  auto buf_size = reinterpret_cast<size_t*>(size);

  *buf_size = evbuffer_get_contiguous_space(input_buf);
  assert(*buf_size > 0);

  char* block = std::allocator<char>().allocate(*buf_size);
  int ret = evbuffer_remove(input_buf, block, *buf_size);

  if (ret != *buf_size) {
    NET_LOG(ERROR, "Unexpected: read less than specified num_bytes : %zu",
            *buf_size);
    std::allocator<char>().deallocate(block, *buf_size);
    *buf_size = 0;
    return nullptr;  // don't return corrupted buffer
  }

  return std::unique_ptr<char[], BlockDeleter>(block, BlockDeleter(*buf_size));
}

absl::string_view ClientEvHTTPRequest::GetResponseHeader(absl::string_view header) const{
  std::string header_str(header.data(), header.size());
  return absl::NullSafeStringView(
      evhttp_find_header(parsed_response_->headers, header_str.c_str()));
}

std::vector<absl::string_view> ClientEvHTTPRequest::response_headers(){
  auto result = std::vector<absl::string_view>();
  const auto ev_headers = parsed_response_->response_headers;

  for (evkeyval* header = ev_headers->tqh_first; header;
       header = header->next.tqe_next) {
    result.emplace_back(header->key);
  }
  return result;
}

//no idea if this would work or not from the client side
void ClientEvHTTPRequest::Abort() {
  evhttp_send_error(parsed_response_->request, HTTP_INTERNAL, nullptr);
  delete this;
}

std::unique_ptr<ParsedEvResponse> ClientEvHTTPRequest::GetParsedResponse(){
  return parsed_response_;
}


}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow




#endif //TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_CC_