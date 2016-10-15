#include <catch.hpp>
#include <lemon-toolkit.hpp>

USING_NS_LEMON_CORE;

struct Explosion
{
    Explosion(int damage) : damage(damage) {}
    int damage;
};

struct Collision
{
    Collision(int damage) : damage(damage) {}
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

struct EventTestContext
{
    EventSystem event;
};

TEST_CASE_METHOD(EventTestContext, "TestEmitReceive")
{
    ExplosionSystem explosion_system;

    event.subscribe<Explosion>(explosion_system);
    event.subscribe<Collision>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    event.emit<Explosion>(10);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);

    event.emit<Collision>(10);
    REQUIRE(20 == explosion_system.damage_received);
    REQUIRE(2 == explosion_system.received_count);
}

TEST_CASE_METHOD(EventTestContext, "TestUntypedEmitReceive")
{
    ExplosionSystem explosion_system;

    event.subscribe<Explosion>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    event.emit<Explosion>(10);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);
}

TEST_CASE_METHOD(EventTestContext, "TestUnsubscription") {
    ExplosionSystem explosion_system;

    event.subscribe<Explosion>(explosion_system);
    REQUIRE(explosion_system.damage_received == 0);

    event.emit<Explosion>(1);
    REQUIRE(explosion_system.damage_received == 1);

    event.unsubscribe<Explosion>(explosion_system);
    event.emit<Explosion>(1);
    REQUIRE(explosion_system.damage_received == 1);
}
