#include "dave_game.h"
#include "bagel.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

#include "Pacman.h"

using namespace bagel;
using namespace std;
namespace dave_game
{

    DaveGame::DaveGame() {
        if (!prepareWindowAndTexture())
            return;
        SDL_srand(time(nullptr));

        prepareBoxWorld();
        prepareWalls();


        createDave();

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
        SDL_Surface *surf = IMG_Load("res/dave.png");
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
            //CollisionSystem();
            AnimationSystem();
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

    void DaveGame::prepareBoxWorld()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0,9.8f};
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

                if (i.up) {
                    // b2Vec2 impulse = {0, -JUMP_IMPULSE}; // negative Y is up in Box2D
                    // b2Body_ApplyLinearImpulse(c.b, impulse, b2Body_GetPosition(c.b), true);

                    b2Body_SetLinearVelocity(c.b,{0, -15});

                }

                const auto& vel = b2Body_GetLinearVelocity(c.b);

                const float x = i.left ? -20 : i.right ? 20 : 0;
                b2Body_SetLinearVelocity(c.b, {x,vel.y});



                if (isDave) {
                    
                    auto& anim = World::getComponent<Animation>(e);

                    if (vel.x >= -ANIMATION_VELOCITY_THRESHOLD && vel.x <= ANIMATION_VELOCITY_THRESHOLD && vel.y >= -ANIMATION_VELOCITY_THRESHOLD && vel.y <= ANIMATION_VELOCITY_THRESHOLD) {
                        // If not moving, set to idle state
                        if (anim.currentState != 0) {
                            anim.currentState = 0; // IDLE
                            anim.currentFrame = 0;
                        }
                    } else if (vel.y >= -ANIMATION_VELOCITY_THRESHOLD && vel.y <= ANIMATION_VELOCITY_THRESHOLD) {
                        // If moving, set to walk state
                        if (anim.currentState != 1) {
                            anim.currentState = 1; // WALK
                            anim.currentFrame = 0;
                        }
                    }
                    else {
                        // If jumping or falling, set to jump state
                        if (anim.currentState != 2) {
                            anim.currentState = 2; // JUMP
                            anim.currentFrame = 0;
                        }
                    }


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
                World::getComponent<Position>(e) = {
                    {t.p.x*BOX_SCALE, t.p.y*BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
            }
        }
    }

    void DaveGame::prepareWalls() const {
            //upper and lower borders
            createWall({WIN_WIDTH  / 2.0f, 0.0f},WIN_WIDTH,5.f);
            createWall({WIN_WIDTH  / 2.0f, WIN_HEIGHT},WIN_WIDTH ,60.f);
            //side borders
            createWall({0.0f, WIN_HEIGHT / 2.0f},5.f, WIN_HEIGHT);
            createWall({WIN_WIDTH, WIN_HEIGHT / 2.0f},5.f, WIN_HEIGHT);

    }
    void DaveGame::createWall(SDL_FPoint p, float w, float h) const {
        const float width = w;
        const float height = h;


        b2BodyDef wallBodyDef = b2DefaultBodyDef();
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};

        b2BodyId wallBody = b2CreateBody(boxWorld, &wallBodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        //shapeDef.enableSensorEvents = true;
        //shapeDef.isSensor = true;
        shapeDef.density = 1; // Not needed for static, but harmless

        b2Polygon box = b2MakeBox(width / 2.0f / BOX_SCALE, height / 2.0f / BOX_SCALE);
        b2ShapeId shape = b2CreatePolygonShape(wallBody, &shapeDef, &box);

        Entity e = Entity::create();
        e.addAll(
                Position{p, 0},
                Collider{wallBody},
                Wall{shape, {width, height}}
        );
        b2Body_SetUserData(wallBody, new ent_type{e.entity()});
    }
    // /// @brief Handles collisions between entities with Position and Collision.
    // /// Optionally reacts to Collectible or Hazard components.
    // void CollisionSystem()
    // {
    //     MaskBuilder builder;
    //     builder.set<Position>().set<Collision>();
    //     Mask required = builder.build();
    //
    //     for (int i = 0; i < World::maxId().id; ++i)
    //     {
    //         ent_type e{i};
    //         if (!World::mask(e).test(required))
    //         {
    //             continue;
    //         }
    //
    //         bool hasCollectible = World::mask(e).test(Component<Collectible>::Bit);
    //         bool hasHazard = World::mask(e).test(Component<Hazard>::Bit);
    //     }
    // }

