#include "NamedPipes.hpp"

namespace named_pipes {
HANDLE
NpCreateNamedPipe() {
  auto pipe_handle =
      CreateNamedPipeW(L"\\\\.\\pipe\\testpipe", PIPE_ACCESS_DUPLEX,
                       PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                           PIPE_REJECT_REMOTE_CLIENTS,
                       1, 1024, 1024, 0, NULL);

  if (pipe_handle == INVALID_HANDLE_VALUE) {
    std::wcout << "[main thread] failed to create named pipe: "
               << GetLastError() << "\n";
    return nullptr;
  }

  std::wcout << "[main thread] pipe created, handle: " << std::hex
             << pipe_handle << "\n";

  return pipe_handle;
}

}  // namespace named_pipes
