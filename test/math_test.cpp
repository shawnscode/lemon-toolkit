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

    REQUIRE( kVector2fZero == (Vector2f { 0.0f, 0.0f }) );
    REQUIRE( kVector2fOne == (Vector2f { 1.0f, 1.0f }) );
    REQUIRE( kVector2fLeft == (Vector2f { -1.0f, 0.0f }) );
    REQUIRE( kVector2fRight == (Vector2f { 1.0f, 0.0f }) );
    REQUIRE( kVector2fUp == (Vector2f { 0.0f, 1.0f }) );
    REQUIRE( kVector2fDown == (Vector2f { 0.0f, -1.0f }) );
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

    REQUIRE( dot(v1, v2) == Approx(32.f) );
    REQUIRE( v1.length_square() == Approx(dot(v1, v1)) );
    REQUIRE( v1.length() == Approx(5.f) );

    Vector2f v4 = normalize(v3);
    REQUIRE( v4[0] == Approx(0.707107f) );
    REQUIRE( v4[1] == Approx(0.707107f) );
    REQUIRE( v4.length() == Approx(1.f) );

    Vector2f v5 = { 0.f, 0.f };
    v5.normalize();
    REQUIRE( isnan(v5) );

    Vector2f v6 = { 1.0f, 2.0f };
    Vector2f v7 = { 1.1f, 2.0f };
    REQUIRE( !equals(v6, v7, 0.05f) );
    REQUIRE( equals(v6, v7, 0.11f) );

    REQUIRE( clamp( Vector2f{-3.f, 3.f}, kVector2fZero, kVector2fInfinity ) == (Vector2f{0.f, 3.f}) );
}

TEST_CASE("TestRectInitializerList")
{
    Rect2f r { 0.0f, 1.0f, 3.0f, 4.0f };
    REQUIRE( r[0][0] == Approx(0.f) );
    REQUIRE( r[0][1] == Approx(1.f) );
    REQUIRE( r[1][0] == Approx(3.f) );
    REQUIRE( r[1][1] == Approx(4.f) );

    Rect2f r2 { 2.0f, 1.0f };
    REQUIRE( r2[0][0] == Approx(2.f) );
    REQUIRE( r2[0][1] == Approx(1.f) );
    REQUIRE( r2[1][0] == Approx(0.f) );
    REQUIRE( r2[1][1] == Approx(0.f) );

    REQUIRE( kRect2fFull == (Rect2f { -1.f, -1.f, 2.f, 2.f }) );
    REQUIRE( kRect2fPositive == (Rect2f { 0.f, 0.f, 1.f, 1.f }) );
}

TEST_CASE("TestRectOperations")
{
    Rect2f r { 0.0f, 0.0f, 1.0f, 1.0f };

    REQUIRE( r.is_inside({ 0.f, 0.f }) );
    REQUIRE( r.is_inside({ 0.f, 0.9f }) );
    REQUIRE( r.is_inside({ 0.5f, 0.5f }) );
    REQUIRE( !r.is_inside({ 1.f, 0.f }) );
    REQUIRE( !r.is_inside({ 1.f, 1.f }) );
    REQUIRE( !r.is_inside({ 2.f, 0.5f }) );

    Rect2f r2 { 2.0f, 2.0f, 3.0f, 3.0f };
    Rect2f r3 = r + r2;
    REQUIRE( r3 == (Rect2f { 0.f, 0.f, 5.f, 5.f }) );

    r2 += r;
    REQUIRE( r3 == r2 );
    REQUIRE( (r + Rect2f { -1.f, 0.f, 2.f, 1.f }) == (Rect2f { -1.f, 0.f, 2.f, 1.f }) );

    Vector2f v { 4.f, -2.f };
    REQUIRE( (r + v) == (Rect2f { 0.f, -2.f, 4.f, 3.f }) );

    REQUIRE( clamp( Rect2f{0.f, 0.f, 5.f, 5.f}, Rect2f{1.f, 1.f, 1.f, 1.f} ) == (Rect2f{1.f, 1.f, 1.f, 1.f}) );
    REQUIRE( clamp( Rect2f{0.f, 0.f, 2.f, 2.f}, Rect2f{1.f, 1.f, 2.f, 2.f} ) == (Rect2f{1.f, 1.f, 1.f, 1.f}) );
}

TEST_CASE("TestColor")
{
    Color c { 1.0f, 0.0f, 0.0f };
    REQUIRE( c == Color::RED );
    REQUIRE( c.to_uint32() == 0xFF0000FF );

    Color c2 { 1.0f, 0.0f, 1.0f };
    REQUIRE( equals(lerp(c, c2, 0.5f), Color {1.f, 0.0f, 0.5f}) );

    Color c3 { 2.0f, 4.3f, 0.5f };
    REQUIRE( equals( c3.clip(), Color {1.f, 1.f, 0.5f} ) );
    REQUIRE( equals( c3.invert(true), Color {0.f, 0.f, 0.5f, 0.f} ) );
}

