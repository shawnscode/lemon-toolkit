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

struct TestContext
{
    TestContext()
    {
        initialize(0);
    }

    ~TestContext()
    {
        dispose();
    }
};

TEST_CASE_METHOD(TestContext, "TestCreateEntity")
{
    REQUIRE( world().size() == 0UL );

    auto e2 = create();
    REQUIRE( is_valid(e2) );
    REQUIRE( world().size() == 1UL );

    auto e1 = e2;
    REQUIRE( is_valid(e1) );
}

// its complex to implementation a generic clone senario,
// when take hierarchy into consideration.
// TEST_CASE_METHOD(TestContext, "TestEntityCreateFromCopy")
// {
//     auto e = create();
//     auto ep = add_component<Position>(e, 1, 2);

//     auto f = _world.clone(e);
//     auto fp  get_component<Position>(f);

//     REQUIRE( ep != fp );
//     REQUIRE( ep->x == fp->x );
//     REQUIRE( ep->y == fp->y );

//     REQUIRE( e != f );
//     REQUIRE( get_components_mask(e) == get_components_mask(f) );

//     REQUIRE( world().size() == 2 );
// }

TEST_CASE_METHOD(TestContext, "TestEntityAsBoolean")
{
    REQUIRE( world().size() == 0UL );

    auto e = create();
    REQUIRE( is_valid(e) );
    REQUIRE( world().size() == 1UL );

    free(e);
    REQUIRE( world().size() == 0UL );
    REQUIRE( !is_valid(e) );
}

TEST_CASE_METHOD(TestContext, "TestEntityReuse")
{
    auto e1 = create();
    auto e2 = e1;
    auto index = e1.get_index();
    auto version = e1.get_version();
    REQUIRE( is_valid(e1) );
    REQUIRE( is_valid(e2) );

    add_component<Position>(e1);
    free(e1);
    REQUIRE( !is_valid(e1) );
    REQUIRE( !is_valid(e2) );

    auto e3 = create();
    REQUIRE( e3.get_index() == index );
    REQUIRE( e3.get_version() != version );
    REQUIRE( !has_components<Position>(e3) );

    add_component<Position>(e3);
    REQUIRE( has_components<Position>(e3) );
}

TEST_CASE_METHOD(TestContext, "TestEntityAssignment")
{
    lemon::Handle a = create();
    lemon::Handle b = a;
    REQUIRE( a == b );

    a.invalidate();
    REQUIRE( a != b );

    REQUIRE( !is_valid(a) );
    REQUIRE( is_valid(b) );
}

TEST_CASE_METHOD(TestContext, "TestComponentConstruction")
{
    auto e = create();
    auto p = add_component<Position>(e, 1, 2);

    auto cp = get_component<Position>(e);
    REQUIRE( p == cp );
    REQUIRE( cp->x == Approx(1.0f) );
    REQUIRE( cp->y == Approx(2.0f) );
}

TEST_CASE_METHOD(TestContext, "TestComponentIdsDiffer")
{
    REQUIRE( get_components_mask<Position>() != get_components_mask<Direction>() );
    REQUIRE( (get_components_mask<Position, Direction>()) != get_components_mask<Direction>() );
}

TEST_CASE_METHOD(TestContext, "TestComponentlemon::HandleInvalidatedWhenEntityDestroyed") {
    auto a = create();
    auto position = add_component<Position>(a, 1, 2);
    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    free(a);
    REQUIRE( !has_components<Position>(a) );
}

TEST_CASE_METHOD(TestContext, "TestComponentAssignmentFromCopy")
{
    auto e = create();
    auto p = Position(1, 2);
    auto h = add_component<Position>(e, p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );

    free(e);
    REQUIRE( !has_components<Position>(e) );
}

TEST_CASE_METHOD(TestContext, "TestDestroyEntity")
{
    auto e = create();
    auto f = create();

    add_component<Position>(e);
    add_component<Position>(f);

    add_component<Direction>(e);
    add_component<Direction>(f);

    REQUIRE( is_valid(e) );
    REQUIRE( is_valid(f) );
    REQUIRE( get_component<Position>(e) != nullptr );
    REQUIRE( get_component<Direction>(e) != nullptr );
    REQUIRE( get_component<Position>(f) != nullptr );
    REQUIRE( get_component<Direction>(f) != nullptr );
    REQUIRE( has_components<Position>(e) );
    REQUIRE( has_components<Position>(f) );

    free(e);
    REQUIRE( !is_valid(e) );
    REQUIRE( is_valid(f) );
    REQUIRE( get_component<Position>(f) != nullptr );
    REQUIRE( get_component<Direction>(f) != nullptr );
    REQUIRE( has_components<Position>(f) );
}

