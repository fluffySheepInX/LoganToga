#pragma once
# include "ClassSkill.h"
# include "EnumSkill.h"
# include "Common.h"
# include "GameUIToolkit.h"
# include "ClassBuildAction.h"

class Unit;

// コンポーネントの基底クラス
class UnitComponent {
public:
	virtual ~UnitComponent() = default;
	virtual void update(Unit& owner) {}
	virtual std::unique_ptr<UnitComponent> clone() const = 0; // 追加
};
class ClassTempStatus
{
public:
	int32 Medical = 0;
	int32 Hp = 0;
	int32 HpMAX = 0;
	int32 Mp = 0;
	int32 Attack = 0;
	int32 Defense = 0;
	int32 Magic = 0;
	int32 MagDef = 0;
	double Speed = 0.0;
	double plus_speed_time = 0.0;
	int32 Dext = 0;
	int32 Move = 0;
	int32 Hprec = 0;
	int32 Mprec = 0;
	int32 Exp = 0;
	int32 Exp_mul = 0;
	int32 Heal_max = 0;
	int32 Summon_max = 0;
	int32 No_knock = 0;
	int32 Loyal = 0;
	int32 Alive_per = 0;
	int32 Escape_range = 0;
};
enum class moveState
{
	/// @brief 何もしない状態
	None,
	/// @brief 移動計算中
	FlagMoveCalc,
	/// @brief AIによる移動計算中
	MoveAI,
	/// @brief AIによる移動中
	Moving,
	/// @brief 移動終
	MovingEnd,
};

