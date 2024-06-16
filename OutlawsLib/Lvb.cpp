
#include "Lvb.h"

#include "../OutlawsLabLib/FileSerializer.h"

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
#include <arbitrary_format/xml_formatters/hex_stringizer.h>

#include <algorithm>
#include <numeric>
#include <sstream>

#include <boost/lexical_cast.hpp>


namespace lvb {

#if 0
struct LabHeader {
    char magic[4];
    uint32_t constant0x10000;
    uint32_t numFiles;
    uint32_t filenameDirectoryLength;
};

struct LvbItem {
    LvbItemKind kind;    /// Kind of item.
    uint8_t count;    /// Number of contained elements.
};

struct LvtVersion : LvbItem {
    uint32_t major;
    uint32_t minor;
};

#endif

enum class LvbItemKind : uint32_t {
    LVT_VERSION = 0x00,
    LEVELNAME = 0x01,
    VERSION = 0x02,
    PALETTES = 0x03,
    GLOBAL_PALETTE = 0x04,
    CMAPS = 0x05,
    GLOBAL_CMAP = 0x06,
    MUSIC = 0x07,
    PARALLAX = 0x08,
    LIGHT_SOURCE =  0x09,
    SHADES = 0x0A,
    SHADE = 0x0B,
    TEXTURES = 0x0C,
    TEXTURE = 0x0D,
    NUMSECTORS = 0x0E,
    SECTOR = 0x0F,
    NAME = 0x10,
    AMBIENT = 0x11,
    SECTOR_PALETTE = 0x12,
    SECTOR_CMAP = 0x13,
    FRICTION = 0x14,
    GRAVITY = 0x15,
    ELASTICITY = 0x16,
    VELOCITY = 0x17,
    VADJOIN = 0x18,
    FLOOR_SOUND =  0x19,
    FLOOR_Y =  0x1A,
    CEILING_Y =  0x1B,
    F_OVERLAY = 0x1C,
    C_OVERLAY = 0x1D,
    FLOOR_OFFSETS =  0x1E,
    OFFSET = 0x1F,
    FLAGS = 0x20,
    SLOPEDFLOOR = 0x21,
    SLOPEDCEILING = 0x22,
    LAYER = 0x23,
    VERTICES = 0x24,
    VERTEX = 0x25,
    WALLS = 0x26,
    WALL = 0x27,
};

enum class LvbFieldType {
    INT,        // 4 bytes, little endian.
    FLOAT,      // 4 bytes, little endian.
    STRING,     // Null-terminated ascii string.
};

struct LvbItemDesc {
    LvbItemKind kind;              ///< ID of the item.
    std::string name;           ///< Name if the item.
    int blankLinesBefore;       ///< Number of blank lines to print before the item.
    int indent;                 ///< How much to indent the item.
    std::vector<LvbFieldType> fields;   ///< Types of fields that should follow.
};

std::vector<LvbItemDesc> lvbItemDescs = {
    { LvbItemKind::LVT_VERSION, "LVT", 0, 0, { LvbFieldType::INT, LvbFieldType::INT } },
    { LvbItemKind::LEVELNAME, "LEVELNAME", 0, 0, { LvbFieldType::STRING } },
    { LvbItemKind::VERSION, "VERSION", 0, 0, { LvbFieldType::INT, LvbFieldType::INT } }, // major, minor (6 digits)
    { LvbItemKind::PALETTES, "PALETTES", 0, 0, { LvbFieldType::INT } },
    { LvbItemKind::GLOBAL_PALETTE, "PALETTE:", 0, 3, { LvbFieldType::STRING } },
    { LvbItemKind::CMAPS, "CMAPS", 0, 0, { LvbFieldType::INT } },
    { LvbItemKind::GLOBAL_CMAP, "CMAP:", 0, 3, { LvbFieldType::STRING } },
    { LvbItemKind::MUSIC, "MUSIC", 0, 0, { LvbFieldType::STRING } },
    { LvbItemKind::PARALLAX, "PARALLAX", 0, 0, { LvbFieldType::FLOAT, LvbFieldType::FLOAT } },
    { LvbItemKind::LIGHT_SOURCE, "LIGHT SOURCE", 0, 0, { LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::FLOAT } },
    { LvbItemKind::SHADES, "SHADES", 0, 0, { LvbFieldType::INT } },
    { LvbItemKind::SHADE, "SHADE:", 0, 3, { LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::STRING } },
    { LvbItemKind::TEXTURES, "TEXTURES", 1, 0, { LvbFieldType::INT } },
    { LvbItemKind::TEXTURE, "TEXTURE:", 0, 2, { LvbFieldType::STRING } },
    { LvbItemKind::NUMSECTORS, "NUMSECTORS", 2, 0, { LvbFieldType::INT } },
    { LvbItemKind::SECTOR, "SECTOR", 2, 0, { LvbFieldType::INT } },
    { LvbItemKind::NAME, "NAME", 0, 2, { LvbFieldType::STRING } },
    { LvbItemKind::AMBIENT, "AMBIENT", 0, 2, { LvbFieldType::INT } },
    { LvbItemKind::SECTOR_PALETTE, "PALETTE", 0, 2, { LvbFieldType::INT } },
    { LvbItemKind::SECTOR_CMAP, "CMAP", 0, 2, { LvbFieldType::INT } },
    { LvbItemKind::FRICTION, "FRICTION", 0, 2, { LvbFieldType::FLOAT } },
    { LvbItemKind::GRAVITY, "GRAVITY", 0, 2, { LvbFieldType::FLOAT } },
    { LvbItemKind::ELASTICITY, "ELASTICITY", 0, 2, { LvbFieldType::FLOAT } },
    { LvbItemKind::VELOCITY, "VELOCITY", 0, 2, { LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::INT } }, // @todo 3 floats???
    { LvbItemKind::VADJOIN, "VADJOIN", 0, 2, { LvbFieldType::INT } }, // Signed int.
    { LvbItemKind::FLOOR_SOUND, "FLOOR SOUND", 0, 2, { LvbFieldType::STRING } }, // Can be string "NULL".
    { LvbItemKind::FLOOR_Y, "FLOOR Y", 0, 2, { LvbFieldType::FLOAT, LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::INT } }, // Last one is float???
    { LvbItemKind::CEILING_Y, "CEILING Y", 0, 2, { LvbFieldType::FLOAT, LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::INT } }, // Last one is float???
    { LvbItemKind::F_OVERLAY, "F_OVERLAY", 0, 2, { LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::INT } }, // Last one is float???
    { LvbItemKind::C_OVERLAY, "C_OVERLAY", 0, 2, { LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::INT } }, // Last one is float???
    { LvbItemKind::FLOOR_OFFSETS, "FLOOR OFFSETS", 0, 2, { LvbFieldType::INT } },
    { LvbItemKind::OFFSET, "OFFSET:", 0, 13, { LvbFieldType::FLOAT, LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, LvbFieldType::INT, LvbFieldType::INT } },
    { LvbItemKind::FLAGS, "FLAGS", 0, 2, { LvbFieldType::INT, LvbFieldType::INT } },
    { LvbItemKind::SLOPEDFLOOR, "SLOPEDFLOOR", 0, 2, { LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::FLOAT } },
    { LvbItemKind::SLOPEDCEILING, "SLOPEDCEILING", 0, 2, { LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::FLOAT } },
    { LvbItemKind::LAYER, "LAYER", 0, 2, { LvbFieldType::FLOAT } },
    { LvbItemKind::VERTICES, "VERTICES", 2, 2, { LvbFieldType::INT } },
    { LvbItemKind::VERTEX, "VERTEX", 0, 4, { LvbFieldType::FLOAT, LvbFieldType::FLOAT } },
    { LvbItemKind::WALLS, "WALLS", 3, 2, { LvbFieldType::INT } },
    { LvbItemKind::WALL, "WALL:", 0, 4, { LvbFieldType::INT, LvbFieldType::INT, LvbFieldType::INT, // sector_id, V1, V2
                                          LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, // MID, offsX, offsY
                                          LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, // TOP, offsX, offsY
                                          LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, // BOT, offsX, offsY
                                          LvbFieldType::INT, LvbFieldType::FLOAT, LvbFieldType::FLOAT, // OVERLAY, offsX, offsY
                                          LvbFieldType::INT, LvbFieldType::INT, // ADJOIN, MIRROR
                                          LvbFieldType::INT, LvbFieldType::INT, // DADJOIN, DMIRROR
                                          LvbFieldType::INT, LvbFieldType::INT, // FLAGS
                                          LvbFieldType::INT, // LIGHT
                                        } },
};

using namespace arbitrary_format;
using namespace arbitrary_format::binary;

struct LvtOutput {
    std::stringstream& lvt; ///< Output stream.
    int sectorNumber;       ///< State used for outputting sector numbers.
    int vertexNumber;       ///< State used for outputting vertex numbers.
};

struct lvb_item_formatter
{
    template<typename TSerializer>
    void load(TSerializer& serializer, LvtOutput& lvtOutput) const
    {
        auto& lvt = lvtOutput.lvt;

        std::map<LvbItemKind, LvbItemDesc> lvbItemDescsMap;
        for (const auto& lvbItemDesc : lvbItemDescs) {
            lvbItemDescsMap[lvbItemDesc.kind] = lvbItemDesc;
        }

        LvbItemKind elementKind;
        serialize<little_endian<4>>(serializer, elementKind);

        const auto& lvbItemDesc = lvbItemDescsMap.at(elementKind);

        size_t numItems;
        serialize<little_endian<1>>(serializer, numItems);

        auto hackedFields = lvbItemDesc.fields;
        if (numItems != lvbItemDesc.fields.size()) {
            if ((elementKind == LvbItemKind::NAME) && (numItems == 0)) {
                // @note NAME is sometimes empty.
                hackedFields.clear();
            } else {
                BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Invalid number of fields."));
            }
        }

        for (auto i = 0; i < lvbItemDesc.blankLinesBefore; ++i) {
            lvt << "\n";
        }

        for (auto i = 0; i < lvbItemDesc.indent; ++i) {
            lvt << " ";
        }

        std::vector<std::string> fields;

        for (auto fieldType : hackedFields) {
            if (fieldType == LvbFieldType::INT) {
                int64_t value;  // Sometimes we get -1, and sometimes unsigned version of it (i.e. in SLOPEDCEILING).
                serialize<little_endian<4>>(serializer, value);
                fields.push_back(boost::lexical_cast<std::string>(value));
            }
            else
            if (fieldType == LvbFieldType::FLOAT) {
                float value;
                serialize<little_endian<4>>(serializer, value);

                constexpr auto max_digits10 = std::numeric_limits<float>::max_digits10;
                std::stringstream str;
                str << std::setprecision(max_digits10) << value;
                fields.push_back(str.str());
            }
            else
            if (fieldType == LvbFieldType::STRING) {
                std::string strvalue;
                while (true) {
                    char value[2] = "!";
                    serialize<little_endian<1>>(serializer, value[0]);
                    if (value[0] == '\0')
                        break;
                    strvalue += value;
                }
                fields.push_back(strvalue);
            }
            else {
                BOOST_THROW_EXCEPTION(serialization_exception() << errinfo_description("Invalid type."));
            }
        }

        if (lvbItemDesc.kind == LvbItemKind::LVT_VERSION) {
            lvtOutput.sectorNumber = 0;
        }
        if (lvbItemDesc.kind == LvbItemKind::VERTICES) {
            lvtOutput.vertexNumber = 0;
        }

        if (lvbItemDesc.kind == LvbItemKind::LVT_VERSION) {
            lvt << "LVT " << fields[0] << "." << fields[1];
        }
        else
        if (lvbItemDesc.kind == LvbItemKind::VERSION) {
            lvt << "VERSION " << fields[0] << "." << fields[1]; // @todo Minor must have 6 digits!
        }
        else
        if (lvbItemDesc.kind == LvbItemKind::SECTOR) {
            uint32_t sectorId = boost::lexical_cast<uint32_t>(fields[0]);
            std::stringstream str;
            str << std::noshowbase << std::uppercase << std::hex << sectorId;
            lvt << "SECTOR " << str.str() << "      # ORD: " << lvtOutput.sectorNumber;
            lvtOutput.sectorNumber++;
        }
        else
        if (lvbItemDesc.kind == LvbItemKind::OFFSET) {
            lvt << "OFFSET: " << fields[0] << " " << fields[1] << " " << fields[2] << " " << fields[3] << " FLAGS: " << fields[4] << " " << fields[5];
        }
        else
        if (lvbItemDesc.kind == LvbItemKind::VERTEX) {
            lvt << "X: " << fields[0] << " Z: " << fields[1] << "      #  " << lvtOutput.vertexNumber;
            lvtOutput.vertexNumber++;
        }
        else
        if (lvbItemDesc.kind == LvbItemKind::WALL) {
            uint32_t wallNumber = boost::lexical_cast<uint32_t>(fields[0]);
            std::stringstream str;
            str << std::noshowbase << std::uppercase << std::hex << wallNumber;

            lvt << "WALL: " << str.str() << " V1:  " << fields[1] << "  V2:  " << fields[2]
                << "  MID:  " << fields[3] << " " << fields[4] << " " << fields[5]
                << "  TOP:  " << fields[6] << " " << fields[7] << " " << fields[8]
                << "  BOT:  " << fields[9] << " " << fields[10] << " " << fields[11]
                << "  OVERLAY:  " << fields[12] << " " << fields[13] << " " << fields[14]
                << "  ADJOIN:  " << fields[15] << "  MIRROR:  " << fields[16]
                << "  DADJOIN:  " << fields[17] << "  DMIRROR:  " << fields[18]
                << "  FLAGS:  " << fields[19] << " " << fields[20]
                << "  LIGHT:  " << fields[21];
        }
        else {
            lvt << lvbItemDesc.name;
            for (const auto& field : fields) {
                lvt << " " << field;
            }
        }

        lvt << "\n";
    }
};

struct lvb_formatter
{
    template<typename TSerializer>
    void load(TSerializer& serializer, std::stringstream& lvt) const
    {
        const char magic[4] = { '.', 'L', 'V', 'T' };
        serialize<const_formatter<little_endian<4>>>(serializer, 0x0);
        serialize<const_formatter<array_formatter<little_endian<1>>>>(serializer, magic);
        LvtOutput lvtOutput { lvt, 0, 0 };
        while (!serializer.isAtEnd()) {
            serialize<lvb_item_formatter>(serializer, lvtOutput);
        }
    }
};

std::string loadLvbFile(const std::string& filePath) {
    FileLoadSerializer serializer(filePath);

    std::stringstream lvt;
    load<lvb_formatter>(serializer, lvt);

    return lvt.str();
}

} // namespace lvb
