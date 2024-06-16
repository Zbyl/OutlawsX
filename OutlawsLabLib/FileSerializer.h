#pragma once

#include <arbitrary_format/binary_serializers/ISerializer.h>

#include <arbitrary_format/serialization_exceptions.h>

#include <cstdint>
#include <type_traits>
#include <cstdio>
#include <string>

namespace arbitrary_format
{
namespace binary
{

class FileSaveSerializer
{
    FILE* file;
    size_t filePosition;
public:
    explicit FileSaveSerializer(const std::string& filePath)
        : filePosition(0)
    {
        file = fopen(filePath.c_str(), "wb");
        if (!file)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error opening file."));
    }

    ~FileSaveSerializer() {
        fclose(file);
    }

    size_t position()
    {
        return filePosition;
    }

    void seek(size_t pos)
    {
        if (fseek(file, static_cast<long>(pos), SEEK_SET))
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error seeking file."));

        filePosition = pos;
    }

public:
    using saving_serializer = std::true_type;

    void saveData(const uint8_t* data, size_t size)
    {
        if (size == 0)
            return;

        if (fwrite(data, size, 1, file) != 1)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error writing to file."));

        filePosition += size;
    }
};

class FileLoadSerializer
{
    FILE* file;
    size_t fileSize;
    size_t filePosition;
public:
    explicit FileLoadSerializer(const std::string& filePath)
        : filePosition(0)
    {
        file = fopen(filePath.c_str(), "rb");
        if (!file)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error opening file."));

        if (fseek(file, 0, SEEK_END))
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error seeking file to end."));

        auto position = ftell(file);
        if (position == -1)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error getting file size."));

        if (fseek(file, 0, SEEK_SET))
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error seeking file to begining."));

        fileSize = static_cast<size_t>(position);
    }

    ~FileLoadSerializer() {
        fclose(file);
    }

    size_t position() const
    {
        return filePosition;
    }

    bool isAtEnd() const
    {
        return filePosition == fileSize;
    }

    void seek(size_t pos)
    {
        if (pos > fileSize)
        {
            BOOST_THROW_EXCEPTION(end_of_input() << errinfo_requested_this_many_bytes_more(pos - fileSize));
        }

        if (fseek(file, static_cast<long>(pos), SEEK_SET))
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error seeking file."));

        filePosition = pos;
    }

public:
    using loading_serializer = std::true_type;

    void loadData(uint8_t* data, size_t size)
    {
        if (filePosition + size > fileSize)
        {
            BOOST_THROW_EXCEPTION(end_of_input() << errinfo_requested_this_many_bytes_more(filePosition + size - fileSize));
        }

        if (size == 0)
            return;

        if (fread(data, size, 1, file) != 1)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Error reading from file."));

        filePosition += size;
    }
};

} // namespace binary
} // namespace arbitrary_format