class Unit
{
private:
	Vec2 firstMergePos_ = Vec2::Zero();
	Vec2 lastMergePos_ = Vec2::Zero();
public:
	Unit() = default;
	// コピーコンストラクタ
	Unit(const Unit& other) {
		components.clear();
		for (const auto& c : other.components) {
			components.push_back(c->clone());
		}
		cts = other.cts;
		Formation = other.Formation;
		currentTask = other.currentTask;
		// taskTimer はコピーできないため再初期化（スタートしない状態）
		taskTimer.reset();

		rowResourceTarget = other.rowResourceTarget;
		colResourceTarget = other.colResourceTarget;
		ID = other.ID;

		pressedUnit = other.pressedUnit;

		classBuild = other.classBuild;
		tempSelectComRight = other.tempSelectComRight;

		visionRadius = other.visionRadius;
		initTilePos = other.initTilePos;
		waitTimeResource = other.waitTimeResource;

		IsBuilding = other.IsBuilding;
		IsBuildingEnable = other.IsBuildingEnable;
		rowBuilding = other.rowBuilding;
		colBuilding = other.colBuilding;
		mapTipObjectType = other.mapTipObjectType;
		NoWall2 = other.NoWall2;
		HPCastle = other.HPCastle;
		CastleDefense = other.CastleDefense;
		CastleMagdef = other.CastleMagdef;

		houkou = other.houkou;
		initXY = other.initXY;

		bLiquidBarBattle = other.bLiquidBarBattle;

		IsLeader = other.IsLeader;
		IsSelect = other.IsSelect;
		IsDone = other.IsDone;
		IsBattleEnable = other.IsBattleEnable;

		NameTag = other.NameTag;
		Name = other.Name;
		Help = other.Help;
		Text = other.Text;
		Race = other.Race;
		Class = other.Class;
		ImageName = other.ImageName;
		Dead = other.Dead;
		Retreat = other.Retreat;
		Join = other.Join;
		Face = other.Face;
		Voice_type = other.Voice_type;
		gender = other.gender;
		Talent = other.Talent;
		Friend = other.Friend;
		Merce = other.Merce;
		Staff = other.Staff;
		InitMember = other.InitMember;
		Enemy = other.Enemy;

		Level = other.Level;
		Level_max = other.Level_max;
		Price = other.Price;
		Cost = other.Cost;
		Medical = other.Medical;
		HasExp = other.HasExp;
		Hp = other.Hp;
		HpMAX = other.HpMAX;
		Mp = other.Mp;
		Attack = other.Attack;
		Defense = other.Defense;
		Magic = other.Magic;
		MagDef = other.MagDef;
		Speed = other.Speed;
		Dext = other.Dext;
		Move = other.Move;
		Hprec = other.Hprec;
		Mprec = other.Mprec;
		Exp = other.Exp;
		Exp_mul = other.Exp_mul;
		Heal_max = other.Heal_max;
		Summon_max = other.Summon_max;
		No_knock = other.No_knock;
		Loyal = other.Loyal;
		Alive_per = other.Alive_per;
		Escape_range = other.Escape_range;

		SkillName = other.SkillName;
		arrSkill = other.arrSkill;
		Finance = other.Finance;
		MoveType = other.MoveType;
		moveState = other.moveState;
		//FlagMoveCalc = other.FlagMoveCalc;
		//FlagMoveAI = other.FlagMoveAI;
		//FlagMoving = other.FlagMoving;
		//FlagMovingEnd = other.FlagMovingEnd;

		yokoUnit = other.yokoUnit;
		TakasaUnit = other.TakasaUnit;
		FlagReachedDestination = other.FlagReachedDestination;
		orderPosiLeftFlagReachedDestination = other.orderPosiLeftFlagReachedDestination;

		nowPosiLeft = other.nowPosiLeft;
		orderPosiLeft = other.orderPosiLeft;
		orderPosiLeftLast = other.orderPosiLeftLast;
		vecMove = other.vecMove;

		FlagMoveSkill = other.FlagMoveSkill;
		FlagMovingSkill = other.FlagMovingSkill;
	}
	Unit& operator=(const Unit& other)
	{
		if (this != &other)
		{
			components.clear();
			for (const auto& c : other.components) {
				components.push_back(c->clone());
			}
			cts = other.cts;
			Formation = other.Formation;
			currentTask = other.currentTask;
			// taskTimer はコピーできないため再初期化（スタートしない状態）
			taskTimer.reset();

			rowResourceTarget = other.rowResourceTarget;
			colResourceTarget = other.colResourceTarget;
			ID = other.ID;

			pressedUnit = other.pressedUnit;

			classBuild = other.classBuild;
			tempSelectComRight = other.tempSelectComRight;

			visionRadius = other.visionRadius;
			initTilePos = other.initTilePos;
			waitTimeResource = other.waitTimeResource;

			IsBuilding = other.IsBuilding;
			IsBuildingEnable = other.IsBuildingEnable;
			rowBuilding = other.rowBuilding;
			colBuilding = other.colBuilding;
			mapTipObjectType = other.mapTipObjectType;
			NoWall2 = other.NoWall2;
			HPCastle = other.HPCastle;
			CastleDefense = other.CastleDefense;
			CastleMagdef = other.CastleMagdef;

			houkou = other.houkou;
			initXY = other.initXY;

			bLiquidBarBattle = other.bLiquidBarBattle;

			IsLeader = other.IsLeader;
			IsSelect = other.IsSelect;
			IsDone = other.IsDone;
			IsBattleEnable = other.IsBattleEnable;

			NameTag = other.NameTag;
			Name = other.Name;
			Help = other.Help;
			Text = other.Text;
			Race = other.Race;
			Class = other.Class;
			ImageName = other.ImageName;
			Dead = other.Dead;
			Retreat = other.Retreat;
			Join = other.Join;
			Face = other.Face;
			Voice_type = other.Voice_type;
			gender = other.gender;
			Talent = other.Talent;
			Friend = other.Friend;
			Merce = other.Merce;
			Staff = other.Staff;
			InitMember = other.InitMember;
			Enemy = other.Enemy;

			Level = other.Level;
			Level_max = other.Level_max;
			Price = other.Price;
			Cost = other.Cost;
			Medical = other.Medical;
			HasExp = other.HasExp;
			Hp = other.Hp;
			HpMAX = other.HpMAX;
			Mp = other.Mp;
			Attack = other.Attack;
			Defense = other.Defense;
			Magic = other.Magic;
			MagDef = other.MagDef;
			Speed = other.Speed;
			Dext = other.Dext;
			Move = other.Move;
			Hprec = other.Hprec;
			Mprec = other.Mprec;
			Exp = other.Exp;
			Exp_mul = other.Exp_mul;
			Heal_max = other.Heal_max;
			Summon_max = other.Summon_max;
			No_knock = other.No_knock;
			Loyal = other.Loyal;
			Alive_per = other.Alive_per;
			Escape_range = other.Escape_range;

			SkillName = other.SkillName;
			arrSkill = other.arrSkill;
			Finance = other.Finance;
			MoveType = other.MoveType;
			moveState = other.moveState;
			//FlagMoveCalc = other.FlagMoveCalc;
			//FlagMoveAI = other.FlagMoveAI;
			//FlagMoving = other.FlagMoving;
			//FlagMovingEnd = other.FlagMovingEnd;

			yokoUnit = other.yokoUnit;
			TakasaUnit = other.TakasaUnit;
			FlagReachedDestination = other.FlagReachedDestination;
			orderPosiLeftFlagReachedDestination = other.orderPosiLeftFlagReachedDestination;

			nowPosiLeft = other.nowPosiLeft;
			orderPosiLeft = other.orderPosiLeft;
			orderPosiLeftLast = other.orderPosiLeftLast;
			vecMove = other.vecMove;

			FlagMoveSkill = other.FlagMoveSkill;
			FlagMovingSkill = other.FlagMovingSkill;
		}
		return *this;
	}
	Array<std::unique_ptr<UnitComponent>> components;

