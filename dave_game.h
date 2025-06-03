#pragma once
#define STARTING_LIVES 3

/**
 * @file dave_game.h
 * @brief ECS module for Dangerous Dave game.
 *
 * This module contains component definitions, system declarations, and entity creation
 * functions for the Dangerous Dave game using the BAGEL ECS engine.
 */

#include "bagel.h"

namespace dave_game
{
    using namespace bagel;
    /// @brief Player control marker – this entity receives keyboard input.
    struct Control {};

/// @brief The location of the entity on screen in pixels.
    struct Position {
        int x = 0;
        int y = 0;
    };

/// @brief Movement direction and speed (pixels per frame).
    struct Course {
        int dx = 0;
        int dy = 0;
    };

/// @brief Visual representation – sprite ID in texture atlas.
    struct Image {
        int spriteId = -1; ///< ID of the sprite (e.g., from pong.png)
    };

/// @brief Enables collision detection for this entity.
    struct Collision {};

/// @brief Health of an entity. When it reaches 0, entity is dead.
    struct Health {
        int hp = STARTING_LIVES;
    };

/// @brief Makes the entity collectable (prize, power-up, etc).
    struct Collectible {};

/// @brief Indicates that the entity has a gun and can shoot.
    struct Gun {};

/// @brief Jetpack component – holds remaining fuel units.
    struct Jetpack {
        int fuel = 100; ///< fuel out of 100
    };

/// @brief Allows Dave to climb objects like trees.
    struct Climbable {};

/// @brief Represents a door – can be open or closed.
    struct Door {
        bool open = false;
    };

/// @brief Score value of a collectible entity.
    struct PrizeValue {
        int score = -1; ///< Actual score value must be set during entity creation
    };

/// @brief Marks the entity as deadly to the player (e.g., fire, water, monsters).
    struct Hazard {};

/// @brief Stores global game state: score, lives, and level.
    struct GameInfo {
        int score = 0;
        int lives = 3;
        int level = 1;
    };

/// @brief Sprite animation handler (frame index and timing).
    struct Animation {
        int frame = 0;
        float timer = 0.0f;
    };

/// @brief Player's current intentions (input state).
    struct Intent {
        bool moveLeft = false;
        bool moveRight = false;
        bool jump = false;
        bool shoot = false;
        bool toggleJetpack = false; ///< Key press to toggle jetpack
    };

/// @brief Marks the entity as dead (for cleanup or state transition).
    struct Dead {};

/// @brief Temporary tag marking door as opened.
    struct OpenTag {};

/// @brief Links a trophy to the door it unlocks.
    struct Unlocks {
        bagel::ent_type doorId = {-1};
    };

/// @brief Placeholder for sound effect or audio ID
    struct SoundEffect {
        int soundId = -1; ///< ID of the sound clip to play
    };


    void MovementSystem();
    void CollisionSystem();
    void RenderSystem();
    void InputSystem();
    void ScoreSystem();
    void CollectSystem();
    void DeathSystem();
    void AnimationSystem();

    Entity createDave(Position pos, Image img);
    Entity createMonster(Position pos, Image img);
    Entity createBlock(Position pos, Image img);
    Entity createPrize(Position pos, Image img, int scoreValue);
    Entity createBackground(Position pos, Image img);
    Entity createInfo(Position pos, Image img);
    Entity createTree(Position pos, Image img);
    Entity createGun(Position pos, Image img);
    Entity createJetpack(Position pos, Image img);
    Entity createObstacle(Position pos, Image img);
    Entity createDoor(Position pos, Image img, int scoreToAdd);
    Entity createTrophy(Position pos, Image img);

}