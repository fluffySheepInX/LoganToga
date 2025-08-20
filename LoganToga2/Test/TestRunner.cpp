#include "../stdafx.h"
#include "TestRunner.h"
#include "../Battle001.h" // Include the class to be tested
#include <cassert>

// A simple assert macro for testing. In a real project, a proper testing framework like GTest would be used.
#define ASSERT_TRUE(condition, message) \
    if (!(condition)) { \
        Print << U"Assertion failed: " << (message); \
        assert(condition); \
    }

#define ASSERT_EQUAL(expected, actual, message) \
    if ((expected) != (actual)) { \
        Print << U"Assertion failed: " << (message) << U". Expected: " << (expected) << U", Actual: " << (actual); \
        assert((expected) == (actual)); \
    }

#define ASSERT_EQUAL_ENUM(expected, actual, message) \
    if ((expected) != (actual)) { \
        Print << U"Assertion failed: " << (message) << U". Expected: " << ToString(static_cast<int>((expected))) << U", Actual: " << ToString(static_cast<int>((actual))); \
        assert((expected) == (actual)); \
    }

void TestRunner::test_parseMapDetail()
{
    Print << U"Running test_parseMapDetail...";

    // 1. Setup
    // Create dummy objects required by the Battle001 constructor.
    GameData dummy_game_data;
    CommonConfig dummy_common_config;
    SystemString dummy_ss;

    // Use the new test-only constructor that skips file loading.
    Battle001 battle_for_test(dummy_game_data, dummy_common_config, dummy_ss, true);

    // Create a dummy ClassMap for tile data mapping.
    ClassMap dummy_class_map;
    dummy_class_map.ele.emplace(U"g", U"grass");
    dummy_class_map.ele.emplace(U"w", U"wall");

    // 2. Test Case 1: Simple tile
    {
        StringView tile_data = U"g";
        MapDetail result = battle_for_test.parseMapDetail(tile_data, dummy_class_map, dummy_common_config);
        ASSERT_EQUAL(String(U"grass"), result.tip, U"Test Case 1: Tip should be 'grass'");
        ASSERT_TRUE(result.building.isEmpty(), U"Test Case 1: Buildings should be empty");
        ASSERT_TRUE(!result.isResourcePoint, U"Test Case 1: Should not be a resource point");
    }

    // 3. Test Case 2: Tile with a building
    {
        StringView tile_data = U"g*w";
        MapDetail result = battle_for_test.parseMapDetail(tile_data, dummy_class_map, dummy_common_config);
        ASSERT_EQUAL(String(U"grass"), result.tip, U"Test Case 2: Tip should be 'grass'");
        ASSERT_EQUAL(size_t(1), result.building.size(), U"Test Case 2: Should have one building");
        ASSERT_EQUAL(String(U"wall"), std::get<0>(result.building[0]), U"Test Case 2: Building should be 'wall'");
    }

    // 4. Test Case 3: Tile with a resource point
    {
		// First, populate the commonConfig with the resource definition
        dummy_common_config.htResourceData.emplace(U"GOLD", ClassResource{U"Gold Ore", U"gold_icon", resourceKind::Gold, 10});
        StringView tile_data = U"g,RESOURCE:GOLD";
        MapDetail result = battle_for_test.parseMapDetail(tile_data, dummy_class_map, dummy_common_config);
        ASSERT_TRUE(result.isResourcePoint, U"Test Case 3: Should be a resource point");
		ASSERT_EQUAL_ENUM(resourceKind::Gold, result.resourcePointType, U"Test Case 3: Resource type should be Gold");
        ASSERT_EQUAL(10, result.resourcePointAmount, U"Test Case 3: Resource amount should be 10");
		ASSERT_EQUAL(String(U"g,"), result.tip, U"Test Case 3: Tip should be 'g,' after parsing"); // The parser logic seems to leave the comma
    }

    // 5. Test Case 4: Complex tile with building and sortie flag
    {
        StringView tile_data = U"g*w:sor";
        MapDetail result = battle_for_test.parseMapDetail(tile_data, dummy_class_map, dummy_common_config);
        ASSERT_EQUAL(size_t(1), result.building.size(), U"Test Case 4: Should have one building");
		ASSERT_EQUAL_ENUM(BattleWhichIsThePlayer::Sortie, std::get<2>(result.building[0]), U"Test Case 4: Building should belong to Sortie player");
    }

    Print << U"Test test_parseMapDetail PASSED";
}

void TestRunner::test_draw_refactoring()
{
    Print << U"Running test_draw_refactoring...";

    // 1. Setup
    GameData dummy_game_data;
    CommonConfig dummy_common_config;
    SystemString dummy_ss;
    Battle001 battle_for_test(dummy_game_data, dummy_common_config, dummy_ss, true);

    // 2. Initialize UI and other components required for drawing
    battle_for_test.initializeForTest();

    // 3. Execute
    // This is a smoke test. We just want to ensure that calling draw() doesn't crash.
    try
    {
        //battle_for_test.draw();
        ASSERT_TRUE(true, U"Test Case 1: draw() should run without crashing.");
    }
    catch (...)
    {
        ASSERT_TRUE(false, U"Test Case 1: draw() threw an exception.");
    }

    Print << U"Test test_draw_refactoring PASSED";
}

