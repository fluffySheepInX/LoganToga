#pragma once
# include <Siv3D.hpp> // Siv3D v0.6.15
#include "libs/CoTaskLib.hpp"
#include "libs/Gaussian.hpp"
#include "Common.h"
#include "Game.hpp"
#include "Battle.hpp"
#include "ClassUnit.h"
#include "ClassCommonConfig.h"

void Init(CommonConfig& commonConfig)
{
	// Unit.jsonからデータを読み込む
	{
		const JSON jsonUnit = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoUnit/Unit.json");

		if (not jsonUnit) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `Unit.json`" };

		Array<Unit> arrayClassUnit;
		for (const auto& [key, value] : jsonUnit[U"Unit"]) {
			Unit cu;
			cu.NameTag = value[U"name_tag"].getString();
			cu.Name = value[U"name"].getString();
			cu.Image = value[U"image"].getString();
			cu.Hp = Parse<int32>(value[U"hp"].getString());
			cu.Mp = Parse<int32>(value[U"mp"].getString());
			cu.HpMAX = cu.Hp;
			if (value.hasElement(U"visionRadius") == true)
				cu.visionRadius = Parse<int32>(value[U"visionRadius"].getString());
			cu.Attack = Parse<int32>(value[U"attack"].getString());
			cu.Defense = Parse<int32>(value[U"defense"].getString());
			cu.Magic = Parse<int32>(value[U"magic"].getString());
			cu.MagDef = Parse<int32>(value[U"magDef"].getString());
			cu.Speed = Parse<double>(value[U"speed"].getString());
			cu.Price = Parse<int32>(value[U"price"].getString());
			cu.Move = Parse<double>(value[U"move"].getString());
			cu.Escape_range = Parse<int32>(value[U"escape_range"].getString());
			if (value.hasElement(U"help") == true)
			{
				cu.Help = (value[U"help"].getString());
			}
			if (value.hasElement(U"bf") == true)
			{
				int32 temp = Parse<int32>(value[U"bf"].getString());

				if (temp == 0)
				{
					cu.Formation = BattleFormation::F;
				}
				else if (temp == 1)
				{
					cu.Formation = BattleFormation::B;
				}
				else if (temp == 2)
				{
					cu.Formation = BattleFormation::M;
				}
				else
				{
					cu.Formation = BattleFormation::F;
				}
			}
			else
			{
				cu.Formation = BattleFormation::F;
			}
			cu.Race = (value[U"race"].getString());
			String sNa = value[U"skill"].getString();
			if (sNa.contains(',') == true)
			{
				cu.SkillName = sNa.split(',');
			}
			else
			{
				cu.SkillName.push_back(sNa);
			}
			arrayClassUnit.push_back(std::move(cu));
		}

		// unitのスキルを読み込み
		const JSON skillData = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + +U"/070_Scenario/InfoSkill/skill.json");

		if (not skillData) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `skill.json`" };

		Array<Skill> arrayClassSkill;
		for (const auto& [key, value] : skillData[U"Skill"]) {
			Skill cu;
			if (value.hasElement(U"sortkey") == true)
				cu.sortKey = Parse<int32>(value[U"sortkey"].get<String>());
			if (value.hasElement(U"help") == true)
				cu.help = ClassStaticCommonMethod::ReplaceNewLine(value[U"help"].get<String>());

			{
				if (value[U"func"].get<String>() == U"missile")
				{
					cu.SkillType = SkillType::missile;
				}
				if (value[U"func"].get<String>() == U"missileAdd")
				{
					cu.SkillType = SkillType::missileAdd;
				}
				if (value[U"func"].get<String>() == U"heal")
				{
					cu.SkillType = SkillType::heal;
				}
			}
			{
				if (value[U"MoveType"].get<String>() == U"throw")
				{
					cu.MoveType = MoveType::thr;
				}
				if (value[U"MoveType"].get<String>() == U"circle")
				{
					cu.MoveType = MoveType::circle;
				}
				if (value[U"MoveType"].get<String>() == U"swing")
				{
					cu.MoveType = MoveType::swing;
				}
			}
			if (value.hasElement(U"Easing") == true)
				if (value[U"Easing"].get<String>() == U"easeOutExpo")
					cu.Easing = SkillEasing::easeOutExpo;
			if (value.hasElement(U"EasingRatio") == true)
			{
				cu.EasingRatio = Parse<int32>(value[U"EasingRatio"].get<String>());
			}
			if (value.hasElement(U"icon") == true)
			{
				cu.icon = (value[U"icon"].get<String>().split(','));
			}
			if (value.hasElement(U"slow_per") == true)
			{
				cu.slowPer = Parse<int32>(value[U"slow_per"].get<String>());
			}
			else
			{
				cu.slowPer = none;
			}
			if (value.hasElement(U"slow_time") == true)
			{
				cu.slowTime = Parse<int32>(value[U"slow_time"].get<String>());
			}
			else
			{
				cu.slowTime = none;
			}
			if (value.hasElement(U"center") == true)
			{
				if (value[U"center"].get<String>() == U"on")
				{
					cu.SkillCenter = SkillCenter::on;
				}
				else if (value[U"center"].get<String>() == U"off")
				{
					cu.SkillCenter = SkillCenter::off;
				}
				else if (value[U"center"].get<String>() == U"end")
				{
					cu.SkillCenter = SkillCenter::end;
				}
			}
			if (value.hasElement(U"bom") == true)
			{
				if (value[U"bom"].get<String>() == U"on")
				{
					cu.SkillBomb = SkillBomb::on;
				}
				else if (value[U"bom"].get<String>() == U"off")
				{
					cu.SkillBomb = SkillBomb::off;
				}
			}
			if (value.hasElement(U"height") == true)
			{
				cu.height = Parse<int32>(value[U"height"].get<String>());
			}
			if (value.hasElement(U"radius") == true)
			{
				cu.radius = Parse<double>(value[U"radius"].get<String>());
			}
			if (value.hasElement(U"rush") == true)
			{
				cu.rush = Parse<int32>(value[U"rush"].get<String>());
			}
			if (value.hasElement(U"rush_random_degree") == true)
			{
				cu.rushRandomDegree = Parse<int32>(value[U"rush_random_degree"].get<String>());
			}
			cu.image = value[U"image"].get<String>();
			if (value.hasElement(U"d360") == true)
			{
				if (value[U"d360"].get<String>() == U"on")
				{
					cu.SkillD360 = SkillD360::on;
					TextureAsset::Register(cu.image + U".png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U".png");
				}
			}
			else
			{
				TextureAsset::Register(cu.image + U"NW.png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U"NW.png");
				TextureAsset::Register(cu.image + U"N.png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U"N.png");
			}
			if (value.hasElement(U"force_ray") == true)
			{
				if (value[U"force_ray"].get<String>() == U"on")
				{
					cu.SkillForceRay = SkillForceRay::on;
				}
				else
				{
					cu.SkillForceRay = SkillForceRay::off;
				}
			}
			if (value.hasElement(U"ray") == true)
			{
				for (auto& temp : value[U"ray"].get<String>().split(','))
				{
					cu.ray.push_back(Parse<double>(temp.trimmed()) / 255);
				}
			}
			if (value.hasElement(U"ray_strokeThickness") == true)
			{
				cu.rayStrokeThickness = Parse<int32>(value[U"ray_strokeThickness"].get<String>());
			}
			if (value.hasElement(U"plus_speed") == true)
			{
				cu.plus_speed = Parse<double>(value[U"plus_speed"].get<String>());
			}
			if (value.hasElement(U"plus_speed_time") == true)
			{
				cu.plus_speed_time = Parse<double>(value[U"plus_speed_time"].get<String>());
			}
			cu.nameTag = value[U"name_tag"].get<String>();
			cu.name = value[U"name"].get<String>();
			cu.range = Parse<int32>(value[U"range"].get<String>());
			cu.w = Parse<int32>(value[U"w"].get<String>());
			cu.h = Parse<int32>(value[U"h"].get<String>());
			cu.str = Parse<int32>(value[U"str"].get<String>());
			if (value[U"str_kind"].get<String>() == U"attack")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"off")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"end")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"attack_magic")
			{
				cu.SkillStrKind = SkillStrKind::attack_magic;
			}

			cu.speed = Parse<double>(value[U"speed"].get<String>());
			if (value.hasElement(U"attr") == true)
			{
				if (value[U"attr"].get<String>() == U"mp")
				{
					cu.attr = U"mp";
				}
				else if (value[U"attr"].get<String>() == U"hp")
				{
					cu.attr = U"hp";
				}
			}

			arrayClassSkill.push_back(std::move(cu));
		}

		//unitのスキル名からスキルクラスを探し、unitに格納
		for (auto& itemUnit : arrayClassUnit)
			for (const auto& itemSkillName : itemUnit.SkillName)
				for (const auto& skill : arrayClassSkill)
					if (skill.nameTag == itemSkillName)
					{
						itemUnit.Skill.emplace_back(skill);
						break;
					}

		commonConfig.arrayUnit = arrayClassUnit;
		//manager.get().get()->classGameStatus.arrayClassUnit = arrayClassUnit;
	}
	//// obj.jsonからデータを読み込む
	//{
	//	const JSON objData = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoObject/obj.json");

	//	if (not objData) // もし読み込みに失敗したら
	//		throw Error{ U"Failed to load `obj.json`" };

	//	Array<ClassObjectMapTip> arrayClassObj;
	//	for (const auto& [key, value] : objData[U"obj"]) {
	//		ClassObjectMapTip cu;
	//		cu.nameTag = value[U"name"].get<String>();
	//		String ty = value[U"type"].get<String>();
	//		if (ty == U"wall2")
	//		{
	//			cu.type = MapTipObjectType::WALL2;
	//		}
	//		else if (ty == U"gate")
	//		{
	//			cu.type = MapTipObjectType::GATE;
	//		}
	//		cu.noWall2 = value[U"no_wall2"].get<int32>();
	//		cu.castle = value[U"castle"].get<int32>();
	//		cu.castleDefense = value[U"castle_defense"].get<int32>();
	//		cu.castleMagdef = value[U"castle_magdef"].get<int32>();

	//		arrayClassObj.push_back(std::move(cu));
	//	}
	//	manager.get().get()->classGameStatus.arrayClassObjectMapTip = arrayClassObj;
	//}
	////enemy
	//{
	//	const JSON jsonUnit = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoEnemy/enemy.json");

	//	if (not jsonUnit) // もし読み込みに失敗したら
	//		throw Error{ U"Failed to load `Unit.json`" };

	//	Array<ClassEnemy> arrayClassUnit;
	//	for (const auto& [key, value] : jsonUnit[U"enemy"]) {
	//		ClassEnemy ce;
	//		ce.name = value[U"name"].getString();
	//		ce.type = value[U"value"].getString().split(',');
	//		arrayClassUnit.push_back(std::move(ce));
	//	}
	//	manager.get().get()->classGameStatus.arrayClassEnemy = arrayClassUnit;
	//}
	//// config.tomlからデータを読み込む
	//{
	//	const TOMLReader tomlConfig{ PATHBASE + PATH_DEFAULT_GAME + U"/config.toml" };
	//	if (not tomlConfig) // もし読み込みに失敗したら
	//		throw Error{ U"Failed to load `config.toml`" };
	//	manager.get().get()->classGameStatus.DistanceBetweenUnit = tomlConfig[U"config.DistanceBetweenUnit"].get<int32>();
	//	manager.get().get()->classGameStatus.DistanceBetweenUnitTate = tomlConfig[U"config.DistanceBetweenUnitTate"].get<int32>();
	//}

}

