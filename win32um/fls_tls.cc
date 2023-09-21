#include <Windows.h>
#include <iostream>

static LPVOID primaryfiber;
static LPVOID readfiber;

void FiberFun(LPVOID) {
    std::cout << "inside fiber" << "\n";
}

static void ThreadFun() {
    std::cout << "inside thread" << "\n";
}


//void main() {
//    primaryfiber = ConvertThreadToFiber(0);
//
//    readfiber = CreateFiber(0, FiberFun, nullptr);
//
//    std::cout << "read fiber address: " << std::hex << readfiber << "\n";
//
//    SwitchToFiber(readfiber);
//}