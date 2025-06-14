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
        bool isStatic = false;
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

/// @brief Represents a  Diamond Prize  and score value of the collectible entity.
    struct Diamond {
        int value = 100;
    };


/// @brief Marks the entity as deadly to the player (e.g., fire, water, monsters).
    struct Hazard {};

/// @brief Stores global game state: score, lives, and level.
    struct GameInfo {
        int score = 0;
        int lives = 3;
        int level = 1;
        float screenOffset = 0.f; ///< Offset for scrolling background
    };

/// @brief Marks the entity as dead (for cleanup or state transition).
    struct Dead {};

/// @brief Marks whether Dave is on the ground
    struct GroundStatus {
        bool onGround = false;
    };

/// @brief Marks if the entity is a trophy
    struct Trophy {
        int value = 1000;
    };

    struct DoorLabel{};

    struct ScoreLabel{};
    struct LevelLabel{};

    struct MoveScreenSensor {
        bool forward = true;
        int col = 0;
    };



    class DaveGame {

        void prepareBoxWorld();

        void MovementSystem();

        void renderGoThruTheDoor();

        void CollisionSystem();
        void RenderSystem();
        void InputSystem();
        void StatusBarSystem();
        void CollectSystem();
        void DeathSystem();
        void AnimationSystem();
        void box_system();

        void prepareWalls() const;
        void createWall(SDL_FPoint p, float w, float h) const;
        void createMap(uint8_t* map, int width, int height);
        void createDiamond(SDL_FPoint p);
        void createDoor(SDL_FPoint p);
        void createStatusBar();
        void createTitles();
        void createScoreBar();
        void createLevelAndHealth();
        void createMoveScreenSensor(SDL_FPoint p,bool forward, int col);

        void createTrophy(SDL_FPoint p);

        void EndGame();

        void createDave();

        static constexpr float	BOX_SCALE = 64.f;
        static constexpr float	DAVE_TEX_SCALE = 0.4f;
        static constexpr float	BLOCK_TEX_SCALE = 0.66f;

        // static constexpr SDL_FRect RED_BLOCK {86,380,11,11};
        // static constexpr SDL_FRect DAVE_HEALTH{ 1, 213, 8, 9 };
        // static constexpr SDL_FRect SCORE_SPRITE{192, 214, 39, 7};
        // static constexpr SDL_FRect LEVEL_SPRITE{146, 214, 33, 7};
        // static constexpr SDL_FRect HEALTH_SPRITE{102, 214, 37, 7};

        // static constexpr SDL_FRect DAVE_STANDING{ 5, 13, 8, 16 };
        // static constexpr SDL_FRect DAVE_WALKING_1{27,13,12,16};
        // static constexpr SDL_FRect DAVE_WALKING_2{78,13,12,16};
        // static constexpr SDL_FRect DAVE_IDLE{ 155, 13, 7, 16 };
        // static constexpr SDL_FRect DAVE_JUMPING{127,13,13,14};
        //
        // static constexpr SDL_FRect SCORE_0{ 294, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_1{ 237, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_2{ 243, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_3{ 250, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_4{ 256, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_5{ 262, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_6{ 269, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_7{ 275, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_8{ 282, 214, 6, 7 };
        // static constexpr SDL_FRect SCORE_9{ 288, 214, 6, 7 };


        static constexpr SDL_FRect DAVE_HEALTH{ 1419, 804, 79, 75 };
        static constexpr SDL_FRect SCORE_SPRITE{1082, 694, 280, 71};
        static constexpr SDL_FRect HEALTH_SPRITE{1082, 804, 273, 70};
        static constexpr SDL_FRect LEVEL_SPRITE{1082, 912, 287, 70};


        static constexpr SDL_FRect DAVE_STANDING{ 75, 38, 109, 155 };
        static constexpr SDL_FRect DAVE_WALKING_1{223,38,117,155};
        static constexpr SDL_FRect DAVE_WALKING_2{373,38,117,155};
        static constexpr SDL_FRect DAVE_IDLE{ 75, 38, 109, 155 };
        static constexpr SDL_FRect DAVE_JUMPING{676,38,131,155};

        static constexpr SDL_FRect DIAMOND{ 231, 370, 118, 118 };
        static constexpr SDL_FRect RED_DIAMOND{ 75, 370, 118, 118 };
        static constexpr SDL_FRect DOOR{ 525, 366, 118, 118 };
        static constexpr SDL_FRect TROPHY{ 373, 370, 118, 118 };
        static constexpr SDL_FRect RED_BLOCK{ 229, 226, 100, 100 };

        static constexpr SDL_FRect SCORE_1{ 1671, 738, 40, 73 };
        static constexpr SDL_FRect SCORE_2{ 1740, 738, 60, 70 };
        static constexpr SDL_FRect SCORE_3{ 1808, 738, 60, 71 };
        static constexpr SDL_FRect SCORE_4{ 1886, 738, 64, 70 };
        static constexpr SDL_FRect SCORE_5{ 1967, 738, 55, 70 };

        static constexpr SDL_FRect SCORE_6{ 1671, 842, 56, 68 };
        static constexpr SDL_FRect SCORE_7{ 1740, 842, 61, 68 };
        static constexpr SDL_FRect SCORE_8{ 1814, 842, 60, 68 };
        static constexpr SDL_FRect SCORE_9{ 1887, 842, 59, 68 };
        static constexpr SDL_FRect SCORE_0{ 1961, 842, 60, 68 };


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
            { SCORE_0, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 0
            { SCORE_1, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 1
            { SCORE_2, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 2
            { SCORE_3, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 3
            { SCORE_4, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 4
            { SCORE_5, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 5
            { SCORE_6, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 6
            { SCORE_7, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 7
            { SCORE_8, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 8
            { SCORE_9, BLOCK_TEX_SCALE, true, false , true}, // Placeholder for index 9
        };



        static constexpr int DAVE_START_COLUMN = 1;
        static constexpr int DAVE_START_ROW = 8;


        static constexpr int STATUS_BAR_HEIGHT = 2;
        static constexpr int WIN_WIDTH = MAP_WIDTH * (RED_BLOCK.w * BLOCK_TEX_SCALE);
        static constexpr int WIN_HEIGHT = MAP_HEIGHT * (RED_BLOCK.h * BLOCK_TEX_SCALE);
        //static constexpr int WIN_HEIGHT = (MAP_HEIGHT + STATUS_BAR_HEIGHT) * (RED_BLOCK.h * BLOCK_TEX_SCALE);

        static constexpr uint8_t GRID_BACKGROUND = 0;
        static constexpr uint8_t GRID_RED_BLOCK = 1;
        static constexpr uint8_t GRID_DIAMOND = 2;
        static constexpr uint8_t GRID_DOOR = 3;
        static constexpr uint8_t GRID_TROPHY = 4;
        static constexpr uint8_t GRID_SENSOR_BACK = 5;
        static constexpr uint8_t GRID_SENSOR_FORWARD = 6;

        static constexpr int SCORE_DIGITS_COUNT = 5;

        SDL_Texture* tex;
        SDL_Renderer* ren;
        SDL_Window* win;
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
      GRID_BACKGROUND, GRID_TROPHY, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
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
      GRID_BACKGROUND, GRID_DOOR, GRID_RED_BLOCK, GRID_DOOR, GRID_BACKGROUND,
      GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK },

    /* row 9 (bottom border) */
    { GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
      GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK }
};

        // Replica of Dangerous Dave Level 2, doubled in width to 40 columns
static inline uint8_t map_stage2[10][40] = {
    // row 0 (top border)
    {
        // first 20 cols
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        // repeated for columns 20–39
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK
    },

    // row 1
    {
        GRID_RED_BLOCK,
        GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 2
    {
        GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND,   GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_DIAMOND, GRID_SENSOR_FORWARD, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_DIAMOND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 3
    {
        GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_SENSOR_FORWARD, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 4
    {
        GRID_RED_BLOCK,
        GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_DIAMOND,
        GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND,
        GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND,
        GRID_RED_BLOCK
    },

    // row 5
    {
        GRID_RED_BLOCK, GRID_BACKGROUND,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_TROPHY,  GRID_RED_BLOCK, GRID_BACKGROUND, GRID_RED_BLOCK,
        GRID_RED_BLOCK,  GRID_RED_BLOCK, GRID_RED_BLOCK,

        // repeat
         GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK
    },

    // row 6
    {
        GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_DIAMOND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND, GRID_BACKGROUND, GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_DIAMOND, GRID_SENSOR_FORWARD, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 7
    {
        GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_DIAMOND, GRID_RED_BLOCK, GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_BACKGROUND,  GRID_RED_BLOCK,  GRID_BACKGROUND,
        GRID_BACKGROUND,  GRID_DIAMOND,  GRID_DIAMOND,
        GRID_DIAMOND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,
        GRID_SENSOR_FORWARD, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_RED_BLOCK,
        GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 8
    {
        GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK, GRID_DIAMOND,      GRID_BACKGROUND,      GRID_RED_BLOCK,      GRID_RED_BLOCK,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD,
        GRID_BACKGROUND,

        // repeat
        GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_SENSOR_BACK, GRID_SENSOR_FORWARD, GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND,      GRID_RED_BLOCK,      GRID_DOOR,      GRID_BACKGROUND,
        GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
        GRID_RED_BLOCK
    },

    // row 9 (bottom border)
    {
        // first 20
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        // repeat
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
        GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK
    }
};




    public:
        DaveGame();
        ~DaveGame();

        bool prepareWindowAndTexture();

        void run();
        bool valid() const { return win != nullptr && ren != nullptr && tex != nullptr; }
    };
}
