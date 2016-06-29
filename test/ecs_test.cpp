#define CATCH_CONFIG_MAIN

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
    EntityManagerFixture() : em(event), sys(em, event) {}
    EntityManager   em;
    EventManager    event;
    SystemManager   sys;
};

TEST_CASE_METHOD(EntityManagerFixture, "TestCreateEntity")
{
    REQUIRE( em.size() == 0UL );

    auto e2 = em.spawn();
    REQUIRE( e2.is_alive() );
    REQUIRE( em.size() == 1UL );

    Entity e1 = e2;
    REQUIRE( e1.is_alive() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityCreateFromCopy")
{
    auto e = em.spawn();
    auto ep = e.add_component<Position>(1, 2);

    auto f = em.clone(e);
    auto fp = f.get_component<Position>();

    REQUIRE( ep != fp );
    REQUIRE( ep->x == fp->x );
    REQUIRE( ep->y == fp->y );

    REQUIRE( e != f );
    REQUIRE( e.get_components_mask() == f.get_components_mask() );

    REQUIRE( em.size() == 2 );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityAsBoolean")
{
    REQUIRE( em.size() == 0UL );

    auto e = em.spawn();
    REQUIRE( e.is_alive() );
    REQUIRE( em.size() == 1UL );
    REQUIRE( !(!e) );

    e.dispose();
    REQUIRE( em.size() == 0UL );
    REQUIRE( !e );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityReuse")
{
    auto e1 = em.spawn();
    auto e2 = e1;
    auto index = e1.get_index();
    auto version = e1.get_version();
    REQUIRE( e1.is_alive() );
    REQUIRE( e2.is_alive() );

    e1.add_component<Position>();
    e1.dispose();
    REQUIRE( !e1.is_alive() );
    REQUIRE( !e2.is_alive() );

    auto e3 = em.spawn();
    REQUIRE( e3.get_index() == index );
    REQUIRE( e3.get_version() != version );
    REQUIRE( !e3.has_component<Position>() );

    e3.add_component<Position>();
    REQUIRE( e3.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityAssignment")
{
    Entity a = em.spawn();
    Entity b = a;
    REQUIRE( a == b );

    a.invalidate();
    REQUIRE( a != b );

    REQUIRE( !a );
    REQUIRE( b );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentConstruction")
{
    auto e = em.spawn();
    auto p = e.add_component<Position>(1, 2);

    auto cp = e.get_component<Position>();
    REQUIRE( p == cp );
    REQUIRE( cp->x == Approx(1.0f) );
    REQUIRE( cp->y == Approx(2.0f) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentIdsDiffer")
{
    REQUIRE( ComponentTraitInfo<Position>::id() != ComponentTraitInfo<Direction>::id() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentHandleInvalidatedWhenEntityDestroyed") {
    auto a = em.spawn();
    auto position = a.add_component<Position>(1, 2);
    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    a.dispose();
    REQUIRE( !a.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentAssignmentFromCopy")
{
    auto e = em.spawn();
    auto p = Position(1, 2);
    auto h = e.add_component<Position>(p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );

    e.dispose();
    REQUIRE( !e.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestDestroyEntity")
{
    auto e = em.spawn();
    auto f = em.spawn();

    e.add_component<Position>();
    f.add_component<Position>();

    e.add_component<Direction>();
    f.add_component<Direction>();

    REQUIRE( e );
    REQUIRE( f );
    REQUIRE( e.get_component<Position>() != nullptr );
    REQUIRE( e.get_component<Direction>() != nullptr );
    REQUIRE( f.get_component<Position>() != nullptr );
    REQUIRE( f.get_component<Direction>() != nullptr );
    REQUIRE( e.has_component<Position>() );
    REQUIRE( f.has_component<Position>() );

    e.dispose();
    REQUIRE( !e );
    REQUIRE( f );
    REQUIRE( f.get_component<Position>() != nullptr );
    REQUIRE( f.get_component<Direction>() != nullptr );
    REQUIRE( f.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyAll")
{
    auto e = em.spawn();
    auto f = em.spawn();
    em.reset();
    REQUIRE( !e );
    REQUIRE( !f );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyHole") {
    std::vector<Entity> entities;

    auto count = [this]()->int {
        auto e = em.find_entities_with<Position>();
        auto cursor = e.begin();
        return std::count_if(e.begin(), e.end(), [](const Entity &) { return true; });
    };

    for (int i = 0; i < 5000; i++) {
        auto e = em.spawn();
        e.add_component<Position>();
        entities.push_back(e);
    }

    REQUIRE(count() ==  5000);
    entities[2500].dispose();
    REQUIRE(count() ==  4999);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdSet")
{
    auto a = em.spawn();
    auto b = em.spawn();
    auto c = em.spawn();
    set<Entity> entitySet;
    
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdMap")
{
    auto a = em.spawn();
    auto b = em.spawn();
    auto c = em.spawn();
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
    auto e = em.spawn();
    auto f = em.spawn();
    auto g = em.spawn();

    e.add_component<Position>(1, 2);
    e.add_component<Direction>();
    f.add_component<Position>(3, 4);
    g.add_component<Direction>();

    REQUIRE( 2 == size(em.find_entities_with<Position>()) );
    REQUIRE( 2 == size(em.find_entities_with<Direction>()) );
    REQUIRE( 1 == size(em.find_entities_with<Position, Direction>()) );

    em.reset();

    for( auto i=0; i<150; i++ )
    {
        auto h = em.spawn();
        if( i % 2 == 0 ) h.add_component<Position>();
        if( i % 3 == 0 ) h.add_component<Direction>();
    }

    REQUIRE( 50 == size(em.find_entities_with<Direction>()) );
    REQUIRE( 75 == size(em.find_entities_with<Position>()) );
    REQUIRE( 25 == size(em.find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestGetComponentsAsTuple") {
  auto e = em.spawn();
  e.add_component<Position>(1, 2);
  e.add_component<Direction>(3, 4);

  auto components = e.get_components<Position, Direction>();
  REQUIRE(std::get<0>(components)->x == 1);
  REQUIRE(std::get<0>(components)->y == 2);
  REQUIRE(std::get<1>(components)->x == 3);
  REQUIRE(std::get<1>(components)->y == 4);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityIteration")
{
    auto e = em.spawn();
    auto f = em.spawn();

    e.add_component<Position>(1, 2);

    auto count = 0;
    em.find_entities_with<Position>().each([&](Entity entity, Position& position)
    {
        count ++;
        REQUIRE( position.x == 1 );
        REQUIRE( position.y == 2 );
        REQUIRE( entity == e );
    });

    REQUIRE( count == 1 );

    for( auto ne : em.find_entities_with<Position>() )
        count ++;
    REQUIRE( count == 2 );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestIterateAllEntitiesSkipsDestroyed") {
    auto e = em.spawn();
    auto f = em.spawn();
    auto g = em.spawn();

    f.dispose();
    auto it = em.find_entities().begin();

    REQUIRE( *it == e );
    ++it;
    REQUIRE( *it == g );
    ++it;

    REQUIRE( it == em.find_entities().end() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentsRemovedFromReusedEntities")
{
  auto e = em.spawn();
  auto eversion = e.get_version();
  auto eindex = e.get_index();
  e.add_component<Position>(1, 2);
  e.dispose();

  auto f = em.spawn();
  REQUIRE( eindex == f.get_index() );
  REQUIRE( eversion < f.get_version() );
  REQUIRE( !f.has_component<Position>() );
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
        EntityManager em(es);
        auto e = em.spawn();
        e.add_component<FreedSentinel>(freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    auto e = em.spawn();
    bool freed = false;
    e.add_component<FreedSentinel>(freed);
    REQUIRE( !freed );
    e.dispose();
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
        auto e = em.spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) e.add_component<Position>(1, 2);
        if (i % 3 == 0) e.add_component<Direction>(1, 1);
        e.add_component<Counter>(0);
    }

    sys.add<MovementSystem>();
    sys.update(0.0f);
    for (auto entity : created_entities)
    {
        auto position = entity.get_component<Position>();
        auto direction = entity.get_component<Direction>();

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
        auto e = em.spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) e.add_component<Position>(1, 2);
        if (i % 3 == 0) e.add_component<Direction>(1, 1);
        e.add_component<Counter>(0);
    }

    sys.add<MovementSystem>();
    sys.add<CounterSystem>();
    sys.update(0.f);

    for (auto entity : created_entities)
    {
        auto position   = entity.get_component<Position>();
        auto direction  = entity.get_component<Direction>();
        auto counter    = entity.get_component<Counter>();

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
