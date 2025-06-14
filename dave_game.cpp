#include "dave_game.h"
#include "bagel.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>



using namespace bagel;
using namespace std;
namespace dave_game
{
    void DaveGame::run()
    {
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        auto start = SDL_GetTicks();
        bool quit = false;

        while (!quit) {

            InputSystem();
            //AISystem();
            MovementSystem();
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
        createMap(&map_stage2[0][0], MAP_WIDTH * 2, MAP_HEIGHT);

        createDave();
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
                        float jumpVelocity = 5.4f;
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
        const auto sensorEvents = b2World_GetSensorEvents(boxWorld);

        for(int i = 0 ; i < sensorEvents.beginCount ; i++)
        {
            b2BodyId sensor = b2Shape_GetBody(sensorEvents.beginEvents[i].sensorShapeId);
            b2BodyId visitor = b2Shape_GetBody(sensorEvents.beginEvents[i].visitorShapeId);
            auto *visitorEntity = static_cast<ent_type*>(b2Body_GetUserData(visitor));
            auto *sensorEntity = static_cast<ent_type*>(b2Body_GetUserData(sensor));

            bool sensorIsDave = World::mask(*sensorEntity).test(Component<Dave>::Bit);

            bool visitorIsWall = World::mask(*visitorEntity).test(Component<Wall>::Bit);
            bool visitorIsDiamond = World::mask(*visitorEntity).test(Component<Diamond>::Bit);
            bool visitorIsDoor = World::mask(*visitorEntity).test(Component<Door>::Bit);
            bool visitorIsTrophy = World::mask(*visitorEntity).test(Component<Trophy>::Bit);
            bool visitorIsMoveScreen = World::mask(*visitorEntity).test(Component<MoveScreenSensor>::Bit);

            if(sensorIsDave && visitorIsWall)
            {
                auto& groundStatus = World::getComponent<GroundStatus>(*visitorEntity);
                groundStatus.onGround = true;
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
                    EndGame();
                }
            }
            else if (sensorIsDave && visitorIsTrophy) {
                //auto& gameInfo = World::getComponent<GameInfo>(*sensorEntity);
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

                // World::getComponent<Position>(e) = {
                //     {t.p.x*BOX_SCALE, t.p.y*BOX_SCALE},
                //     RAD_TO_DEG * b2Rot_GetAngle(t.q)
                // };
            }
        }
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

        for (id_type i =0; i<World::maxId().id; ++i) {
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

            if (World::mask(e).test(Component<Dave>::Bit) || World::mask(e).test(Component<Wall>::Bit))
            {
                const Collider& collider = World::getComponent<Collider>(e);
                //const auto& pos = World::getComponent<Position>(e);

                    b2BodyId body = collider.b;

                    // Get Box2D center position (in meters)
                    b2Vec2 pos = b2Body_GetPosition(body);

                    // Convert to pixels
                    float centerX = pos.x * BOX_SCALE;
                    float centerY = pos.y * BOX_SCALE;
                    float w,h;
                    if ( World::mask(e).test(Component<Wall>::Bit)) {

                        w = RED_BLOCK.w * BLOCK_TEX_SCALE; // back to pixels
                        h = RED_BLOCK.h * BLOCK_TEX_SCALE;
                    }else {
                        w = DAVE_JUMPING.w * DAVE_TEX_SCALE; // back to pixels
                        h = DAVE_JUMPING.h * DAVE_TEX_SCALE;
                    }

                    // Top-left corner
                    SDL_FRect boxRect = {
                        centerX - w/2 ,
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

            const SDL_FRect dst = {
                pos.p.x - drawable.part.w / 2 - (gameInfo.screenOffset * (!drawable.isStatic) * WIN_WIDTH),
                    pos.p.y - drawable.part.h / 2,
                    drawable.part.w * drawable.scale,
                    drawable.part.h * drawable.scale
                };

            SDL_FlipMode flip = drawable.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

            SDL_RenderTextureRotated(
                ren, tex, &drawable.part, &dst, 0,
                nullptr, flip);
        }

        SDL_RenderPresent(ren);
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
    void DaveGame::createDave()
    {
    // Calculate top-left corner of Dave's starting cell in pixels
    SDL_FPoint topLeft = {
        DAVE_START_COLUMN * RED_BLOCK.w * BLOCK_TEX_SCALE,
        DAVE_START_ROW * RED_BLOCK.h * BLOCK_TEX_SCALE
    };

    // Calculate Dave's center position for Box2D
    SDL_FPoint center = {
        topLeft.x + DAVE_JUMPING.w / 2.0f,
        topLeft.y + DAVE_JUMPING.h / 2.0f
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

    cout<< b2Body_GetMass(daveBody) << endl;
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

    std::cout << "Creating Dave entity " << std::endl;

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
            for (int col = 0; col < width; ++col) {
                int row_to_print = row + 1; // Offset by 1 to account for the status bar
                uint8_t* map_row = (map + row * width);
                if (map_row[col] == GRID_RED_BLOCK) {
                    SDL_FPoint p = {(col * RED_BLOCK.w * BLOCK_TEX_SCALE), row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createWall(p);
                }
                else if (map_row[col] == GRID_DIAMOND) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createDiamond(p);
                }
                // else if (map_row[col] == GRID_DOOR) {
                //     SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                //     createDoor(p);
                // }
                else if (map_row[col] == GRID_TROPHY) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createTrophy(p);
                }
                // else if (map_row[col] == GRID_SENSOR_BACK) {
                //     SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                //     createMoveScreenSensor(p, false, col/20);
                // }
                // else if (map_row[col] == GRID_SENSOR_FORWARD) {
                //     SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row_to_print * RED_BLOCK.h * BLOCK_TEX_SCALE};
                //     createMoveScreenSensor(p, true, col/20);
                // }
            }
        }
    }

    void DaveGame::createWall(SDL_FPoint p) const {
        SDL_FPoint center = {
            p.x + RED_BLOCK.w / 2.0f,
            p.y + RED_BLOCK.h / 2.0f
        };

        b2BodyDef wallBodyDef = b2DefaultBodyDef();
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position = {center.x / BOX_SCALE, center.y / BOX_SCALE};
        wallBodyDef.fixedRotation = true;

        b2BodyId wallBody = b2CreateBody(boxWorld, &wallBodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();

        shapeDef.enableSensorEvents = true;

        b2Polygon box = b2MakeBox((RED_BLOCK.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (RED_BLOCK.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2ShapeId shape = b2CreatePolygonShape(wallBody, &shapeDef, &box);

        Entity e = Entity::create();
        e.addAll(
            Position{center, 0},  // Still use top-left for rendering if needed
            Collider{wallBody},
            Wall{shape, {RED_BLOCK.w, RED_BLOCK.h}},
            Drawable{RED_BLOCK, BLOCK_TEX_SCALE, true, false}
        );
        b2Body_SetUserData(wallBody, new ent_type{e.entity()});

    }

    void DaveGame::createDiamond(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + DIAMOND.w / 2.0f,
            p.y + DIAMOND.h / 2.0f
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
            Diamond{}
        );
        b2Body_SetUserData(diamondBody, new ent_type{diamond.entity()});
    }

    void DaveGame::createTrophy(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + DIAMOND.w / 2.0f,
            p.y + DIAMOND.h / 2.0f
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
    }

    void DaveGame::createDoor(SDL_FPoint p) {

        SDL_FPoint center = {
            p.x + DIAMOND.w / 2.0f,
            p.y + DIAMOND.h / 2.0f
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
            Door{false}
        );
        b2Body_SetUserData(doorBody, new ent_type{door.entity()});
    }


    void DaveGame::createMoveScreenSensor(SDL_FPoint p, bool forward, int col) {

        SDL_FPoint center = {
            p.x + DIAMOND.w / 2.0f,
            p.y + DIAMOND.h / 2.0f
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
            MoveScreenSensor{forward, col}
        );
        b2Body_SetUserData(sensorBody, new ent_type{ent.entity()});

    }

    void DaveGame::createStatusBar() {
        createTitles();
        createScoreBar();
        createLevelAndHealth();

    }

    void DaveGame::createTitles() {

        auto score = Entity::create();
        score.addAll(
            Position{{25, 10}, 0},
            Drawable{SCORE_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );


        auto level = Entity::create();
        level.addAll(
            Position{{500, 10}, 0},
            Drawable{LEVEL_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );

        auto daves = Entity::create();
        daves.addAll(
            Position{{800, 10}, 0},
            Drawable{HEALTH_SPRITE, BLOCK_TEX_SCALE, true, false, true}
        );

        auto openDoor = Entity::create();
        openDoor.addAll(
            Position{{300, 11 * RED_BLOCK.h * BLOCK_TEX_SCALE}, 0},
            Drawable{{1, 223, 123, 10}, BLOCK_TEX_SCALE, false, false, true},
            DoorLabel{}
        );
    }

    void DaveGame::createScoreBar() {

        for (int i=SCORE_DIGITS_COUNT - 1; i >= 0; --i) {
            auto entity = Entity::create();
            entity.addAll(
                Position{{(i+1) * 40.f + 210, 10}, 0},
                Drawable{NUMBERS_SPRITES[i+5]},
                ScoreLabel{}
            );
        }

    }

    void DaveGame::createLevelAndHealth() {

        Entity level = Entity::create();
        level.addAll(
            Position{{700, 10}, 0},
            Drawable{NUMBERS_SPRITES[0]},
            LevelLabel{}
        );

        Entity health1 = Entity::create();
        health1.addAll(
            Position{{1020, 10}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true}
        );
        Entity health2 = Entity::create();
        health2.addAll(
            Position{{1070, 10}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true}
        );
        Entity health3 = Entity::create();
        health3.addAll(
            Position{{1120, 10}, 0},
            Drawable{DAVE_HEALTH, BLOCK_TEX_SCALE, true, false, true}
        );



    }





    void DaveGame::EndGame() {
        Mask required = MaskBuilder()
            .set<Collider>()
            .build();
        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type e{id};
            if (World::mask(e).test(required)) {
                auto& c = World::getComponent<Collider>(e);
                b2DestroyBody(c.b);
            }
            World::destroyEntity(e);
        }
    }
}
