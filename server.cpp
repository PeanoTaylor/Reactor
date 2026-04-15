#include "TcpServer.h"

namespace {
constexpr size_t kThreadNum = 4;
constexpr size_t kQueueSize = 16;
constexpr size_t kSubReactorNum = 3;

}  // namespace

int main()
{
    TcpServer server("127.0.0.1", 22222, kThreadNum, kQueueSize, kSubReactorNum);
    server.start();

    return 0;
}
