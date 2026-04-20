# pragma once
# include <Siv3D.hpp>
# include "MainSettingsInternal.hpp"

namespace MainSupport::TomlSchema
{
    [[nodiscard]] inline String MakeKey(const StringView prefix, const StringView suffix)
    {
        return (String{ prefix } + String{ suffix });
    }

    // --- Codecs ---------------------------------------------------------
    // Each codec is a stateless (or const-stateful) struct with member
    // functions Load / Save. Visitors dispatch through these member calls,
    // so adding a new field type only requires defining a new codec.

    struct DoubleCodec
    {
        void Load(const TOMLReader& toml, const String& key, double& value) const
        {
            if (const auto v = toml[key].getOpt<double>())
            {
                value = *v;
            }
        }

        void Save(TextWriter& writer, const String& key, const double value) const
        {
            writer.writeln(U"{} = {:.3f}"_fmt(key, value));
        }
    };

    struct Int32Codec
    {
        void Load(const TOMLReader& toml, const String& key, int32& value) const
        {
            if (const auto v = toml[key].getOpt<int64>())
            {
                value = static_cast<int32>(*v);
            }
        }

        void Save(TextWriter& writer, const String& key, const int32 value) const
        {
            writer.writeln(U"{} = {}"_fmt(key, value));
        }
    };

    struct BoolCodec
    {
        void Load(const TOMLReader& toml, const String& key, bool& value) const
        {
            if (const auto v = toml[key].getOpt<bool>())
            {
                value = *v;
            }
        }

        void Save(TextWriter& writer, const String& key, const bool value) const
        {
            writer.writeln(U"{} = {}"_fmt(key, value ? U"true" : U"false"));
        }
    };

    struct StringCodec
    {
        void Load(const TOMLReader& toml, const String& key, String& value) const
        {
            if (const auto v = toml[key].getOpt<String>())
            {
                value = *v;
            }
        }

        void Save(TextWriter& writer, const String& key, const String& value) const
        {
            const String escaped = String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
            writer.writeln(U"{} = \"{}\""_fmt(key, escaped));
        }
    };

    struct Vec3Codec
    {
        void Load(const TOMLReader& toml, const String& key, Vec3& value) const
        {
            value = SettingsDetail::ReadTomlVec3(toml, key, value);
        }

        void Save(TextWriter& writer, const String& key, const Vec3& value) const
        {
            writer.writeln(U"{} = [{:.3f}, {:.3f}, {:.3f}]"_fmt(key, value.x, value.y, value.z));
        }
    };

    struct ColorFCodec
    {
        void Load(const TOMLReader& toml, const String& key, ColorF& value) const
        {
            value = SettingsDetail::ReadTomlColorF(toml, key, value);
        }

        void Save(TextWriter& writer, const String& key, const ColorF& value) const
        {
            SettingsDetail::WriteTomlColorF(writer, key, value);
        }
    };

    struct PointCodec
    {
        void Load(const TOMLReader& toml, const String& key, Point& value) const
        {
            value = SettingsDetail::ReadTomlPoint(toml, key, value);
        }

        void Save(TextWriter& writer, const String& key, const Point& value) const
        {
            writer.writeln(U"{} = [{}, {}]"_fmt(key, value.x, value.y));
        }
    };

    template <class E>
    struct EnumCodec
    {
        E (*parse)(StringView) = nullptr;
        StringView (*format)(E) = nullptr;

        void Load(const TOMLReader& toml, const String& key, E& value) const
        {
            if (const auto s = toml[key].getOpt<String>())
            {
                value = parse(*s);
            }
        }

        void Save(TextWriter& writer, const String& key, const E value) const
        {
            writer.writeln(U"{} = \"{}\""_fmt(key, format(value)));
        }
    };

    // --- Visitors -------------------------------------------------------
    // Schema functions (per-struct) accept any visitor whose operator()
    // matches: void(StringView suffix, FieldRef, const Codec&).
    // LoadVisitor mutates the field; SaveVisitor reads it.

    struct LoadVisitor
    {
        const TOMLReader& toml;
        StringView prefix;

        template <class V, class Codec>
        void operator()(const StringView suffix, V& field, const Codec& codec) const
        {
            codec.Load(toml, MakeKey(prefix, suffix), field);
        }
    };

    struct SaveVisitor
    {
        TextWriter& writer;
        StringView prefix;

        template <class V, class Codec>
        void operator()(const StringView suffix, const V& field, const Codec& codec) const
        {
            codec.Save(writer, MakeKey(prefix, suffix), field);
        }
    };
}
