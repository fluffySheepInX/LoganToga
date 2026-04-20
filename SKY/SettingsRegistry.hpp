# pragma once
# include "TomlSchema.hpp"

namespace MainSupport
{
    // Generic descriptor for a settings TOML file. Each settings type
    // declares one of these (typically as a constexpr-style helper) and
    // the LoadSetting / SaveSetting helpers handle the boilerplate of
    // opening readers / writers and creating directories.
    template <class T>
    struct SettingDescriptor
    {
        StringView name;
        FilePathView path;

        // Load: invoked only when the TOML file exists & opens. It
        // mutates an instance that has already been default-constructed
        // (or initialized by makeDefault, if provided).
        void (*loadFn)(const TOMLReader&, T&) = nullptr;

        // Save: invoked after CreateDirectories + writer open success.
        void (*saveFn)(TextWriter&, const T&) = nullptr;

        // Optional: produce a default-initialized value. When nullptr,
        // T{} is used.
        T (*makeDefault)() = nullptr;
    };

    // Open the writer for `path`, creating parent directories first.
    // Returns false when the writer cannot be opened.
    template <class Fn>
    bool SaveSettingsFile(const FilePathView path, Fn&& body)
    {
        const String directory = FileSystem::ParentPath(FilePath{ path });
        if (not directory.isEmpty())
        {
            FileSystem::CreateDirectories(directory);
        }

        TextWriter writer{ path };
        if (not writer)
        {
            return false;
        }

        body(writer);
        return true;
    }

    template <class T>
    T LoadSetting(const SettingDescriptor<T>& descriptor)
    {
        T value = (descriptor.makeDefault ? descriptor.makeDefault() : T{});

        const TOMLReader toml{ descriptor.path };
        if (toml && descriptor.loadFn)
        {
            descriptor.loadFn(toml, value);
        }

        return value;
    }

    template <class T>
    bool SaveSetting(const SettingDescriptor<T>& descriptor, const T& value)
    {
        return SaveSettingsFile(descriptor.path, [&](TextWriter& writer)
        {
            if (descriptor.saveFn)
            {
                descriptor.saveFn(writer, value);
            }
        });
    }
}
