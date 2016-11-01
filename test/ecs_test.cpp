#include <catch.hpp>
#include <lemon-toolkit.hpp>

#include <map>

using namespace std;
using namespace lemon;
using namespace lemon::core;

template<typename T> int size(const T& t)
{
    auto n = 0;
    for( auto i : t ) { n++; (void)i; }
    return n;
}

struct Position : public Component
{
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    bool operator == (const Position& rh) const
    {
        return x == rh.x && y == rh.y;
    }

    float x, y;
};

struct Direction : public Component
{
    Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    bool operator == (const Direction& rh) const
    {
        return x == rh.x && y == rh.y;
    }

    float x, y;
};

struct EcsTestContext
{
    EcsTestContext()
    {
        core::details::initialize();
        ecs = core::add_subsystem<core::EntityComponentSystem>();
    }

    ~EcsTestContext()
    {
        core::details::dispose();
    }

    INLINE Entity* get(Handle handle)
    {
        return ecs->get(handle);
    }

    template<typename T, typename ... Args> T* add_component(Handle handle, Args&&... args)
    {
        auto entity = get(handle);
        return entity == nullptr ? nullptr : entity->add_component<T>(std::forward<Args>(args)...);
    }

    template<typename T> T* get_component(Handle handle)
    {
        auto entity = get(handle);
        return entity == nullptr ? nullptr : entity->get_component<T>();
    }

    template<typename ... Args> std::tuple<Args*...> get_components(Handle handle)
    {
        return std::make_tuple(get_component<Args>(handle)...);
    }

    template<typename T> void remove_component(Handle handle)
    {
        auto entity = get(handle);
        if( entity != nullptr )
            entity->remove_component<T>();
    }

    template<typename T> bool has_component(Handle handle)
    {
        auto entity = get(handle);
        return entity == nullptr ? false : entity->has_component<T>();
    }

    template<typename ... Args> bool has_components(Handle handle)
    {
        auto entity = get(handle);
        return entity == nullptr ? false : entity->has_components<Args...>();
    }

    core::EntityComponentSystem* ecs;
};

TEST_CASE_METHOD(EcsTestContext, "TestCreateEntity")
{
    REQUIRE( ecs->size() == 0UL );

    auto e2 = ecs->create()->handle;
    REQUIRE( ecs->is_alive(e2) );
    REQUIRE( ecs->size() == 1UL );

    auto e1 = e2;
    REQUIRE( ecs->is_alive(e1) );
}

// its complex to implementation a generic clone senario,
// when take hierarchy into consideration.
// TEST_CASE_METHOD(EcsTestContext, "TestEntityCreateFromCopy")
// {
//     auto e = ecs->create();
//     auto ep = add_component<Position>(e, 1, 2);

//     auto f = _world.clone(e);
//     auto fp  get_component<Position>(f);

//     REQUIRE( ep != fp );
//     REQUIRE( ep->x == fp->x );
//     REQUIRE( ep->y == fp->y );

//     REQUIRE( e != f );
//     REQUIRE( get_components_mask(e) == get_components_mask(f) );

//     REQUIRE( ecs->size() == 2 );
// }

