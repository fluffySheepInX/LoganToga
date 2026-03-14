#include "ContinueRunSave.h"

String EscapeTomlString(String value)
{
	value.replace(U"\\", U"\\\\");
	value.replace(U"\"", U"\\\"");
	value.replace(U"\n", U"\\n");
	return value;
}

String QuoteTomlString(const String& value)
{
	return U"\"" + EscapeTomlString(value) + U"\"";
}

String BuildTomlStringArray(const Array<String>& values)
{
	String result = U"[";
	for (size_t index = 0; index < values.size(); ++index)
	{
		if (index > 0)
		{
			result += U", ";
		}

		result += QuoteTomlString(values[index]);
	}

	result += U"]";
	return result;
}

String BuildTomlIntArray(const Array<int32>& values)
{
	String result = U"[";
	for (size_t index = 0; index < values.size(); ++index)
	{
		if (index > 0)
		{
			result += U", ";
		}

		result += Format(values[index]);
	}

	result += U"]";
	return result;
}

void AppendTomlLine(String& content, const String& key, const String& value)
{
	content += key + U" = " + value + U"\n";
}

void AppendTomlStringArrayLine(String& content, const String& key, const Array<String>& values)
{
	AppendTomlLine(content, key, BuildTomlStringArray(values));
}

void AppendTomlIntArrayLine(String& content, const String& key, const Array<int32>& values)
{
	AppendTomlLine(content, key, BuildTomlIntArray(values));
}

Array<String> ReadTomlStringArray(const TOMLReader& toml, const String& key)
{
	Array<String> values;
	for (const auto& value : toml[key].arrayView())
	{
		values << value.get<String>();
	}

	return values;
}

Array<int32> ReadTomlIntArray(const TOMLReader& toml, const String& key)
{
	Array<int32> values;
	for (const auto& value : toml[key].arrayView())
	{
		values << value.get<int32>();
	}

	return values;
}