void loop()
{
	Optional<std::pair<Point, Point>> dragStartWindow;

	while (System::Update())
	{
		if ((KeyEscape.down() && downKeyEscape(downKeyEscapeResult) == true)
			||
			EXITBTNRECT.leftClicked())
			break;

		GaussianFSAddon::DragProcessWindow(dragStartWindow);
	}
}

void Main()
{
	Co::Init();

	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,800 });
	EXITBTNRECT = GaussianFSAddon::GetStairsAsPo();

	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });

	GameData saveData;
	{
		Deserializer<BinaryReader> reader{ U"game.save" };

		if (reader)
			reader(saveData);
	}

	CommonConfig commonConfig;
	Init(commonConfig);

	int32 argc = System::GetArgc();
	char** argv = System::GetArgv();
	// 0:パス
	// 1:コマンドライン引数一つ目
	Array<String> args;
	for (size_t i = 0; i < argc; i++)
		args.push_back(Unicode::Widen(argv[i]));

	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	if (argc == 2 && args[1] == U"-Battle")
	{
		SystemString ss;
		ss.BattleMessage001 = U"BattleMessage001";
		ss.StatusName = U"StatusName";
		ss.StatusRace = U"";
		ss.StatusPrice = U"";
		ss.StatusHp = U"";
		ss.StatusMp = U"";
		ss.StatusAttack = U"";
		ss.StatusDefense = U"";
		ss.StatusMagic = U"";
		ss.StatusMagDef = U"";
		ss.StatusSpeed = U"";
		ss.StatusMove = U"";
		ss.StatusSkill = U"";
		ss.StatusSetumei = U"";
		ss.SkillAttack = U"SkillAttack";
		ss.Zinkei.push_back(U"密集");
		ss.Zinkei.push_back(U"横列");
		ss.Zinkei.push_back(U"正方");
		systemString = ss;

		const auto battle = Co::PlaySceneFrom<Battle>(saveData, commonConfig, systemString).runScoped();
		loop();
	}
	else
	{
		const auto game = Co::PlaySceneFrom<Game>(saveData).runScoped();
		loop();
	}


	// メインループの後、終了時にゲームをセーブ
	{
		Serializer<BinaryWriter> writer{ U"game.save" };
		writer(GameData{ 0, U"Player", saveData.cookies, saveData.itemCounts });
	}
}

