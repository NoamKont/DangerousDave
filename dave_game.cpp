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
        createStatusBar();

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
        SDL_Surface *surf = IMG_Load("res/Dave1.png");
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

                float JUMP_IMPULSE = 230.5f;

                const auto& vel = b2Body_GetLinearVelocity(c.b);

                const float x = i.left ? -3 : i.right ? 3 : 0;
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

                        //b2Body_SetLinearVelocity(c.b,{0, -15});
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

            if(sensorIsDave && visitorIsWall)
            {
                auto& groundStatus = World::getComponent<GroundStatus>(*sensorEntity);
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
                auto& trophy = World::getComponent<Trophy>(*sensorEntity);
                gameInfo.score += trophy.value; // Increase score by 100 for collecting a trophy
                World::destroyEntity(*visitorEntity);
                b2DestroyBody(visitor);
                renderGoThruTheDoor();
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
                World::getComponent<Position>(e) = {
                    {t.p.x*BOX_SCALE, t.p.y*BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
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

        SDL_SetRenderDrawColor(ren,0,255,0,0);

        for (int i = 0; i <= World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            const auto& pos = World::getComponent<Position>(e);
            const auto& drawable = World::getComponent<Drawable>(e);

            if (!drawable.visible)
            {
                continue; // Skip rendering if not visible
            }

            const SDL_FRect dst = {
                pos.p.x - drawable.part.w / 2 - gameInfo.screenOffset * (!drawable.isStatic) * WIN_WIDTH + BLOCK_TEX_SCALE,
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
        SDL_FPoint p = {WIN_WIDTH/2, WIN_HEIGHT/2};//TODO start position

        b2BodyDef daveBodyDef = b2DefaultBodyDef();
        daveBodyDef.type = b2_dynamicBody;
        daveBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        b2BodyId daveBody = b2CreateBody(boxWorld, &daveBodyDef);

        //Define shape
        b2ShapeDef daveShapeDef = b2DefaultShapeDef();
        daveShapeDef.density = 28.9;
        daveShapeDef.enableSensorEvents = false;
        daveShapeDef.isSensor = false;

        b2Polygon daveBox = b2MakeBox((DAVE_STANDING.w*DAVE_TEX_SCALE/BOX_SCALE)/2, (DAVE_STANDING.h*DAVE_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(daveBody, &daveShapeDef, &daveBox);

        b2ShapeDef daveShapeDef2 = b2DefaultShapeDef();
        daveShapeDef2.enableSensorEvents = true;
        daveShapeDef2.isSensor = true;
        b2Polygon daveBox2 = b2MakeBox((DAVE_STANDING.w*DAVE_TEX_SCALE/BOX_SCALE)/2, (DAVE_STANDING.h*DAVE_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(daveBody, &daveShapeDef2, &daveBox2);



        DAVE_ANIMATION = new Drawable*[3] {
            new Drawable[4] { //IDLE
                {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
                {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
                    {DAVE_IDLE, DAVE_TEX_SCALE, true, false},
                {DAVE_IDLE, DAVE_TEX_SCALE, true, false}
            },
            new Drawable[4] { //Walking
                {DAVE_STANDING, DAVE_TEX_SCALE, true, false},
                {DAVE_WALKING_1, DAVE_TEX_SCALE, true, false},
                    {DAVE_STANDING, DAVE_TEX_SCALE, true, false},
                {DAVE_WALKING_2, DAVE_TEX_SCALE, true, false}
            },
            new Drawable[4] { //JUMP
                {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
                {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
                    {DAVE_JUMPING, DAVE_TEX_SCALE, true, false},
                {DAVE_JUMPING, DAVE_TEX_SCALE, true, false}
            }
        };

        std:: cout << "Creating Dave entity " << std::endl;

        Entity e = Entity::create();
        e.addAll(
         Position{{},0},
         Drawable{DAVE_STANDING, DAVE_TEX_SCALE, true, false},
         Collider{daveBody},
         Intent{},
         Animation{DAVE_ANIMATION, 1, 4, 0, 0, Animation::Type::DAVE}, // 1 state, 4 frames, current state 0, current frame 0
         Input{SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT},
         Dave{},
         GroundStatus{true}
         );
        b2Body_SetUserData(daveBody, new ent_type{e.entity()});

        std::cout << "Dave entity created with ID: " << e.entity().id << std::endl;
    }

    void DaveGame::createWall(SDL_FPoint p, float w, float h) const {
        const float width = w;
        const float height = h;


        b2BodyDef wallBodyDef = b2DefaultBodyDef();
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};

        b2BodyId wallBody = b2CreateBody(boxWorld, &wallBodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableSensorEvents = true;

        b2Polygon box = b2MakeBox((width/ BOX_SCALE) / 2.f, (height /  BOX_SCALE) / 2.f);
        b2ShapeId shape = b2CreatePolygonShape(wallBody, &shapeDef, &box);

        Entity e = Entity::create();
        e.addAll(
            Position{p, 0},
            Collider{wallBody},
            Wall{shape, {width, height}},
            Drawable{{86,380,11,11},BLOCK_TEX_SCALE, true,false}
        );
        b2Body_SetUserData(wallBody, new ent_type{e.entity()});
    }

    void DaveGame::createMap(uint8_t* map, int width, int height) {

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                int row_to_print = row + 1;
                uint8_t* map_row = (map + row * width);
                if (map_row[col] == DaveGame::GRID_RED_BLOCK) {
                    SDL_FPoint p = {col * DaveGame::RED_BLOCK.w * DaveGame::BLOCK_TEX_SCALE, row_to_print * DaveGame::RED_BLOCK.h * DaveGame::BLOCK_TEX_SCALE};
                    createWall(p, DaveGame::RED_BLOCK.w * DaveGame::BLOCK_TEX_SCALE, DaveGame::RED_BLOCK.h * DaveGame::BLOCK_TEX_SCALE);
                }
                else if (map_row[col] == DaveGame::GRID_DIAMOND) {
                    SDL_FPoint p = {col * DaveGame::RED_BLOCK.w * DaveGame::BLOCK_TEX_SCALE, row_to_print * DaveGame::RED_BLOCK.h * DaveGame::BLOCK_TEX_SCALE};
                    createDiamond(p);
                }
                else if (map_row[col] == DaveGame::GRID_DOOR) {
                    SDL_FPoint p = {col * DaveGame::RED_BLOCK.w * DaveGame::BLOCK_TEX_SCALE, row_to_print * DaveGame::RED_BLOCK.h * DaveGame::BLOCK_TEX_SCALE};
                    createDoor(p);
                }
                else if (map_row[col] == DaveGame::GRID_TROPHY) {
                    SDL_FPoint p = {col * DaveGame::RED_BLOCK.w * DaveGame::BLOCK_TEX_SCALE, row_to_print * DaveGame::RED_BLOCK.h * DaveGame::BLOCK_TEX_SCALE};
                    createTrophy(p);
                }
            }
        }

    }

    void DaveGame::createDiamond(SDL_FPoint p) {

        Entity diamond = Entity::create();

        b2BodyDef diamondBodyDef = b2DefaultBodyDef();
        diamondBodyDef.type = b2_staticBody;
        diamondBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        b2BodyId diamondBody = b2CreateBody(boxWorld, &diamondBodyDef);

        b2ShapeDef diamondShapeDef = b2DefaultShapeDef();
        diamondShapeDef.enableSensorEvents = true;

        b2Polygon diamondBox = b2MakeBox((DIAMOND.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DIAMOND.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(diamondBody, &diamondShapeDef, &diamondBox);

        diamond.addAll(
            Position{p, 0},
            Drawable{DIAMOND, BLOCK_TEX_SCALE, true, false},
            Diamond{}
        );
        b2Body_SetUserData(diamondBody, new ent_type{diamond.entity()});
    }

    void DaveGame::createDoor(SDL_FPoint p) {

        Entity door = Entity::create();

        b2BodyDef doorBodyDef = b2DefaultBodyDef();
        doorBodyDef.type = b2_staticBody;
        doorBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        b2BodyId doorBody = b2CreateBody(boxWorld, &doorBodyDef);

        b2ShapeDef doorShapeDef = b2DefaultShapeDef();
        doorShapeDef.enableSensorEvents = true;
        //doorShapeDef.isSensor = true;

        b2Polygon diamondBox = b2MakeBox((DOOR.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (DOOR.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(doorBody, &doorShapeDef, &diamondBox);

        door.addAll(
            Position{p, 0},
            Drawable{DOOR, BLOCK_TEX_SCALE, true, false},
            Door{false}
        );
        b2Body_SetUserData(doorBody, new ent_type{door.entity()});

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
    void DaveGame::createTrophy(SDL_FPoint p) {


        b2BodyDef trophyBodyDef = b2DefaultBodyDef();
        trophyBodyDef.type = b2_staticBody;
        trophyBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        b2BodyId trophyBody = b2CreateBody(boxWorld, &trophyBodyDef);

        b2ShapeDef trophyShapeDef = b2DefaultShapeDef();
        trophyShapeDef.enableSensorEvents = true;

        b2Polygon diamondBox = b2MakeBox((TROPHY.w*BLOCK_TEX_SCALE/BOX_SCALE)/2, (TROPHY.h*BLOCK_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(trophyBody, &trophyShapeDef, &diamondBox);


        Entity trophy = Entity::create();
        trophy.addAll(
            Position{p, 0},
            Drawable{TROPHY, BLOCK_TEX_SCALE, true, false},
            Trophy{}
        );
        b2Body_SetUserData(trophyBody, new ent_type{trophy.entity()});
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
