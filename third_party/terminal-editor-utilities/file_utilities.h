#pragma once

#include "zerrors.h"

#include <string>

namespace terminal_editor {

/// Exception thrown when a file is missing.
class FileNotFoundException : public GenericException {};

/// Loads given file into a string.
/// Throws on errors.
/// Throws FileNotFoundException if the file is not found.
std::string readFileAsString(const std::string& fileName);

/// Saves given string into a file.
/// Replaces any previous file contents.
/// Throws on errors.
void writeStringToFile(const std::string& fileName, const std::string& text);

} // namespace terminal_editor
