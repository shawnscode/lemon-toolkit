#include <catch.hpp>
#include <flow2d.hpp>

USING_NS_FLOW2D_MATH;

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

    REQUIRE( (v1 * v2 ) == (Vector2f { 2.0f, 3.0f }) );
    REQUIRE( (v1 / v2 ) == (Vector2f { 0.5f, 1.f/3.f }) );

    REQUIRE( (v1 += v2) == (Vector2f{3.0f, 4.0f}) );
    REQUIRE( (v2 -= v1) == (Vector2f{-1.0f, -1.0f}) );
}

TEST_CASE("TestVectorMethods")
{
    Vector2f v1 = { 3.0f, 4.0f };
    Vector2f v2 = { 4.0f, 5.0f };
    Vector2f v3 = { 3.0f, 3.0f };
    REQUIRE( v1.length_square() == Approx(dot(v1, v1)) );
    REQUIRE( v1.length() == Approx(5.f) );

    // dot
    REQUIRE( dot(v1, v2) == Approx(32.f) );

    // compare
    Vector2f v4 = { 1.0f, 2.0f };
    Vector2f v5 = { 1.1f, 2.0f };
    REQUIRE( !equals(v4, v5, 0.05f) );
    REQUIRE( equals(v4, v5, 0.11f) );

    REQUIRE( min(v1, v2) == Vector2f({3.0f, 4.0f}) );
    REQUIRE( max(v1, v2) == Vector2f({4.0f, 5.0f}) );
    REQUIRE( min(v1, 3.5f) == Vector2f({3.0f, 3.5f}) );
    REQUIRE( max(v1, 3.5f) == Vector2f({3.5f, 4.0f}) );

    // normalize
    Vector2f v6 = normalize(v3);
    REQUIRE( v6[0] == Approx(0.707107f) );
    REQUIRE( v6[1] == Approx(0.707107f) );
    REQUIRE( v6.length() == Approx(1.f) );
    REQUIRE( isnan(normalize(Vector2f{0.f, 0.f})) );
    REQUIRE( isnan(normalize(Vector2f{inf<float>(), 1.f})) );
    REQUIRE( isnan(normalize(Vector2f{1.f, inf<float>()})) );

    // abs
    REQUIRE( abs(Vector2f({1.0f, -1.0f})) == Vector2f({1.0f, 1.0f}) );
    REQUIRE( abs(Vector2f({-1.0f, -1.0f})) == Vector2f({1.0f, 1.0f}) );
    REQUIRE( abs(Vector2f({-1.0f, 1.0f})) == Vector2f({1.0f, 1.0f}) );

    // lerp
    Vector2f v7 = {0.f, 0.f};
    Vector2f v8 = {5.f, -3.f};
    REQUIRE( lerp(v7, v8, 0) == v7 );
    REQUIRE( lerp(v7, v8, 1) == v8 );
    REQUIRE( lerp(v7, v8, 0.5) == Vector2f({2.5f, -1.5f}) );

    // clamp
    REQUIRE( clamp( Vector2f({0.5f, 3.0f}), {0.0f, 4.0f}, {0.25f, 5.0f} ) == Vector2f({0.25f, 4.0f}) );
    REQUIRE( clamp( Vector2f{-3.f, 3.f}, {0.f, 0.f}, {inf<float>(), inf<float>()} ) == (Vector2f{0.f, 3.f}) );

    // hlift/project
    REQUIRE( hlift( Vector2f{0.5f, 3.0f} ) == (Vector<3, float>({0.5f, 3.0f, 0.f})) );
    REQUIRE( hlift( Vector2f{0.5f, 3.0f}, 1.f ) == (Vector<3, float>({0.5f, 3.0f, 1.f})) );
    REQUIRE( hproject( Vector2f{0.5f, 3.0f} ) == (Vector<1, float>({0.5f})) );
}

TEST_CASE("TestRectConstruct")
{
    Rect2f r { {0.0f, 1.0f}, {3.0f, 5.0f} };

    REQUIRE( r.min == Vector2f({0.f, 1.f}) );
    REQUIRE( r.max == Vector2f({3.f, 5.f}) );

    REQUIRE( r.length<0>() == Approx(3.0f) );
    REQUIRE( r.length<1>() == Approx(4.0f) );
}

