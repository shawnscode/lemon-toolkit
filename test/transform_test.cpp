#include <catch.hpp>
#include <lemon-toolkit.hpp>

USING_NS_LEMON;
USING_NS_LEMON_CORE;
USING_NS_LEMON_MATH;

struct TransformFixture
{
    template<typename ... Args> Transform* create_with(Args&& ... args)
    {
        auto handle = ecs.create();
        auto object = ecs.get(handle);
        return object->add_component<Transform>(*object, std::forward<Args>(args)...);
    }

    core::EntityComponentSystem ecs;
};

static void dump_heriachy(Transform* t, int index = 1, int level = 0)
{
    if( t->find_children().count() == 0 )
        return;
    
    for( size_t i = 0; i < level; i++ )
        printf("\t");
    printf("[%d]", index);
    for( auto transform : t->find_children() )
        printf("%d ", transform->entity.handle.get_index() );
    printf("\n");
    
    size_t count = 0;
    for( auto transform : t->find_children() )
        dump_heriachy(transform, ++count, level+1);
}

TEST_CASE_METHOD(TransformFixture, "TestConstructor")
{
    auto t1 = create_with(Vector3f{10.f, 10.f});
    auto t2 = create_with(Vector3f{20.f, 20.f});
    auto t3 = create_with(Vector3f{40.f, 40.f}, Vector3f{2.0f, 2.0f});
    auto t4 = create_with(Vector3f{50.f, 50.f}, Vector3f{2.0f, 2.0f}, Quaternion());

    REQUIRE( equals(t2->get_position(), {20.f, 20.f}) );
    REQUIRE( equals(t1->get_scale(), t2->get_scale()) );
}

TEST_CASE_METHOD(TransformFixture, "TestTransform")
{
    auto t1 = create_with(Vector3f{10.f, 10.f, 0.f});
    REQUIRE( t1->transform_point({0.f, 5.f, 0.f}) == (Vector3f{10.f, 15.f, 0.f}) );
    REQUIRE( t1->inverse_transform_point({0.f, 5.f, 0.f}) == (Vector3f{-10.f, -5.f, 0.f}) );
    REQUIRE( t1->transform_vector({0.f, 5.f, 0.f}) == (Vector3f{0.f, 5.f, 0.f}) );
    REQUIRE( t1->inverse_transform_vector({0.f, 5.f, 0.f}) == (Vector3f{0.f, 5.f, 0.f}) );
    REQUIRE( t1->transform_direction({0.f, 5.f, 0.f}) == (Vector3f{0.f, 5.f, 0.f}) );
    REQUIRE( t1->inverse_transform_direction({0.f, 5.f, 0.f}) == (Vector3f{0.f, 5.f, 0.f}) );

    t1->set_position({0.f, 0.f, 0.f});
    t1->set_scale({2.0f, 0.5f, 1.0f});
    REQUIRE( t1->transform_point({10.f, 10.f, 10.f}) == (Vector3f{20.f, 5.f, 10.f}) );
    REQUIRE( t1->inverse_transform_point({10.f, 10.f, 10.f}) == (Vector3f{5.f, 20.f, 10.f}) );
    REQUIRE( t1->transform_vector({10.f, 10.f, 10.f}) == (Vector3f{20.f, 5.f, 10.f}) );
    REQUIRE( t1->inverse_transform_vector({10.f, 10.f, 10.f}) == (Vector3f{5.f, 20.f, 10.f}) );
    REQUIRE( t1->transform_direction({10.f, 10.f, 10.f}) == (Vector3f{10.f, 10.f, 10.f}) );
    REQUIRE( t1->inverse_transform_direction({10.f, 10.f, 10.f}) == (Vector3f{10.f, 10.f, 10.f}) );

    t1->set_position({10.f, 5.f, 0.f});
    REQUIRE( t1->transform_point({10.f, 10.f, 10.f}) == (Vector3f{30.f, 10.f, 10.f}) );
    REQUIRE( t1->inverse_transform_point({10.f, 10.f, 10.f}) == (Vector3f{0.f, 10.f, 10.f}) );
    REQUIRE( t1->transform_vector({10.f, 10.f, 10.f}) == (Vector3f{20.f, 5.f, 10.f}) );
    REQUIRE( t1->inverse_transform_vector({10.f, 10.f, 10.f}) == (Vector3f{5.f, 20.f, 10.f}) );
    REQUIRE( t1->transform_direction({10.f, 10.f, 10.f}) == (Vector3f{10.f, 10.f, 10.f}) );
    REQUIRE( t1->inverse_transform_direction({10.f, 10.f, 10.f}) == (Vector3f{10.f, 10.f, 10.f}) );

    t1->set_position({0.f, 0.f, 0.f});
    t1->set_scale({1.f, 1.f, 1.f});
    t1->set_rotation(from_euler_angles({0.f, 90.f, 0.f}));
    REQUIRE( equals(t1->transform_point({0.f, 0.f, 1.f}), {1.f, 0.f, 0.f}) );
    REQUIRE( equals(t1->inverse_transform_point({1.f, 0.f, 0.f}), {0.f, 0.f, 1.f}, 1e-5f) );

    t1->set_scale({2.f, 1.f, 1.f});
    REQUIRE( equals(t1->transform_point({0.f, 0.f, 1.f}), {2.f, 0.f, 0.f}, 1e-5f) );
    REQUIRE( equals(t1->inverse_transform_point({1.f, 0.f, 0.f}), {0.f, 0.f, 0.5f}, 1e-5f) );
    REQUIRE( equals(t1->transform_direction({0.f, 0.f, 1.f}), {1.f, 0.f, 0.f}, 1e-5f));
    REQUIRE( equals(t1->inverse_transform_direction({1.f, 0.f, 0.f}), {0.f, 0.f, 1.f}, 1e-5f));

    t1->set_position({10.f, 0.f, 0.f});
    REQUIRE( equals(t1->transform_point({0.f, 0.f, 1.f}), {12.f, 0.f, 0.f}, 1e-5f) );
    REQUIRE( equals(t1->inverse_transform_point({1.f, 0.f, 0.f}), {0.f, 0.f, -4.5f}, 1e-5f) );
    REQUIRE( equals(t1->transform_direction({0.f, 0.f, 1.f}), {1.f, 0.f, 0.f}, 1e-5f));
    REQUIRE( equals(t1->inverse_transform_direction({1.f, 0.f, 0.f}), {0.f, 0.f, 1.f}, 1e-5f));
}

