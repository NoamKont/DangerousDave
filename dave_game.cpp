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

    DaveGame::DaveGame() {
        if (!prepareWindowAndTexture())
            return;
        SDL_srand(time(nullptr));

        prepareBoxWorld();
        createMap();


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

                        float jumpVelocity = 5.4f;
                        float mass = b2Body_GetMass(c.b);
                        b2Vec2 impulse = {0.0f, -mass * jumpVelocity};
                        b2Body_ApplyLinearImpulseToCenter(c.b, impulse, true);

                        //b2Body_SetLinearVelocity(c.b,{0, -15});
                        groundStatus.onGround = false;
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
        //daveShapeDef.enableSensorEvents = true;
        //daveShapeDef.isSensor = false;
        daveShapeDef.density = 28.9; // Not needed for static, but harmless

        b2Polygon daveBox = b2MakeBox((DAVE_STANDING.w*DAVE_TEX_SCALE/BOX_SCALE)/2, (DAVE_STANDING.h*DAVE_TEX_SCALE/BOX_SCALE)/2);
        b2CreatePolygonShape(daveBody, &daveShapeDef, &daveBox);

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
        //b2Body_SetUserData(pacmanBody, new ent_type{e.entity()});

        std::cout << "Dave entity created with ID: " << e.entity().id << std::endl;
    }

    // /// @brief Creates a monster entity.
    // void DaveGame::prepareWalls() const {
    //     //upper and lower borders
    //     createWall({WIN_WIDTH  / 2.0f, 0.0f},WIN_WIDTH,5.f);
    //     createWall({WIN_WIDTH  / 2.0f, WIN_HEIGHT},WIN_WIDTH ,125.f);
    //     //side borders
    //     createWall({0.0f, WIN_HEIGHT / 2.0f},5.f, WIN_HEIGHT);
    //     createWall({WIN_WIDTH, WIN_HEIGHT / 2.0f},5.f, WIN_HEIGHT);
    //
    // }

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

        b2Polygon box = b2MakeBox((width/ BOX_SCALE) / 2.f, (height /  BOX_SCALE) / 2.f);
        b2ShapeId shape = b2CreatePolygonShape(wallBody, &shapeDef, &box);

        Entity e = Entity::create();
        e.addAll(
            Position{p, 0},
            Collider{wallBody},
            Wall{shape, {width, height}},
            Drawable{{86,380,11,11},DAVE_TEX_SCALE,true,false}
        );
        b2Body_SetUserData(wallBody, new ent_type{e.entity()});
    }

    void DaveGame::createMap() {

        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                if (map[row][col]) {
                    SDL_FPoint p = {col * RED_BLOCK.w * BLOCK_TEX_SCALE, row * RED_BLOCK.h * BLOCK_TEX_SCALE};
                    createWall(p, RED_BLOCK.w * BLOCK_TEX_SCALE, RED_BLOCK.h * BLOCK_TEX_SCALE);
                }
            }
        }

    }
}
