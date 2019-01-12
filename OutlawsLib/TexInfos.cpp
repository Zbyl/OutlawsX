#include "TexInfos.h"

#include "file_utilities.h"

#include <cctype>

#include <nlohmann/json.hpp>

namespace outlaws {

struct TexRect {
    int x, y, width, height;
};

struct TexInfo {
    std::unordered_map<std::string, TexRect> textureRects;
    int width;
    int height;
};

void from_json(const nlohmann::json& json, TexInfo& texInfo) {
    texInfo.width = json["meta"]["size"]["w"].get<int>();
    texInfo.height = json["meta"]["size"]["h"].get<int>();

    auto frames = json["frames"];
    for (auto& frame : frames) {
        std::string texName = frame["filename"].get<std::string>();
        TexRect rect = {
            frame["frame"]["x"].get<int>(),
            frame["frame"]["y"].get<int>(),
            frame["frame"]["w"].get<int>(),
            frame["frame"]["h"].get<int>(),
        };

        texInfo.textureRects[texName] = rect;
    }
}

TexInfos loadTexInfos(const std::string& filePath) {
    auto texInfoJson = terminal_editor::readFileAsString(filePath);
    TexInfo texInfo = nlohmann::json::parse(texInfoJson);

    TexInfos texInfos;
    for (const auto& kv : texInfo.textureRects) {
        auto& texName = kv.first;
        auto& rect = kv.second;
        auto tu0 = static_cast<float>(rect.x) / texInfo.width;
        auto tu1 = static_cast<float>(rect.x + rect.width) / texInfo.width;
        auto tv0 = static_cast<float>(rect.y) / texInfo.height;
        auto tv1 = static_cast<float>(rect.y + rect.height) / texInfo.height;
        tv0 = 1.0f - tv0;
        tv1 = 1.0f - tv1;

        TextureUvs uvs { tu0, tv0, tu1, tv1 };
        auto canonicalTexName = canonicalTextureName(texName);
        texInfos[canonicalTexName] = uvs;
    }

    return texInfos;
}

std::string canonicalTextureName(const std::string& textureName) {
    auto texName = textureName.substr(0, textureName.size() - 4); // Cut off the extension.
    std::transform(texName.begin(), texName.end(), texName.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return texName;
}

} // namespace outlaws
