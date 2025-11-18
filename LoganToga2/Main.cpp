#pragma once
# include <Siv3D.hpp> // Siv3D v0.6.15
#include "libs/CoTaskLib.hpp"
#include "libs/Gaussian.hpp"
#include "Common.h"
#include "Game.hpp"
#include "Battle.hpp"
#include "Battle001.h"
#include "ClassUnit.h"
#include "ClassCommonConfig.h"
#include "ClassBuildAction.h"
#include "Test/TestRunner.h"

void Init(CommonConfig& commonConfig)
{
	// Unit.jsonを読み込む
	{
		const JSON jsonUnit = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoUnit/Unit.json");

		if (not jsonUnit) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `Unit.json`" };

		Array<Unit> arrayClassUnit;
		for (const auto& [key, value] : jsonUnit[U"Unit"]) {
			Unit cu;
			cu.NameTag = value[U"name_tag"].getString();
			cu.Name = value[U"name"].getString();
			cu.ImageName = value[U"image"].getString();
			if (value.hasElement(U"isBuilding") == true)
			{
				cu.IsBuilding = Parse<bool>(value[U"isBuilding"].getString());
				if (value.hasElement(U"objKind") == true)
				{
					if (value[U"objKind"].getString() == U"gate")
					{
						cu.mapTipObjectType = MapTipObjectType::GATE;
					}
				}
			}
			if (value.hasElement(U"hp") == true)
			{
				if (value.hasElement(U"hp_castle") == true)
				{
					if (Parse<int32>(value[U"hp"].getString()) != 0)
					{
						cu.Hp = Parse<int32>(value[U"hp"].getString());
					}
					else
					{
						cu.HPCastle = Parse<int32>(value[U"hp_castle"].getString());
					}
				}
				else
				{
					cu.Hp = Parse<int32>(value[U"hp"].getString());
				}
			}
			else
			{
				if (value.hasElement(U"hp_castle") == true)
				{
					cu.HPCastle = Parse<int32>(value[U"hp_castle"].getString());
				}
			}
			cu.Mp = Parse<int32>(value[U"mp"].getString());
			cu.HpMAX = cu.Hp;
			if (value.hasElement(U"visionRadius") == true)
				cu.visionRadius = Parse<int32>(value[U"visionRadius"].getString());
			if (value.hasElement(U"classBuild") == true)
				cu.classBuild = value[U"classBuild"].getString();
			if (value.hasElement(U"MaintainRange") == true)
				cu.MaintainRange = Parse<double>(value[U"MaintainRange"].get<String>());
			cu.Attack = Parse<int32>(value[U"attack"].getString());
			cu.Defense = Parse<int32>(value[U"defense"].getString());
			cu.Magic = Parse<int32>(value[U"magic"].getString());
			cu.MagDef = Parse<int32>(value[U"magDef"].getString());
			cu.Speed = Parse<double>(value[U"speed"].getString());
			cu.Price = Parse<int32>(value[U"price"].getString());
			if (value.hasElement(U"move") == true)
				cu.Move = Parse<double>(value[U"move"].getString());
			if (value.hasElement(U"escape_range") == true)
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
			String sNa = U"";
			if (value.hasElement(U"skill") == true)
				sNa = value[U"skill"].getString();
			if (sNa.contains(',') == true)
			{
				cu.SkillName = sNa.split(',');
			}
			else
			{
				cu.SkillName.push_back(sNa);
			}

			if (value.hasElement(U"isCarrierUnit") == true)
			{
				if (Parse<bool>(value[U"isCarrierUnit"].getString()))
				{
					cu.isCarrierUnit = true;
					auto carrierComponent = std::make_unique<CarrierComponent>();
					if (value.hasElement(U"carrierCapacity") == true)
					{
						carrierComponent->capacity = Parse<int32>(value[U"carrierCapacity"].getString());
					}
					cu.components.push_back(std::move(carrierComponent));
				}
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
			if (value.hasElement(U"special") == true)
				cu.Special = Parse<int32>(value[U"special"].get<String>());
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
			if (value.hasElement(U"delay") == true)
			{
				cu.Delay = Parse<double>(value[U"delay"].get<String>());
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

			if (value.hasElement(U"homing") == true)
			{
				String tempHoming = value[U"homing"].get<String>();
				if (tempHoming == U"on")
				{
					cu.homing = true;
				}
			}
			if (value.hasElement(U"start_degree") == true)
			{
				cu.startDegree = Parse<double>(value[U"start_degree"].get<String>());
			}
			if (value.hasElement(U"start_degree_type") == true)
			{
				cu.startDegreeType = Parse<int32>(value[U"start_degree_type"].get<String>());
			}
			if (cu.MoveType == MoveType::swing) {
				// 角度用
				cu.arcDeg = cu.range; // JSON range を arc に転用
				// 距離用（別途項目を追加できるなら "reach" を使う）
				cu.reachDist = cu.rangeMin; // 暫定値
			}
			else {
				cu.arcDeg = 0.0;
				cu.reachDist = cu.range;
			}
			arrayClassSkill.push_back(std::move(cu));
		}

		//unitのスキル名からスキルクラスを探し、unitに格納
		for (auto& itemUnit : arrayClassUnit)
			for (const auto& itemSkillName : itemUnit.SkillName)
				for (auto& skill : arrayClassSkill)
					if (skill.nameTag == itemSkillName)
					{
						itemUnit.arrSkill.emplace_back(skill);
						break;
					}

		commonConfig.arrayInfoUnit = std::move(arrayClassUnit);
	}
	// buildメニューを読み込む
	{
		const JSON jsonBuildMenu = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoBuildMenu/aaa.json");

		if (not jsonBuildMenu)
			throw Error{ U"Failed to load `aaa.json`" };

		for (auto&& [index, object] : jsonBuildMenu)
		{
			Array<BuildAction> arrBa;
			for (auto&& [index2, object2] : object)
			{
				BuildAction ba;
				ba.id = object2[U"id"].getString();
				ba.name = object2[U"name"].getString();
				ba.description = object2[U"description"].getString();
				ba.icon = object2[U"icon"].getString();
				if (object2.hasElement(U"createCount") == true)
					ba.createCount = Parse<int32>(object2[U"createCount"].getString());
				HashTable<String, int32> costTemp;
				if (object2.hasElement(U"cost") == true)
				{
					for (auto&& [index3, object3] : object2[U"cost"])
					{
						int32 ioudiu = object3.get<int32>();
						costTemp.emplace(index3, object3.get<int32>());
					}
				}
				ba.buildTime = object2[U"buildTime"].get<double>();
				ba.category = object2[U"category"].getString();
				Array<String> requiresTemp;
				for (auto&& [index3, object3] : object2[U"requires"])
				{
					requiresTemp.push_back(object3.getString());
				}
				if (object2.hasElement(U"isMove") == true)
					ba.isMove = ParseBool(object2[U"isMove"].getString());
				for (auto&& [key, resultObj] : object2[U"result"])
				{
					if (resultObj.hasElement(U"spawn") == true)
						ba.result.spawn = resultObj[U"spawn"].getString();
					ba.result.type = resultObj[U"type"].getString();
				}
				arrBa.push_back(ba);
			}
			commonConfig.htBuildMenuBaseData.emplace(index, arrBa);
		}
	}

	{
		const JSON jsonInfoResource = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoResource/aaa.json");

		if (not jsonInfoResource)
			throw Error{ U"Failed to load `aaa.json`" };

		for (auto&& [index, object] : jsonInfoResource)
		{
			for (auto&& [index2, object2] : object)
			{
				ClassResource cr;
				cr.resourceName = object2[U"name"].getString();
				cr.resourceIcon = object2[U"icon"].getString();
				cr.resourceAmount = Parse<int32>(object2[U"amount"].getString());

				if (object2[U"kind"].getString() == U"gold")
				{
					cr.resourceType = resourceKind::Gold;
				}
				else if (object2[U"kind"].getString() == U"trust")
				{
					cr.resourceType = resourceKind::Trust;
				}
				else if (object2[U"kind"].getString() == U"food")
				{
					cr.resourceType = resourceKind::Food;
				}

				commonConfig.htResourceData.emplace(cr.resourceName, cr);
			}
		}
	}
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
	else if (argc == 2 && args[1] == U"-Battle001")
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

		const auto game = Co::PlaySceneFrom<Battle001>(saveData, commonConfig, systemString).runScoped();
		loop();
	}
	else if (argc == 2 && args[1] == U"-test")
	{
		TestRunner runner;
		runner.run();
		// Tests finished, wait for user to close window.
		while (System::Update());
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

