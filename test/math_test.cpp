#include <catch.hpp>
#include <flow2d.hpp>

USING_NS_FLOW2D;

TEST_CASE("TestVectorInitializerList")
{
    Vector2f v { 1.0f, 2.0f };
    REQUIRE( v[0] == Approx(1.0f) );
    REQUIRE( v[1] == Approx(2.0f) );

    v = Vector2f { 3.0f, 4.0f, 1.0f, 2.0f };
    REQUIRE( v[0] == Approx(3.0f) );
    REQUIRE( v[1] == Approx(4.0f) );

    Vector2f v2 { };
    Vector2f v3 { 1.0f };

    REQUIRE( Vector2f::ZERO == (Vector2f { 0.0f, 0.0f }) );
    REQUIRE( Vector2f::ONE == (Vector2f { 1.0f, 1.0f }) );
    REQUIRE( Vector2f::LEFT == (Vector2f { -1.0f, 0.0f }) );
    REQUIRE( Vector2f::RIGHT == (Vector2f { 1.0f, 0.0f }) );
    REQUIRE( Vector2f::UP == (Vector2f { 0.0f, 1.0f }) );
    REQUIRE( Vector2f::DOWN == (Vector2f { 0.0f, -1.0f }) );
}

TEST_CASE("TestVectorComparisions")
{
    Vector2f v1 = { 1.0f, 1.0f };
    Vector2f v2 = { 2.0f, 1.0f };
    Vector2f v3 = { 1.0f, 2.0f };
    Vector2f v4 = { 1.0f, 1.0f };

    REQUIRE( v1 == v4 );
    REQUIRE( v1 != v2 );
    REQUIRE( v1 < v2 );
    REQUIRE( v1 <= v2 );
    REQUIRE( v1 <= v4 );
    REQUIRE( v1 < v3 );
    REQUIRE( v1 <= v3 );
    REQUIRE( v2 > v1 );
    REQUIRE( v2 >= v1 );
    REQUIRE( v3 >= v1 );
    REQUIRE( v2 > v3 );
    REQUIRE( v3 < v2 );
}

TEST_CASE("TestVectorOtherOperations")
{
    Vector2f v1 = { 1.0f, 1.0f };
    Vector2f v2 = { 2.0f, 3.0f };

    REQUIRE( v1[0] == Approx(1.0f) );
    REQUIRE( v1[1] == Approx(1.0f) );

    REQUIRE( (v1 - v2) == (Vector2f { -1.0f, -2.0f }) );
    REQUIRE( (v1 + v2) == (Vector2f { 3.0f, 4.0f }) );
    REQUIRE( (v2 * 2.0f) == (Vector2f { 4.0f, 6.0f }) );
    REQUIRE( (v2 / 2.0f) == (Vector2f { 1.0f, 1.5f }) );
}

TEST_CASE("TestVectorMethods")
{
    Vector2f v1 = { 3.0f, 4.0f };
    Vector2f v2 = { 4.0f, 5.0f };
    Vector2f v3 = { 3.0f, 3.0f };

    REQUIRE( v1.dot(v2) == Approx(32.f) );
    REQUIRE( v1.length_square() == Approx(v1.dot(v1)) );
    REQUIRE( v1.length() == Approx(5.f) );

    Vector2f v4 = v3.normalize();
    REQUIRE( v4[0] == Approx(0.707107f) );
    REQUIRE( v4[1] == Approx(0.707107f) );
    REQUIRE( v4.length() == Approx(1.f) );

    Vector2f v5 = (Vector2f { 0.0f, 0.0f }).normalize();
    REQUIRE( v5.isnan() );

    Vector2f v6 = { 1.0f, 2.0f };
    Vector2f v7 = { 1.1f, 2.0f };
    REQUIRE( !v6.equals(v7, 0.05f) );
    REQUIRE( v6.equals(v7, 0.11f) );
}
