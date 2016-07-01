#include <ostream>
#include <set>
#include <map>
#include <vector>
#include <string>

#include <catch.hpp>
#include <flow2d.hpp>

using namespace std;
using namespace flow2d;

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

struct EntityManagerFixture
{
    EntityManagerFixture() : _world(event), sys(_world, event) {}
    EntityManager   _world;
    EventManager    event;
    SystemManager   sys;
};

TEST_CASE_METHOD(EntityManagerFixture, "TestCreateEntity")
{
    REQUIRE( _world.size() == 0UL );

    auto e2 = _world.spawn();
    REQUIRE( _world.is_alive(e2) );
    REQUIRE( _world.size() == 1UL );

    Entity e1 = e2;
    REQUIRE( _world.is_alive(e1) );
}

// its complex to implementation a generic clone senario,
// when take hierarchy into consideration.
// TEST_CASE_METHOD(EntityManagerFixture, "TestEntityCreateFromCopy")
// {
//     auto e = _world.spawn();
//     auto ep = _world.add_component<Position>(e, 1, 2);

//     auto f = _world.clone(e);
//     auto fp = _world.get_component<Position>(f);

//     REQUIRE( ep != fp );
//     REQUIRE( ep->x == fp->x );
//     REQUIRE( ep->y == fp->y );

//     REQUIRE( e != f );
//     REQUIRE( _world.get_components_mask(e) == _world.get_components_mask(f) );