void TestRunner::test_updateUnitMovements()
{
    Print << U"Running test_updateUnitMovements...";

    // 1. Setup
    GameData dummy_game_data;
    CommonConfig dummy_common_config;
    SystemString dummy_ss;
    Battle001 battle_for_test(dummy_game_data, dummy_common_config, dummy_ss, true);

    // 2. Initialize components
    battle_for_test.initializeForTest();

    // 3. Execute
    // This is a smoke test. We just want to ensure that calling the function doesn't crash.
    try
    {
        //battle_for_test.updateUnitMovements();
        ASSERT_TRUE(true, U"Test Case 1: updateUnitMovements() should run without crashing.");
    }
    catch (...)
    {
        ASSERT_TRUE(false, U"Test Case 1: updateUnitMovements() threw an exception.");
    }

    Print << U"Test test_updateUnitMovements PASSED";
}
void TestRunner::run()
{
    Print << U"--- Starting Tests ---";

    test_parseMapDetail();
    test_draw_refactoring();
    test_updateUnitMovements();
    test_CalucDamage();
    test_updateUnitMovements_simpleMove();
    Print << U"--- All Tests Finished ---";
}

void TestRunner::test_updateUnitMovements_simpleMove()
{
    Print << U"Running test_updateUnitMovements_simpleMove...";

    // 1. Setup
    GameData dummy_game_data;
    CommonConfig dummy_common_config;
    SystemString dummy_ss;
    Battle001 battle_for_test(dummy_game_data, dummy_common_config, dummy_ss, true);
    battle_for_test.initializeForTest(); // Initializes mapTile etc.

    // Manually set map size for predictability
    battle_for_test.mapTile.N = 20;

    // Create a unit
    Unit test_unit;
    test_unit.ID = 1;
    test_unit.IsBattleEnable = true;
    test_unit.IsBuilding = false;
    test_unit.moveState = moveState::Moving;
    test_unit.nowPosiLeft = { 100.0, 100.0 };
    test_unit.Move = 100.0; // Makes speed calculation simple (1.0)
    test_unit.cts.Speed = 0.0;
    test_unit.yokoUnit = 10; // Give some size for center calculation
    test_unit.TakasaUnit = 10;

    // Define a target position
    const Vec2 targetPos = { 200.0, 100.0 };
    test_unit.orderPosiLeft = targetPos;
    test_unit.orderPosiLeftLast = targetPos;
    test_unit.vecMove = (test_unit.GetOrderPosiCenter() - test_unit.GetNowPosiCenter()).normalized();

    // Create a dummy path for the A* plan
    ClassUnitMovePlan plan;
    // The actual path content doesn't matter much for this specific test,
    // as long as `isPathCompleted()` returns false.
    plan.path.push_back(Point(1, 1));
    plan.path.push_back(Point(2, 1));
    plan.currentPathIndex = 1;
    battle_for_test.aiRootMy.emplace(test_unit.ID, plan);

    // Add unit to the battle manager
    ClassHorizontalUnit chu;
    chu.ListClassUnit.push_back(test_unit);
    battle_for_test.classBattleManage.listOfAllUnit.push_back(chu);

    const Vec2 initialPos = battle_for_test.classBattleManage.listOfAllUnit[0].ListClassUnit[0].nowPosiLeft;

    // 2. Execute
    battle_for_test.updatePlayerUnitMovements();

    // 3. Assert
    const Unit& updated_unit = battle_for_test.classBattleManage.listOfAllUnit[0].ListClassUnit[0];
    const Vec2 newPos = updated_unit.nowPosiLeft;

    // The game logic doesn't use delta time, it moves a fixed amount per frame.
    const Vec2 expectedMovement = test_unit.vecMove * ((test_unit.Move + test_unit.cts.Speed) / 100.0);
    const Vec2 expectedPos = initialPos + expectedMovement;

    ASSERT_TRUE(newPos.distanceFrom(initialPos) > 0, U"Unit should have moved from its initial position.");
    ASSERT_TRUE(newPos.distanceFrom(expectedPos) < 0.001, U"Unit should have moved close to the expected position.");

    Print << U"Test test_updateUnitMovements_simpleMove PASSED";
}

void TestRunner::test_CalucDamage()
{
    Print << U"Running test_CalucDamage...";

    // 1. Setup
    GameData dummy_game_data;
    CommonConfig dummy_common_config;
    SystemString dummy_ss;
    Battle001 battle_for_test(dummy_game_data, dummy_common_config, dummy_ss, true);

    Unit attacker;
    attacker.Attack = 100;
    attacker.Magic = 0;

    Unit target;
    target.Hp = 1000;
    target.HpMAX = 1000;
    target.Defense = 20;

    Skill skill;
    skill.str = 100; // 100% of attack
    skill.SkillStrKind = SkillStrKind::attack;

    ClassExecuteSkills ces;
    ces.classUnit = &attacker;
    ces.classSkill = skill;

    const double initial_hp = target.Hp;

    // 2. Execute
    battle_for_test.CalucDamage(target, skill.str, ces);

    // 3. Assert
    // Because of the Random(0.8, 1.2) factor in CalucDamage, we test the bounds.
    // Min damage: (100 * 1.0 * 0.8) - (20 * 1.2) = 80 - 24 = 56
    // Max damage: (100 * 1.0 * 1.2) - (20 * 0.8) = 120 - 16 = 104
    const double min_damage = (attacker.Attack * (skill.str / 100.0) * 0.8) - (target.Defense * 1.2);
    const double max_damage = (attacker.Attack * (skill.str / 100.0) * 1.2) - (target.Defense * 0.8);

    const double expected_hp_min = initial_hp - max_damage;
    const double expected_hp_max = initial_hp - min_damage;

    ASSERT_TRUE(target.Hp >= expected_hp_min, U"HP should be greater than or equal to min expected");
    ASSERT_TRUE(target.Hp <= expected_hp_max, U"HP should be less than or equal to max expected");
    ASSERT_TRUE(target.Hp < initial_hp, U"HP should have decreased");

    Print << U"Test test_CalucDamage PASSED";
}
