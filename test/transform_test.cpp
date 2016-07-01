#include <catch.hpp>
#include <flow2d.hpp>

USING_NS_FLOW2D;

struct TransformFixture
{
    EntityManager   _world;
    EventManager    _dispatcher;

    TransformFixture() : _world(_dispatcher) {}
};

TEST_CASE_METHOD(TransformFixture, "TestTransformConstructor")
{
    Entity e1 = _world.spawn();
    Transform* t1 = _world.add_component<Transform>(e1, Vector2f{10.f, 10.f});

    Entity e2 = _world.spawn();
    Transform* t2 = _world.add_component<Transform>(e2, Vector2f{20.f, 20.f});

    Entity e3 = _world.spawn();
    Transform* t3 = _world.add_component<Transform>(e3, Vector2f{40.f, 40.f}, Vector2f{2.0f, 2.0f});

    Entity e4 = _world.spawn();
    Transform* t4 = _world.add_component<Transform>(e4, Vector2f{50.f, 50.f}, Vector2f{2.0f, 2.0f}, 90.f);

    REQUIRE( equals(t2->get_position(), {20.f, 20.f}) );
    REQUIRE( equals(t1->get_scale(), t2->get_scale()) );
}

TEST_CASE_METHOD(TransformFixture, "TestHierachy")
{
    Entity e1 = _world.spawn();
    Transform* t1 = _world.add_component<Transform>(e1, Vector2f{10.f, 10.f});

    Entity e2 = _world.spawn();
    Transform* t2 = _world.add_component<Transform>(e2, Vector2f{20.f, 20.f});

    Entity e3 = _world.spawn();
    Transform* t3 = _world.add_component<Transform>(e3, Vector2f{40.f, 40.f}, Vector2f{2.0f, 2.0f});

    Entity e4 = _world.spawn();
    Transform* t4 = _world.add_component<Transform>(e4, Vector2f{-50.f, -10.f}, Vector2f{3.0f, 3.0f});

    t1->append_child(*t2);
    REQUIRE( t1->get_children_count() == 1 );
    REQUIRE( t2->get_children_count() == 0 );
    REQUIRE( t2->get_parent() == t1 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t2->get_position(), {20.f, 20.f}) );
    REQUIRE( equals(t2->get_position(TransformSpace::WORLD), {30.f, 30.f}) );

    // keep world pose of t3
    t1->append_child(*t3, true);
    REQUIRE( t1->get_children_count() == 2 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t3->get_position(), {30.f, 30.f}) );
    REQUIRE( t3->get_parent() == t1 );
    REQUIRE( equals(t3->get_position(TransformSpace::WORLD), {40.f, 40.f}) );

    // nested hierachies
    t3->append_child(*t4);
    REQUIRE( t1->get_children_count() == 2 );
    REQUIRE( t1->get_children_count(true) == 3 );
    REQUIRE( t3->get_children_count(true) == 1 );
    REQUIRE( t4->get_parent() == t3 );
    REQUIRE( equals(t4->get_position(TransformSpace::WORLD), {-10.f, 30.f}) );
    REQUIRE( equals(t4->get_scale(TransformSpace::WORLD), {6.f, 6.f}) );
}

TEST_CASE_METHOD(TransformFixture, "TestHierachyReconstructWhenDispose")
{
    Entity e1 = _world.spawn_with<Transform>(Vector2f{10.f, 10.f});
    Transform* t1 = _world.get_component<Transform>(e1);

    Entity e2 = _world.spawn_with<Transform>(Vector2f{10.f, 10.f});
    Transform* t2 = _world.get_component<Transform>(e2);

    Entity e3 = _world.spawn_with<Transform>(Vector2f{10.f, 10.f});
    Transform* t3 = _world.get_component<Transform>(e3);

    Entity e4 = _world.spawn_with<Transform>(Vector2f{10.f, 10.f});
    Transform* t4 = _world.get_component<Transform>(e4);

    t1->append_child(*t2);
    t1->append_child(*t3);
    t3->append_child(*t4);

    REQUIRE( t1->is_root() );
    REQUIRE( !t2->is_root() );
    REQUIRE( !t3->is_root() );
    REQUIRE( !t4->is_root() );

    REQUIRE( !t1->is_leaf() );
    REQUIRE( t2->is_leaf() );
    REQUIRE( !t3->is_leaf() );
    REQUIRE( t4->is_leaf() );

    REQUIRE( t1->get_children_count() == 2 );
    REQUIRE( t1->get_children_count(true) == 3 );

    t3->remove_from_parent();

    REQUIRE( t1->is_root() );
    REQUIRE( !t2->is_root() );
    REQUIRE( t3->is_root() );
    REQUIRE( !t4->is_root() );

    REQUIRE( t1->get_children_count() == 1 );
    REQUIRE( t1->get_children_count(true) == 1 );
}