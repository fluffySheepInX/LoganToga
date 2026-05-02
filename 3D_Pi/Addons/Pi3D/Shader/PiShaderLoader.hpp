# pragma once
# include <Siv3D.hpp>

namespace Pi3D
{
    class PiShaderLoader
    {
    public:
        static void SetShaderBaseDirectory(StringView directory)
        {
            ShaderBaseDirectory() = String{ directory };
        }

        [[nodiscard]] static const String& GetShaderBaseDirectory()
        {
            return ShaderBaseDirectory();
        }

        [[nodiscard]] static FilePath HLSL(StringView name)
        {
            return (ShaderBaseDirectory() + U"/hlsl/" + String{ name } + U".hlsl");
        }

        [[nodiscard]] static FilePath GLSLFragment(StringView name)
        {
            return (ShaderBaseDirectory() + U"/glsl/" + String{ name } + U".frag");
        }

        [[nodiscard]] static FilePath GLSLVertex(StringView name)
        {
            return (ShaderBaseDirectory() + U"/glsl/" + String{ name } + U".vert");
        }

    private:
        [[nodiscard]] static String& ShaderBaseDirectory()
        {
            static String baseDirectory = U"example/shader";
            return baseDirectory;
        }
    };
}
