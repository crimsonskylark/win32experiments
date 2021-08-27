#include <algorithm>
#include <vector>
#include "NamedPipes.hpp"
#include "Introspect.hh"
#include <winternl.h>

extern "C" int myFunc();
extern "C" void* TestMethod();
extern "C" PVOID GetTeb();

using MessageBufTy = struct {
  std::string msg_buf;
  size_t timestamp;
  bool delivered;
};

int main() {
  std::wcout << "[main thread] starting experiments"
             << "\n";

  
  auto v = (PPEB_LDR_DATA*)TestMethod();

  auto teb = (_TEB*)GetTeb();
  
  /*introspect();

  named_pipes::RunNamedPipesExperiment();  */

  std::cin.get();
}