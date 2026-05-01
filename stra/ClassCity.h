#pragma once

using CityId = int32;
using TileIndex = int32;
using PlayerId = int32;
using UnitTypeId = int32;
using FacilityTypeId = int32;

static constexpr CityId InvalidCityId = -1;
static constexpr TileIndex InvalidTileIndex = -1;
static constexpr PlayerId InvalidPlayerId = -1;

struct UnitStack
{
	UnitTypeId type = -1;
	int32 count = 0;

	// あると嬉しい（戦術時の強さに反映）
	int32 exp = 0;      // 経験
	int32 morale = 100; // 士気
};

struct GarrisonPool
{
	Array<UnitStack> stacks;

	int32 totalCount() const
	{
		int32 sum = 0;
		for (const auto& s : stacks) sum += s.count;
		return sum;
	}

	// 要求兵力を引き出す（戦術出撃や召喚）
	bool take(UnitTypeId type, int32 n)
	{
		for (auto& s : stacks)
		{
			if (s.type == type && s.count >= n)
			{
				s.count -= n;
				return true;
			}
		}
		return false;
	}
};

struct FacilityState
{
	FacilityTypeId type = -1;
	int32 level = 1;

	// 呼び出し施設ならこれを持つ（戦術で使う）
	int32 cooldown = 0;
	int32 charges = -1; // -1 無限、0で不可

	// 施設固有の状態が増えてもここに寄せる
};

struct BuildOrder
{
	enum class Kind : uint8 { Unit, Facility, Upgrade };

	Kind kind = Kind::Unit;
	int32 targetId = -1;       // UnitTypeId or FacilityTypeId or UpgradeId
	int32 remainingCost = 0;   // 生産ポイント残り
};

struct BuildQueue
{
	Array<BuildOrder> queue;

	bool empty() const { return queue.isEmpty(); }
	void push(const BuildOrder& o) { queue.push_back(o); }
};

class City
{
public:
	CityId id = InvalidCityId;
	String name = U"";
	TileIndex centerTile = InvalidTileIndex;

	PlayerId owner = InvalidPlayerId;

	// 発展
	int32 level = 1;
	int32 population = 1000;

	// 耐久（包囲・略奪があるなら）
	int32 hp = 100;
	int32 fortLevel = 0;

	// 経済（ターン更新）
	int32 gold = 0;
	int32 prod = 0;

	int32 food = 0;
	int32 ammo = 0;
	int32 fuel = 0;

	int32 unrest = 0; // 反乱値

	// 都市機能
	Array<FacilityState> facilities;

	// 生産
	BuildQueue buildQueue;

	// 守備隊（＝戦術で呼び出す兵の元）
	GarrisonPool garrison;

	// 都市の支配/影響（必要なら）
	int32 influenceRadius = 2;

public:
	// ターン進行（戦略）
	void onTurnTick()
	{
		// 1) クールダウン等
		for (auto& f : facilities)
		{
			if (f.cooldown > 0) f.cooldown--;
		}

		// 2) 生産処理（BuildQueue消化）
		processBuildQueue();

		// 3) 経済
		// gold += ...
		// prod += ...
		// unrest += ...
	}

private:
	void processBuildQueue()
	{
		if (buildQueue.empty())
			return;

		// 例：prod を使って先頭を進める
		auto& top = buildQueue.queue.front();
		const int32 spend = Min(prod, top.remainingCost);
		top.remainingCost -= spend;
		prod -= spend;

		if (top.remainingCost <= 0)
		{
			// 完成物を反映
			applyBuildComplete(top);
			buildQueue.queue.erase(buildQueue.queue.begin());
		}
	}

	void applyBuildComplete(const BuildOrder& o)
	{
		switch (o.kind)
		{
		case BuildOrder::Kind::Unit:
			// 完成した兵は「守備隊に入る」か「軍団に送る」か設計で決める
			garrison.stacks.push_back(UnitStack{ o.targetId, 1, 0, 100 });
			break;

		case BuildOrder::Kind::Facility:
			facilities.push_back(FacilityState{ o.targetId, 1, 0, -1 });
			break;

		case BuildOrder::Kind::Upgrade:
			level++;
			break;
		}
	}
};
