#define CATCH_CONFIG_MAIN

#include <ostream>
#include <set>
#include <map>
#include <vector>

#include "catch.hpp"
#include "flow2d.hpp"
#include "entity.hpp"

using namespace std;
using namespace flow2d;

template<typename T> int size(const T& t)
{
    auto n = 0;
    for( auto i : t ) { n++; (void)i; }
    return n;
}

struct Position
{
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    bool operator == (const Position& rh) const
    {
        return x == rh.x && y == rh.y;
    }

    float x, y;
};

struct Direction
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
    EntityManagerFixture() : em(event) {}
    EntityManager   em;
    EventManager    event;
};

TEST_CASE_METHOD(EntityManagerFixture, "TestCreateEntity")
{
    REQUIRE( em.size() == 0UL );

    Entity e1;
    REQUIRE( !e1.is_valid() );

    auto e2 = em.create();
    REQUIRE( e2.is_valid() );
    REQUIRE( em.size() == 1UL );

    e1 = e2;
    REQUIRE( e1.is_valid() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityCreateFromCopy")
{
    auto e = em.create();
    auto ep = e.add_component<Position>(1, 2);

    auto f = em.create_from(e);
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

    auto e = em.create();
    REQUIRE( e.is_valid() );
    REQUIRE( em.size() == 1UL );
    REQUIRE( !(!e) );

    e.dispose();
    REQUIRE( em.size() == 0UL );
    REQUIRE( !e );

    Entity e2;  // not initialized
    REQUIRE( !e2 );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityReuse")
{
    auto e1 = em.create();
    auto e2 = e1;
    auto id = e1.get_uid();
    REQUIRE( e1.is_valid() );
    REQUIRE( e2.is_valid() );

    e1.add_component<Position>();
    e1.dispose();
    REQUIRE( !e1.is_valid() );
    REQUIRE( !e2.is_valid() );

    auto e3 = em.create();
    REQUIRE( e3.get_uid() != id );
    REQUIRE( e3.get_uid().index() == id.index() );
    REQUIRE( !e3.has_component<Position>() );

    e3.add_component<Position>();
    REQUIRE( e3.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityAssignment")
{
    Entity a, b;
    a = em.create();
    REQUIRE( a != b );
    b = a;
    REQUIRE( a == b );
    a.invalidate();
    REQUIRE( a != b );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentConstruction")
{
    auto e = em.create();
    auto p = e.add_component<Position>(1, 2);

    auto cp = e.get_component<Position>();
    REQUIRE( p == cp );
    REQUIRE( cp->x == Approx(1.0f) );
    REQUIRE( cp->y == Approx(2.0f) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentIdsDiffer")
{
    REQUIRE( ComponentTrait<Position>::type() != ComponentTrait<Direction>::type() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentHandleInvalidatedWhenEntityDestroyed") {
    auto a = em.create();
    auto position = a.add_component<Position>(1, 2);
    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    a.dispose();
    REQUIRE(!position);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentAssignmentFromCopy")
{
    auto e = em.create();
    auto p = Position(1, 2);
    auto h = e.add_component<Position>(p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );

    e.dispose();
    REQUIRE( !h );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestDestroyEntity")
{
    auto e = em.create();
    auto f = em.create();

    e.add_component<Position>();
    f.add_component<Position>();

    e.add_component<Direction>();
    f.add_component<Direction>();

    REQUIRE( e );
    REQUIRE( f );
    REQUIRE( e.get_component<Position>().is_valid() );
    REQUIRE( e.get_component<Direction>().is_valid() );
    REQUIRE( f.get_component<Position>().is_valid() );
    REQUIRE( f.get_component<Direction>().is_valid() );
    REQUIRE( e.has_component<Position>() );
    REQUIRE( f.has_component<Position>() );

    e.dispose();
    REQUIRE( !e );
    REQUIRE( f );
    REQUIRE( f.get_component<Position>().is_valid() );
    REQUIRE( f.get_component<Direction>().is_valid() );
    REQUIRE( f.has_component<Position>() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyAll")
{
    auto e = em.create();
    auto f = em.create();
    em.reset();
    REQUIRE( !e );
    REQUIRE( !f );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityDestroyHole") {
  std::vector<Entity> entities;

  auto count = [this]()->int {
    auto e = em.find_entities_with<Position>();
    return std::count_if(e.begin(), e.end(), [](const Entity &) { return true; });
  };

  for (int i = 0; i < 5000; i++) {
    auto e = em.create();
    e.add_component<Position>();
    entities.push_back(e);
  }

  REQUIRE(count() ==  5000);

  entities[2500].dispose();
  REQUIRE(count() ==  4999);
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdSet")
{
    auto a = em.create();
    auto b = em.create();
    auto c = em.create();
    set<Entity> entitySet;
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestEntityInStdMap")
{
    auto a = em.create();
    auto b = em.create();
    auto c = em.create();
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
    auto e = em.create();
    auto f = em.create();
    auto g = em.create();

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
        auto h = em.create();
        if( i % 2 == 0 ) h.add_component<Position>();
        if( i % 3 == 0 ) h.add_component<Direction>();
    }

    REQUIRE( 50 == size(em.find_entities_with<Direction>()) );
    REQUIRE( 75 == size(em.find_entities_with<Position>()) );
    REQUIRE( 25 == size(em.find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestGetComponentsAsTuple") {
  auto e = em.create();
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
    auto e = em.create();
    auto f = em.create();

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
    auto e = em.create();
    auto f = em.create();
    auto g = em.create();

    f.dispose();
    auto it = em.get_entities().begin();

    REQUIRE( *it == e );
    ++it;
    REQUIRE( *it == g );
    ++it;

    REQUIRE( it == em.get_entities().end() );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentsRemovedFromReusedEntities")
{
  auto e = em.create();
  auto eid = e.get_uid();
  e.add_component<Position>(1, 2);
  e.dispose();

  auto f = em.create();
  auto fid = f.get_uid();

  REQUIRE( eid.index() == fid.index() );
  REQUIRE( eid.version() < fid.version() );
  REQUIRE( !f.has_component<Position>() );
}

struct FreedSentinel
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
        auto e = em.create();
        e.add_component<FreedSentinel>(freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(EntityManagerFixture, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    auto e = em.create();
    bool freed = false;
    e.add_component<FreedSentinel>(freed);
    REQUIRE( !freed );
    e.dispose();
    REQUIRE( freed );
}