	template<typename T>
	T* getComponent() {
		for (auto& c : components) {
			if (auto ptr = dynamic_cast<T*>(c.get())) return ptr;
		}
		return nullptr;
	}

	void update() {
		for (auto& c : components) c->update(*this);
	}

	ClassTempStatus cts = ClassTempStatus();

	BattleFormation Formation = BattleFormation::F;
	UnitTask currentTask = UnitTask::None;
	/// @brief buildなど
	Stopwatch taskTimer;
	/// @brief ゲージを出す為
	double progressTime = -1.0;
	Array<BuildAction> arrYoyakuBuild;
	BuildAction tempIsBuildSelectTragetBuildAction;
	/// @brief 確保する場所
	int32 rowResourceTarget = -1;
	int32 colResourceTarget = -1;

	long long ID = 0;

	/// @brief 建築メニューなどを表示するかどうかのフラグ
	bool pressedUnit = false;
	String classBuild;
	BuildAction tempSelectComRight;
	RectF GetRectNowPosi() const
	{
		return RectF(nowPosiLeft, yokoUnit, TakasaUnit);
	}

	int32 visionRadius = 3;
	Point initTilePos = Point(0, 0); // ユニットのタイル上の位置

	int32 waitTimeResource = 0; // 資源ポイントを獲得するまでの待ち時間

	// IsLeader
	bool IsBuilding = false;
	bool isValidBuilding() const {
		return IsBuilding;
	}
	bool IsBuildingEnable = false;
	/// @brief 建っている場所
	int32 rowBuilding = 0;
	int32 colBuilding = 0;
	MapTipObjectType mapTipObjectType = MapTipObjectType::None;
	/// @brief wall2で攻撃側飛び道具がすり抜ける確率
	int32 NoWall2 = 0;
	int32 HPCastle = 0;
	int32 CastleDefense = 0;
	int32 CastleMagdef = 0;

	String houkou = U"";
	Point initXY = Point();

	GameUIToolkit::LiquidBarBattle bLiquidBarBattle;

	// IsLeader
	bool IsLeader = false;

	// IsSelect
	bool IsSelect = false;

	// IsDone
	bool IsDone = false;

	// IsBattleEnable
	bool IsBattleEnable = true;

	// NameTag
	String NameTag;

	// Name
	String Name;

	// Help
	String Help;

	// Text
	String Text;

	// Race
	String Race;

	// Class
	String Class;

	// Image
	String ImageName;

	// Dead
	String Dead;

	// Retreat
	String Retreat;

	// Join
	String Join;

	// Face
	String Face;

	// Voice_type
	String Voice_type;

	// Gender
	Gender gender = Gender::Neuter;

	// Talent
	String Talent;

	// Friend
	String Friend;

	// Merce
	String Merce;

	// Staff
	String Staff;

	// InitMember
	String InitMember;

	// Enemy
	String Enemy;

	// Level
	int32 Level = 0;

	// Level_max
	int32 Level_max = 0;

	// Price
	int32 Price = 0;

	// Cost
	int32 Cost = 0;

	// Medical
	int32 Medical = 0;