    /// @brief Renders entities with Image, Position, and GameInfo components.
    /// Also checks optional components like Course, Collision, Gun, and Jetpack.
    void DaveGame::RenderSystem()
    {
        MaskBuilder builder;
        builder.set<Position>().set<Drawable>();
        Mask required = builder.build();

        SDL_RenderClear(ren);

        //SDL_SetRenderDrawColor(ren,0,255,0,0);

        for (int i = 0; i <= World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            const auto& pos = World::getComponent<Position>(e);
            const auto& drawable = World::getComponent<Drawable>(e);

            const SDL_FRect dst = {
                pos.p.x - drawable.part.w / 2,
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

    //
    //
    // /// @brief Manages score, level, and lives using the GameInfo component.
    // /// Checks optional Health, Door, and PrizeValue components.
    // void ScoreSystem()
    // {
    //     MaskBuilder builder;
    //     builder.set<GameInfo>();
    //     Mask required = builder.build();
    //
    //     for (int i = 0; i < World::maxId().id; ++i)
    //     {
    //         ent_type e{i};
    //         if (!World::mask(e).test(required))
    //         {
    //             continue;
    //         }
    //
    //         bool hasHealth = World::mask(e).test(Component<Health>::Bit);
    //         bool hasDoor = World::mask(e).test(Component<Door>::Bit);
    //         bool hasPrizeValue = World::mask(e).test(Component<PrizeValue>::Bit);
    //     }
    // }
    //
    // /// @brief Handles item collection (prizes, weapons, trophies) and level progression.
    // /// Requires Collision, Position, and Collectible. Checks optional Gun, Jetpack, Door, and Unlocks.
    // void CollectSystem()
    // {
    //     MaskBuilder builder;
    //     builder.set<Collision>().set<Position>().set<Collectible>();
    //     Mask required = builder.build();
    //
    //     for (int i = 0; i < World::maxId().id; ++i)
    //     {
    //         ent_type e{i};
    //         if (!World::mask(e).test(required))
    //         {
    //             continue;
    //         }
    //
    //         bool hasGun = World::mask(e).test(Component<Gun>::Bit);
    //         bool hasJetpack = World::mask(e).test(Component<Jetpack>::Bit);
    //         bool hasDoor = World::mask(e).test(Component<Door>::Bit);
    //         bool hasUnlocks = World::mask(e).test(Component<Unlocks>::Bit);
    //     }
    // }
    //
    // /// @brief Handles entity death caused by hazards or depleted health.
    // /// Requires Health and Dead components.
    // void DeathSystem()
    // {
    //     MaskBuilder builder;
    //     builder.set<Health>().set<Dead>();
    //     Mask required = builder.build();
    //
    //     for (int i = 0; i < World::maxId().id; ++i)
    //     {
    //         ent_type e{i};
    //         if (!World::mask(e).test(required))
    //         {
    //             continue;
    //         }
    //
    //     }
    // }
    //
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
        SDL_FPoint p = {13.f, 240.f};//TODO start position

        b2BodyDef daveBodyDef = b2DefaultBodyDef();
        daveBodyDef.type = b2_dynamicBody;
        daveBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        b2BodyId daveBody = b2CreateBody(boxWorld, &daveBodyDef);

        //Define shape
        b2ShapeDef daveShapeDef = b2DefaultShapeDef();
        //daveShapeDef.enableSensorEvents = true;
        //daveShapeDef.isSensor = false;
        daveShapeDef.density = 1; // Not needed for static, but harmless

        b2Polygon daveBox = b2MakeBox((Dave_IDLE.w*CHARACTER_TEX_SCALE/BOX_SCALE)/2, (Dave_IDLE.h*CHARACTER_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(daveBody, &daveShapeDef, &daveBox);

        DAVE_ANIMATION = new Drawable*[3] {
            new Drawable[4] { //IDLE
                {Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
                {Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
                    {Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
                {Dave_IDLE, CHARACTER_TEX_SCALE, true, false}
            },
            new Drawable[4] { //Walking
                {Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
                {{27,13,12,15}, CHARACTER_TEX_SCALE, true, false},
                    {Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
                {{78,13,12,15}, CHARACTER_TEX_SCALE, true, false}
            },
            new Drawable[4] { //JUMP
                {{127,12,13,14}, CHARACTER_TEX_SCALE, true, false},
                {{127,12,13,14}, CHARACTER_TEX_SCALE, true, false},
                    {{127,12,13,14}, CHARACTER_TEX_SCALE, true, false},
                {{127,12,13,14}, CHARACTER_TEX_SCALE, true, false}
            }
        };

        std:: cout << "Creating Dave entity " << std::endl;

        Entity e = Entity::create();
        e.addAll(
         Position{{},0},
         Drawable{Dave_IDLE, CHARACTER_TEX_SCALE, true, false},
         Collider{daveBody},
         Intent{},
         Animation{DAVE_ANIMATION, 1, 4, 0, 0, Animation::Type::DAVE}, // 1 state, 4 frames, current state 0, current frame 0
         Input{SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT},
         Dave{}
         );
        //b2Body_SetUserData(pacmanBody, new ent_type{e.entity()});

        std::cout << "Dave entity created with ID: " << e.entity().id << std::endl;
    }

    /// @brief Creates a monster entity.
    // Entity createMonster(Position pos, Image img)
    // {
    //     Entity e = Entity::create();
    //
    //     e.add(pos);
    //     //e.add(Course{});
    //     e.add(img);
    //     //e.add(Collision{});
    //     e.add(Health{1});
    //     e.add(Hazard{});
    //     e.add(Gun{});
    //     //e.add(Animation{});
    //
    //     return e;
    // }

// /// @brief Creates a static block (e.g. ground or wall).
//     Entity createBlock(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//
//         return e;
//     }
//
// /// @brief Creates a collectible prize with a score value.
//     Entity createPrize(Position pos, Image img, int scoreValue)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Collectible{});
//         e.add(PrizeValue{scoreValue});
//
//         return e;
//     }
//
// /// @brief Creates the game background.
//     Entity createBackground(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//
//         return e;
//     }
//
// /// @brief Creates the info UI entity (e.g. score, level).
//     Entity createInfo(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(GameInfo{});
//
//         return e;
//     }
//
// /// @brief Creates a climbable tree.
//     Entity createTree(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Climbable{});
//
//         return e;
//     }
//
// /// @brief Creates a gun pickup.
//     Entity createGun(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Collectible{});
//
//         return e;
//     }
//
// /// @brief Creates a jetpack pickup.
//     Entity createJetpack(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Collectible{});
//
//         return e;
//     }
//
// /// @brief Creates a deadly obstacle.
//     Entity createObstacle(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Hazard{});
//         e.add(Animation{});
//         return e;
//     }
//
// /// @brief Creates a door entity.
//     Entity createDoor(Position pos, Image img, int scoreToAdd)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Door{});
//         e.add(PrizeValue{scoreToAdd});
//
//         return e;
//     }
//
// /// @brief Creates a trophy entity that unlocks a door.
//     Entity createTrophy(Position pos, Image img)
//     {
//         Entity e = Entity::create();
//
//         e.add(pos);
//         e.add(img);
//         e.add(Collision{});
//         e.add(Collectible{});
//         e.add(PrizeValue{100});
//         e.add(Animation{});
//         e.add(Unlocks{});
//
//         return e;
//     }




}
