#include "dave_game.h"
#include "bagel.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>



using namespace bagel;
using namespace std;
namespace dave_game{
    void DaveGame::run()
    {
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        auto start = SDL_GetTicks();
        bool quit = false;

        while (!quit) {

            InputSystem();
            MovementSystem();
            CircularMotionSystem();
            ShooterSystem();
            box_system();
            CollisionSystem();
            AnimationSystem();
            StatusBarSystem();
            RenderSystem();

            auto end = SDL_GetTicks();
            if (end-start < GAME_FRAME) {
                SDL_Delay(GAME_FRAME - (end-start));
            }
            start += GAME_FRAME;

            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT)
                    quit = true;
                else if ((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE))
                    quit = true;
            }
        }
    }

    DaveGame::DaveGame() {
        if (!prepareWindowAndTexture())
            return;
        SDL_srand(time(nullptr));

        prepareBoxWorld();
        loadLevel(gameInfo.level);

        //createDave(DAVE_START_COLUMN, 3);
        //createMap(&map_stage2[0][0], MAP_WIDTH * 2, MAP_HEIGHT);
        //createDave();
        //createStatusBar();

    }

    /**
    * @brief Cleans up and destroys SDL and Box2D resources.
    */
    DaveGame::~DaveGame()
    {
        if (b2World_IsValid(boxWorld))
            b2DestroyWorld(boxWorld);
        if (tex != nullptr)
            SDL_DestroyTexture(tex);
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    bool DaveGame::prepareWindowAndTexture()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return false;
        }
        if (!SDL_CreateWindowAndRenderer(
            "Dangerous Niv", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return false;
            }
        SDL_Surface *surf = IMG_Load("res/DangerousNiv.png");
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return false;
        }

        tex = SDL_CreateTextureFromSurface(ren, surf);
        if (tex == nullptr) {
            cout << SDL_GetError() << endl;
            return false;
        }

        SDL_DestroySurface(surf);
        return true;
    }

    void DaveGame::prepareBoxWorld()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0, 9.8};
        boxWorld = b2CreateWorld(&worldDef);
    }

    /// @brief Converts player input into high-level intentions (Intent component).
    /// Requires Control and Intent. Checks optional Gun and Jetpack components.
    void DaveGame::InputSystem()
    {
        Mask required = MaskBuilder()
                        .set<Input>()
                        .set<Intent>()
                        .set<Dave>()
                        .build();

        SDL_PumpEvents();
        const bool* keys = SDL_GetKeyboardState(nullptr);
        uint32_t now = SDL_GetTicks();
        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type e{id};
            if (World::mask(e).test(required)) {
                const auto& k = World::getComponent<Input>(e);
                auto& in = World::getComponent<Intent>(e);
                in.up = keys[k.up];
                in.down = keys[k.down];
                in.left = keys[k.left];
                in.right = keys[k.right];

                Drawable& d = World::getComponent<Drawable>(e);
                if (in.left || in.right) {
                    d.flip = in.left; // Flip sprite if moving left
                }

                if (keys[SDL_SCANCODE_SPACE] && World::mask(e).test(Component<Gun>::Bit)) {
                    auto& lastShot = World::getComponent<LastShot>(e);
                    if (now - lastShot.time >= COOLDOWN_MS) {
                        const auto& pos = World::getComponent<Position>(e);
                        bool facingLeft = d.flip;
                        createBullet(pos.p, facingLeft);
                        lastShot.time = now;
                    }
                }
            }
        }
    }
    /// @brief Controls shooting of AI entities
    void DaveGame::ShooterSystem()
    {
        uint32_t now = SDL_GetTicks();

        Mask required = MaskBuilder()
            .set<Gun>()
            .set<Monster>()
            .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(required)) continue;

            auto& pos = World::getComponent<Position>(e);
            auto& drawable = World::getComponent<Drawable>(e);
            auto& lastShot = World::getComponent<LastShot>(e);

            if (now - lastShot.time >= MONSTER_COOLDOWN_MS) {
                createMonsterBullet(pos.p, true);
                lastShot.time = now;
            }
        }
    }

    /// @brief Controls movement of all entities with Position and Course.
    void DaveGame::MovementSystem()
    {
        static const Mask mask = MaskBuilder()
                   .set<Intent>()
                   .set<Collider>()
                   .set<Position>()
                   .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                auto& i = World::getComponent<Intent>(e);
                const auto& c = World::getComponent<Collider>(e);
                bool isDave = World::mask(e).test(Component<Dave>::Bit);

                const auto& vel = b2Body_GetLinearVelocity(c.b);

                const float x = i.left ? -3.f : i.right ? 3.f : 0.f;
                b2Body_SetLinearVelocity(c.b, {x,vel.y});


                if (isDave) {

                    auto& anim = World::getComponent<Animation>(e);
                    auto& groundStatus = World::getComponent<GroundStatus>(e);

                    if (i.up && groundStatus.onGround) {

                        groundStatus.onGround = false;
                        float jumpVelocity = 7.4f;
                        float mass = b2Body_GetMass(c.b);
                        b2Vec2 impulse = {0.0f, -mass * jumpVelocity};
                        b2Body_ApplyLinearImpulseToCenter(c.b, impulse, true);
                    }

                    if (vel.x >= -ANIMATION_VELOCITY_THRESHOLD && vel.x <= ANIMATION_VELOCITY_THRESHOLD && vel.y >= -ANIMATION_VELOCITY_THRESHOLD && vel.y <= ANIMATION_VELOCITY_THRESHOLD) {
                        // If not moving, set to idle state
                        if (anim.currentState != 0) {
                            anim.currentState = 0; // IDLE
                            anim.currentFrame = 0;
                            groundStatus.onGround = true;
                        }

                    } else if (vel.y >= -ANIMATION_VELOCITY_THRESHOLD && vel.y <= ANIMATION_VELOCITY_THRESHOLD) {
                        // If moving, set to walk state
                        if (anim.currentState != 1) {
                            anim.currentState = 1; // WALK
                            anim.currentFrame = 0;
                            groundStatus.onGround = true;
                        }
                    }
                    else {
                        // If jumping or falling, set to jump state
                        if (anim.currentState != 2) {
                            anim.currentState = 2; // JUMP
                            anim.currentFrame = 0;
                            groundStatus.onGround = false;
                        }
                    }
                }
            }
        }
    }

    void DaveGame::renderGoThruTheDoor() {
        static const Mask labelMask = MaskBuilder()
                   .set<DoorLabel>()
                   .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(labelMask)) {
                auto& d = World::getComponent<Drawable>(e);
                d.visible = true;
            }
        }

        static const Mask doorMask = MaskBuilder()
           .set<Door>()
           .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(doorMask)) {
                auto& d = World::getComponent<Door>(e);
                d.open=true;
            }
        }
    }

    void DaveGame::CollisionSystem()
    {
        if (skipSensorEvents) return;
        const auto sensorEvents = b2World_GetSensorEvents(boxWorld);

        for(int i = 0 ; i < sensorEvents.beginCount ; i++)
        {
            b2BodyId sensor = b2Shape_GetBody(sensorEvents.beginEvents[i].sensorShapeId);
            b2BodyId visitor = b2Shape_GetBody(sensorEvents.beginEvents[i].visitorShapeId);
            auto *visitorEntity = static_cast<ent_type*>(b2Body_GetUserData(visitor));
            auto *sensorEntity = static_cast<ent_type*>(b2Body_GetUserData(sensor));


            bool sensorIsDave = World::mask(*sensorEntity).test(Component<Dave>::Bit);
            bool sensorIsBullet = World::mask(*sensorEntity).test(Component<Bullet>::Bit);

            bool visitorIsWall = World::mask(*visitorEntity).test(Component<Wall>::Bit);
            bool visitorIsDiamond = World::mask(*visitorEntity).test(Component<Diamond>::Bit);
            bool visitorIsDoor = World::mask(*visitorEntity).test(Component<Door>::Bit);
            bool visitorIsTrophy = World::mask(*visitorEntity).test(Component<Trophy>::Bit);
            bool visitorIsMoveScreen = World::mask(*visitorEntity).test(Component<MoveScreenSensor>::Bit);
            bool visitorIsSpikes = World::mask(*visitorEntity).test(Component<Spikes>::Bit);
            bool visitorIsMonster = World::mask(*visitorEntity).test(Component<Monster>::Bit);
            bool visitorIsGun = World::mask(*visitorEntity).test(Component<Gun>::Bit);
            bool visitorIsBullet = World::mask(*visitorEntity).test(Component<Bullet>::Bit);

            if (sensorIsDave && visitorIsWall)
            {
                auto& davePos = World::getComponent<Position>(*sensorEntity);
                auto& wallPos = World::getComponent<Position>(*visitorEntity);

                float daveBottom = davePos.p.y + (DAVE_JUMPING.h * DAVE_TEX_SCALE / 2);
                float wallTop = wallPos.p.y - (RED_BLOCK.h * BLOCK_TEX_SCALE / 2);

                if (daveBottom <= wallTop + 4.f) { // 4px margin to allow slight overlap
                    auto& groundStatus = World::getComponent<GroundStatus>(*sensorEntity);
                    groundStatus.onGround = true;
                }
            }

            else if (sensorIsDave && visitorIsDiamond) {
                auto& diamond = World::getComponent<Diamond>(*visitorEntity);
                gameInfo.score +=  diamond.value;
                World::destroyEntity(*visitorEntity);
                b2DestroyBody(visitor);
            }
            else if (sensorIsDave && visitorIsDoor) {
                auto& door = World::getComponent<Door>(*visitorEntity);
                if (door.open) {
                    //levelAnimation();
                    loadLevel(++gameInfo.level);
                    std::cout << "finish load new level: " << std::endl;
                    break;
                }
            }
            else if (sensorIsDave && visitorIsTrophy) {
                auto& trophy = World::getComponent<Trophy>(*visitorEntity);
                gameInfo.score += trophy.value; // Increase score by 100 for collecting a trophy
                World::destroyEntity(*visitorEntity);
                b2DestroyBody(visitor);
                renderGoThruTheDoor();
            }
            else if (sensorIsDave && visitorIsMoveScreen) {
                auto& moveScreen = World::getComponent<MoveScreenSensor>(*visitorEntity);
                int screen = gameInfo.screenOffset / 0.5f;
                if (moveScreen.forward &&  moveScreen.col == screen) {
                    gameInfo.screenOffset += .5f; // Move screen forward
                } else if (!moveScreen.forward && moveScreen.col == screen - 1) {
                    gameInfo.screenOffset -= .5f; // Move screen backward
                }
            }
            else if (sensorIsDave && visitorIsSpikes || sensorIsDave && visitorIsMonster) {
                gameInfo.lives--;
                if (gameInfo.lives <= 0) {
                    EndGame();
                    break;// End game if no lives left
                }
                gameInfo.screenOffset = 0.f;
                World::destroyEntity(*sensorEntity);
                b2DestroyBody(sensor);
                createDave(DAVE_START_COLUMN, DAVE_START_ROW);
                break;
            }
            else if (sensorIsDave && visitorIsGun) {
                auto& gun = World::getComponent<Gun>(*visitorEntity);
                World::destroyEntity(*visitorEntity);
                b2DestroyBody(visitor);

                auto gunEquipped = Entity::create();
                gunEquipped.addAll(
                    Position{{0.5 * RED_BLOCK.w * BLOCK_TEX_SCALE, 11.5 * RED_BLOCK.h * BLOCK_TEX_SCALE}, 0},
                    Drawable{GUN, BLOCK_TEX_SCALE, true, false, true}
                );

                World::addComponent(*sensorEntity, Gun{});
                World::addComponent<LastShot>(*sensorEntity, LastShot{});
            }
            else if (sensorIsBullet) {
                if (visitorIsBullet) {
                    continue;
                }

                bool bulletFromMonster = World::mask(*sensorEntity).test(Component<Monster>::Bit);
                if (visitorIsMonster && !bulletFromMonster) {
                    World::destroyEntity(*sensorEntity);
                    World::destroyEntity(*visitorEntity);
                    b2DestroyBody(sensor);
                    b2DestroyBody(visitor);
                    break;
                }
                if (visitorIsWall) {
                    World::destroyEntity(*sensorEntity);
                    b2DestroyBody(sensor);
                    break;
                }
            }


        }
    }

    /**
    * @brief Synchronizes Box2D world step and updates entity positions and angles from physics bodies.
    */

    void DaveGame::box_system()
    {
        static const Mask mask = MaskBuilder()
            .set<Collider>()
            .set<Position>()
            .build();
        static constexpr float	BOX2D_STEP = 1.f/FPS;
        b2World_Step(boxWorld, BOX2D_STEP, 4);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                b2Transform t = b2Body_GetTransform(World::getComponent<Collider>(e).b);

                auto & c = World::getComponent<Collider>(e);
                b2BodyId body = c.b;
                // Get Box2D center position (in meters)
                b2Vec2 pos = b2Body_GetPosition(body);

                // Convert to pixels
                float centerX = pos.x * BOX_SCALE;
                float centerY = pos.y * BOX_SCALE;
                World::getComponent<Position>(e) = {
                    {centerX, centerY},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
            }
        }
    }

    void DaveGame::loadLevel(int level) {
        unloadLevel();
        gameInfo.screenOffset = 0.f;
        (void)Component<Bullet>::Bit;
        if (level == 2) {//TODO temp for DEBUG - need to switch with level 1
            createMap(&map[0][0], MAP_WIDTH, MAP_HEIGHT);
            createDave(DAVE_START_COLUMN, DAVE_START_ROW);
            createStatusBar();
        } else if (level == 1) {
            SDL_FPoint batMonsterSpawnPoint = {
                BAT_MONSTER_START_COLUMN * RED_BLOCK.w * BLOCK_TEX_SCALE,
                BAT_MONSTER_START_ROW * RED_BLOCK.h * BLOCK_TEX_SCALE
            };

            //createBatMonster(batMonsterSpawnPoint, true);
            //createMushroom(DAVE_START_COLUMN + 1, DAVE_START_ROW);
            //createGhost(DAVE_START_COLUMN + 2, DAVE_START_ROW);
            createMap(&map_stage2[0][0], MAP_WIDTH * 2, MAP_HEIGHT);
            cout << "Loaded map of level: " << level << endl;
            createDave(DAVE_START_COLUMN, DAVE_START_ROW);
            cout << "create dave in level " << level << endl;
            createStatusBar();
            cout << "create status bar in level " << level << endl;
        } else {
            cout << "Invalid level: " << level << endl;
            EndGame();
            return;
        }
    }

    void DaveGame::unloadLevel() {
        Mask required = MaskBuilder()
            .set<Collider>()
            .build();
        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type e{id};
            if (World::mask(e).test(required)) {
                auto& c = World::getComponent<Collider>(e);
                World::destroyEntity(e);
                b2DestroyBody(c.b);
            }else {
                World::destroyEntity(e);
            }
        }
        cout<< "Unloaded level: " << gameInfo.level - 1 << endl;
    }

    void DaveGame::levelAnimation() {
        unloadLevel();
        createMap(&walkingMap[0][0], MAP_WIDTH, 5);
        createDave(0, 3);

        const float tileWidth = RED_BLOCK.w * BLOCK_TEX_SCALE;
        const float finalX = (MAP_WIDTH - 2) * tileWidth;  // Last column in pixels

        for (id_type i = 0; i <= World::maxId().id; ++i) {
            ent_type e{i};
            if (World::mask(e).test(Component<Dave>::Bit)) {
                auto& pos = World::getComponent<Position>(e);
                auto& in = World::getComponent<Intent>(e);
                in.right = true;
                bool end = false;
                auto start = SDL_GetTicks();
                while (!end) {
                    // Update input, physics, movement, etc.
                    MovementSystem();
                    box_system();
                    AnimationSystem();
                    RenderSystem();

                    // Check Dave's current position
                    if (pos.p.x >= finalX) {
                        end = true; // Dave reached the last column
                    }

                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_EVENT_QUIT)
                            end = true;
                        else if ((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE))
                            end = true;
                    }

                    auto end = SDL_GetTicks();
                    if (end-start < GAME_FRAME) {
                        SDL_Delay(GAME_FRAME - (end-start));
                    }
                    start += GAME_FRAME;
                }
                break;
            }
        }
        unloadLevel(); // Clean up
    }


    void DaveGame::StatusBarSystem() {
        int score = gameInfo.score;
        int digit = 0;

        static const Mask score_mask = MaskBuilder()
            .set<ScoreLabel>()
            .set<Drawable>()
            .build();

        static const Mask level_mask = MaskBuilder()
            .set<LevelLabel>()
            .set<Drawable>()
            .build();
        static const Mask Lives = MaskBuilder()
            .set<LivesHead>()
            .set<Drawable>()
            .build();

        for (id_type i =0; i <= World::maxId().id; ++i) {
            ent_type e{i};

            if (World::mask(e).test(score_mask)) {
                digit = score % 10;
                score /= 10;
                auto& drawable = World::getComponent<Drawable>(e);
                drawable.part = NUMBERS_SPRITES[digit].part;
            }
            else if (World::mask(e).test(level_mask)) {
                auto& drawable = World::getComponent<Drawable>(e);
                drawable.part = NUMBERS_SPRITES[gameInfo.level].part;
            }
            else if (World::mask(e).test(Lives)) {
                auto& lh = World::getComponent<LivesHead>(e);
                if (gameInfo.lives == lh.index) {
                    World::destroyEntity(e);
                }
            }
        }
    }


    /// @brief Renders entities with Image, Position, and GameInfo components.
    /// Also checks optional components like Course, Collision, Gun, and Jetpack.
    void DaveGame::RenderSystem()
    {
        MaskBuilder builder;
        builder.set<Position>().set<Drawable>();
        Mask required = builder.build();

        SDL_RenderClear(ren);

        for (int i = 0; i <= World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }
            //debug printing pyisic
             if (World::mask(e).test(Component<Collider>::Bit))
             {
                 const Collider& collider = World::getComponent<Collider>(e);
                 const auto& dr = World::getComponent<Drawable>(e);

                     b2BodyId body = collider.b;

                     // Get Box2D center position (in meters)
                     b2Vec2 pos = b2Body_GetPosition(body);

                     // Convert to pixels
                     float centerX = pos.x * BOX_SCALE;
                     float centerY = pos.y * BOX_SCALE;
                     float w,h;
                     if ( World::mask(e).test(Component<Wall>::Bit)) {///DEBUG
                         const Wall& wall = World::getComponent<Wall>(e); // stores shape ID
                         w = wall.size.x * BLOCK_TEX_SCALE;
                         h = wall.size.y * BLOCK_TEX_SCALE;
                     }else if ( World::mask(e).test(Component<Monster>::Bit)) {
                         w = BAT_MONSTER_1.w * BLOCK_TEX_SCALE; // back to pixels
                         h = BAT_MONSTER_1.h * BLOCK_TEX_SCALE;
                     }
                     else {
                         w = DAVE_JUMPING.w * DAVE_TEX_SCALE; // back to pixels
                         h = DAVE_JUMPING.h * DAVE_TEX_SCALE;
                     }

                     // Top-left corner
                     SDL_FRect boxRect = {
                         centerX - w/2 - (gameInfo.screenOffset * (!dr.isStatic) * WIN_WIDTH),
                         centerY - h/2,
                         w,
                         h
                     };
                 SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
                 SDL_RenderRect(ren, &boxRect);
                 SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
             }

            const auto& pos = World::getComponent<Position>(e);
            const auto& drawable = World::getComponent<Drawable>(e);

            if (!drawable.visible)
            {
                continue; // Skip rendering if not visible
            }

            if (World::mask(e).test(Component<Wall>::Bit)) {
                auto& wall = World::getComponent<Wall>(e);
                const SDL_FRect dst = {
                    pos.p.x - (wall.size.x * drawable.scale / 2) - (gameInfo.screenOffset * (!drawable.isStatic) * WIN_WIDTH),
                    pos.p.y - (wall.size.y * drawable.scale / 2),
                    drawable.part.w * drawable.scale,
                    drawable.part.h * drawable.scale
                };
                int tilesNum = wall.size.x / RED_BLOCK.w ;

                for (int j = 0; j < tilesNum; ++j) {
                    SDL_FRect tileDst = dst;
                    tileDst.x += j * RED_BLOCK.w * BLOCK_TEX_SCALE;
                    SDL_RenderTextureRotated(ren, tex, &drawable.part, &tileDst, 0, nullptr, SDL_FLIP_NONE);
                }
            }
            else {
            const SDL_FRect dst = {
                pos.p.x - (drawable.part.w * drawable.scale / 2) - (gameInfo.screenOffset * (!drawable.isStatic) * WIN_WIDTH),
                pos.p.y - (drawable.part.h * drawable.scale / 2),
                drawable.part.w * drawable.scale,
                drawable.part.h * drawable.scale
            };

            SDL_FlipMode flip = drawable.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

            SDL_RenderTextureRotated(
                ren, tex, &drawable.part, &dst, 0,
                nullptr, flip);
            }
        }
        SDL_RenderPresent(ren);
    }

    void DaveGame::CircularMotionSystem()
    {
        static const Mask mask = MaskBuilder()
            .set<CircularMotion>()
            .set<Collider>()
            .build();

        float dt = PHYSICS_TIME_STEP; // seconds per frame

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(mask)) continue;

            auto& motion = World::getComponent<CircularMotion>(e);
            auto& col = World::getComponent<Collider>(e);

            motion.angle += motion.angularSpeed * dt;
            if (motion.angle > 2 * M_PI) motion.angle -= 2 * M_PI;

            float newX = motion.center.x + motion.radius * cosf(motion.angle);
            float newY = motion.center.y + motion.radius * sinf(motion.angle);

            b2Body_SetTransform(col.b, {newX / BOX_SCALE, newY / BOX_SCALE}, b2MakeRot(0.0f));

        }
    }



    /// @brief Updates animation state for entities with visual animations.
    /// Requires Animation and Image components.
    void DaveGame::AnimationSystem()
    {
        static int frameCounter = 0;

        ++frameCounter; // Increment time counter by frame duration

        if (frameCounter < ANIMATION_INTERVAL) return;


        MaskBuilder builder;
        builder.set<Animation>().set<Drawable>();
        Mask required = builder.build();

        for (int i = 0; i <= World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }


            auto& anim = World::getComponent<Animation>(e);
            auto& sprite = World::getComponent<Drawable>(e);
            auto flip = sprite.flip;

            sprite = anim.states_frames[anim.currentState][anim.currentFrame];
            sprite.flip = flip;

            anim.currentFrame++;
            anim.currentFrame %= anim.framesCount;
        }

        frameCounter = 0;
    }

    /// @brief Creates the player entity (Dave) with default attributes.
    void DaveGame::createDave(int startCol, int startRow)
    {
    // Calculate top-left corner of Dave's starting cell in pixels
    SDL_FPoint topLeft = {
        startCol * RED_BLOCK.w * BLOCK_TEX_SCALE,
        startRow * RED_BLOCK.h * BLOCK_TEX_SCALE
    };

    // Calculate Dave's center position for Box2D
    SDL_FPoint center = {
        topLeft.x + DAVE_JUMPING.w * DAVE_TEX_SCALE / 2.0f,
        topLeft.y + DAVE_JUMPING.h * DAVE_TEX_SCALE/ 2.0f
    };

    b2BodyDef daveBodyDef = b2DefaultBodyDef();
    daveBodyDef.type = b2_dynamicBody;
    daveBodyDef.position = {
        center.x / BOX_SCALE,
        center.y / BOX_SCALE
    };
    daveBodyDef.fixedRotation = true;
    b2BodyId daveBody = b2CreateBody(boxWorld, &daveBodyDef);


    b2ShapeDef daveShapeDef = b2DefaultShapeDef();
    daveShapeDef.density = 20.f;
    daveShapeDef.enableSensorEvents = false;
    daveShapeDef.isSensor = false;
    b2SurfaceMaterial mat = {
        .friction = 0.0f,
        .restitution = 0.0f,
        .rollingResistance = 0.0f,
        .tangentSpeed = 0.0f,
        .userMaterialId = 0,
        .customColor = 0  // Or 0xFFFFFFFF if you want to debug
    };
    daveShapeDef.material = mat;





    b2Polygon daveBox = b2MakeBox(
        (DAVE_JUMPING.w * DAVE_TEX_SCALE / BOX_SCALE) / 2,
        (DAVE_JUMPING.h * DAVE_TEX_SCALE / BOX_SCALE) / 2
    );
    b2CreatePolygonShape(daveBody, &daveShapeDef, &daveBox);

    // Optional sensor shape (e.g., for ground detection)
    b2ShapeDef daveShapeDef2 = b2DefaultShapeDef();
    daveShapeDef2.enableSensorEvents = true;
    daveShapeDef2.isSensor = true;


    b2Polygon daveBox2 = b2MakeBox(
        (DAVE_JUMPING.w * DAVE_TEX_SCALE / BOX_SCALE) / 2,
        (DAVE_JUMPING.h * DAVE_TEX_SCALE / BOX_SCALE) / 2
    );

    b2CreatePolygonShape(daveBody, &daveShapeDef2, &daveBox2);
    // Set up animation frames
    DAVE_ANIMATION = new Drawable*[3]{
        new Drawable[4]{ // IDLE
            {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
            {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
            {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
            {DAVE_IDLE, DAVE_TEX_SCALE, true, false}
        },
        new Drawable[4]{ // Walking
            {DAVE_STANDING, DAVE_TEX_SCALE, true, false},
            {DAVE_WALKING_1, DAVE_TEX_SCALE, true, false},
            {DAVE_STANDING, DAVE_TEX_SCALE, true, false},
            {DAVE_WALKING_2, DAVE_TEX_SCALE, true, false}
        },
        new Drawable[4]{ // Jump
            {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
            {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
            {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
            {DAVE_JUMPING, DAVE_TEX_SCALE, true, false}
        }
    };
    Entity e = Entity::create();
    e.addAll(
        Position{center, 0},
        Drawable{DAVE_STANDING, DAVE_TEX_SCALE, true, false},
        Collider{daveBody},
        Intent{},
        Animation{DAVE_ANIMATION, 1, 4, 0, 0, Animation::Type::DAVE},
        Input{SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT},
        Dave{},
        GroundStatus{true}
    );

    b2Body_SetUserData(daveBody, new ent_type{e.entity()});
    std::cout << "Dave entity created with ID: " << e.entity().id << std::endl;
    }


    void DaveGame::createMap(uint8_t* map, int width, int height) {
        for (int row = 0; row < height; ++row) {
            int wallStartCol = -1;
            for (int col = 0; col < width; ++col) {
                int row_to_print = row + 1; // Offset by 1 to account for the status bar
                uint8_t* map_row = (map + row * width);
                if (map_row[col] == GRID_RED_BLOCK) {
                    if (wallStartCol == -1) {
                        wallStartCol = col;  // start of new wall segment
                    }
                    if (col + 1 >= width || map_row[col+1] != GRID_RED_BLOCK) {
                        // end of a wall segment
                        int wallEndCol = col;  // exclusive

                        float wallWidth = ((wallEndCol - wallStartCol) + 1)* RED_BLOCK.w ;
                        float wallHeight = RED_BLOCK.h;

                        SDL_FPoint p = {(wallStartCol * RED_BLOCK.w * BLOCK_TEX_SCALE), row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                        createWall(p, wallWidth, wallHeight);
                        wallStartCol = -1;

                    }

                    //SDL_FPoint p = {(col * RED_BLOCK.w * BLOCK_TEX_SCALE), row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    //createWall(p,RED_BLOCK.w,RED_BLOCK.h);
                }
                else if (map_row[col] == GRID_DIAMOND) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createDiamond(p);
                }
                else if (map_row[col] == GRID_DOOR) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createDoor(p);
                }
                else if (map_row[col] == GRID_TROPHY) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createTrophy(p);
                }
                else if (map_row[col] == GRID_SENSOR_BACK) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createMoveScreenSensor(p, false, col/20);
                }
                else if (map_row[col] == GRID_SENSOR_FORWARD) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createMoveScreenSensor(p, true, col/20);
                }
                else if (map_row[col] == GRID_SPIKES) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createSpikes(p);
                }else if (map_row[col] == GRID_SKY) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createBlock(p, SKY);
                } else if (map_row[col] == GRID_SAND) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createBlock(p, SAND);
                }
                else if (map_row[col] == GRID_GUN) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createGun(p);
                }
            }
        }
    }

    void DaveGame::createMushroom(int startCol, int startRow)
    {
    SDL_FPoint topLeft = {
        startCol * RED_BLOCK.w * BLOCK_TEX_SCALE,
        startRow * RED_BLOCK.h * BLOCK_TEX_SCALE
    };

    SDL_FPoint center = {
        topLeft.x + MUSHROOM1.w * BLOCK_TEX_SCALE / 2.0f,
        topLeft.y + MUSHROOM1.h * BLOCK_TEX_SCALE/ 2.0f
    };

    b2BodyDef mushroomBodyDef = b2DefaultBodyDef();
    mushroomBodyDef.type = b2_dynamicBody;
    mushroomBodyDef.position = {
        center.x / BOX_SCALE,
        center.y / BOX_SCALE
    };
    mushroomBodyDef.fixedRotation = true;
    b2BodyId mushroomBody = b2CreateBody(boxWorld, &mushroomBodyDef);


    b2ShapeDef mushroomShapeDef = b2DefaultShapeDef();
    mushroomShapeDef.density = 20.f;
    mushroomShapeDef.enableSensorEvents = true;
    b2SurfaceMaterial mat = {
        .friction = 0.0f,
        .restitution = 0.0f,
        .rollingResistance = 0.0f,
        .tangentSpeed = 0.0f,
        .userMaterialId = 0,
        .customColor = 0  // Or 0xFFFFFFFF if you want to debug
    };
    mushroomShapeDef.material = mat;
    b2Polygon mushroomBox = b2MakeBox(
        (MUSHROOM1.w * BLOCK_TEX_SCALE / BOX_SCALE) / 2,
        (MUSHROOM1.h * BLOCK_TEX_SCALE / BOX_SCALE) / 2
    );
    b2CreatePolygonShape(mushroomBody, &mushroomShapeDef, &mushroomBox);
    // Set up animation frames
    MUSHROOM_ANIMATION = new Drawable*[1]{
        new Drawable[8]{ // Walking
            {MUSHROOM1, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM2, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM3, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM4, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM5, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM6, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM7, BLOCK_TEX_SCALE, true, false},
            {MUSHROOM8, BLOCK_TEX_SCALE, true, false}
        }
    };
    Entity e = Entity::create();
    e.addAll(
        Position{center, 0},
        Drawable{MUSHROOM1, BLOCK_TEX_SCALE, true, false},
        Collider{mushroomBody},
        Monster{},
        Animation{MUSHROOM_ANIMATION, 1, 8, 0, 0, Animation::Type::MUSHROOM}
    );

    b2Body_SetUserData(mushroomBody, new ent_type{e.entity()});
    std::cout << "Mushroom entity created with ID: " << e.entity().id << std::endl;
    }

    void DaveGame::createGhost(int startCol, int startRow)
    {
    SDL_FPoint topLeft = {
        startCol * RED_BLOCK.w * BLOCK_TEX_SCALE,
        startRow * RED_BLOCK.h * BLOCK_TEX_SCALE
    };

    SDL_FPoint center = {
        topLeft.x + GHOST1.w * BLOCK_TEX_SCALE / 2.0f,
        topLeft.y + GHOST1.h * BLOCK_TEX_SCALE/ 2.0f
    };

    b2BodyDef ghostBodyDef = b2DefaultBodyDef();
    ghostBodyDef.type = b2_dynamicBody;
    ghostBodyDef.position = {
        center.x / BOX_SCALE,
        center.y / BOX_SCALE
    };
    ghostBodyDef.fixedRotation = true;
    b2BodyId ghostBody = b2CreateBody(boxWorld, &ghostBodyDef);


    b2ShapeDef ghostShapeDef = b2DefaultShapeDef();
    ghostShapeDef.density = 20.f;
    ghostShapeDef.enableSensorEvents = true;
    b2SurfaceMaterial mat = {
        .friction = 0.0f,
        .restitution = 0.0f,
        .rollingResistance = 0.0f,
        .tangentSpeed = 0.0f,
        .userMaterialId = 0,
        .customColor = 0  // Or 0xFFFFFFFF if you want to debug
    };
    ghostShapeDef.material = mat;
    b2Polygon ghostBox = b2MakeBox(
        (GHOST1.w * BLOCK_TEX_SCALE / BOX_SCALE) / 2,
        (GHOST1.h * BLOCK_TEX_SCALE / BOX_SCALE) / 2
    );
    b2CreatePolygonShape(ghostBody, &ghostShapeDef, &ghostBox);
    // Set up animation frames
    GHOST_ANIMATION = new Drawable*[1]{
        new Drawable[2]{ // Walking
            {GHOST1, BLOCK_TEX_SCALE, true, false},
            {GHOST2, BLOCK_TEX_SCALE, true, false}
        }
    };
    Entity e = Entity::create();
    e.addAll(
        Position{center, 0},
        Drawable{GHOST1, BLOCK_TEX_SCALE, true, false},
        Collider{ghostBody},
        Monster{},
        Animation{GHOST_ANIMATION, 1, 2, 0, 0, Animation::Type::GHOST}
    );

    b2Body_SetUserData(ghostBody, new ent_type{e.entity()});
    std::cout << "GHOST entity created with ID: " << e.entity().id << std::endl;
    }
    void DaveGame::createBlock(SDL_FPoint p,SDL_FRect r) {
        SDL_FPoint center = {
            p.x + r.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + r.h * BLOCK_TEX_SCALE / 2.0f
        };
        b2BodyDef spikeBodyDef = b2DefaultBodyDef();
        spikeBodyDef.type = b2_staticBody;
        spikeBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId blockBody = b2CreateBody(boxWorld, &spikeBodyDef);

        b2ShapeDef blockShapeDef = b2DefaultShapeDef();
        blockShapeDef.enableSensorEvents = true;

        b2Polygon blockBox = b2MakeBox((DIAMOND.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DIAMOND.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(blockBody, &blockShapeDef, &blockBox);

        Entity ent = Entity::create();
        ent.addAll(
            Position{center, 0},
            Drawable{r, BLOCK_TEX_SCALE, true, false},
            Collider{blockBody}
            );
        b2Body_SetUserData(blockBody, new ent_type{ent.entity()});
        std::cout << "BLock entity created with ID: " << ent.entity().id << std::endl;
    }

    void DaveGame::createWall(SDL_FPoint p, float width, float height) const {
        SDL_FPoint center = {
            p.x + width * BLOCK_TEX_SCALE / 2.0f,
            p.y + height * BLOCK_TEX_SCALE / 2.0f
        };

        b2BodyDef wallBodyDef = b2DefaultBodyDef();
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        wallBodyDef.fixedRotation = true;

        b2BodyId wallBody = b2CreateBody(boxWorld, &wallBodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableSensorEvents = true;

        b2SurfaceMaterial wallMat = {
            .friction = 0.0f,
            .restitution = 0.0f,
            .rollingResistance = 0.0f,
            .tangentSpeed = 0.0f,
            .userMaterialId = 0,
            .customColor = 0
        };
        shapeDef.material = wallMat;

        b2Polygon box = b2MakeBox((width*BLOCK_TEX_SCALE/BOX_SCALE)/2, (height*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2ShapeId shape = b2CreatePolygonShape(wallBody, &shapeDef, &box);

        Entity e = Entity::create();
        e.addAll(
            Position{{}, 0},  // Still use top-left for rendering if needed
            Collider{wallBody},
            Wall{shape, {width, height}},
            Drawable{RED_BLOCK, BLOCK_TEX_SCALE, true, false}
        );
        b2Body_SetUserData(wallBody, new ent_type{e.entity()});
        std::cout << "Wall entity created with ID: " << e.entity().id << std::endl;

    }

    void DaveGame::createSpikes(SDL_FPoint p) {
        SDL_FPoint center = {
            p.x + SPIKES.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + SPIKES.h * BLOCK_TEX_SCALE / 2.0f
        };
        b2BodyDef spikeBodyDef = b2DefaultBodyDef();
        spikeBodyDef.type = b2_staticBody;
        spikeBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId spikeBody = b2CreateBody(boxWorld, &spikeBodyDef);

        b2ShapeDef spikeShapeDef = b2DefaultShapeDef();
        spikeShapeDef.enableSensorEvents = true;

        b2Polygon spikeBox = b2MakeBox((DIAMOND.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DIAMOND.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(spikeBody, &spikeShapeDef, &spikeBox);

        Entity ent = Entity::create();
        ent.addAll(
            Position{center, 0},
            Drawable{SPIKES, BLOCK_TEX_SCALE, true, false},
            Collider{spikeBody},
            Spikes{}
        );
        b2Body_SetUserData(spikeBody, new ent_type{ent.entity()});
        std::cout << "Spikes entity created with ID: " << ent.entity().id << std::endl;
    }

    void DaveGame::createDiamond(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + DIAMOND.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + DIAMOND.h * BLOCK_TEX_SCALE / 2.0f
        };
        b2BodyDef diamondBodyDef = b2DefaultBodyDef();
        diamondBodyDef.type = b2_staticBody;
        diamondBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId diamondBody = b2CreateBody(boxWorld, &diamondBodyDef);

        b2ShapeDef diamondShapeDef = b2DefaultShapeDef();
        diamondShapeDef.enableSensorEvents = true;

        b2Polygon diamondBox = b2MakeBox((DIAMOND.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DIAMOND.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(diamondBody, &diamondShapeDef, &diamondBox);

        Entity diamond = Entity::create();
        diamond.addAll(
            Position{center, 0},
            Drawable{DIAMOND, BLOCK_TEX_SCALE, true, false},
            Collider{diamondBody},
            Diamond{}
        );
        b2Body_SetUserData(diamondBody, new ent_type{diamond.entity()});
        std::cout << "Diamond entity created with ID: " << diamond.entity().id << std::endl;
    }

    void DaveGame::createTrophy(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + TROPHY.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + TROPHY.h * BLOCK_TEX_SCALE / 2.0f
        };

        b2BodyDef trophyBodyDef = b2DefaultBodyDef();
        trophyBodyDef.type = b2_staticBody;
        trophyBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId trophyBody = b2CreateBody(boxWorld, &trophyBodyDef);

        b2ShapeDef trophyShapeDef = b2DefaultShapeDef();
        trophyShapeDef.enableSensorEvents = true;

        b2Polygon diamondBox = b2MakeBox((TROPHY.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (TROPHY.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(trophyBody, &trophyShapeDef, &diamondBox);


        Entity trophy = Entity::create();
        trophy.addAll(
            Position{center, 0},
            Drawable{TROPHY, BLOCK_TEX_SCALE, true, false},
            Collider{trophyBody},
            Trophy{}
        );
        b2Body_SetUserData(trophyBody, new ent_type{trophy.entity()});
        std::cout << "Trophy entity created with ID: " << trophy.entity().id << std::endl;
    }

    void DaveGame::createDoor(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + DOOR.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + DOOR.h * BLOCK_TEX_SCALE / 2.0f
        };

        b2BodyDef doorBodyDef = b2DefaultBodyDef();
        doorBodyDef.type = b2_staticBody;
        doorBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId doorBody = b2CreateBody(boxWorld, &doorBodyDef);

        b2ShapeDef doorShapeDef = b2DefaultShapeDef();
        doorShapeDef.enableSensorEvents = true;
        //doorShapeDef.isSensor = true;

        b2Polygon diamondBox = b2MakeBox((DOOR.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DOOR.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(doorBody, &doorShapeDef, &diamondBox);

        Entity door = Entity::create();
        door.addAll(
            Position{center, 0},
            Drawable{DOOR, BLOCK_TEX_SCALE, true, false},
            Collider{doorBody},
            Door{}
        );
        b2Body_SetUserData(doorBody, new ent_type{door.entity()});
        std::cout << "Door entity created with ID: " << door.entity().id << std::endl;
    }

    void DaveGame::createMoveScreenSensor(SDL_FPoint p, bool forward, int col) {

        SDL_FPoint center = {
            p.x + RED_BLOCK.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + RED_BLOCK.h * BLOCK_TEX_SCALE / 2.0f
        };

        b2BodyDef sensorBodyDef = b2DefaultBodyDef();
        sensorBodyDef.type = b2_staticBody;
        sensorBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId sensorBody = b2CreateBody(boxWorld, &sensorBodyDef);

        b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
        sensorShapeDef.enableSensorEvents = true;
        sensorShapeDef.isSensor = true;

        b2Polygon sensorBox = b2MakeBox((RED_BLOCK.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (RED_BLOCK.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(sensorBody, &sensorShapeDef, &sensorBox);


        Entity ent = Entity::create();
        ent.addAll(
            Position{center, 0},
            Collider{sensorBody},
            MoveScreenSensor{forward, col}
        );
        b2Body_SetUserData(sensorBody, new ent_type{ent.entity()});
        std::cout << "Sensor entity created with ID: " << ent.entity().id << std::endl;

    }


    void DaveGame::createStatusBar() {
        createTitles();
        createScoreBar();
        createLevelAndHealth();

    }

    void DaveGame::createTitles() {

        auto score = Entity::create();
        score.addAll(
            Position{{2 * RED_BLOCK.w * BLOCK_TEX_SCALE, 35}, 0},
            Drawable{SCORE_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );
        std::cout << "Score label entity created with ID: " << score.entity().id << std::endl;


        auto level = Entity::create();
        level.addAll(
            Position{{9 * RED_BLOCK.w * BLOCK_TEX_SCALE, 35}, 0},
            Drawable{LEVEL_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );

        auto daves = Entity::create();
        daves.addAll(
            Position{{13 * RED_BLOCK.w * BLOCK_TEX_SCALE, 35}, 0},
            Drawable{HEALTH_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );

        auto openDoor = Entity::create();
        openDoor.addAll(
            Position{{WIN_WIDTH/2, (12 * RED_BLOCK.h * BLOCK_TEX_SCALE) - (RED_BLOCK.h / 2) * BLOCK_TEX_SCALE}, 0},
            Drawable{{1084, 37, 1496, 118}, BLOCK_TEX_SCALE, false, false, true},
            DoorLabel{}
        );
    }

    void DaveGame::createScoreBar() {

        for (int i=SCORE_DIGITS_COUNT - 1; i >= 0; --i) {
            auto entity = Entity::create();
            entity.addAll(
                Position{{(i+1) * 40.f + 210, 35}, 0},
                Drawable{NUMBERS_SPRITES[i+5]},
                ScoreLabel{}
            );
        }

    }

    void DaveGame::createLevelAndHealth() {

        Entity level = Entity::create();
        level.addAll(
            Position{{700, 35}, 0},
            Drawable{NUMBERS_SPRITES[0]},
            LevelLabel{}
        );
        cout << "Created level icon" <<  level.entity().id <<endl;

        Entity health1 = Entity::create();
        health1.addAll(
            Position{{1020, 35}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true},
            LivesHead{0}
        );
        cout << "Created health icon" <<  health1.entity().id <<endl;
        Entity health2 = Entity::create();
        health2.addAll(
            Position{{1070, 35}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true},
            LivesHead{1}
        );
        cout << "Created health icon" <<  health2.entity().id <<endl;
        Entity health3 = Entity::create();
        health3.addAll(
            Position{{1120, 35}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true},
            LivesHead{2}
        );
        cout << "Created health icon" <<  health3.entity().id <<endl;
    }

    void DaveGame::createGun(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + GUN.w * BLOCK_TEX_SCALE / 2.0f,
            p.y + GUN.h * BLOCK_TEX_SCALE / 2.0f
        };
        b2BodyDef gunBodyDef = b2DefaultBodyDef();
        gunBodyDef.type = b2_staticBody;
        gunBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId gunBody = b2CreateBody(boxWorld, &gunBodyDef);

        b2ShapeDef gunShapeDef = b2DefaultShapeDef();
        gunShapeDef.enableSensorEvents = true;

        b2Polygon gunBox = b2MakeBox((GUN.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (GUN.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(gunBody, &gunShapeDef, &gunBox);

        Entity gun = Entity::create();
        gun.addAll(
            Position{center, 0},
            Drawable{GUN, BLOCK_TEX_SCALE, true, false},
            Collider{gunBody},
            Gun{}
        );
        b2Body_SetUserData(gunBody, new ent_type{gun.entity()});
    }

    void DaveGame::createBullet(SDL_FPoint davePos, bool goingLeft) {

        constexpr float bulletSpeed = 8.f;

        SDL_FPoint center = {
            davePos.x + (goingLeft ? -BULLET.w : BULLET.w),
            davePos.y
        };

        b2BodyDef bulletBodyDef = b2DefaultBodyDef();
        bulletBodyDef.type = b2_kinematicBody;
        bulletBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId bulletBody = b2CreateBody(boxWorld, &bulletBodyDef);

        b2ShapeDef bulletShapeDef = b2DefaultShapeDef();
        bulletShapeDef.enableSensorEvents = true;
        bulletShapeDef.isSensor = true;

        b2Polygon bulletBox = b2MakeBox(
            (BULLET.w / BOX_SCALE) / 2.0f,
            (BULLET.h / BOX_SCALE) / 2.0f
        );
        b2CreatePolygonShape(bulletBody, &bulletShapeDef, &bulletBox);

        b2Vec2 velocity = { goingLeft ? -bulletSpeed : bulletSpeed, 0.f };
        b2Body_SetLinearVelocity(bulletBody, velocity);

        Entity bullet = Entity::create();
        bullet.addAll(
            Position{center, 0},
            Drawable{BULLET, DAVE_TEX_SCALE, true, goingLeft}, // cropped part
            Collider{bulletBody},
            Bullet{}
        );

        b2Body_SetUserData(bulletBody, new ent_type{bullet.entity()});
    }

    void DaveGame::createMonsterBullet(SDL_FPoint monsterPos, bool goingLeft) {

        constexpr float bulletSpeed = 8.f;

        SDL_FPoint center = {
            monsterPos.x + (goingLeft ? -MONSTER_BULLET.w : MONSTER_BULLET.w),
            monsterPos.y
        };

        b2BodyDef bulletBodyDef = b2DefaultBodyDef();
        bulletBodyDef.type = b2_kinematicBody;
        bulletBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId bulletBody = b2CreateBody(boxWorld, &bulletBodyDef);

        b2ShapeDef bulletShapeDef = b2DefaultShapeDef();
        bulletShapeDef.enableSensorEvents = true;
        bulletShapeDef.isSensor = true;

        b2Polygon bulletBox = b2MakeBox(
            (MONSTER_BULLET.w / BOX_SCALE) / 2.0f,
            (MONSTER_BULLET.h / BOX_SCALE) / 2.0f
        );
        b2CreatePolygonShape(bulletBody, &bulletShapeDef, &bulletBox);

        b2Vec2 velocity = { goingLeft ? -bulletSpeed : bulletSpeed, 0.f };
        b2Body_SetLinearVelocity(bulletBody, velocity);

        Entity bullet = Entity::create();
        bullet.addAll(
            Position{center, 0},
            Drawable{MONSTER_BULLET, DAVE_TEX_SCALE, true, goingLeft}, // cropped part
            Collider{bulletBody},
            Bullet{},
            Monster{}
        );

        b2Body_SetUserData(bulletBody, new ent_type{bullet.entity()});
    }


    void DaveGame::EndGame() {
        Mask required = MaskBuilder()
            .set<Collider>()
            .build();
        Mask statEnt = MaskBuilder()
        .set<Drawable>()
        .build();
        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type e{id};
            if (World::mask(e).test(statEnt)) {
                auto& d = World::getComponent<Drawable>(e);

                if (World::mask(e).test(Component<DoorLabel>::Bit))
                    d.visible = false; // Hide door label

                if (d.isStatic)
                    continue;

            }
            if (World::mask(e).test(required)) {
                auto& c = World::getComponent<Collider>(e);
                b2DestroyBody(c.b);
            }
            World::destroyEntity(e);
        }
    }

    void DaveGame::createBatMonster(SDL_FPoint p, bool isGunMonster) {
        // Step 1: Correct center calculation (scaled)
        SDL_FPoint center = {
        p.x + (BAT_MONSTER_1.w * BLOCK_TEX_SCALE) / 2.0f,
        p.y + (BAT_MONSTER_1.h * BLOCK_TEX_SCALE) / 2.0f
    };

        // Step 2: Box2D body creation
        b2BodyDef monsterBodyDef = b2DefaultBodyDef();
        monsterBodyDef.type = b2_kinematicBody;  // change to dynamic if you want movement
        monsterBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        b2BodyId monsterBody = b2CreateBody(boxWorld, &monsterBodyDef);

        b2ShapeDef monsterShapeDef = b2DefaultShapeDef();
        monsterShapeDef.enableSensorEvents = true;

        b2Polygon monsterBox = b2MakeBox(
            (BAT_MONSTER_1.w * BLOCK_TEX_SCALE / BOX_SCALE) / 2,
            (BAT_MONSTER_1.h * BLOCK_TEX_SCALE / BOX_SCALE) / 2
        );
        b2CreatePolygonShape(monsterBody, &monsterShapeDef, &monsterBox);

        // Step 3: Create animation frames (1 state, 2 frames)
        auto* batFrames = new Drawable[2] {
            { BAT_MONSTER_1, BLOCK_TEX_SCALE, true, false },
            { BAT_MONSTER_2, BLOCK_TEX_SCALE, true, false }
        };

        auto** batStates = new Drawable*[1] {
            batFrames
        };

        // Step 4: Entity creation with animation
        Entity monster = Entity::create();
        monster.addAll(
            Position{center, 0},
            Drawable{BAT_MONSTER_1, BLOCK_TEX_SCALE, true, false},
            Collider{monsterBody},
            Monster{},
            Animation{batStates, 1, 2, 0, 0, Animation::Type::DAVE},
            CircularMotion{center, 50.0f, 1.5f}
        );

        if (isGunMonster) {
            World::addComponent(monster.entity(), Gun{});
            World::addComponent(monster.entity(), LastShot{});
        }

        b2Body_SetUserData(monsterBody, new ent_type{monster.entity()});
    }
}
