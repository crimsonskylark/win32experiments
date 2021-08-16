#include <algorithm>
#include <vector>
#include "NamedPipes.hpp"
#include "Introspect.hh"
#include <winternl.h>

static bool g_NpCreatedServerThread = false;
static bool g_NpCreatedClitThread = false;

static constexpr wchar_t PIPE_NAME[] = L"\\\\.\\pipe\\testpipe";
static constexpr int PIPE_BUFFER_SZ = 4096;

extern "C" int myFunc();
extern "C" void* TestMethod();

_PEB peb;

static int NpServerThread(PVOID Param) {
  auto pipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX,
                              PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE |
                                  PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                              2, PIPE_BUFFER_SZ, PIPE_BUFFER_SZ, 0, nullptr);

  if (pipe == INVALID_HANDLE_VALUE) {
    std::wcout << "[server thread] unable to create named pipe: "
               << GetLastError() << "\n";
    return -1;
  }

  std::wcout << "[server thread] created named pipe, handle: " << pipe << "\n";

  wchar_t buffer[PIPE_BUFFER_SZ] = {0};

  DWORD numBytesRead = 0;

  std::wcout << "[server thread] polling for messages..."
             << "\n";

  auto connected = ConnectNamedPipe(pipe, nullptr);

  for (;;) {
    auto buf_read =
        ReadFile(pipe, buffer, PIPE_BUFFER_SZ, &numBytesRead, nullptr);

    if (!buf_read || GetLastError() == ERROR_PIPE_LISTENING) {
      std::wcout << "[server thread] unable to read from pipe: "
                 << GetLastError() << "\n";
      return -1;
    }

    if (connected == 0 && numBytesRead > 0) {
      std::wcout << "[server thread] received buffer from client: " << buffer
                 << "\n";
    } else {
      std::wcout << "[server thread] failed to connect to pipe: "
                 << GetLastError() << " bytes read: " << numBytesRead << "\n";
    }
    if (wcsncmp(buffer, L"quit", 4) == 0) {
      return 1;
    }
  }
}

static int NpClientThread(PVOID Param) {
  auto pipe_handle = CreateFile(PIPE_NAME, FILE_ALL_ACCESS, 0, nullptr,
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (pipe_handle == INVALID_HANDLE_VALUE) {
    std::wcout << "[client thread] unable to open pipe: " << GetLastError()
               << "\n";
    return -1;
  }

  for (;;) {
    std::wstring buffer{};
    std::wcout << "message> ";
    std::wcin >> buffer;

    DWORD numOfBytesWritten = 0;

    std::wcout << "[client thread] sending message: " << buffer << "\n";

    auto write_status = WriteFile(pipe_handle, buffer.data(), buffer.size(),
                                  &numOfBytesWritten, nullptr);

    if (buffer.compare(L"quit") == 0) {
      CloseHandle(pipe_handle);
      return -1;
    }

    if (!write_status) {
      std::wcout << "[client thread] unable to write to pipe: "
                 << GetLastError() << "\n";
    } else {
      std::wcout << "[client thread] wrote " << numOfBytesWritten
                 << " bytes to pipe"
                 << "\n";
    }
  }
}

using MessageBufTy = struct {
  std::string msg_buf;
  size_t timestamp;
  bool delivered;
};

int main() {
  std::wcout << "[main thread] starting experiments"
             << "\n";

  
  auto v = (PPEB_LDR_DATA*)TestMethod();

  introspect();

  std::cout << myFunc() << "\n";
  

  auto sv_thread = CreateThread(
      NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(NpServerThread),
      nullptr, 0, nullptr);
  auto cl_thread = CreateThread(
      NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(NpClientThread),
      nullptr, 0, nullptr);

  if (cl_thread == INVALID_HANDLE_VALUE || sv_thread == INVALID_HANDLE_VALUE) {
    std::cout << "[main thread] unable to create thread: " << GetLastError()
              << "\n";
    return -1;
  }

  HANDLE tids[2] = {cl_thread, sv_thread};

  WaitForMultipleObjects(2, tids, true, INFINITE);

  CloseHandle(tids[0]);
  CloseHandle(tids[1]);

  std::cin.get();
}