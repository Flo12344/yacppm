#pragma once

#include "utils/logger.hpp"
namespace yacppm {
inline void version() { Logger::info("YACPPM version {}", VERSION); }

} // namespace yacppm
