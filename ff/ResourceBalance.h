# pragma once
# include "ConfigFilePaths.h"

namespace ff
{
	struct ResourceBalanceConfig
	{
		double passiveResourcePerSecond = 0.15;
	};

	// 同梱リソース設定の利用可能なパスを取得します。
	[[nodiscard]] inline String GetBundledResourceBalancePath()
	{
		return ResolveBundledConfigPath({ U"resourceBalance.toml", U"App/resourceBalance.toml", U"save/resourceBalance.toml" });
	}

	// ユーザー編集用リソース設定の保存先を取得します。
	[[nodiscard]] inline String GetUserResourceBalancePath()
	{
		return U"save/resourceBalance.toml";
	}

	// ユーザー編集を優先したリソース設定の読込先を取得します。
	[[nodiscard]] inline String GetResourceBalancePath()
	{
		return ResolveConfigPath({ U"resourceBalance.toml", U"App/resourceBalance.toml", GetUserResourceBalancePath() });
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
