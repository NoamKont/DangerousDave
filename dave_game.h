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
#include "box2d/id.h"
#include "SDL3/SDL_render.h"
using namespace bagel;
namespace dave_game {

    /**
     * @brief Component representing an entity's position on the grid.
     */
    using Position = struct {SDL_FPoint p; float a;};

    /**
     * @brief Component representing sprite animation state for rendering.
     */
    using Drawable = struct {
        SDL_FRect part;

        float scale;

        bool visible;
        bool flip= false;
    };

    using Animation = struct {

        enum class Type {
            DAVE
        };

        Drawable** states_frames;
        int statesCount;
        int framesCount;

        int currentState = 0;
        int currentFrame = 0;
        Type type;
    };


    /**
     * @brief Component that defines an entity's hitbox size for collision detection.
     */
    using Collider = struct { b2BodyId b; };


    /**
     * @brief Component that stores the last input from a player.
     */
    using Input = struct { SDL_Scancode up, down, right, left; };


    /**
     * @brief Component that expresses the current intended action of an entity.
     */
    using Intent = struct {
        bool up = false, down = false, left = false, right = false;
        bool blockedUp = false, blockedDown = false, blockedLeft = false, blockedRight = false;
    };

    /**
     * @brief Tag component to identify player-controlled entities.
     */
    struct Dave { };

    /**
     * @brief Tag component to identify ghost entities.
     */
    struct Ghost { };

    /**
     * @brief Tag component for wall entities.
     */
    struct Wall {b2ShapeId s; SDL_FPoint size;};

    /**
    * @brief Tag component for Background entities.
    */
    struct Background { };


/// @brief Health of an entity. When it reaches 0, entity is dead.
    struct Health {
        int hp = STARTING_LIVES;
    };

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

/// @brief Marks the entity as dead (for cleanup or state transition).
    struct Dead {};

/// @brief Marks whether Dave is on the ground

    struct GroundStatus {
        bool onGround = false;
    };





    class DaveGame {

        void prepareBoxWorld();

        void MovementSystem();
        void CollisionSystem();
        void RenderSystem();
        void InputSystem();
        void ScoreSystem();
        void CollectSystem();
        void DeathSystem();
        void AnimationSystem();
        void box_system();

        void prepareWalls() const;
        void createWall(SDL_FPoint p, float w, float h) const;

        void createDave();
        // Entity createMonster(Position pos, Image img);
        // Entity createBlock(Position pos, Image img);
        // Entity createPrize(Position pos, Image img, int scoreValue);
        // Entity createBackground(Position pos, Image img);
        // Entity createInfo(Position pos, Image img);
        // Entity createTree(Position pos, Image img);
        // Entity createGun(Position pos, Image img);
        // Entity createJetpack(Position pos, Image img);
        // Entity createObstacle(Position pos, Image img);
        // Entity createDoor(Position pos, Image img, int scoreToAdd);
        // Entity createTrophy(Position pos, Image img);

        static constexpr float	BOX_SCALE = 10;
        static constexpr float	CHARACTER_TEX_SCALE = 6.f;

        static constexpr int WIN_WIDTH = 800;
        static constexpr int WIN_HEIGHT = 800;
        static constexpr int FPS = 60;
        static constexpr float ANIMATION_INTERVAL = 10;

        static constexpr float GAME_FRAME = 1000.f/FPS;
        static constexpr float PHYSICS_TIME_STEP = 1.0f / FPS;
        static constexpr float	RAD_TO_DEG = 57.2958f;

        static constexpr float	ANIMATION_VELOCITY_THRESHOLD = 0.5f; // Velocity threshold to switch between animation states

        static inline  Drawable** DAVE_ANIMATION = nullptr;


        static constexpr SDL_FRect Dave_IDLE{ 5, 12, 8, 16 };

        SDL_Texture* tex;
        SDL_Renderer* ren;
        SDL_Window* win;

        b2WorldId boxWorld = b2_nullWorldId;

    public:
        DaveGame();
        ~DaveGame();

        bool prepareWindowAndTexture();

        void run();
        bool valid() const { return win != nullptr && ren != nullptr && tex != nullptr; }

    };
}
