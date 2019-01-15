#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace outlaws {

struct Uv {
    float u, v;
};

struct TextureUvs {
    /// Position in texture atlas.
    Uv start;
    Uv end;

    /// Dimensions of the textures. Needed to map Outlaws texture coordinates to 0.0 - 1.0 range.
    int width;
    int height;
};

using TexInfos = std::unordered_map<std::string, TextureUvs>;

/// Returns texture name lowercase and without extension.
std::string canonicalTextureName(const std::string& textureName);

/// Loads information about texture atlas from a .json file in TexturePackerGUI format.
TexInfos loadTexInfos(const std::string& filePath);

/// Returns TextureUvs for given texture.
/// Throws if texture was not found in texture atlas.
/// @param texInfos             Information about used texture atlas.
/// @param textureName          Name of the texture. Will be canonicalized.
/// @return TextureUvs for given texture.
TextureUvs getTextureUvs(const TexInfos& texInfos, const std::string& textureName);

/// Returns TextureUvs for given texture.
/// @param texInfos             Information about used texture atlas.
/// @param textureName          Name of the texture. Will be canonicalized.
/// @param defaultTextureName   Name of the replacement texture to use if given texture was not found.
/// @return TextureUvs for given texture, or for defaultTextureName texture if textureName is not found.
TextureUvs getTextureUvs(const TexInfos& texInfos, const std::string& textureName, const std::string& defaultTextureName);

} // namespace outlaws
