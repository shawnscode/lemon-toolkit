#include <catch.hpp>
#include <lemon-toolkit.hpp>

USING_NS_LEMON_CORE;

struct Explosion
{
    explicit Explosion(int damage) : damage(damage) {}
    int damage;
};

struct Collision
{
    explicit Collision(int damage) : damage(damage) {}
    int damage;
};

struct ExplosionSystem
{
    void receive(const Explosion& explosion)
    {
        damage_received += explosion.damage;
        received_count ++;
    }

    void receive(const Collision& collision)
    {
        damage_received += collision.damage;
        received_count ++;
    }

    int received_count = 0;
    int damage_received = 0;
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

TEST_CASE_METHOD(TestContext, "TestEmitReceive")
{
    ExplosionSystem explosion_system;

    subscribe<Explosion>(explosion_system);
    subscribe<Collision>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    emit<Explosion>(10);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);

    emit<Collision>(10);
    REQUIRE(20 == explosion_system.damage_received);
    REQUIRE(2 == explosion_system.received_count);
}


TEST_CASE_METHOD(TestContext, "TestUntypedEmitReceive")
{
    ExplosionSystem explosion_system;

    subscribe<Explosion>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    Explosion explosion(10);
    emit(explosion);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);
}

TEST_CASE_METHOD(TestContext, "TestUnsubscription") {
    ExplosionSystem explosion_system;

    subscribe<Explosion>(explosion_system);
    REQUIRE(explosion_system.damage_received == 0);

    emit<Explosion>(1);
    REQUIRE(explosion_system.damage_received == 1);

    unsubscribe<Explosion>(explosion_system);
    emit<Explosion>(1);
    REQUIRE(explosion_system.damage_received == 1);
}
