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
            DAVE,
            MUSHROOM,
            GHOST,
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

    struct BackAndForthMotion {
        SDL_FPoint direction;
        float speed = 60.0f;
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
    /// @brief Represents a  Diamond Prize  and score value of the collectible entity.
    struct Diamond {
        int value = 100;
    };


    /// @brief Marks the entity as bullet.
    struct Bullet {};

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
        uint32_t lastLandedTime = 0;
    };

    /// @brief Marks if the entity is a trophy
    struct Trophy {
        int value = 1000;
    };

    struct Spikes{};

    struct LastShot {
        uint32_t time = 0; // in milliseconds
    };

    struct DoorLabel{};
    struct ScoreLabel{};
    struct LevelLabel{};
    struct GunEquipedLabel{};

    struct LivesHead {
        int index;
    };

    struct MoveScreenSensor {
        bool forward = true;
        int col = 0;
    };

    struct CircularMotion {
        SDL_FPoint center;  // center of the circular path (in pixels)
        float radius;
        float angularSpeed;  // radians per second
        float angle = 0.0f;  // current angle
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
        void CircularMotionSystem();
        void ShooterSystem();
        void BackAndForthMotionSystem();
        void MenuInputSystem();

        void loadLevel(int level);
        void unloadLevel();
        void levelAnimation();
        void createMap(uint8_t* map, int width, int height);

        void createMushroom(int startCol, int startRow);

        void createGhost(int startCol, int startRow);

        void createDave(int startCol, int startRow);
        void createWall(SDL_FPoint p, float width, float height) const;
        void createDiamond(SDL_FPoint p);
        void createDoor(SDL_FPoint p);
        void createTrophy(SDL_FPoint p);
        void createSpikes(SDL_FPoint p);
        void createMoveScreenSensor(SDL_FPoint p,bool forward, int col);
        void createBlock(SDL_FPoint p,SDL_FRect r);
        void createBatMonster(SDL_FPoint p, bool isGunMonster = false);
        void createGun(SDL_FPoint p);
        void createBullet(SDL_FPoint davePos, bool goingLeft);
        void createMonsterBullet(SDL_FPoint monsterPos, bool goingLeft);

        ent_type getGunEquipedEntity();
        void renderMenuOptions();

        void createStatusBar();
        void createTitles();
        void createScoreBar();
        void createLevelAndHealth();


        void EndGame();



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

        //static constexpr float	BOX_SCALE = 64.f;
        static constexpr float	BOX_SCALE = 35.5f;
        static constexpr float	DAVE_TEX_SCALE = 0.42f;
        static constexpr float	BLOCK_TEX_SCALE = 0.56f;


        static constexpr SDL_FRect DAVE_HEALTH{ 1419, 804, 79, 75 };
        static constexpr SDL_FRect SCORE_SPRITE{1082, 694, 280, 71};
        static constexpr SDL_FRect HEALTH_SPRITE{1082, 804, 273, 70};
        static constexpr SDL_FRect LEVEL_SPRITE{1082, 912, 287, 70};


        static constexpr SDL_FRect DAVE_STANDING{ 75, 38, 109, 155 };
        static constexpr SDL_FRect DAVE_WALKING_1{223,38,117,155};
        static constexpr SDL_FRect DAVE_WALKING_2{373,38,117,155};
        static constexpr SDL_FRect DAVE_IDLE{ 75, 38, 109, 155 };
        static constexpr SDL_FRect DAVE_JUMPING{676,38,131,155};

        static constexpr SDL_FRect BAT_MONSTER_1{840,521,122,113};
        static constexpr SDL_FRect BAT_MONSTER_2{683,521,147,111};

        static constexpr SDL_FRect DIAMOND{ 231, 370, 118, 118 };
        static constexpr SDL_FRect RED_DIAMOND{ 75, 370, 118, 118 };
        static constexpr SDL_FRect DOOR{ 525, 366, 118, 118 };
        static constexpr SDL_FRect TROPHY{ 373, 370, 118, 118 };
        static constexpr SDL_FRect RED_BLOCK{ 221, 218, 118, 118 };
        static constexpr SDL_FRect GUN{ 1396, 890, 118, 118 };
        static constexpr SDL_FRect BULLET{ 1538, 917, 53, 24 };
        static constexpr SDL_FRect MONSTER_BULLET{ 1535, 966, 53, 24 };

        static constexpr SDL_FRect SAND{ 525, 218, 118, 118 };
        static constexpr SDL_FRect SKY{ 66, 667, 118, 118 };


        static constexpr SDL_FRect FIRE1{ 1674, 189, 83, 104 };
        static constexpr SDL_FRect FIRE2{ 1790, 196, 84, 95 };

        static constexpr SDL_FRect SPIKES{ 839, 235, 118, 118 };



        static constexpr SDL_FRect MUSHROOM1{ 1096, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM2{ 1294, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM3{ 1491, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM4{ 1688, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM5{ 1885, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM6{ 2082, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM7{ 2279, 179, 75, 120 };
        static constexpr SDL_FRect MUSHROOM8{ 2476, 179, 75, 120 };

        static constexpr SDL_FRect GHOST1{ 66, 520, 116, 120 };
        static constexpr SDL_FRect GHOST2{216, 520, 116, 120 };

        static constexpr SDL_FRect LOGO{72, 668, 690, 207 };
        static constexpr SDL_Point LOGO_POS{308, 80};

        static constexpr SDL_FRect START_GAME{2096, 700, 300, 120 };
        static constexpr SDL_FRect EXIT{2097, 428, 300, 120 };
        static constexpr SDL_FRect START_GAME_SELECTED{2096, 835, 300, 120 };
        static constexpr SDL_FRect EXIT_SELECTED{2096, 558, 300, 120 };


        static constexpr SDL_Point START_GAME_POS{503, 400};
        static constexpr SDL_Point EXIT_POS{503, 600};

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

        bool skipSensorEvents = false;


        static constexpr uint32_t DAVE_FIRE_COOLDOWN_MS = 1000;
        static constexpr uint32_t MONSTER_FIRE_COOLDOWN_MS = 3500;
        static constexpr uint32_t DAVE_JUMP_COOLDOWN_MS = 50;


        static constexpr int MAP_WIDTH = 20;
        static constexpr int MAP_HEIGHT = 10;

        static constexpr int FPS = 60;
        static constexpr float ANIMATION_INTERVAL = 10;

        static constexpr float GAME_FRAME = 1000.f/FPS;
        static constexpr float PHYSICS_TIME_STEP = 1.0f / FPS;
        static constexpr float	RAD_TO_DEG = 57.2958f;

        static constexpr float	ANIMATION_VELOCITY_THRESHOLD = 0.5f; // Velocity threshold to switch between animation states

        static inline  Drawable** DAVE_ANIMATION = nullptr;
        static inline  Drawable** MUSHROOM_ANIMATION = nullptr;
        static inline  Drawable** GHOST_ANIMATION = nullptr;
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
        static constexpr int DAVE_START_ROW = 9;

        static constexpr int BAT_MONSTER_START_COLUMN = 17;
        static constexpr int BAT_MONSTER_START_ROW = 3;


        static constexpr int STATUS_BAR_HEIGHT = 2;
        static constexpr int WIN_WIDTH = MAP_WIDTH * (RED_BLOCK.w * BLOCK_TEX_SCALE);
        static constexpr int WIN_HEIGHT = (MAP_HEIGHT + STATUS_BAR_HEIGHT) * (RED_BLOCK.h * BLOCK_TEX_SCALE);

        static constexpr uint8_t GRID_BACKGROUND = 0;
        static constexpr uint8_t GRID_RED_BLOCK = 1;
        static constexpr uint8_t GRID_DIAMOND = 2;
        static constexpr uint8_t GRID_DOOR = 3;
        static constexpr uint8_t GRID_TROPHY = 4;
        static constexpr uint8_t GRID_SENSOR_BACK = 5;
        static constexpr uint8_t GRID_SENSOR_FORWARD = 6;
        static constexpr uint8_t GRID_SPIKES = 7;
        static constexpr uint8_t GRID_SKY = 8;
        static constexpr uint8_t GRID_SAND = 9;
        static constexpr uint8_t GRID_GUN = 10;

        static constexpr int SCORE_DIGITS_COUNT = 5;

        SDL_Texture* tex;
        SDL_Renderer* ren;
        SDL_Window* win;
        GameInfo gameInfo;

        b2WorldId boxWorld = b2_nullWorldId;

        static inline uint8_t walkingMap[5][20] = {
            /* row 0 (sky) */
            { GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY },

            /* row 1 (sky) */
            { GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY,
              GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY, GRID_SKY },

            /* row 2 (background) */
            { GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
                GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
                GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
              GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, },


            /* row 3 (background) */
            { GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
                GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
                GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
              GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_DOOR, },

            /* row 4 (sand) */
            { GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND,
              GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND,
              GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND,
              GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND, GRID_SAND }
        };


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
                GRID_BACKGROUND,   GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND,
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
                GRID_BACKGROUND,  GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_BACKGROUND,
                GRID_BACKGROUND,  GRID_RED_BLOCK, GRID_TROPHY, GRID_BACKGROUND,
                GRID_BACKGROUND,  GRID_RED_BLOCK, GRID_BACKGROUND, GRID_RED_BLOCK,
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
                GRID_RED_BLOCK,  GRID_RED_BLOCK,  GRID_BACKGROUND,  GRID_BACKGROUND,
                GRID_BACKGROUND, GRID_DIAMOND, GRID_RED_BLOCK, GRID_BACKGROUND,
                GRID_BACKGROUND,  GRID_GUN,  GRID_RED_BLOCK,  GRID_BACKGROUND,
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
                GRID_BACKGROUND, GRID_BACKGROUND, GRID_BACKGROUND, GRID_GUN,
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
                GRID_SPIKES, GRID_SPIKES, GRID_SPIKES, GRID_SPIKES, GRID_SPIKES,
                // repeat
                GRID_SPIKES, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
                GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
                GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK,
                GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK, GRID_RED_BLOCK
            }
        };
    public:
        enum class GameState {
            MENU,
            PLAYING,
            EXIT
        };
        enum class MenuOptions {
            START_GAME,
            EXIT
        };

        DaveGame();
        ~DaveGame();

        bool prepareWindowAndTexture();

        void run();
        bool valid() const { return win != nullptr && ren != nullptr && tex != nullptr; }

    private:
        GameState m_gameState = GameState::MENU;
        int m_selectedOption = 0;
        const int m_optionCount = 2;
    };
}