TEST_CASE_METHOD(TransformFixture, "TestOpsWithoutHierachy")
{
    auto t1 = create_with(Vector3f{10.f, 10.f});
    t1->set_position({20.f, 20.f});
    REQUIRE( t1->get_position() == Vector3f({20.f, 20.f}) );
    REQUIRE( t1->get_position() == t1->get_position(TransformSpace::WORLD) );

    t1->set_position({30.f, 30.f}, TransformSpace::WORLD);
    REQUIRE( t1->get_position() == Vector3f({30.f, 30.f}) );
    REQUIRE( t1->get_position() == t1->get_position(TransformSpace::WORLD) );

    t1->set_scale({2.f, 2.f});
    REQUIRE( t1->get_scale() == Vector3f({2.f, 2.f}) );
    REQUIRE( t1->get_scale() == t1->get_scale(TransformSpace::WORLD) );

    t1->set_scale({3.f, 3.f}, TransformSpace::WORLD);
    REQUIRE( t1->get_scale() == Vector3f({3.f, 3.f}) );
    REQUIRE( t1->get_scale() == t1->get_scale(TransformSpace::WORLD) );

    t1->set_rotation({60.f, 45.f});
    auto rotation = to_euler_angles(t1->get_rotation());
    REQUIRE( rotation[0] == Approx(60.f) );
    REQUIRE( rotation[1] == Approx(45.f) );
    REQUIRE( t1->get_rotation() == t1->get_rotation(TransformSpace::WORLD) );
}

