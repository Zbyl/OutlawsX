#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace lvb {

/// Converts LVB file into an LVT string.
std::string loadLvbFile(const std::string& filePath);

} // namespace lvb
