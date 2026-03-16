namespace
{
	constexpr int32 EnemyAiSearchGroupCountLight = 2;
	constexpr int32 EnemyAiSearchGroupCountHeavy = 4;
	constexpr size_t EnemyAiAsyncMinUnitCount = 8;
	constexpr size_t EnemyAiAsyncMinBuildingCount = 4;

	enum class EnemyAiObjectiveType
	{
		Rally,
		CaptureResource,
		Defend,
		Assault
	};

	struct EnemyAiDecision
	{
		EnemyAiObjectiveType objective = EnemyAiObjectiveType::Rally;
		Vec2 strategicDestination = Vec2::Zero();
		Optional<int32> targetUnitId;
	};

	struct EnemyAiStrategicTarget
	{
		double score = -Math::Inf;
		Vec2 position = Vec2::Zero();
		Optional<int32> unitId;
	};

	struct EnemyAiContext
	{
		Vec2 enemyAnchor = Vec2::Zero();
		bool canAssaultPlayerBase = false;
		Optional<int32> defenseTargetUnitId;
		Vec2 defenseTargetPosition = Vec2::Zero();
		Optional<size_t> captureTargetIndex;
		Vec2 captureTargetPosition = Vec2::Zero();
		int32 enemyCombatUnits = 0;
		int32 searchGroupCount = EnemyAiSearchGroupCountLight;
		int32 currentSearchPhase = 0;
		EnemyAiStrategicTarget strategicTarget;
		bool useStagingAssault = false;
		bool hasStrategicTarget = false;
		bool canStageAssault = false;
		Vec2 stagingRallyPoint = Vec2::Zero();
		int32 stagingMinUnits = 1;
		int32 stagingReadyUnits = 0;
		bool shouldAssault = false;
		Vec2 rallyPoint = Vec2::Zero();
		Vec2 assaultDestination = Vec2::Zero();
		Optional<int32> assaultTargetUnitId;
	};

	struct EnemyAiPendingUpdate
	{
		bool shouldApply = false;
		EnemyAiDecision decision;
	};

	struct EnemyAiDefenseResponderCandidate
	{
		size_t unitIndex = 0;
		double distanceSq = Math::Inf;
	};
}
