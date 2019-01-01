#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace lab_fuse {

struct LabFile {
    std::string typeAndExtension;   ///< Length must always be 4.
    std::vector<uint8_t> data;
};

struct Lab {
    std::map<std::string, LabFile> files;   ///< Map from file name to file data.
    std::vector<std::string> filesOrder;   ///< Original order of files in LAB file.
};

/// Loads LAB file.
Lab loadLabFile(const std::string& filePath);

/// Saves LAB file.
void saveLabFile(const std::string& filePath, const Lab& lab);

} // namespace lab_fuse
