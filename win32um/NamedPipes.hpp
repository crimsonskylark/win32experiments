#pragma once

#include "Common.hpp"

namespace named_pipes {

HANDLE NpCreateNamedPipe();
size_t SvWriteToPipe(PVOID buffer);
size_t SvReadFromPipe(PVOID buffer);

void RunNamedPipesExperiment();

}  // namespace named_pipes