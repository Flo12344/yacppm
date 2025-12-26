#pragma once

#include "utils/logger.hpp"
namespace yacppm {
inline void version() { Loggger::info("YACPPM version {}", VERSION); }

} // namespace yacppm