TEST_CASE("TestRectMethods")
{
    Rect2f r { {0.0f, 0.0f}, {1.0f, 1.0f} };
    Rect2f r2 { {2.0f, 2.0f}, {3.0f, 3.0f} };
    // equals
    REQUIRE( equals(r, r) );
    REQUIRE( equals(r, Rect2f{ {0.f, 0.f}, {1.f, 1.f} }) );
    REQUIRE( !equals(r, r2) );

    // intersect
    REQUIRE( intersect( Rect2f({0.f, 0.f}, {5.f, 5.f}), Rect2f({1.f, 1.f}, {2.f, 2.f}) ) == (Rect2f({1.f, 1.f}, {2.f, 2.f})) );
    REQUIRE( intersect( Rect2f({0.f, 0.f}, {2.f, 2.f}), Rect2f({1.f, 1.f}, {3.f, 3.f}) ) == (Rect2f({1.f, 1.f}, {2.f, 2.f})) );
    REQUIRE( isnan( intersect( Rect2f({0.f, 0.f}, {2.f, 2.f}), Rect2f({3.f, 3.f}, {4.f, 4.f}) ) ) );

    // merge
    Rect2f r3 { {-1.f, 0.f}, {2.f, 1.f} };
    Vector2f v { 4.f, -2.f };
    REQUIRE( merge(r, r2) == (Rect2f({0.f, 0.f}, {3.f, 3.f})) );
    REQUIRE( merge(r, r3) == (Rect2f({-1.f, 0.f}, {2.f, 1.f })) );
    REQUIRE( merge(r, v) == (Rect2f({ 0.f, -2.f}, {4.f, 1.f })) );

    // is_inside
    REQUIRE( is_inside(r, { 0.f, 0.f }) );
    REQUIRE( is_inside(r, { 0.f, 0.9f }) );
    REQUIRE( is_inside(r, { 0.5f, 0.5f }) );

    REQUIRE( !is_inside(r, { 1.f, 0.f }) );
    REQUIRE( !is_inside(r, { 1.f, 1.f }) );
    REQUIRE( !is_inside(r, { 2.f, 0.5f }) );

    // to_vector
    REQUIRE( to_vector(r) == (Vector<4, float>({0.f, 0.f, 1.f, 1.f})) );
}

TEST_CASE("TestQuaternionOperations")
{
    Quaternion q    = Quaternion(1.f, 1.f, 1.f, 1.f);
    Quaternion q1   = Quaternion(1.f, 1.f, 1.f, 1.f);
    Quaternion q2   = Quaternion(0.f, 1.f, 1.f, 1.f);

    REQUIRE( q == q1 );
    REQUIRE( q != q2 );

    q   = Quaternion(1.f, 0.f, 1.f, 0.f);
    q1  = Quaternion(1.f, 0.5f, 0.5f, 0.75f);

    REQUIRE( q * q1 == Quaternion(0.5f, 1.25f, 1.5f, 0.25f) );
    REQUIRE( q2 * 2.f == Quaternion(0.f, 2.f, 2.f, 2.f) );
    REQUIRE( q + q1 == Quaternion(2.f, 0.5f, 1.5f, 0.75f) );
    REQUIRE( conjugate(q) == Quaternion(1.f, 0.f, -1.f, 0.f) );
}

TEST_CASE("TestQuaternionAngle")
{
    Quaternion q = from_axis_angle(45.f, {0.f, 0.f, 1.f});
    Quaternion n = normalize(q);
    REQUIRE( length_square(n) == Approx(1.0f) );
    REQUIRE( angle(n) == Approx(45.f) );

    q = from_axis_angle(45.f, {0.f, 1.f, 1.f});
    n = normalize(q);
    REQUIRE( length_square(n) == Approx(1.0f) );
    REQUIRE( angle(n) == Approx(45.f) );

    q = from_axis_angle(45.f, {1.f, 2.f, 3.f});
    n = normalize(q);
    REQUIRE( length_square(n) == Approx(1.0f) );
    REQUIRE( angle(n) == Approx(45.f) );
}
