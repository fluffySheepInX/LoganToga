# pragma once
# include <Siv3D.hpp>
# include "../Pi3DConfig.hpp"

namespace Pi3D
{
    class PiShaderLoader
    {
    public:
        // 互換 API として System 構築前の shader ディレクトリを設定する。
        static void SetShaderBaseDirectory(StringView directory)
        {
            Config config = GetConfig();
            config.shaderDirectory = FilePath{ directory };
            SetConfig(config);
        }

        // 現在の shader ディレクトリを返す。
        [[nodiscard]] static const String& GetShaderBaseDirectory()
        {
            return GetConfig().shaderDirectory;
        }

        // HLSL shader の構成済みパスを返す。
        [[nodiscard]] static FilePath HLSL(StringView name)
        {
            return JoinPath(GetConfig().shaderDirectory, U"hlsl/" + String{ name } + U".hlsl");
        }

        // GLSL fragment shader の構成済みパスを返す。
        [[nodiscard]] static FilePath GLSLFragment(StringView name)
        {
            return JoinPath(GetConfig().shaderDirectory, U"glsl/" + String{ name } + U".frag");
        }

        // GLSL vertex shader の構成済みパスを返す。
        [[nodiscard]] static FilePath GLSLVertex(StringView name)
        {
            return JoinPath(GetConfig().shaderDirectory, U"glsl/" + String{ name } + U".vert");
        }
    };
}
