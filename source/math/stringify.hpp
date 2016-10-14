// @date 2016/06/08
// @author Mao Jingkai(oammix@gmail.com)
// string serialization with stream

#pragma once

#include <math/vector.hpp>
#include <math/matrix.hpp>
#include <math/color.hpp>

#include <iosfwd>
#include <string>

NS_LEMON_MATH_BEGIN

template<size_t N, typename T>
std::ostream& operator<< (std::ostream& out, const Vector<N, T>& v)
{
    out << "Vector(";
    for( size_t i = 0; i < N; i++ )
    {
        out << v[i];
        if( i != N-1 ) out << ", ";
    }
    return out << ")";
}

template<size_t R, size_t C, typename T>
std::ostream& operator << (std::ostream& out, const Matrix<R, C, T>& m)
{
    out << "Matrix(";
    for( unsigned r = 0; r < R; r++ )
    {
        out << "[";
        for( unsigned c = 0; c < C; c++ )
        {
            out << m[r][c];
            if( c != C - 1 ) out <<",";
        }
        out << "]";
        if( r != R - 1 ) out << ",";
    }
    return out << ")";
}

template<size_t N, typename T>
std::ostream& operator<< (std::ostream& out, const Rect<N, T>& rect)
{
    out << "Rect({";
    for( size_t i = 0; i < N; i++ )
    {
        out << rect.min[i];
        if( i != N-1 ) out << ", ";
    }
    out << "}, {";
    for( size_t i = 0; i < N; i++ )
    {
        out << rect.max[i];
        if( i != N-1 ) out << ", ";
    }
    return out << "})";
}

INLINE std::ostream& operator << (std::ostream& out, const Quaternion& rhs)
{
    return out
        << "Quaternion("
        << rhs[0] << ", "
        << rhs[1] << ", "
        << rhs[2] << ", "
        << rhs[3] << ")";
}

INLINE std::ostream& operator << (std::ostream& out, const Color& c)
{
    char buffer[32];
    sprintf(buffer, "0x%08X", c.to_uint32());
    out << "Color(" << buffer << ")";
    return out;
}


NS_LEMON_MATH_END