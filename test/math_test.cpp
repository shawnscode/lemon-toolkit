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

    REQUIRE( dot(v1, v2) == Approx(32.f) );
    REQUIRE( length_square(v1) == Approx(dot(v1, v1)) );
    REQUIRE( length(v1) == Approx(5.f) );

    REQUIRE( normalize(v1) == Approx(5.f) );
    REQUIRE( length(v1) == Approx(1.f) );
}
