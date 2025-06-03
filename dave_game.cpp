#include "dave_game.h"
#include "bagel.h"

namespace dave_game
{
    using namespace bagel;

    /// @brief Controls movement of all entities with Position and Course.
    void MovementSystem()
    {
        MaskBuilder builder;
        builder.set<Position>().set<Course>();
        Mask required = builder.build();

        for (int i = 0; i <= World::maxId().id; ++i)
        {
            ent_type e{i};

            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasIntent = World::mask(e).test(Component<Intent>::Bit);
            bool hasJetpack = World::mask(e).test(Component<Jetpack>::Bit);
            bool hasClimbable = World::mask(e).test(Component<Climbable>::Bit);
        }
    }

    /// @brief Handles collisions between entities with Position and Collision.
    /// Optionally reacts to Collectible or Hazard components.
    void CollisionSystem()
    {
        MaskBuilder builder;
        builder.set<Position>().set<Collision>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasCollectible = World::mask(e).test(Component<Collectible>::Bit);
            bool hasHazard = World::mask(e).test(Component<Hazard>::Bit);
        }
    }

    /// @brief Renders entities with Image, Position, and GameInfo components.
    /// Also checks optional components like Course, Collision, Gun, and Jetpack.
    void RenderSystem()
    {
        MaskBuilder builder;
        builder.set<Position>().set<Image>().set<GameInfo>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasCourse = World::mask(e).test(Component<Course>::Bit);
            bool hasCollision = World::mask(e).test(Component<Collision>::Bit);
            bool hasGun = World::mask(e).test(Component<Gun>::Bit);
            bool hasJetpack = World::mask(e).test(Component<Jetpack>::Bit);
        }
    }

    /// @brief Converts player input into high-level intentions (Intent component).
    /// Requires Control and Intent. Checks optional Gun and Jetpack components.
    void InputSystem()
    {
        MaskBuilder builder;
        builder.set<Control>().set<Intent>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasGun = World::mask(e).test(Component<Gun>::Bit);
            bool hasJetpack = World::mask(e).test(Component<Jetpack>::Bit);
        }
    }

    /// @brief Manages score, level, and lives using the GameInfo component.
    /// Checks optional Health, Door, and PrizeValue components.
    void ScoreSystem()
    {
        MaskBuilder builder;
        builder.set<GameInfo>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasHealth = World::mask(e).test(Component<Health>::Bit);
            bool hasDoor = World::mask(e).test(Component<Door>::Bit);
            bool hasPrizeValue = World::mask(e).test(Component<PrizeValue>::Bit);
        }
    }

    /// @brief Handles item collection (prizes, weapons, trophies) and level progression.
    /// Requires Collision, Position, and Collectible. Checks optional Gun, Jetpack, Door, and Unlocks.
    void CollectSystem()
    {
        MaskBuilder builder;
        builder.set<Collision>().set<Position>().set<Collectible>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

            bool hasGun = World::mask(e).test(Component<Gun>::Bit);
            bool hasJetpack = World::mask(e).test(Component<Jetpack>::Bit);
            bool hasDoor = World::mask(e).test(Component<Door>::Bit);
            bool hasUnlocks = World::mask(e).test(Component<Unlocks>::Bit);
        }
    }

    /// @brief Handles entity death caused by hazards or depleted health.
    /// Requires Health and Dead components.
    void DeathSystem()
    {
        MaskBuilder builder;
        builder.set<Health>().set<Dead>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

        }
    }

    /// @brief Updates animation state for entities with visual animations.
    /// Requires Animation and Image components.
    void AnimationSystem()
    {
        MaskBuilder builder;
        builder.set<Animation>().set<Image>();
        Mask required = builder.build();

        for (int i = 0; i < World::maxId().id; ++i)
        {
            ent_type e{i};
            if (!World::mask(e).test(required))
            {
                continue;
            }

        }
    }

    /// @brief Creates the player entity (Dave) with default attributes.
    Entity createDave(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(Control{});
        e.add(pos);
        e.add(Course{});
        e.add(img);
        e.add(Collision{});
        e.add(Health{STARTING_LIVES});
        e.add(Animation{});
        e.add(Intent{});

        return e;
    }

    /// @brief Creates a monster entity.
    Entity createMonster(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(Course{});
        e.add(img);
        e.add(Collision{});
        e.add(Health{1});
        e.add(Hazard{});
        e.add(Gun{});
        e.add(Animation{});

        return e;
    }

/// @brief Creates a static block (e.g. ground or wall).
    Entity createBlock(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});

        return e;
    }

/// @brief Creates a collectible prize with a score value.
    Entity createPrize(Position pos, Image img, int scoreValue)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Collectible{});
        e.add(PrizeValue{scoreValue});

        return e;
    }

/// @brief Creates the game background.
    Entity createBackground(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);

        return e;
    }

/// @brief Creates the info UI entity (e.g. score, level).
    Entity createInfo(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(GameInfo{});

        return e;
    }

/// @brief Creates a climbable tree.
    Entity createTree(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Climbable{});

        return e;
    }

/// @brief Creates a gun pickup.
    Entity createGun(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Collectible{});

        return e;
    }

/// @brief Creates a jetpack pickup.
    Entity createJetpack(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Collectible{});

        return e;
    }

/// @brief Creates a deadly obstacle.
    Entity createObstacle(Position pos, Image img)
    {
        Entity e = Entity::create();
        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Hazard{});
        e.add(Animation{});
        return e;
    }

/// @brief Creates a door entity.
    Entity createDoor(Position pos, Image img, int scoreToAdd)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Door{});
        e.add(PrizeValue{scoreToAdd});

        return e;
    }

/// @brief Creates a trophy entity that unlocks a door.
    Entity createTrophy(Position pos, Image img)
    {
        Entity e = Entity::create();

        e.add(pos);
        e.add(img);
        e.add(Collision{});
        e.add(Collectible{});
        e.add(PrizeValue{100});
        e.add(Animation{});
        e.add(Unlocks{});

        return e;
    }




}