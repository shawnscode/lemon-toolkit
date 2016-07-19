#include <catch.hpp>
#include <flow2d.hpp>

USING_NS_FLOW2D;

struct TransformFixture
{
    EntityManager   _world;
    TransformFixture() {}
};

TEST_CASE_METHOD(TransformFixture, "TestConstructor")
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

TEST_CASE_METHOD(TransformFixture, "TestOpsWithoutHierachy")
{
    Entity e1 = _world.spawn();
    Transform* t1 = _world.add_component<Transform>(e1, Vector2f{10.f, 10.f});

    t1->set_position({20.f, 20.f});
    REQUIRE( t1->get_position() == Vector2f({20.f, 20.f}) );
    REQUIRE( t1->get_position() == t1->get_position(TransformSpace::WORLD) );

    t1->set_position({30.f, 30.f}, TransformSpace::WORLD);
    REQUIRE( t1->get_position() == Vector2f({30.f, 30.f}) );
    REQUIRE( t1->get_position() == t1->get_position(TransformSpace::WORLD) );

    t1->set_scale({2.f, 2.f});
    REQUIRE( t1->get_scale() == Vector2f({2.f, 2.f}) );
    REQUIRE( t1->get_scale() == t1->get_scale(TransformSpace::WORLD) );

    t1->set_scale({3.f, 3.f}, TransformSpace::WORLD);
    REQUIRE( t1->get_scale() == Vector2f({3.f, 3.f}) );
    REQUIRE( t1->get_scale() == t1->get_scale(TransformSpace::WORLD) );

    t1->set_rotation(45.f);
    REQUIRE( t1->get_rotation() == 45.f);
    REQUIRE( t1->get_rotation() == t1->get_rotation(TransformSpace::WORLD) );

    t1->set_rotation(60.f, TransformSpace::WORLD);
    REQUIRE( t1->get_rotation() == Approx(60.f));
    REQUIRE( t1->get_rotation() == Approx(t1->get_rotation(TransformSpace::WORLD)) );

    t1->set_rotation(480.f, Transform::WORLD);
    REQUIRE( t1->get_rotation() == Approx(120.f));
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
    REQUIRE( t1->get_children().count() == 1 );
    REQUIRE( t2->get_children().count() == 0 );
    REQUIRE( t2->get_parent() == t1 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t2->get_position(), {20.f, 20.f}) );
    REQUIRE( equals(t2->get_position(TransformSpace::WORLD), {30.f, 30.f}) );

    // keep world pose of t3
    t1->append_child(*t3, true);
    REQUIRE( t1->get_children().count() == 2 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t3->get_position(), {30.f, 30.f}) );
    REQUIRE( t3->get_parent() == t1 );
    REQUIRE( equals(t3->get_position(TransformSpace::WORLD), {40.f, 40.f}) );

    // nested hierachies
    t3->append_child(*t4);
    REQUIRE( t1->get_children().count() == 2 );
    REQUIRE( t1->get_children(true).count() == 3 );
    REQUIRE( t3->get_children(true).count() == 1 );
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

    REQUIRE( t1->get_children().count() == 2 );
    REQUIRE( t1->get_children(true).count() == 3 );

    t3->remove_from_parent();

    REQUIRE( t1->is_root() );
    REQUIRE( !t2->is_root() );
    REQUIRE( t3->is_root() );
    REQUIRE( !t4->is_root() );

    REQUIRE( t1->get_children().count() == 1 );
    REQUIRE( t1->get_children(true).count() == 1 );
}

static void dump_heriachy(Transform* t, int index = 1, int level = 0)
{
    if( t->get_children().count() == 0 )
        return;

    for( size_t i = 0; i < level; i++ )
        printf("\t");
    printf("[%d]", index);
    for( auto& transform : t->get_children() )
        printf("%d ", transform.get_object().get_index() );
    printf("\n");

    size_t count = 0;
    for( auto& transform : t->get_children() )
        dump_heriachy(&transform, ++count, level+1);
}

TEST_CASE_METHOD(TransformFixture, "TestIteration")
{
    std::vector<Transform*> transforms;
    for( size_t i = 0; i < 255; i++ )
    {
        auto e = _world.spawn_with<Transform>();
        transforms.push_back(_world.get_component<Transform>(e));
    }

    std::srand(std::time(0));
    std::vector<Transform*> constructed;
    constructed.push_back(transforms.back());
    transforms.pop_back();

    size_t count = 0;
    for( size_t i = 0; i < 255-1; i++ )
    {
        size_t index = std::rand() % transforms.size();
        size_t pindex = std::rand() % constructed.size();

        if( pindex == 0 ) count ++;
        constructed[pindex]->append_child(*transforms[index]);

        // if( constructed[0]->get_children(true).count() != i+ 1)
        //     dump_heriachy(constructed[0]);
        REQUIRE( constructed[0]->get_children(true).count() == i+1 );

        constructed.push_back(transforms[index]);
        transforms.erase(transforms.begin()+index);
    }

    size_t direct = 0;
    constructed[0]->get_children().visit([&](Transform& t)
    {
        direct ++;
    });

    REQUIRE( direct == count );

    direct = 0;
    for( auto& transform : constructed[0]->get_children() )
        direct ++;

    REQUIRE( direct == count );

    size_t total = 0;
    for( auto& transform : constructed[0]->get_children(true) )
        total ++;

    REQUIRE( total == 254 );
}