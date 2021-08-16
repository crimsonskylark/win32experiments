#pragma once

#include "Common.hpp"

namespace named_pipes {

HANDLE NpCreateNamedPipe();
size_t SvWriteToPipe(PVOID buffer);
size_t SvReadFromPipe(PVOID buffer);

}  // namespace named_pipes