TEST_CASE_METHOD(TransformFixture, "TestHierachy")
{
    auto t1 = create_with(Vector3f{10.f, 10.f});
    auto t2 = create_with(Vector3f{20.f, 20.f});
    auto t3 = create_with(Vector3f{40.f, 40.f}, Vector3f{2.0f, 2.0f});
    auto t4 = create_with(Vector3f{-50.f, -10.f}, Vector3f{3.0f, 3.0f});

    t1->append_child(*t2);
    auto iter = t1->find_children();
    auto begin = iter.begin();

    REQUIRE( t1->find_children().count() == 1 );
    REQUIRE( t2->find_children().count() == 0 );
    REQUIRE( t2->get_parent() == t1 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t2->get_position(), {20.f, 20.f}) );
    REQUIRE( equals(t2->get_position(TransformSpace::WORLD), {30.f, 30.f}) );

    // keep world pose of t3
    t1->append_child(*t3, true);
    REQUIRE( t1->find_children().count() == 2 );
    REQUIRE( equals(t1->get_position(), {10.f, 10.f}) );
    REQUIRE( equals(t3->get_position(), {30.f, 30.f}) );
    REQUIRE( t3->get_parent() == t1 );
    REQUIRE( equals(t3->get_position(TransformSpace::WORLD), {40.f, 40.f}) );

    // nested hierachies
    t3->append_child(*t4);
    REQUIRE( t1->find_children().count() == 2 );
    REQUIRE( t1->find_children(true).count() == 3 );
    REQUIRE( t3->find_children(true).count() == 1 );
    REQUIRE( t4->get_parent() == t3 );
    REQUIRE( equals(t4->get_position(TransformSpace::WORLD), {-10.f, 30.f}) );
    REQUIRE( equals(t4->get_scale(TransformSpace::WORLD), {6.f, 6.f}) );
}

TEST_CASE_METHOD(TransformFixture, "TestHierachyReconstructWhenDispose")
{
    auto t1 = create_with(Vector3f{10.f, 10.f});
    auto t2 = create_with(Vector3f{10.f, 10.f});
    auto t3 = create_with(Vector3f{10.f, 10.f});
    auto t4 = create_with(Vector3f{10.f, 10.f});

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

    REQUIRE( t1->find_children().count() == 2 );
    REQUIRE( t1->find_children(true).count() == 3 );

    t3->remove_from_parent();

    REQUIRE( t1->is_root() );
    REQUIRE( !t2->is_root() );
    REQUIRE( t3->is_root() );
    REQUIRE( !t4->is_root() );

    REQUIRE( t1->find_children().count() == 1 );
    REQUIRE( t1->find_children(true).count() == 1 );
}

TEST_CASE_METHOD(TransformFixture, "TestIteration")
{
    std::vector<Transform*> transforms;
    for( size_t i = 0; i < 255; i++ )
        transforms.push_back(create_with());

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

        if( (constructed[0]->find_children(true).count() != i+ 1) || (constructed[0]->find_children().count() != count) )
            dump_heriachy(constructed[0]);

        REQUIRE( constructed[0]->find_children(true).count() == i+1 );

        constructed.push_back(transforms[index]);
        transforms.erase(transforms.begin()+index);
    }

    size_t direct = 0;
    constructed[0]->find_children().visit([&](Transform& t)
    {
        direct ++;
    });

    REQUIRE( direct == count );

    direct = 0;
    for( auto transform : constructed[0]->find_children() )
        direct ++;

    REQUIRE( direct == count );

    size_t total = 0;
    for( auto transform : constructed[0]->find_children(true) )
        total ++;

    REQUIRE( total == 254 );
}

struct Widget : Component
{};

TEST_CASE_METHOD(TransformFixture, "TestGenericIteration")
{
    std::vector<Transform*> transforms;
    std::vector<Widget*> widgets;

    auto root = create_with();

    for( size_t i = 0; i < 3; i++ )
    {
        auto t = create_with();
        transforms.push_back(t);
        root->append_child(*transforms.back());

        if( (i % 2) == 1 )
            widgets.push_back( t->entity.add_component<Widget>() );
    }

    REQUIRE( transforms.size() == root->find_children_with<Transform>(true).count() );
    REQUIRE( widgets.size() == root->find_children_with<Widget>(true).count() );
}