TEST_CASE_METHOD(TestContext, "TestEntityDestroyAll")
{
    auto e = create();
    auto f = create();
    world().free_all();

    REQUIRE( !is_valid(e) );
    REQUIRE( !is_valid(f) );
}

TEST_CASE_METHOD(TestContext, "TestEntityDestroyHole") {
    std::vector<Handle> entities;

    auto count = [this]()->int {
        auto view = find_entities_with<Position>();
        auto cursor = view.begin();
        return std::count_if(view.begin(), view.end(), [](const lemon::Handle &) { return true; });
    };

    for (int i = 0; i < 5000; i++) {
        auto e = create();
        add_component<Position>(e);
        entities.push_back(e);
    }

    REQUIRE(count() ==  5000);
    free(entities[2500]);
    REQUIRE(count() ==  4999);
}

TEST_CASE_METHOD(TestContext, "TestEntityInStdSet")
{
    auto a = create();
    auto b = create();
    auto c = create();
    set<Handle> entitySet;
    
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(TestContext, "TestEntityInStdMap")
{
    auto a = create();
    auto b = create();
    auto c = create();
    map<Handle, int> entityMap;
    REQUIRE( entityMap.insert(pair<Handle, int>(a, 1)).second );
    REQUIRE( entityMap.insert(pair<Handle, int>(b, 2)).second );
    REQUIRE( entityMap.insert(pair<Handle, int>(c, 3)).second );
    REQUIRE( entityMap[a] == 1 );
    REQUIRE( entityMap[b] == 2 );
    REQUIRE( entityMap[c] == 3 );
}

TEST_CASE_METHOD(TestContext, "TestGetEntitiesWithComponent")
{
    auto e = create();
    auto f = create();
    auto g = create();

    add_component<Position>(e, 1, 2);
    add_component<Direction>(e);
    add_component<Position>(f, 3, 4);
    add_component<Direction>(g);

    REQUIRE( 2 == size(find_entities_with<Position>()) );
    REQUIRE( 2 == size(find_entities_with<Direction>()) );
    REQUIRE( 1 == size(find_entities_with<Position, Direction>()) );

    world().free_all();

    for( auto i=0; i<150; i++ )
    {
        auto h = create();
        if( i % 2 == 0 ) add_component<Position>(h);
        if( i % 3 == 0 ) add_component<Direction>(h);
    }

    REQUIRE( 50 == size(find_entities_with<Direction>()) );
    REQUIRE( 75 == size(find_entities_with<Position>()) );
    REQUIRE( 25 == size(find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(TestContext, "TestGetComponentsAsTuple") {
    auto e = create();
    add_component<Position>(e, 1, 2);
    add_component<Direction>(e, 3, 4);

    auto components = get_components<Position, Direction>(e);
    REQUIRE(std::get<0>(components)->x == 1);
    REQUIRE(std::get<0>(components)->y == 2);
    REQUIRE(std::get<1>(components)->x == 3);
    REQUIRE(std::get<1>(components)->y == 4);
}

TEST_CASE_METHOD(TestContext, "TestEntityIteration")
{
    auto e = create();
    auto f = create();

    add_component<Position>(e, 1, 2);

    auto count = 0;
    find_entities_with<Position>().visit([&](lemon::Handle entity, Position& position)
    {
        count ++;
        REQUIRE( position.x == 1 );
        REQUIRE( position.y == 2 );
        REQUIRE( entity == e );
    });

    REQUIRE( count == 1 );

    for( auto ne : find_entities_with<Position>() )
        count ++;
    REQUIRE( count == 2 );
}

TEST_CASE_METHOD(TestContext, "TestIterateAllEntitiesSkipsDestroyed") {
    auto e = create();
    auto f = create();
    auto g = create();

    free(f);
    auto it = find_entities().begin();

    REQUIRE( *it == e );
    ++it;
    REQUIRE( *it == g );
    ++it;

    REQUIRE( it == find_entities().end() );
}

TEST_CASE_METHOD(TestContext, "TestComponentsRemovedFromReusedEntities")
{
  auto e = create();
  auto eversion = e.get_version();
  auto eindex = e.get_index();
  add_component<Position>(e, 1, 2);
  free(e);

  auto f = create();
  REQUIRE( eindex == f.get_index() );
  REQUIRE( eversion < f.get_version() );
  REQUIRE( !has_components<Position>(f) );
}

struct FreedSentinel : public Component
{
  explicit FreedSentinel(bool &yes) : yes(yes) {}
  ~FreedSentinel() { yes = true; }
  bool &yes;
};

TEST_CASE_METHOD(TestContext, "TestComponentDestruction")
{
    bool freed = false;
    auto e = create();
    add_component<FreedSentinel>(e, freed);
    remove_component<FreedSentinel>(e);

    REQUIRE( freed );
}

TEST_CASE("TestComponentDestructorCalledWhenManagerDestroyed")
{
    bool freed = false;
    {
        TestContext context;
        auto e = create();
        add_component<FreedSentinel>(e, freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(TestContext, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    bool freed = false;
    auto e = create();
    add_component<FreedSentinel>(e, freed);
    REQUIRE( !freed );
    free(e);
    REQUIRE( freed );
}

// struct Base : public VTraitComponent<Base, 32>
// {
//     virtual void add(int&) {}
// };

// struct Derived : public Base
// {
//     virtual void add(int& v) override { v++; }
// };

// TEST_CASE_METHOD(TestContext, "TestTraitComponent")
// {
//     auto e = create();
//     add_component<Derived>(e);

//     REQUIRE( has_components<Base::Trait>(e) );

//     int c = 0;
//     auto bt = get_component<Base::Trait>(e);
//     (*bt)->add(c);

//     REQUIRE( c == 1 );
// }

// struct Counter : public Component
// {
//     explicit Counter(int counter = 0) : counter(counter) {}
//     int counter;
// };

// struct MovementSystem : public SubsystemWithEntities<Position, Direction>
// {
//     explicit MovementSystem(string label = "") : label(label) {}

//     void update(float dt)
//     {
//         for( auto pair : *this )
//         {
//             std::get<0>(pair.second)->x += std::get<1>(pair.second)->x;
//             std::get<0>(pair.second)->y += std::get<1>(pair.second)->y;
//         }
//     }

//     std::string label;
// };

// struct CounterSystem : public SubsystemWithEntities<Counter>
// {
//     void update(float dt)
//     {
//         visit([&](lemon::Handle object, Counter& c)
//         {
//             c.counter ++;
//         });
//     }
// };

// TEST_CASE_METHOD(TestContext, "TestConstructSystemWithArgs")
// {
//     add_subsystem<MovementSystem>("movement");
//     REQUIRE("movement" == get_subsystem<MovementSystem>()->label);
// }

// TEST_CASE_METHOD(TestContext, "TestApplySystem")
// {

//     std::vector<Entity> created_entities;
//     for (int i = 0; i < 75; ++i)
//     {
//         auto e = create();
//         created_entities.push_back(e);
//         if (i % 2 == 0) add_component<Position>(e, 1, 2);
//         if (i % 3 == 0) add_component<Direction>(e, 1, 1);
//         add_component<Counter>(e, 0);
//     }

//     add_subsystem<MovementSystem>();
//     for (int i = 0; i < 75; ++i)
//     {
//         auto e = create();
//         created_entities.push_back(e);
//         if (i % 2 == 0) add_component<Position>(e, 1, 2);
//         if (i % 3 == 0) add_component<Direction>(e, 1, 1);
//         add_component<Counter>(e, 0);
//     }

//     get_subsystem<MovementSystem>()->update(0.f);

//     for (auto entity : created_entities)
//     {
//         auto position = get_component<Position>(entity);
//         auto direction = get_component<Direction>(entity);

//         if (position && direction)
//         {
//             REQUIRE(2.0 == Approx(position->x));
//             REQUIRE(3.0 == Approx(position->y));
//         }
//         else if (position)
//         {
//             REQUIRE(1.0 == Approx(position->x));
//             REQUIRE(2.0 == Approx(position->y));
//         }
//     }
// }

// TEST_CASE_METHOD(TestContext, "TestApplyAllSystems")
// {
//     std::vector<Entity> created_entities;
//     for (int i = 0; i < 150; ++i) {
//         auto e = create();
//         created_entities.push_back(e);
//         if (i % 2 == 0) add_component<Position>(e, 1, 2);
//         if (i % 3 == 0) add_component<Direction>(e, 1, 1);
//         add_component<Counter>(e, 0);
//     }

//     add_subsystem<MovementSystem>();
//     add_subsystem<CounterSystem>();

//     get_subsystem<MovementSystem>()->update(0.f);
//     get_subsystem<CounterSystem>()->update(0.f);

//     for (auto entity : created_entities)
//     {
//         auto position   = get_component<Position>(entity);
//         auto direction  = get_component<Direction>(entity);
//         auto counter    = get_component<Counter>(entity);

//         if (position && direction)
//         {
//             REQUIRE(2.0 == Approx(position->x));
//             REQUIRE(3.0 == Approx(position->y));
//         }
//         else if (position)
//         {
//             REQUIRE(1.0 == Approx(position->x));
//             REQUIRE(2.0 == Approx(position->y));
//         }

//         REQUIRE( 1 == counter->counter );
//     }
// }
