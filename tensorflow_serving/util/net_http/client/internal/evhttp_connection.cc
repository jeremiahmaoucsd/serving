/*copyright stuff*/

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_CC_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_CC_

#include "tensorflow_serving/util/net_http/client/internal/evhttp_connection.h"

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


bool EvHTTPConnection::Connect(absl::string_view host, int port){
  ev_base_ = event_base_new();
  if (ev_base_ == nullptr) {
    NET_LOG(ERROR, "Failed to connect : event_base_new()");
    return false;
  }

  // blocking call (DNS resolution)
  std::string host_str(host.data(), host.size());
  evcon_ = evhttp_connection_base_bufferevent_new(
      ev_base_, nullptr, nullptr, host_str.c_str(),
      static_cast<uint16_t>(port));
  if (evcon_ == nullptr) {
    NET_LOG(ERROR,
            "Failed to connect : evhttp_connection_base_bufferevent_new()");
    return false;
  }

  evhttp_connection_set_retries(evcon_, 0);

  // TODO(wenboz): make this an option (default to 5s)
  evhttp_connection_set_timeout(evcon_, 5);

  return true;
}

bool EvHTTPConnection::Connect(absl::string_view uri){
    std::string url_str(url.data(), url.size());
  struct evhttp_uri* http_uri = evhttp_uri_parse(url_str.c_str());
  if (http_uri == nullptr) {
    NET_LOG(ERROR, "Failed to connect : event_base_new()");
    return false;
  }

  const char* host = evhttp_uri_get_host(http_uri);
  if (host == nullptr) {
    NET_LOG(ERROR, "url must have a host %.*s", static_cast<int>(url.size()),
            url.data());
    return false;
  }

  int port = evhttp_uri_get_port(http_uri);
  if (port == -1) {
    port = 80;
  }

  Connect(host, port);
  evhttp_uri_free(http_uri);
  return true;
}

bool EvHTTPConnection::is_connected(){
  if(evhttp_connection_get_server(evcon_) == nullptr){
    return false;
  }
  return true;
}

evhttp_cmd_type GetMethodEnum(absl::string_view method, bool with_body) {
  if (method.compare("GET") == 0) {
    return EVHTTP_REQ_GET;
  } else if (method.compare("POST") == 0) {
    return EVHTTP_REQ_POST;
  } else if (method.compare("HEAD") == 0) {
    return EVHTTP_REQ_HEAD;
  } else if (method.compare("PUT") == 0) {
    return EVHTTP_REQ_PUT;
  } else if (method.compare("DELETE") == 0) {
    return EVHTTP_REQ_DELETE;
  } else if (method.compare("OPTIONS") == 0) {
    return EVHTTP_REQ_OPTIONS;
  } else if (method.compare("TRACE") == 0) {
    return EVHTTP_REQ_TRACE;
  } else if (method.compare("CONNECT") == 0) {
    return EVHTTP_REQ_CONNECT;
  } else if (method.compare("PATCH") == 0) {
    return EVHTTP_REQ_PATCH;
  } else {
    if (with_body) {
      return EVHTTP_REQ_POST;
    } else {
      return EVHTTP_REQ_GET;
    }
  }
}

//THIS IS A HEAVY WORK IN PROGRESS
bool EvHTTPConnection::SendRequest(ClientEvHTTPRequest* request, bool asynchronous){
  ParsedEvResponse* parsed_response = request->parsed_response_;
  int r = evhttp_make_request(
      evcon_, parsed_response->request, GetMethodEnum(parsed_response->method, false),
      parsed_response->path);
  if (r != 0) {
    NET_LOG(ERROR, "evhttp_make_request() failed");
    return false;
  }
}

}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif //TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_EVHTTP_CONNECTION_CC_