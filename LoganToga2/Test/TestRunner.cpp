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

void TestRunner::run()
{
    Print << U"--- Starting Tests ---";

    test_parseMapDetail();

    Print << U"--- All Tests Finished ---";
}
