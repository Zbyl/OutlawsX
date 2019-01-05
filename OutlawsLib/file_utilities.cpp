#include "file_utilities.h"

#include "zerrors.h"

#include <fstream>
#include <iterator>

namespace terminal_editor {

std::string readFileAsString(const std::string& fileName) {
    std::ifstream input;
    input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        input.open(fileName, std::ios::binary);
    }
    catch (const std::exception& exc) {
        ZTHROW(FileNotFoundException()) << "Could not open input file: '" << fileName << "'. Error: " << exc.what();
    }

    try {
        // @todo This is an extremely inefficient implementation.
        std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return text;
    }
    catch (const std::exception& exc) {
        ZTHROW() << "Error while reading file: '" << fileName << "'. Error: " << exc.what();
    }
}

void writeStringToFile(const std::string& fileName, const std::string& text) {
    std::ofstream output;
    output.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        output.open(fileName, std::ios::binary);
    }
    catch (const std::exception& exc) {
        ZTHROW() << "Could not open output file: '" << fileName << "'. Error: " << exc.what();
    }

    try {
        output.write(text.data(), text.size());
    }
    catch (const std::exception& exc) {
        ZTHROW() << "Error while writing to file: '" << fileName << "'. Error: " << exc.what();
    }
}

} // namespace terminal_editor