//     REQUIRE( _world.size() == 2 );
// }

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityAsBoolean")
{
    REQUIRE( _world.size() == 0UL );

    auto e = _world.spawn();
    REQUIRE( _world.is_alive(e) );
    REQUIRE( _world.size() == 1UL );

    _world.dispose(e);
    REQUIRE( _world.size() == 0UL );
    REQUIRE( !_world.is_alive(e) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityReuse")
{
    auto e1 = _world.spawn();
    auto e2 = e1;
    auto index = e1.get_index();
    auto version = e1.get_version();
    REQUIRE( _world.is_alive(e1) );
    REQUIRE( _world.is_alive(e2) );

    _world.add_component<Position>(e1);
    _world.dispose(e1);
    REQUIRE( !_world.is_alive(e1) );
    REQUIRE( !_world.is_alive(e2) );

    auto e3 = _world.spawn();
    REQUIRE( e3.get_index() == index );
    REQUIRE( e3.get_version() != version );
    REQUIRE( !_world.has_component<Position>(e3) );

    _world.add_component<Position>(e3);
    REQUIRE( _world.has_component<Position>(e3) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityAssignment")
{
    Entity a = _world.spawn();
    Entity b = a;
    REQUIRE( a == b );

    a.invalidate();
    REQUIRE( a != b );

    REQUIRE( !_world.is_alive(a) );
    REQUIRE( _world.is_alive(b) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentConstruction")
{
    auto e = _world.spawn();
    auto p = _world.add_component<Position>(e, 1, 2);

    auto cp = _world.get_component<Position>(e);
    REQUIRE( p == cp );
    REQUIRE( cp->x == Approx(1.0f) );
    REQUIRE( cp->y == Approx(2.0f) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentIdsDiffer")
{
    REQUIRE( ComponentTraitInfo<Position>::id() != ComponentTraitInfo<Direction>::id() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentHandleInvalidatedWhenEntityDestroyed") {
    auto a = _world.spawn();
    auto position = _world.add_component<Position>(a, 1, 2);
    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    _world.dispose(a);
    REQUIRE( !_world.has_component<Position>(a) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentAssignmentFromCopy")
{
    auto e = _world.spawn();
    auto p = Position(1, 2);
    auto h = _world.add_component<Position>(e, p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );

    _world.dispose(e);
    REQUIRE( !_world.has_component<Position>(e) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestDestroyEntity")
{
    auto e = _world.spawn();
    auto f = _world.spawn();

    _world.add_component<Position>(e);
    _world.add_component<Position>(f);

    _world.add_component<Direction>(e);
    _world.add_component<Direction>(f);

    REQUIRE( _world.is_alive(e) );
    REQUIRE( _world.is_alive(f) );
    REQUIRE( _world.get_component<Position>(e) != nullptr );
    REQUIRE( _world.get_component<Direction>(e) != nullptr );
    REQUIRE( _world.get_component<Position>(f) != nullptr );
    REQUIRE( _world.get_component<Direction>(f) != nullptr );
    REQUIRE( _world.has_component<Position>(e) );
    REQUIRE( _world.has_component<Position>(f) );

    _world.dispose(e);
    REQUIRE( !_world.is_alive(e) );
    REQUIRE( _world.is_alive(f) );
    REQUIRE( _world.get_component<Position>(f) != nullptr );
    REQUIRE( _world.get_component<Direction>(f) != nullptr );
    REQUIRE( _world.has_component<Position>(f) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyAll")
{
    auto e = _world.spawn();
    auto f = _world.spawn();
    _world.reset();
    REQUIRE( !_world.is_alive(e) );
    REQUIRE( !_world.is_alive(f) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyHole") {
    std::vector<Entity> entities;

    auto count = [this]()->int {
        auto view = _world.find_entities_with<Position>();
        auto cursor = view.begin();
        return std::count_if(view.begin(), view.end(), [](const Entity &) { return true; });
    };

    for (int i = 0; i < 5000; i++) {
        auto e = _world.spawn();
        _world.add_component<Position>(e);
        entities.push_back(e);
    }

    REQUIRE(count() ==  5000);
    _world.dispose(entities[2500]);
    REQUIRE(count() ==  4999);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdSet")
{
    auto a = _world.spawn();
    auto b = _world.spawn();
    auto c = _world.spawn();
    set<Entity> entitySet;
    
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdMap")
{
    auto a = _world.spawn();
    auto b = _world.spawn();
    auto c = _world.spawn();
    map<Entity, int> entityMap;
    REQUIRE( entityMap.insert(pair<Entity, int>(a, 1)).second );
    REQUIRE( entityMap.insert(pair<Entity, int>(b, 2)).second );
    REQUIRE( entityMap.insert(pair<Entity, int>(c, 3)).second );
    REQUIRE( entityMap[a] == 1 );
    REQUIRE( entityMap[b] == 2 );
    REQUIRE( entityMap[c] == 3 );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestGetEntitiesWithComponent")
{
    auto e = _world.spawn();
    auto f = _world.spawn();
    auto g = _world.spawn();

    _world.add_component<Position>(e, 1, 2);
    _world.add_component<Direction>(e);
    _world.add_component<Position>(f, 3, 4);
    _world.add_component<Direction>(g);

    REQUIRE( 2 == size(_world.find_entities_with<Position>()) );
    REQUIRE( 2 == size(_world.find_entities_with<Direction>()) );
    REQUIRE( 1 == size(_world.find_entities_with<Position, Direction>()) );

    _world.reset();

    for( auto i=0; i<150; i++ )
    {
        auto h = _world.spawn();
        if( i % 2 == 0 ) _world.add_component<Position>(h);
        if( i % 3 == 0 ) _world.add_component<Direction>(h);
    }

    REQUIRE( 50 == size(_world.find_entities_with<Direction>()) );
    REQUIRE( 75 == size(_world.find_entities_with<Position>()) );
    REQUIRE( 25 == size(_world.find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestGetComponentsAsTuple") {
    auto e = _world.spawn();
    _world.add_component<Position>(e, 1, 2);
    _world.add_component<Direction>(e, 3, 4);

    auto components = _world.get_components<Position, Direction>(e);
    REQUIRE(std::get<0>(components)->x == 1);
    REQUIRE(std::get<0>(components)->y == 2);
    REQUIRE(std::get<1>(components)->x == 3);
    REQUIRE(std::get<1>(components)->y == 4);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityIteration")
{
    auto e = _world.spawn();
    auto f = _world.spawn();

    _world.add_component<Position>(e, 1, 2);

    auto count = 0;
    _world.find_entities_with<Position>().each([&](Entity entity, Position& position)
    {
        count ++;
        REQUIRE( position.x == 1 );
        REQUIRE( position.y == 2 );
        REQUIRE( entity == e );
    });

    REQUIRE( count == 1 );

    for( auto ne : _world.find_entities_with<Position>() )
        count ++;
    REQUIRE( count == 2 );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestIterateAllEntitiesSkipsDestroyed") {
    auto e = _world.spawn();
    auto f = _world.spawn();
    auto g = _world.spawn();

    _world.dispose(f);
    auto it = _world.find_entities().begin();

    REQUIRE( *it == e );
    ++it;
    REQUIRE( *it == g );
    ++it;

    REQUIRE( it == _world.find_entities().end() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentsRemovedFromReusedEntities")
{
  auto e = _world.spawn();
  auto eversion = e.get_version();
  auto eindex = e.get_index();
  _world.add_component<Position>(e, 1, 2);
  _world.dispose(e);

  auto f = _world.spawn();
  REQUIRE( eindex == f.get_index() );
  REQUIRE( eversion < f.get_version() );
  REQUIRE( !_world.has_component<Position>(f) );
}

struct FreedSentinel : public Component
{
  explicit FreedSentinel(bool &yes) : yes(yes) {}
  ~FreedSentinel() { yes = true; }
  bool &yes;
};

TEST_CASE("TestComponentDestructorCalledWhenManagerDestroyed")
{
    bool freed = false;
    {
        EventManager es;
        EntityManager world(es);
        auto e = world.spawn();
        world.add_component<FreedSentinel>(e, freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    auto e = _world.spawn();
    bool freed = false;
    _world.add_component<FreedSentinel>(e, freed);
    REQUIRE( !freed );
    _world.dispose(e);
    REQUIRE( freed );
}

struct Counter : public Component
{
    explicit Counter(int counter = 0) : counter(counter) {}
    int counter;
};

struct MovementSystem : public SystemTrait<MovementSystem>
{
    explicit MovementSystem(string label = "") : label(label) {}

    void update(float dt) override
    {
        world().find_entities_with<Position, Direction>()
            .each([&](Entity ent, Position& pos, Direction& dir)
            {
                pos.x += dir.x;
                pos.y += dir.y;
            });
    }

    std::string label;
};

struct CounterSystem : public SystemTrait<CounterSystem>
{
    void update(float dt) override
    {
        world().find_entities_with<Counter>()
            .each([&](Entity ent, Counter& c)
            {
                c.counter ++;
            });
    }
};

TEST_CASE_METHOD(EntityManagerFixture, "TestConstructSystemWithArgs")
{
    sys.add<MovementSystem>("movement");
    REQUIRE("movement" == sys.get<MovementSystem>()->label);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestApplySystem")
{
    std::vector<Entity> created_entities;
    for (int i = 0; i < 150; ++i)
    {
        auto e = _world.spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) _world.add_component<Position>(e, 1, 2);
        if (i % 3 == 0) _world.add_component<Direction>(e, 1, 1);
        _world.add_component<Counter>(e, 0);
    }

    sys.add<MovementSystem>();
    sys.update(0.0f);
    for (auto entity : created_entities)
    {
        auto position = _world.get_component<Position>(entity);
        auto direction = _world.get_component<Direction>(entity);

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

TEST_CASE_METHOD(EntityManagerFixture, "TestApplyAllSystems")
{
    std::vector<Entity> created_entities;
    for (int i = 0; i < 150; ++i) {
        auto e = _world.spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) _world.add_component<Position>(e, 1, 2);
        if (i % 3 == 0) _world.add_component<Direction>(e, 1, 1);
        _world.add_component<Counter>(e, 0);
    }

    sys.add<MovementSystem>();
    sys.add<CounterSystem>();
    sys.update(0.f);

    for (auto entity : created_entities)
    {
        auto position   = _world.get_component<Position>(entity);
        auto direction  = _world.get_component<Direction>(entity);
        auto counter    = _world.get_component<Counter>(entity);

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
