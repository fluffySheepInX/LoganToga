# pragma once
# include "UnitTypes.h"
# include "WaveDefinitionRepository.h"

namespace ff
{
	class DefinitionRepository
	{
	public:
		DefinitionRepository();

		// ユニット定義を取得します。
		[[nodiscard]] const std::array<UnitDefinition, AllyBehaviorCount>& GetUnitDefinitions() const;

		// ユニット定義を更新します。
		void SetUnitDefinition(UnitDefinition definition);

		// ディスクからユニット定義を再読込します。
		void ReloadUnitDefinitions();

		// 敵定義を取得します。
		[[nodiscard]] const std::array<EnemyDefinition, EnemyKindCount>& GetEnemyDefinitions() const;

		// 敵定義を更新します。
		void SetEnemyDefinition(EnemyDefinition definition);

		// ディスクから敵定義を再読込します。
		void ReloadEnemyDefinitions();

		// ウェーブ定義を取得します。
		[[nodiscard]] const WaveConfig& GetWaveConfig() const;

		// ウェーブ定義を更新します。
		void SetWaveConfig(WaveConfig config);

		// ディスクからウェーブ定義を再読込します。
		void ReloadWaveDefinitions();

	private:
		std::array<UnitDefinition, AllyBehaviorCount> m_unitDefinitions;
		std::array<EnemyDefinition, EnemyKindCount> m_enemyDefinitions;
		WaveConfig m_waveConfig;
	};

	// アプリケーションで使用する定義Repositoryを登録します。
	void RegisterDefinitionRepository(DefinitionRepository& repository);

	// 登録済みの定義Repositoryを取得します。
	[[nodiscard]] DefinitionRepository& GetDefinitionRepository();

	// アプリケーションで使用する定義Repositoryの登録を解除します。
	void UnregisterDefinitionRepository(DefinitionRepository& repository);
}