TEST_CASE_METHOD(EcsTestContext, "TestEntityAsBoolean")
{
    REQUIRE( ecs->size() == 0UL );

    auto e = ecs->create()->handle;
    REQUIRE( ecs->is_alive(e) );
    REQUIRE( ecs->size() == 1UL );

    ecs->free(e);
    REQUIRE( ecs->size() == 0UL );
    REQUIRE( !ecs->is_alive(e) );
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityReuse")
{
    auto e1 = ecs->create();
    auto e2 = e1->handle;
    auto index = e1->handle.get_index();
    auto version = e1->handle.get_version();
    REQUIRE( ecs->is_alive(e1->handle) );
    REQUIRE( ecs->is_alive(e2) );

    e1->add_component<Position>();
    ecs->free(e1);
    REQUIRE( !ecs->is_alive(e1->handle) );
    REQUIRE( !ecs->is_alive(e2) );

    auto e3 = ecs->create();
    REQUIRE( e3->handle.get_index() == index );
    REQUIRE( e3->handle.get_version() != version );
    REQUIRE( !e3->has_components<Position>() );

    e3->add_component<Position>();
    REQUIRE( e3->has_components<Position>() );
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityAssignment")
{
    lemon::Handle a = ecs->create()->handle;
    lemon::Handle b = a;
    REQUIRE( a == b );

    a.invalidate();
    REQUIRE( a != b );

    REQUIRE( !ecs->is_alive(a) );
    REQUIRE( ecs->is_alive(b) );
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentConstruction")
{
    auto e = ecs->create();
    auto p = e->add_component<Position>(1, 2);

    auto cp = e->get_component<Position>();
    REQUIRE( p == cp );
    REQUIRE( cp->x == Approx(1.0f) );
    REQUIRE( cp->y == Approx(2.0f) );
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentIdsDiffer")
{
    REQUIRE( Component::calculate<Position>() != Component::calculate<Direction>() );
    REQUIRE( (Component::calculate<Position, Direction>()) != Component::calculate<Direction>() );
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentInvalidatedWhenEntityDestroyed") {
    auto a = ecs->create();
    auto position = a->add_component<Position>(1, 2);

    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    auto handle = a->handle;
    REQUIRE(ecs->is_alive(handle));
    ecs->free(a);
    REQUIRE(!ecs->is_alive(handle));
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentAssignmentFromCopy")
{
    auto e = ecs->create();
    auto p = Position(1, 2);
    auto h = e->add_component<Position>(p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );
}

TEST_CASE_METHOD(EcsTestContext, "TestDestroyEntity")
{
    auto e = ecs->create();
    auto f = ecs->create();

    e->add_component<Position>();
    f->add_component<Position>();

    e->add_component<Direction>();
    f->add_component<Direction>();

    REQUIRE( ecs->is_alive(e->handle) );
    REQUIRE( ecs->is_alive(f->handle) );
    REQUIRE( e->get_component<Position>() != nullptr );
    REQUIRE( e->get_component<Direction>() != nullptr );
    REQUIRE( f->get_component<Position>() != nullptr );
    REQUIRE( f->get_component<Direction>() != nullptr );
    REQUIRE( e->has_components<Position>() );
    REQUIRE( e->has_components<Direction>() );

    auto handle = e->handle;
    ecs->free(e);

    REQUIRE( !ecs->is_alive(handle) );
    REQUIRE( ecs->is_alive(f->handle) );
    REQUIRE( f->get_component<Position>() != nullptr );
    REQUIRE( f->get_component<Direction>() != nullptr );
    REQUIRE( f->has_components<Position>() );
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityDestroyAll")
{
    auto e = ecs->create()->handle;
    auto f = ecs->create()->handle;
    ecs->free_all();
    REQUIRE( !ecs->is_alive(e) );
    REQUIRE( !ecs->is_alive(f) );
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityDestroyHole") {
    std::vector<Handle> entities;

    auto count = [this]()->int {
        auto view = ecs->find_entities_with<Position>();
        return std::count_if(view.begin(), view.end(), [](const Entity *) { return true; });
    };

    for (int i = 0; i < 5000; i++) {
        auto e = ecs->create();
        e->add_component<Position>();
        entities.push_back(e->handle);
    }

    REQUIRE(count() == 5000);

    size_t removed = 0;
    for(size_t i = 0; i< 2500; i++ )
    {
        auto index = std::rand() % 5000;
        if( entities[index].is_valid() )
        {
            ecs->free(entities[index]);
            entities[index].invalidate();
            removed ++;
        }
    }

    REQUIRE(count() == 5000-removed);
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityInStdSet")
{
    auto a = ecs->create()->handle;
    auto b = ecs->create()->handle;
    auto c = ecs->create()->handle;
    set<Handle> entitySet;
    
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityInStdMap")
{
    auto a = ecs->create()->handle;
    auto b = ecs->create()->handle;
    auto c = ecs->create()->handle;
    map<Handle, int> entityMap;
    REQUIRE( entityMap.insert(pair<Handle, int>(a, 1)).second );
    REQUIRE( entityMap.insert(pair<Handle, int>(b, 2)).second );
    REQUIRE( entityMap.insert(pair<Handle, int>(c, 3)).second );
    REQUIRE( entityMap[a] == 1 );
    REQUIRE( entityMap[b] == 2 );
    REQUIRE( entityMap[c] == 3 );
}

TEST_CASE_METHOD(EcsTestContext, "TestGetEntitiesWithComponent")
{
    auto e = ecs->create();
    auto f = ecs->create();
    auto g = ecs->create();

    e->add_component<Position>(1, 2);
    e->add_component<Direction>();
    f->add_component<Position>(3, 4);
    g->add_component<Direction>();

    REQUIRE( 2 == size(ecs->find_entities_with<Position>()) );
    REQUIRE( 2 == size(ecs->find_entities_with<Direction>()) );
    REQUIRE( 1 == size(ecs->find_entities_with<Position, Direction>()) );

    ecs->free_all();

    for( auto i=0; i<150; i++ )
    {
        auto h = ecs->create();
        if( i % 2 == 0 ) h->add_component<Position>();
        if( i % 3 == 0 ) h->add_component<Direction>();
    }

    REQUIRE( 50 == size(ecs->find_entities_with<Direction>()) );
    REQUIRE( 75 == size(ecs->find_entities_with<Position>()) );
    REQUIRE( 25 == size(ecs->find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(EcsTestContext, "TestGetComponentsAsTuple") {
    auto e = ecs->create();
    e->add_component<Position>(1, 2);
    e->add_component<Direction>(3, 4);

    auto components = e->get_components<Position, Direction>();
    REQUIRE(std::get<0>(components)->x == 1);
    REQUIRE(std::get<0>(components)->y == 2);
    REQUIRE(std::get<1>(components)->x == 3);
    REQUIRE(std::get<1>(components)->y == 4);
}

TEST_CASE_METHOD(EcsTestContext, "TestEntityIteration")
{
    auto e = ecs->create();
    auto f = ecs->create();

    e->add_component<Position>(1, 2);

    auto count = 0;
    ecs->find_entities_with<Position>().visit([&](Entity& object, Position& position)
    {
        count ++;
        REQUIRE( position.x == 1 );
        REQUIRE( position.y == 2 );
        REQUIRE( object.handle == e->handle );
    });

    REQUIRE( count == 1 );

    for( auto ne : ecs->find_entities_with<Position>() )
        count ++;
    REQUIRE( count == 2 );

    std::vector<Entity*> entities;
    ecs->find_entities_with<Position>().collect(entities);

    REQUIRE(entities.size() == 1);
}

TEST_CASE_METHOD(EcsTestContext, "TestIterateAllEntitiesSkipsDestroyed") {
    auto e = ecs->create();
    auto f = ecs->create();
    auto g = ecs->create();

    ecs->free(f);
    auto it = ecs->find_entities().begin();

    REQUIRE( (*it) == e );
    ++it;
    REQUIRE( (*it) == g );
    ++it;

    REQUIRE( it == ecs->find_entities().end() );
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentsRemovedFromReusedEntities")
{
    auto e = ecs->create();
    auto eversion = e->handle.get_version();
    auto eindex = e->handle.get_index();
    e->add_component<Position>(1, 2); 
    ecs->free(e);

    auto f = ecs->create();
    REQUIRE( eindex == f->handle.get_index() );
    REQUIRE( eversion < f->handle.get_version() );
    REQUIRE( !f->has_components<Position>() );
}

struct FreedSentinel : public Component
{
    explicit FreedSentinel(bool &yes) : yes(yes) {}
    ~FreedSentinel() { yes = true; }
    bool &yes;
};

TEST_CASE_METHOD(EcsTestContext, "TestComponentDestruction")
{
    bool freed = false;
    auto e = ecs->create();
    e->add_component<FreedSentinel>(freed);
    e->remove_component<FreedSentinel>();

    REQUIRE( freed );
}

TEST_CASE("TestComponentDestructorCalledWhenManagerDestroyed")
{
    bool freed = false;
    {
        EcsTestContext context;
        auto e = context.ecs->create();
        e->add_component<FreedSentinel>(freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(EcsTestContext, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    bool freed = false;
    auto e = ecs->create();
    e->add_component<FreedSentinel>(freed);
    REQUIRE( !freed );
    ecs->free(e);
    REQUIRE( freed );
}

struct Counter : public Component
{
    explicit Counter(int counter = 0) : counter(counter) {}
    int counter;
};

struct MovementSystem : public Subsystem
{
    explicit MovementSystem(string label = "") : label(label) {}

    void update(float dt)
    {
        auto ecs = core::get_subsystem<core::EntityComponentSystem>();
        ecs->find_entities_with<Position, Direction>().visit(
            [&](Entity& object, Position& position, Direction& direction)
            {
                position.x += direction.x;
                position.y += direction.y;
            });
    }

    std::string label;
};

struct CounterSystem : public Subsystem
{
    void update(float dt)
    {
        auto ecs = core::get_subsystem<core::EntityComponentSystem>();
        ecs->find_entities_with<Counter>().visit(
            [&](Entity& object, Counter& c)
            {
                c.counter++;
            });
    }
};

TEST_CASE_METHOD(EcsTestContext, "TestConstructSystemWithArgs")
{
    add_subsystem<MovementSystem>("movement");
    REQUIRE("movement" == get_subsystem<MovementSystem>()->label);
}

TEST_CASE_METHOD(EcsTestContext, "TestApplySystem")
{
    std::vector<Entity*> created_entities;
    for (int i = 0; i < 75; ++i)
    {
        auto e = ecs->create();
        created_entities.push_back(e);
        if (i % 2 == 0) e->add_component<Position>(1, 2);
        if (i % 3 == 0) e->add_component<Direction>(1, 1);
        e->add_component<Counter>(0);
    }

    add_subsystem<MovementSystem>();
    for (int i = 0; i < 75; ++i)
    {
        auto e = ecs->create();
        created_entities.push_back(e);
        if (i % 2 == 0) e->add_component<Position>(1, 2);
        if (i % 3 == 0) e->add_component<Direction>(1, 1);
        e->add_component<Counter>(0);
    }

    get_subsystem<MovementSystem>()->update(0.f);
    for (auto entity : created_entities)
    {
        auto position = entity->get_component<Position>();
        auto direction = entity->get_component<Direction>();

        if (position && direction)
        {
            REQUIRE(2.0 == Approx(position->x));
            REQUIRE(3.0 == Approx(position->y));
        }
        else if (position)
        {
            REQUIRE(1.0 == Approx(position->x));
            REQUIRE(2.0 == Approx(position->y));
        }
    }
}

TEST_CASE_METHOD(EcsTestContext, "TestApplyAllSystems")
{
    std::vector<Entity*> created_entities;
    for (int i = 0; i < 150; ++i) {
        auto e = ecs->create();
        created_entities.push_back(e);
        if (i % 2 == 0) e->add_component<Position>(1, 2);
        if (i % 3 == 0) e->add_component<Direction>(1, 1);
        e->add_component<Counter>(0);
    }

    add_subsystem<MovementSystem>();
    add_subsystem<CounterSystem>();

    get_subsystem<MovementSystem>()->update(0.f);
    get_subsystem<CounterSystem>()->update(0.f);

    for (auto entity : created_entities)
    {
        auto position   = entity->get_component<Position>();
        auto direction  = entity->get_component<Direction>();
        auto counter    = entity->get_component<Counter>();

        if (position && direction)
        {
            REQUIRE(2.0 == Approx(position->x));
            REQUIRE(3.0 == Approx(position->y));
        }
        else if (position)
        {
            REQUIRE(1.0 == Approx(position->x));
            REQUIRE(2.0 == Approx(position->y));
        }

        REQUIRE( 1 == counter->counter );
    }
}
