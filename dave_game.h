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
     * @brief Tag component to identify monster entities.
     */
    struct Monster { };

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

    struct Diamond {};

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

/// @brief Marks if the entity is a trophy
    struct Trophy {};

    struct DoorLabel{};



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
        void createMap();
        void createDiamond(SDL_FPoint p);
        void createDoor(SDL_FPoint p);
        void createStatusBar();
        void createTitles();
        void createScoreBar();

        void createDave();

        static constexpr float	BOX_SCALE = 64.f;
        static constexpr float	DAVE_TEX_SCALE = 6.f;
        static constexpr float	BLOCK_TEX_SCALE = 6.f;

        static constexpr SDL_FRect RED_BLOCK {86,380,11,11};

        static constexpr int MAP_WIDTH = 20;
        static constexpr int MAP_HEIGHT = 10;

        static constexpr int FPS = 60;
        static constexpr float ANIMATION_INTERVAL = 10;

        static constexpr float GAME_FRAME = 1000.f/FPS;
        static constexpr float PHYSICS_TIME_STEP = 1.0f / FPS;
        static constexpr float	RAD_TO_DEG = 57.2958f;

        static constexpr float	ANIMATION_VELOCITY_THRESHOLD = 0.5f; // Velocity threshold to switch between animation states

        static inline  Drawable** DAVE_ANIMATION = nullptr;
        static inline Drawable* NUMBERS_SPRITES = new Drawable[10] {
            { { 294, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 0
            { { 237, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 1
            { { 243, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 2
            { { 250, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 3
            { { 256, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 4
            { { 262, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 5
            { { 269, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 6
            { { 275, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 7
            { { 282, 214, 6, 7 }, DAVE_TEX_SCALE, true, false }, // Placeholder for index 8
            { { 288, 214, 6, 7 }, DAVE_TEX_SCALE ,true ,false }  // Placeholder for index 9
        };


        static constexpr SDL_FRect DAVE_STANDING{ 5, 13, 8, 16 };
        static constexpr SDL_FRect DAVE_WALKING_1{27,13,12,16};
        static constexpr SDL_FRect DAVE_WALKING_2{78,13,12,16};


        static constexpr SDL_FRect DAVE_IDLE{ 155, 13, 7, 16 };
        static constexpr SDL_FRect DAVE_JUMPING{127,13,13,14};

        static constexpr int STATUS_BAR_HEIGHT = 2;
        static constexpr int WIN_WIDTH = MAP_WIDTH * RED_BLOCK.w * DAVE_TEX_SCALE;
        static constexpr int WIN_HEIGHT = (MAP_HEIGHT + STATUS_BAR_HEIGHT) * RED_BLOCK.h * DAVE_TEX_SCALE;
        static constexpr SDL_FRect DIAMOND{ 14, 429, 10, 11 };
        static constexpr SDL_FRect DOOR{ 13, 234, 10, 11 };

        static constexpr uint8_t GRID_BACKGROUND = 0;
        static constexpr uint8_t GRID_RED_BLOCK = 1;
        static constexpr uint8_t GRID_DIAMOND = 2;
        static constexpr uint8_t GRID_DOOR = 3;

        static constexpr int SCORE_DIGITS_COUNT = 5;

        SDL_Texture* tex;
        SDL_Renderer* ren;
        SDL_Window* win;
        ent_type scoreEntities[SCORE_DIGITS_COUNT];
        GameInfo gameInfo;

        b2WorldId boxWorld = b2_nullWorldId;

        static inline uint8_t map[10][20] = {
    /* row 0 (top border) */
    { GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK },

    /* row 1 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 2 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 3 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 4 */
    { GRID_RED_BLOCK, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_RED_BLOCK },

    /* row 5 */
    { GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_RED_BLOCK },

    /* row 6 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 7 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 8 */
    { GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_DOOR, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 9 (bottom border) */
    { GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK }
};




    public:
        DaveGame();
        ~DaveGame();

        bool prepareWindowAndTexture();

        void run();
        bool valid() const { return win != nullptr && ren != nullptr && tex != nullptr; }
    };
}
