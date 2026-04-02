# pragma once
# include <Siv3D.hpp>

namespace ff
{
	struct ResourceBalanceConfig
	{
		double passiveResourcePerSecond = 0.15;
	};

	[[nodiscard]] inline String GetBundledResourceBalancePath()
	{
		const String runtimeRelativePath = U"resourceBalance.toml";
		if (FileSystem::Exists(runtimeRelativePath))
		{
			return runtimeRelativePath;
		}

		const String projectRelativePath = U"App/resourceBalance.toml";
		if (FileSystem::Exists(projectRelativePath))
		{
			return projectRelativePath;
		}

		return runtimeRelativePath;
	}

	[[nodiscard]] inline String GetUserResourceBalancePath()
	{
		return U"save/resourceBalance.toml";
	}

	[[nodiscard]] inline String GetResourceBalancePath()
	{
		const String userPath = GetUserResourceBalancePath();
		if (FileSystem::Exists(userPath))
		{
			return userPath;
		}

		return GetBundledResourceBalancePath();
	}

	[[nodiscard]] inline ResourceBalanceConfig LoadResourceBalanceConfig()
	{
		ResourceBalanceConfig config;
		const TOMLReader toml{ GetResourceBalancePath() };

		if (!toml)
		{
			return config;
		}

		try
		{
			config.passiveResourcePerSecond = Max(0.0, toml[U"passive_resource_per_second"].get<double>());
		}
		catch (const std::exception&)
		{
		}

		return config;
	}

	[[nodiscard]] inline const ResourceBalanceConfig& GetResourceBalanceConfig()
	{
		static ResourceBalanceConfig config = LoadResourceBalanceConfig();
		return config;
	}

	[[nodiscard]] inline double GetPassiveResourcePerSecond()
	{
		return GetResourceBalanceConfig().passiveResourcePerSecond;
	}
}
