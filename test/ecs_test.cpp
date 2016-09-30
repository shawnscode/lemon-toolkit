#include <catch.hpp>
#include <lemon-toolkit.hpp>

#include <map>

using namespace std;
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
    size_of_world();

    REQUIRE( size_of_world() == 0UL );

    auto e2 = spawn();
    REQUIRE( alive(e2) );
    REQUIRE( size_of_world() == 1UL );

    Entity e1 = e2;
    REQUIRE( alive(e1) );
}

// its complex to implementation a generic clone senario,
// when take hierarchy into consideration.
// TEST_CASE_METHOD(TestContext, "TestEntityCreateFromCopy")
// {
//     auto e = spawn();
//     auto ep = add_component<Position>(e, 1, 2);

//     auto f = _world.clone(e);
//     auto fp = get_component<Position>(f);

//     REQUIRE( ep != fp );
//     REQUIRE( ep->x == fp->x );
//     REQUIRE( ep->y == fp->y );

//     REQUIRE( e != f );
//     REQUIRE( get_components_mask(e) == get_components_mask(f) );

//     REQUIRE( size_of_world() == 2 );
// }

TEST_CASE_METHOD(TestContext, "TestEntityAsBoolean")
{
    REQUIRE( size_of_world() == 0UL );

    auto e = spawn();
    REQUIRE( alive(e) );
    REQUIRE( size_of_world() == 1UL );

    recycle(e);
    REQUIRE( size_of_world() == 0UL );
    REQUIRE( !alive(e) );
}

TEST_CASE_METHOD(TestContext, "TestEntityReuse")
{
    auto e1 = spawn();
    auto e2 = e1;
    auto index = e1.get_index();
    auto version = e1.get_version();
    REQUIRE( alive(e1) );
    REQUIRE( alive(e2) );

    add_component<Position>(e1);
    recycle(e1);
    REQUIRE( !alive(e1) );
    REQUIRE( !alive(e2) );

    auto e3 = spawn();
    REQUIRE( e3.get_index() == index );
    REQUIRE( e3.get_version() != version );
    REQUIRE( !has_components<Position>(e3) );

    add_component<Position>(e3);
    REQUIRE( has_components<Position>(e3) );
}

TEST_CASE_METHOD(TestContext, "TestEntityAssignment")
{
    Entity a = spawn();
    Entity b = a;
    REQUIRE( a == b );

    a.invalidate();
    REQUIRE( a != b );

    REQUIRE( !alive(a) );
    REQUIRE( alive(b) );
}

