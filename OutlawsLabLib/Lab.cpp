
#include "Lab.h"

#include "FileSerializer.h"

#include <arbitrary_format/serialize.h>
#include <arbitrary_format/binary_serializers/VectorSaveSerializer.h>
#include <arbitrary_format/binary_serializers/MemorySerializer.h>
#include <arbitrary_format/binary_formatters/endian_formatter.h>
#include <arbitrary_format/binary_formatters/string_formatter.h>
#include <arbitrary_format/formatters/vector_formatter.h>
#include <arbitrary_format/formatters/map_formatter.h>
#include <arbitrary_format/formatters/const_formatter.h>
#include <arbitrary_format/formatters/array_formatter.h>
#include <arbitrary_format/formatters/external_value.h>

#include <algorithm>
#include <numeric>

namespace lab_fuse {

struct LabHeader {
    char magic[4];
    uint32_t constant0x10000;
    uint32_t numFiles;
    uint32_t filenameDirectoryLength;
};

struct LabFileEntry {
    uint32_t nameOffset;    /// Relative to file name list start, which is right after file header.
    uint32_t dataOffset;    /// Relative to begining of file.
    uint32_t size;          ///< Size of the file.
    char typeAndExtension[4];   ///< Some kind of file type. Can be null.
};

using namespace arbitrary_format;
using namespace arbitrary_format::binary;

struct lab_file_entry_formatter
{
    template<typename TSerializer>
    void save(TSerializer& serializer, const LabFileEntry& labFileEntry) const
    {
        serialize<little_endian<4>>(serializer, labFileEntry.nameOffset);
        serialize<little_endian<4>>(serializer, labFileEntry.dataOffset);
        serialize<little_endian<4>>(serializer, labFileEntry.size);
        serialize<array_formatter<little_endian<1>>>(serializer, labFileEntry.typeAndExtension);
    }

    template<typename TSerializer>
    void load(TSerializer& serializer, LabFileEntry& labFileEntry) const
    {
        serialize<little_endian<4>>(serializer, labFileEntry.nameOffset);
        serialize<little_endian<4>>(serializer, labFileEntry.dataOffset);
        serialize<little_endian<4>>(serializer, labFileEntry.size);
        serialize<array_formatter<little_endian<1>>>(serializer, labFileEntry.typeAndExtension);
    }
};

struct lab_formatter
{
    template<typename TSerializer>
    void save(TSerializer& serializer, const Lab& lab) const
    {
        const char magic[4] = { 'L', 'A', 'B', 'N' };
        serialize<const_formatter<array_formatter<little_endian<1>>>>(serializer, magic);
        serialize<const_formatter<little_endian<4>>>(serializer, 0x10000);
        serialize<little_endian<4>>(serializer, lab.files.size());
        int fileNameListLength = std::accumulate(lab.filesOrder.begin(), lab.filesOrder.end(), 0, [](int len, const std::string& fileName) { return len + static_cast<int>(fileName.size()) + 1; });
        serialize<little_endian<4>>(serializer, fileNameListLength);

        int nameOffset = 0;
        int headerSize = static_cast<int>(serializer.position());
        int entriesSize = static_cast<int>(lab.files.size()) * 16;
        int dataStart = headerSize + entriesSize + fileNameListLength;
        int dataOffset = dataStart;
        for (const auto& fileName : lab.filesOrder) {
            const auto& labFile = lab.files.at(fileName);

            if (labFile.typeAndExtension.size() != 4)
                BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("typeAndExtension must have size 4."));

            LabFileEntry fileEntry;
            fileEntry.nameOffset = nameOffset;
            fileEntry.dataOffset = dataOffset;
            fileEntry.size = static_cast<uint32_t>(labFile.data.size());
            std::copy(labFile.typeAndExtension.data(), labFile.typeAndExtension.data() + 4,  fileEntry.typeAndExtension);

            nameOffset += static_cast<int>(fileName.size()) + 1;
            dataOffset += static_cast<int>(labFile.data.size());

            serialize<lab_file_entry_formatter>(serializer, fileEntry);
        }

        if (nameOffset != fileNameListLength)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Inconsistent length of file name list."));

        auto names_format = create_string_formatter( create_external_value(fileNameListLength), little_endian<1>{} );
        for (const auto& fileName : lab.filesOrder) {
            serializer.saveData(reinterpret_cast<const uint8_t*>(fileName.c_str()), fileName.size() + 1);
        }

        if (serializer.position() != dataStart)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Inconsistent offset of start of data."));

        for (const auto& fileName : lab.filesOrder) {
            const auto& labFile = lab.files.at(fileName);

            serializer.saveData(labFile.data.data(), labFile.data.size());
        }

        if (serializer.position() != dataOffset)
            BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Inconsistent offset of end of data."));
    }

    template<typename TSerializer>
    void load(TSerializer& serializer, Lab& lab) const
    {
        const char magic[4] = { 'L', 'A', 'B', 'N' };
        serialize<const_formatter<array_formatter<little_endian<1>>>>(serializer, magic);
        serialize<const_formatter<little_endian<4>>>(serializer, 0x10000);
        uint32_t fileCount;
        serialize<little_endian<4>>(serializer, fileCount);
        uint32_t fileNameListLength;
        serialize<little_endian<4>>(serializer, fileNameListLength);

        auto vec_format = create_vector_formatter( create_external_value(fileCount), lab_file_entry_formatter{} );
        std::vector<LabFileEntry> fileEntries;
        serialize(serializer, fileEntries, vec_format);

        auto names_format = create_string_formatter( create_external_value(fileNameListLength), little_endian<1>{} );
        std::string fileNameList;
        serialize(serializer, fileNameList, names_format);

        uint32_t maxPosition = 0;

        for (const auto& fileEntry : fileEntries) {
            maxPosition = std::max(maxPosition, fileEntry.dataOffset + fileEntry.size);

            LabFile labFile;
            labFile.typeAndExtension = std::string(fileEntry.typeAndExtension, fileEntry.typeAndExtension + 4);

            serializer.seek(fileEntry.dataOffset);
            auto data_format = create_vector_formatter( create_external_value(fileEntry.size), little_endian<1>{} );
            serialize(serializer, labFile.data, data_format);

            std::string fileName = std::string(fileNameList.data() + fileEntry.nameOffset);
            lab.files[fileName] = std::move(labFile);
            lab.filesOrder.push_back(fileName);
        }

        serializer.seek(maxPosition);
    }
};

Lab loadLabFile(const std::string& filePath) {
    FileLoadSerializer serializer(filePath);

    Lab lab;
    load<lab_formatter>(serializer, lab);

    return lab;
}

void saveLabFile(const std::string& filePath, const Lab& lab) {
    FileSaveSerializer serializer(filePath);

    save<lab_formatter>(serializer, lab);
}

} // namespace lab_fuse
