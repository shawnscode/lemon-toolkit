// @date 2016/06/08
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <flow2d/math/vector.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<size_t R, size_t C, typename T> struct Matrix
{
    Matrix();
    Matrix(std::initializer_list<T>);

    Matrix(const Matrix&) = default;
    Matrix operator = (const Matrix&) = default;

    const T& operator()(size_t, size_t) const;
    T& operator(size_t, size_t) const;

    // index in row major
    const T& operator[](size_t) const;
    T& operator[](size_t);

    bool operator == (const Matrix&) const;
    bool operator != (const Matrix&) const;

    // M^T
    Matrix<C, R, T> transpose();
    // extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix
    Matrix<N-1, N-1, T> hproject();
    // create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to the
    // input N-by-N matrix and all other entries to 0 except for the last row
    // and last column entry which is set to 1
    Matrix<N+1, N+1, T> hlift();

    void zero();
    void unit(size_t r, size_t c);
    void identity();

protected:
    std::array<std::array<T, C>, R> m_values;
};

template<typename T>
using Matrix2   = Matrix<2, T>;
using Matrix2f  = Matrix<2, float>;

template<typename T>
using Matrix3   = Matrix<3, T>;
using Matrix3f  = Matrix<3, float>;

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

#include <flow2d/math/matrix.inl>
NS_FLOW2D_END