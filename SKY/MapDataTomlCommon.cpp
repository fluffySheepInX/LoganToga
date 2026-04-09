# include "MapDataTomlInternal.hpp"
# include <fstream>

namespace MapDataTomlDetail
{
	double SanitizeMillAttackRange(const double value)
	{
		return Clamp(value, 1.0, 20.0);
	}

	double SanitizeMillAttackDamage(const double value)
	{
		return Clamp(value, 1.0, 80.0);
	}

	double SanitizeMillAttackInterval(const double value)
	{
		return Clamp(value, 0.2, 5.0);
	}

	int32 SanitizeMillAttackTargetCount(const int64 value)
	{
		return Clamp<int32>(static_cast<int32>(value), 1, 6);
	}

	double SanitizeMillSuppressionDuration(const double value)
	{
		return Clamp(value, 0.2, 10.0);
	}

	double SanitizeMillSuppressionMoveSpeedMultiplier(const double value)
	{
		return Clamp(value, 0.1, 1.0);
	}

	double SanitizeMillSuppressionAttackDamageMultiplier(const double value)
	{
		return Clamp(value, 0.1, 1.0);
	}

	double SanitizeMillSuppressionAttackIntervalMultiplier(const double value)
	{
		return Clamp(value, 1.0, 10.0);
	}

	double SanitizeNavPointRadius(const double value)
	{
		return Clamp(value, 0.5, 8.0);
	}

	double SanitizeNavLinkCostMultiplier(const double value)
	{
		return Clamp(value, 0.1, 10.0);
	}

	double SanitizeWallLength(const double value)
	{
		return Clamp(value, 2.0, 80.0);
	}

	Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 3)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	Vec3 ReadTomlVec3(const TOMLValue& tomlValue, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : tomlValue[key].arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 3)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	void WriteTomlVec3(TextWriter& writer, const String& key, const Vec3& value)
	{
		writer.writeln(U"{} = [{:.3f}, {:.3f}, {:.3f}]"_fmt(key, value.x, value.y, value.z));
	}

	void AppendLoadMessage(String& message, const StringView detail)
	{
		if (message.isEmpty())
		{
			message = detail;
			return;
		}

		message += U" / ";
		message += detail;
	}

	bool HasTomlTableArraySection(FilePathView path, const String& key)
	{
		std::ifstream reader{ Unicode::ToUTF8(FileSystem::FullPath(path)) };

		if (not reader)
		{
			return false;
		}

		const std::string sectionHeader = ("[[" + Unicode::ToUTF8(key) + "]]" );
		std::string line;

		while (std::getline(reader, line))
		{
			if (not line.empty() && static_cast<unsigned char>(line.front()) == 0xEF)
			{
				if ((3 <= line.size())
					&& (static_cast<unsigned char>(line[0]) == 0xEF)
					&& (static_cast<unsigned char>(line[1]) == 0xBB)
					&& (static_cast<unsigned char>(line[2]) == 0xBF))
				{
					line.erase(0, 3);
				}
			}

			const size_t start = line.find_first_not_of(" \t");
			if (start == std::string::npos)
			{
				continue;
			}

			const size_t end = line.find_last_not_of(" \t");
			if (line.substr(start, (end - start + 1)) == sectionHeader)
			{
				return true;
			}
		}

		return false;
	}

	Optional<PlaceableModelType> ParsePlaceableModelType(const String& value)
	{
		if (value == U"Mill")
		{
			return PlaceableModelType::Mill;
		}

		if (value == U"Tree")
		{
			return PlaceableModelType::Tree;
		}

		if (value == U"Pine")
		{
			return PlaceableModelType::Pine;
		}

		if (value == U"Rock")
		{
			return PlaceableModelType::Rock;
		}

		if (value == U"Wall")
		{
			return PlaceableModelType::Wall;
		}

		return none;
	}

	Optional<MainSupport::ResourceType> ParseResourceType(const String& value)
	{
		if (value == U"Budget")
		{
			return MainSupport::ResourceType::Budget;
		}

		if (value == U"Gunpowder")
		{
			return MainSupport::ResourceType::Gunpowder;
		}

		if (value == U"Mana")
		{
			return MainSupport::ResourceType::Mana;
		}

		return none;
	}
}
