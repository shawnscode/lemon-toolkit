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

    REQUIRE( equals(scene::get_position(_world, *t2), {20.f, 20.f}) );
    REQUIRE( equals(scene::get_scale(_world, *t1), scene::get_scale(_world, *t2)) );
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

    scene::append_child(_world, *t1, *t2);
    REQUIRE( scene::get_children_count(_world, *t1) == 1 );
    REQUIRE( scene::get_children_count(_world, *t2) == 0 );
    REQUIRE( scene::get_parent(_world, *t2) == e1 );
    REQUIRE( equals(scene::get_position(_world, *t1), {10.f, 10.f}) );
    REQUIRE( equals(scene::get_position(_world, *t2), {20.f, 20.f}) );
    REQUIRE( equals(scene::get_position(_world, *t2, TransformSpace::WORLD), {30.f, 30.f}) );

    // keep world pose of t3
    scene::append_child(_world, *t1, *t3, true);
    REQUIRE( scene::get_children_count(_world, *t1) == 2 );
    REQUIRE( equals(scene::get_position(_world, *t1), {10.f, 10.f}) );
    REQUIRE( equals(scene::get_position(_world, *t3), {30.f, 30.f}) );
    REQUIRE( scene::get_parent(_world, *t3) == e1 );
    REQUIRE( equals(scene::get_position(_world, *t3, TransformSpace::WORLD), {40.f, 40.f}) );

    // nested hierachies
    scene::append_child(_world, *t3, *t4);
    REQUIRE( scene::get_children_count(_world, *t1) == 2 );
    REQUIRE( scene::get_children_count(_world, *t1, true) == 3 );
    REQUIRE( scene::get_children_count(_world, *t3, true) == 1 );
    REQUIRE( scene::get_parent(_world, *t4) == e3 );
    REQUIRE( equals(scene::get_position(_world, *t4, TransformSpace::WORLD), {-10.f, 30.f}) );
    REQUIRE( equals(scene::get_scale(_world, *t4, TransformSpace::WORLD), {6.f, 6.f}) );
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

    scene::append_child(_world, *t1, *t2);
    scene::append_child(_world, *t1, *t3);
    scene::append_child(_world, *t3, *t4);

    REQUIRE( scene::is_root(_world, *t1) );
    REQUIRE( !scene::is_root(_world, *t2) );
    REQUIRE( !scene::is_root(_world, *t3) );
    REQUIRE( !scene::is_root(_world, *t4) );

    REQUIRE( !scene::is_leaf(_world, *t1) );
    REQUIRE( scene::is_leaf(_world, *t2) );
    REQUIRE( !scene::is_leaf(_world, *t3) );
    REQUIRE( scene::is_leaf(_world, *t4) );

    REQUIRE( scene::get_children_count(_world, *t1) == 2 );
    REQUIRE( scene::get_children_count(_world, *t1, true) == 3 );

    scene::remove_from_parent(_world, *t3);

    REQUIRE( scene::is_root(_world, *t1) );
    REQUIRE( !scene::is_root(_world, *t2) );
    REQUIRE( scene::is_root(_world, *t3) );
    REQUIRE( !scene::is_root(_world, *t4) );

    REQUIRE( scene::get_children_count(_world, *t1) == 1 );
    REQUIRE( scene::get_children_count(_world, *t1, true) == 1 );
}