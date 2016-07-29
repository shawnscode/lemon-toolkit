// @date 2016/06/08
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/defines.hpp>
#include <math/vector.hpp>

NS_FLOW2D_MATH_BEGIN

// row-major based matrix
template<size_t R, size_t C, typename T> struct Matrix
{
    Matrix();
    Matrix(std::initializer_list<T>);

    Matrix(const Matrix&) = default;
    Matrix& operator = (const Matrix&) = default;

    const Vector<C, T>& operator[](size_t) const;
    Vector<C, T>& operator[](size_t);

    bool operator == (const Matrix&) const;
    bool operator != (const Matrix&) const;

    void zero();
    void unit(size_t r, size_t c);
    void identity();

protected:
    Vector<R, Vector<C, T>> _values;
};

template<typename T>
using Matrix2   = Matrix<2, 2, T>;
using Matrix2f  = Matrix<2, 2, float>;

template<typename T>
using Matrix3   = Matrix<3, 3, T>;
using Matrix3f  = Matrix<3, 3, float>;

template<typename T>
using Matrix4   = Matrix<4, 4, T>;
using Matrix4f  = Matrix<4, 4, float>;

static const Matrix2f kMatrix2fIdentity = { 1.0f, 0.0f, 1.0f, 0.0f };

// unary operations
template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator + (const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator - (const Matrix<R, C, T>&);

// linear-algebraic operations
template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator + (const Matrix<R, C, T>&, const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator - (const Matrix<R, C, T>&, const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator * (const Matrix<R, C, T>&, T);

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator * (T, const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator / (const Matrix<R, C, T>&, T);

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator += (Matrix<R, C, T>&, const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator -= (Matrix<R, C, T>&, const Matrix<R, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator *= (Matrix<R, C, T>&, T);

template<size_t R, size_t S, size_t C, typename T>
Matrix<R, C, T>& operator *= (Matrix<R, S, T>&, const Matrix<S, C, T>&);

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator /= (Matrix<R, C, T>&, T);

// M*V
template<size_t R, size_t C, typename T>
Vector<R, T> operator * (const Matrix<R, C, T>&, const Vector<C, T>&);

// V^T*M
template<size_t R, size_t C, typename T>
Vector<C, T> operator * (const Vector<R, T>&, const Matrix<R, C, T>&);

// A*B
template<size_t R, size_t C, size_t S, typename T>
Matrix<R, C, T> operator * (const Matrix<R, S, T>&, const Matrix<S, C, T>&);

// M^T
template<size_t R, size_t C, typename T>
Matrix<C, R, T> transpose(const Matrix<R, C, T>&);

// extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix
template<size_t R, size_t C, typename T>
Matrix<R-1, C-1, T> hproject(const Matrix<R, C, T>&);

// create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to the
// input N-by-N matrix and all other entries to 0 except for the last row
// and last column entry which is set to 1
template<size_t R, size_t C, typename T>
Matrix<R+1, C+1, T> hlift(const Matrix<R, C, T>&);

// M*(V-HLift)
template<size_t N, typename T>
Vector<N-1, T> operator * (const Matrix<N, N, T>& M, const Vector<N-1, T>& V);

template<size_t N, typename T>
Matrix<N, N, T> make_scale(const Vector<N, T>&);

template<size_t N, typename T>
Matrix<N, N, T> make_rotation(const Vector<N, T>&);

template<size_t N, typename T>
Matrix<N+1, N+1, T> make_translation(const Vector<N, T>&);

template<typename T>
Matrix2<T> make_rotation(T radians);

template<typename T>
Matrix3<T> make_ortho(T xmin, T xmax, T ymin, T ymax);

#include <math/matrix.inl>
NS_FLOW2D_MATH_END