TEST_CASE_METHOD(TestContext, "TestComponentConstruction")
{
    auto e = spawn();
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

TEST_CASE_METHOD(TestContext, "TestComponentHandleInvalidatedWhenEntityDestroyed") {
    auto a = spawn();
    auto position = add_component<Position>(a, 1, 2);
    REQUIRE(position);
    REQUIRE(position->x == 1);
    REQUIRE(position->y == 2);

    recycle(a);
    REQUIRE( !has_components<Position>(a) );
}

TEST_CASE_METHOD(TestContext, "TestComponentAssignmentFromCopy")
{
    auto e = spawn();
    auto p = Position(1, 2);
    auto h = add_component<Position>(e, p);

    REQUIRE( h );
    REQUIRE( h->x == p.x );
    REQUIRE( h->y == p.y );

    recycle(e);
    REQUIRE( !has_components<Position>(e) );
}

TEST_CASE_METHOD(TestContext, "TestDestroyEntity")
{
    auto e = spawn();
    auto f = spawn();

    add_component<Position>(e);
    add_component<Position>(f);

    add_component<Direction>(e);
    add_component<Direction>(f);

    REQUIRE( alive(e) );
    REQUIRE( alive(f) );
    REQUIRE( get_component<Position>(e) != nullptr );
    REQUIRE( get_component<Direction>(e) != nullptr );
    REQUIRE( get_component<Position>(f) != nullptr );
    REQUIRE( get_component<Direction>(f) != nullptr );
    REQUIRE( has_components<Position>(e) );
    REQUIRE( has_components<Position>(f) );

    recycle(e);
    REQUIRE( !alive(e) );
    REQUIRE( alive(f) );
    REQUIRE( get_component<Position>(f) != nullptr );
    REQUIRE( get_component<Direction>(f) != nullptr );
    REQUIRE( has_components<Position>(f) );
}

TEST_CASE_METHOD(TestContext, "TestEntityDestroyAll")
{
    auto e = spawn();
    auto f = spawn();
    reset_world();

    REQUIRE( !alive(e) );
    REQUIRE( !alive(f) );
}

TEST_CASE_METHOD(TestContext, "TestEntityDestroyHole") {
    std::vector<Entity> entities;

    auto count = [this]()->int {
        auto view = find_entities_with<Position>();
        auto cursor = view.begin();
        return std::count_if(view.begin(), view.end(), [](const Entity &) { return true; });
    };

    for (int i = 0; i < 5000; i++) {
        auto e = spawn();
        add_component<Position>(e);
        entities.push_back(e);
    }

    REQUIRE(count() ==  5000);
    recycle(entities[2500]);
    REQUIRE(count() ==  4999);
}

TEST_CASE_METHOD(TestContext, "TestEntityInStdSet")
{
    auto a = spawn();
    auto b = spawn();
    auto c = spawn();
    set<Entity> entitySet;
    
    REQUIRE( entitySet.insert(a).second );
    REQUIRE( entitySet.insert(b).second );
    REQUIRE( entitySet.insert(c).second );
}

TEST_CASE_METHOD(TestContext, "TestEntityInStdMap")
{
    auto a = spawn();
    auto b = spawn();
    auto c = spawn();
    map<Entity, int> entityMap;
    REQUIRE( entityMap.insert(pair<Entity, int>(a, 1)).second );
    REQUIRE( entityMap.insert(pair<Entity, int>(b, 2)).second );
    REQUIRE( entityMap.insert(pair<Entity, int>(c, 3)).second );
    REQUIRE( entityMap[a] == 1 );
    REQUIRE( entityMap[b] == 2 );
    REQUIRE( entityMap[c] == 3 );
}

TEST_CASE_METHOD(TestContext, "TestGetEntitiesWithComponent")
{
    auto e = spawn();
    auto f = spawn();
    auto g = spawn();

    add_component<Position>(e, 1, 2);
    add_component<Direction>(e);
    add_component<Position>(f, 3, 4);
    add_component<Direction>(g);

    REQUIRE( 2 == size(find_entities_with<Position>()) );
    REQUIRE( 2 == size(find_entities_with<Direction>()) );
    REQUIRE( 1 == size(find_entities_with<Position, Direction>()) );

    reset_world();

    for( auto i=0; i<150; i++ )
    {
        auto h = spawn();
        if( i % 2 == 0 ) add_component<Position>(h);
        if( i % 3 == 0 ) add_component<Direction>(h);
    }

    REQUIRE( 50 == size(find_entities_with<Direction>()) );
    REQUIRE( 75 == size(find_entities_with<Position>()) );
    REQUIRE( 25 == size(find_entities_with<Direction, Position>()) );
}

TEST_CASE_METHOD(TestContext, "TestGetComponentsAsTuple") {
    auto e = spawn();
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
    auto e = spawn();
    auto f = spawn();

    add_component<Position>(e, 1, 2);

    auto count = 0;
    find_entities_with<Position>().visit([&](Entity entity, Position& position)
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
    auto e = spawn();
    auto f = spawn();
    auto g = spawn();

    recycle(f);
    auto it = find_entities().begin();

    REQUIRE( *it == e );
    ++it;
    REQUIRE( *it == g );
    ++it;

    REQUIRE( it == find_entities().end() );
}

TEST_CASE_METHOD(TestContext, "TestComponentsRemovedFromReusedEntities")
{
  auto e = spawn();
  auto eversion = e.get_version();
  auto eindex = e.get_index();
  add_component<Position>(e, 1, 2);
  recycle(e);

  auto f = spawn();
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
    auto e = spawn();
    add_component<FreedSentinel>(e, freed);
    remove_component<FreedSentinel>(e);

    REQUIRE( freed );
}

TEST_CASE("TestComponentDestructorCalledWhenManagerDestroyed")
{
    bool freed = false;
    {
        TestContext context;
        auto e = spawn();
        add_component<FreedSentinel>(e, freed);
        REQUIRE( !freed );
    }
    REQUIRE( freed );
}

TEST_CASE_METHOD(TestContext, "TestComponentDestructorCalledWhenEntityDestroyed")
{
    bool freed = false;
    auto e = spawn();
    add_component<FreedSentinel>(e, freed);
    REQUIRE( !freed );
    recycle(e);
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
//     auto e = spawn();
//     add_component<Derived>(e);

//     REQUIRE( has_components<Base::Trait>(e) );

//     int c = 0;
//     auto bt = get_component<Base::Trait>(e);
//     (*bt)->add(c);

//     REQUIRE( c == 1 );
// }

struct Counter : public Component
{
    explicit Counter(int counter = 0) : counter(counter) {}
    int counter;
};

struct MovementSystem : public SubsystemWithEntities<Position, Direction>
{
    explicit MovementSystem(string label = "") : label(label) {}

    void update(float dt)
    {
        for( auto pair : *this )
        {
            std::get<0>(pair.second)->x += std::get<1>(pair.second)->x;
            std::get<0>(pair.second)->y += std::get<1>(pair.second)->y;
        }
    }

    std::string label;
};

struct CounterSystem : public SubsystemWithEntities<Counter>
{
    void update(float dt)
    {
        visit([&](Entity object, Counter& c)
        {
            c.counter ++;
        });
    }
};

TEST_CASE_METHOD(TestContext, "TestConstructSystemWithArgs")
{
    add_subsystem<MovementSystem>("movement");
    REQUIRE("movement" == get_subsystem<MovementSystem>()->label);
}

TEST_CASE_METHOD(TestContext, "TestApplySystem")
{

    std::vector<Entity> created_entities;
    for (int i = 0; i < 75; ++i)
    {
        auto e = spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) add_component<Position>(e, 1, 2);
        if (i % 3 == 0) add_component<Direction>(e, 1, 1);
        add_component<Counter>(e, 0);
    }

    add_subsystem<MovementSystem>();
    for (int i = 0; i < 75; ++i)
    {
        auto e = spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) add_component<Position>(e, 1, 2);
        if (i % 3 == 0) add_component<Direction>(e, 1, 1);
        add_component<Counter>(e, 0);
    }

    get_subsystem<MovementSystem>()->update(0.f);

    for (auto entity : created_entities)
    {
        auto position = get_component<Position>(entity);
        auto direction = get_component<Direction>(entity);

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

TEST_CASE_METHOD(TestContext, "TestApplyAllSystems")
{
    std::vector<Entity> created_entities;
    for (int i = 0; i < 150; ++i) {
        auto e = spawn();
        created_entities.push_back(e);
        if (i % 2 == 0) add_component<Position>(e, 1, 2);
        if (i % 3 == 0) add_component<Direction>(e, 1, 1);
        add_component<Counter>(e, 0);
    }

    add_subsystem<MovementSystem>();
    add_subsystem<CounterSystem>();

    get_subsystem<MovementSystem>()->update(0.f);
    get_subsystem<CounterSystem>()->update(0.f);

    for (auto entity : created_entities)
    {
        auto position   = get_component<Position>(entity);
        auto direction  = get_component<Direction>(entity);
        auto counter    = get_component<Counter>(entity);

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
