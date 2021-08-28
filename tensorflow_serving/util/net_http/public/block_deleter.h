//TODO: add copyright liscense information

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_PUBLIC_BLOCK_DELETER_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_PUBLIC_BLOCK_DELETER_H_

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

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


}  // namespace net_http
}  // namespace serving
}  // namespace tensorflow

#endif //TENSORFLOW_SERVING_UTIL_NET_HTTP_PUBLIC_BLOCK_DELETER_H_