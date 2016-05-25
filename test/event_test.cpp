#include "catch.hpp"
#include "flow2d.hpp"
#include "event.hpp"

#include <string>
#include <vector>

USING_NS_FLOW2D;

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

TEST_CASE("TestEmitReceive")
{
    EventManager em;
    ExplosionSystem explosion_system;

    em.subscribe<Explosion>(explosion_system);
    em.subscribe<Collision>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    em.emit<Explosion>(10);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);

    em.emit<Collision>(10);
    REQUIRE(20 == explosion_system.damage_received);
    REQUIRE(2 == explosion_system.received_count);
}


TEST_CASE("TestUntypedEmitReceive")
{
    EventManager em;
    ExplosionSystem explosion_system;

    em.subscribe<Explosion>(explosion_system);
    REQUIRE(0 == explosion_system.damage_received);

    Explosion explosion(10);
    em.emit(explosion);
    REQUIRE(1 == explosion_system.received_count);
    REQUIRE(10 == explosion_system.damage_received);
}

TEST_CASE("TestUnsubscription") {
    ExplosionSystem explosion_system;
    {
        EventManager em;

        em.subscribe<Explosion>(explosion_system);
        REQUIRE(explosion_system.damage_received == 0);

        em.emit<Explosion>(1);
        REQUIRE(explosion_system.damage_received == 1);

        em.unsubscribe<Explosion>(explosion_system);
        em.emit<Explosion>(1);
        REQUIRE(explosion_system.damage_received == 1);
    }
}
