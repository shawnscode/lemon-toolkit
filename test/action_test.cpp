#include <catch.hpp>
#include <flow2d.hpp>

USING_NS_FLOW2D;

struct ActionFixture
{
    ActionFixture() : _world(), _manager(_world) {}
    EntityManager   _world;
    SystemManager   _manager;
};

TEST_CASE_METHOD(ActionFixture, "TestActionTransform")
{
    auto as = _manager.add<ActionSystem>();

    auto object = _world.spawn();
    auto transform = _world.add_component<Transform>(object);
    auto executor = _world.add_component<ActionExecutor>(object);

    //auto env = executor->get_environment();
    executor->run( ActionMoveTo::spawn(2.0f, Vector2f {5.0f, 5.0f}) );
    REQUIRE( equals(transform->get_position(), {0.f, 0.f}) );
    REQUIRE( !executor->is_finished() );

    _manager.update(1.f);
    REQUIRE( equals(transform->get_position(), {2.5f, 2.5f}) );
    REQUIRE( !executor->is_finished() );

    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {5.f, 5.f}) );
    REQUIRE( executor->is_finished() );

    executor->run( ActionMoveBy::spawn(1.0f, Vector2f{ -1.0f, -1.0f}) );
    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {4.f, 4.f}) );
}

TEST_CASE_METHOD(ActionFixture, "TestActionReuse")
{
    _manager.add<ActionSystem>();

    for( auto iteration = 0; iteration < 32; iteration++ )
    {
        std::vector<Action*> alives;
        auto factor = std::rand() % 5 + 1;
        for( auto c = 0; c < 64; c++ )
        {
            auto action = ActionMoveTo::spawn(2.0f, Vector2f {5.0f, 5.0f});

            if( c % factor != 0 ) delete action;
            else alives.push_back(action);
        }

        REQUIRE( Action::get_size_of<ActionMoveTo>() == alives.size() );

        for( auto c = 0; c < alives.size(); c++ )
            delete alives[c];

        REQUIRE( Action::get_size_of<ActionMoveTo>() == 0 );
    }
}

TEST_CASE_METHOD(ActionFixture, "TestActionCompositor")
{
    auto as = _manager.add<ActionSystem>();

    auto object = _world.spawn();
    auto transform = _world.add_component<Transform>(object);
    auto executor = _world.add_component<ActionExecutor>(object);

    executor->run( ActionSequence::spawn(
        ActionMoveTo::spawn(1.0f, Vector2f{2.0f, 2.0f}),
        ActionMoveTo::spawn(1.0f, Vector2f{3.0f, 3.0f})
    ) );

    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {2.f, 2.f}) );
    REQUIRE( !executor->is_finished() );

    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {3.f, 3.f}) );
    REQUIRE( executor->is_finished() );

    executor->run( ActionRepeat::spawn(ActionMoveBy::spawn(1.0f, Vector2f{2.0f, 2.0f}), 2) );
    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {5.f, 5.f}) );
    REQUIRE( !executor->is_finished() );

    _manager.update(1.0f);
    REQUIRE( equals(transform->get_position(), {7.f, 7.f}) );
    REQUIRE( executor->is_finished() );
}