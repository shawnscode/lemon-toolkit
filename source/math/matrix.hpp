// @date 2016/06/08
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/vector.hpp>

NS_LEMON_MATH_BEGIN

// row-major based matrix
template<size_t R, size_t C, typename T> struct Matrix
{
    Matrix();
    Matrix(std::initializer_list<T>);

    Matrix(const Matrix&) = default;
    Matrix& operator = (const Matrix&) = default;

    const Vector<C, T>& operator[](size_t) const;
    Vector<C, T>& operator[](size_t);
    operator Matrix<R+1, C+1, T> () const;

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

//
template<size_t R, size_t C, typename T>
std::ostream& operator << (std::ostream&, const Matrix<R, C, T>&);

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

/// builds a scale 4 * 4 matrix created from scalars.
template<size_t N, typename T>
Matrix<N, N, T> scale(const Vector<N, T>&);

// builds a rotation 4 * 4 matrix created from an angle.
template<typename T>
Matrix3<T> rotation(T degree);

// builds a rotation 4 * 4 matrix created from an axis vector and an angle.
template<typename T>
Matrix3<T> rotation(T degree, const Vector3<T>&);

// builds a translation 4 * 4 matrix created from a vector.
template<size_t N, typename T>
Matrix<N+1, N+1, T> translation(const Vector<N, T>&);

// creates a matrix for a symetric perspective-view frustum based on the left handedness.
template<typename T>
Matrix4<T> perspective(T fov, T aspect, T znear, T zfar);

// creates a matrix for an orthographic parallel viewing volume, using the left handedness.
template<typename T>
Matrix4<T> ortho(T left, T right, T bottom, T top, T znear, T zfar);

// build a look at view matrix based on the left handedness.
template<typename T>
Matrix4<T> look_at(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up);

#include <math/matrix.inl>
NS_LEMON_MATH_END