	// HasExp
	int32 HasExp = 0;

	// Hp
	int32 Hp = 0;
	// HpMAX
	int32 HpMAX = 0;

	// Mp
	int32 Mp = 0;

	// Attack
	int32 Attack = 0;

	// Defense
	int32 Defense = 0;

	// Magic
	int32 Magic = 0;

	//MagDef
	int32 MagDef = 0;

	// Speed
	double Speed = 0.0;

	// Dext
	int32 Dext = 0;

	// Move
	double Move = 0;

	// Hprec
	int32 Hprec = 0;

	// Mprec
	int32 Mprec = 0;

	// Exp
	int32 Exp = 0;

	// Exp_mul
	int32 Exp_mul = 0;

	// Heal_max
	int32 Heal_max = 0;

	// Summon_max
	int32 Summon_max = 0;

	// No_knock
	int32 No_knock = 0;

	// Loyal
	int32 Loyal = 0;

	// Alive_per
	int32 Alive_per = 0;

	// Escape_range
	int32 Escape_range = 0;

	Array<String> SkillName;

	Array<Skill> arrSkill;

	int32 Finance = 0;

	/// @brief 恐らく地形で移動する際のタイプを示す
	String MoveType;

	moveState moveState = moveState::None;

	//// これを今から計算対象としますよ、というフラグ
	//bool FlagMoveCalc = false;

	//// 経路探索の対象としますよ、というフラグ
	//bool FlagMoveAI = false;

	//// FlagMoving
	////実際に座標を動かす処理に突入する為のフラグ
	//bool FlagMoving = false;

	//// FlagMoving
	///// @brief 移動が終わったかどうかのフラグ(恐らく
	//bool FlagMovingEnd = true;

	int32 yokoUnit = 32;
	int32 TakasaUnit = 32;
	bool FlagReachedDestination = false;
	Vec2 orderPosiLeftFlagReachedDestination = Vec2{ 0, 0 };

	// NowPosiLeft
	Vec2 nowPosiLeft;

	/// @brief 
	/// @return 
	Vec2 GetNowPosiCenter() const
	{
		return Vec2(nowPosiLeft.x + (yokoUnit / 2), nowPosiLeft.y + (TakasaUnit / 2));
	}
	// OrderPosiLeft
	Vec2 orderPosiLeft;
	Vec2 orderPosiLeftLast;

	// OrderPosiCenter
	Vec2 GetOrderPosiCenter() const
	{
		return Vec2(orderPosiLeft.x + (yokoUnit / 2), orderPosiLeft.y + (TakasaUnit / 2));
	}

	// VecMove
	Vec2 vecMove;

	// FlagMoveSkill
	bool FlagMoveSkill = false;

	// FlagMovingSkill
	bool FlagMovingSkill = false;

	// ファースト合流地点
	void setFirstMergePos(const Vec2& pos) { firstMergePos_ = pos; }
	Vec2 getFirstMergePos() const { return firstMergePos_; }

	// ラスト合流地点
	void setLastMergePos(const Vec2& pos) { lastMergePos_ = pos; }
	Vec2 getLastMergePos() const { return lastMergePos_; }
};

// キャリアー機能
class CarrierComponent : public UnitComponent {
public:
	int32 capacity = 4;
	std::vector<Unit*> storedUnits;

	/// @brief unitをマップ・AI・描画から除外
	/// @param unit 
	bool store(Unit* unit)
	{
		if (storedUnits.size() >= capacity) return false;
		storedUnits.push_back(std::move(unit));
		unit->IsBattleEnable = false; // 戦闘不可にする

		return true;
	}
	void releaseAll(const Vec2& pos)
	{
		for (auto& unit : storedUnits) {
			unit->IsBattleEnable = true; // 戦闘可能に戻す
			unit->nowPosiLeft = unit->nowPosiLeft.movedBy(Random(-10,10), Random(-10, 10));
		}
		storedUnits.clear();
	}
	std::unique_ptr<UnitComponent> clone() const override {
		// CarrierComponentの内容をコピー
		auto ptr = std::make_unique<CarrierComponent>(*this);
		// storedUnitsはポインタなので、必要に応じてコピー方法を工夫
		ptr->storedUnits.clear(); // 通常は空にする
		return ptr;
	}
};
