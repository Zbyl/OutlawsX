#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace outlaws {

struct Uv {
    float u, v;
};

struct TextureUvs {
    Uv start;
    Uv end;
};

using TexInfos = std::unordered_map<std::string, TextureUvs>;

std::string canonicalTextureName(const std::string& textureName);

TexInfos loadTexInfos(const std::string& filePath);

} // namespace outlaws
