// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF MATRIX

template<size_t R, size_t C, typename T>
Matrix<R, C, T>::Matrix() {}

template<size_t R, size_t C, typename T>
Matrix<R, C, T>::Matrix(std::initializer_list<T> values)
{
    auto size = values.size();
    auto cursor = values.begin();

    size_t r, c, i;
    for( r = 0, i = 0; r < R; r++ )
    {
        for( c = 0; c < C; c++, i++ )
        {
            if( i < size ) _values[r][c] = *cursor++;
            else break;
        }

        if( c < C )
        {
            for( ; c < C; c++ )
                _values[r][c] = (T)0;
            r++;
            break;
        }
    }

    if( r < R )
    {
        for( ; r < R; r++ )
            for( c = 0; c < C; c++ )
                _values[r][c] = (T)0;
    }
}

template<size_t R, size_t C, typename T>
INLINE const Vector<C, T>& Matrix<R, C, T>::operator[] (size_t index) const
{
    return _values[index];
}

template<size_t R, size_t C, typename T>
INLINE Vector<C, T>& Matrix<R, C, T>::operator[] (size_t index)
{
    return _values[index];
}

template<size_t R, size_t C, typename T>
INLINE bool Matrix<R, C, T>::operator == (const Matrix<R, C, T>& rh) const
{
    return _values == rh._values;
}

template<size_t R, size_t C, typename T>
INLINE bool Matrix<R, C, T>::operator != (const Matrix<R, C, T>& rh) const
{
    return _values != rh._values;
}

template<size_t R, size_t C, typename T>
void Matrix<R, C, T>::zero()
{
    auto zero = (T)0;
    for( size_t i = 0; i < R; i++ )
        for( size_t j = 0; j < C; j++ )
            _values[i][j] = zero;
}

template<size_t R, size_t C, typename T>
void Matrix<R, C, T>::unit(size_t r, size_t c)
{
    zero();
    if( 0 <= r && r < R && 0 <= c && c <= C )
        _values[r][c] = (T)1;
}

template<size_t R, size_t C, typename T>
void Matrix<R, C, T>::identity()
{
    zero();
    auto diagonal = R <= C ? R : C;
    for( size_t i = 0; i < diagonal; i++ )
        _values[i][i] = (T)1;
}

// unary operations
template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator + (const Matrix<R, C, T>& rh)
{
    return rh;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator - (const Matrix<R, C, T>& rh)
{
    auto result = rh;
    for( size_t i = 0; i < R; i++ )
        result[i] = -result[i];
    return result;
}

// linear-algebraic operations
template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator + (const Matrix<R, C, T>& M0, const Matrix<R, C, T>& M1)
{
    auto result = M0;
    return result += M1;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator - (const Matrix<R, C, T>& M0, const Matrix<R, C, T>& M1)
{
    auto result = M0;
    return result -= M1;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator * (const Matrix<R, C, T>& M0, T scalar)
{
    auto result = M0;
    return result *= scalar;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator * (T scalar, const Matrix<R, C, T>& M0)
{
    auto result = M0;
    return result *= scalar;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T> operator / (const Matrix<R, C, T>& M0, T scalar)
{
    auto result = M0;
    return result /= scalar;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator += (Matrix<R, C, T>& M0, const Matrix<R, C, T>& M1)
{
    for( size_t i = 0; i < R; i++ )
        M0[i] += M1[i];
    return M0;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator -= (Matrix<R, C, T>& M0, const Matrix<R, C, T>& M1)
{
    for( size_t i = 0; i < R; i++ )
        M0[i] -= M1[i];
    return M0;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator *= (Matrix<R, C, T>& M0, T scalar)
{
    for( size_t i = 0; i < R; i++ )
        M0[i] *= scalar;
    return M0;
}

template<size_t R, size_t S, size_t C, typename T>
Matrix<R, C, T>& operator *= (Matrix<R, S, T>& M0, const Matrix<S, C, T>& M1)
{
    M0 = M0 * M1;
    return M0;
}

template<size_t R, size_t C, typename T>
Matrix<R, C, T>& operator /= (Matrix<R, C, T>& M0, T scalar)
{
    for( size_t i = 0; i < R; i++ )
        M0[i] /= scalar;
    return M0;
}

// M*V
template<size_t R, size_t C, typename T>
Vector<R, T> operator * (const Matrix<R, C, T>& M, const Vector<C, T>& V)
{
    Vector<R, T> result;
    for( auto r = 0; r < R; r++ )
    {
        result[r] = (T)0;
        for( auto c = 0; c < C; c++ )
            result[r] += M[r][c] * V[c];
    }
    return result;
}

// M*(V-HLift)
template<size_t N, typename T>
Vector<N-1, T> operator * (const Matrix<N, N, T>& M, const Vector<N-1, T>& V)
{
    Vector<N, T> L = hlift(V, (T)1);
    for( auto r = 0; r < N; r++ )
    {
        L[r] = (T)0;
        for( auto c = 0; c < N; c++ )
            L[r] += M[r][c] * L[c];
    }

    return hproject(L);
}

// V^T*M
template<size_t R, size_t C, typename T>
Vector<C, T> operator * (const Vector<R, T>& V, const Matrix<R, C, T>& M)
{
    Vector<C, T> result;
    for( auto c = 0; c < C; c++ )
    {
        result[c] = (T)0;
        for( auto r = 0; r < R; r++ )
            result[c] += V[c] * M[r][c];
    }
    return result;
}

// A*B
template<size_t R, size_t C, size_t S, typename T>
Matrix<R, C, T> operator * (const Matrix<R, S, T>& M0, const Matrix<S, C, T>& M1)
{
    Matrix<R, C, T> result;
    for( auto r = 0; r < R; r ++ )
    {
        for( auto c = 0; c < C; c ++ )
        {
            result[r][c] = (T)0;
            for( auto i = 0; i < S; i ++ )
                result[r][c] += M0[r][i] * M1[i][c];
        }
    }
    return result;
}

// M^T
template<size_t R, size_t C, typename T>
Matrix<C, R, T> transpose(const Matrix<R, C, T>& M)
{
    Matrix<C, R, T> result;
    for( auto r = 0; r < R; r ++ )
        for( auto c = 0; c < C; c ++ )
            result[c][r] = M[r][c];
    return result;
}

template<size_t R, size_t C, typename T>
Matrix<R-1, C-1, T> hproject(const Matrix<R, C, T>& M)
{
    static_assert( R >= 2 && C >= 2, "invalid matrix dimension." );

    Matrix<R-1, C-1, T> result;
    for( auto r = 0; r < R-1; r ++ )
        for( auto c = 0; c < C-1; c ++ )
            result[r][c] = M[r][c];
    return result;
}

template<size_t R, size_t C, typename T>
Matrix<R+1, C+1, T> hlift(const Matrix<R, C, T>& M)
{
    Matrix<R+1, C+1, T> result;
    result.identity();
    for( auto r = 0; r < R; r ++ )
        for( auto c = 0; c < C; c ++ )
            result[r][c] = M[r][c];
    return result;
}

template<size_t N, typename T>
Matrix<N+1, N+1, T> scale(const Vector<N, T>& V)
{
    Matrix<N+1, N+1, T> result;
    result.identity();
    for( auto i = 0; i < N; i++ )
        result[i][i] = V[i];
    return result;
}

template<size_t N, typename T>
Matrix<N+1, N+1, T> translation(const Vector<N, T>& V)
{
    Matrix<N+1, N+1, T> result;
    result.identity();
    for( auto i = 0; i < N; i++ )
        result[N][i] = V[i];
    return result;
}

template<typename T>
Matrix3<T> rotation(T degree)
{
    T cos = std::cos(to_radians(degree));
    T sin = std::sin(to_radians(degree));

    return Matrix3<T> {
        cos, sin, 0,
        -sin, cos, 0,
        0, 0, 1
    };
}

template<typename T>
Matrix4<T> rotation(T degree, const Vector3<T>& v)
{
    T cos = std::cos(to_radians(degree));
    T sin = std::sin(to_radians(degree));

    const Vector3<T> axis(normalize(v));
    const Vector3<T> temp((T(1) - cos) * axis);

    Matrix4<T> result;
    result.identity();
    result[0][0] = cos + temp[0] * axis[0];
    result[0][1] = temp[0] * axis[1] + sin * axis[2];
    result[0][2] = temp[0] * axis[2] - sin * axis[1];

    result[1][0] = temp[1] * axis[0] - sin * axis[2];
    result[1][1] = cos + temp[1] * axis[1];
    result[1][2] = temp[1] * axis[2] + sin * axis[0];

    result[2][0] = temp[2] * axis[0] + sin * axis[1];
    result[2][1] = temp[2] * axis[1] - sin * axis[0];
    result[2][2] = cos + temp[2] * axis[2];
    return result;
}

template<typename T>
Matrix4<T> ortho(T left, T right, T bottom, T top, T znear, T zfar)
{
    Matrix4<T> result;
    result.identity();
    result[0][0] = static_cast<T>(2) / (right - left);
    result[1][1] = static_cast<T>(2) / (top - bottom);
    result[2][2] = static_cast<T>(2) / (zfar - znear);

    result[3][0] = - (right + left) / (right - left);
    result[3][1] = - (top + bottom) / (top - bottom);
    result[3][2] = - (zfar + znear) / (zfar - znear);
    return result;
}

template<typename T>
INLINE Matrix4<T> perspective(T fov, T aspect, T znear, T zfar)
{
    ENSURE( std::abs(aspect - epsilon<T>()) > static_cast<T>(0) );
    const T scale = static_cast<T>(1) / std::tan(to_radians(fov) / static_cast<T>(2));

    Matrix4<T> result;
    result.zero();
    result[0][0] = scale / aspect;
    result[1][1] = scale;
    result[2][2] = (zfar + znear) / (zfar - znear);

    result[3][2] = - (static_cast<T>(2) * zfar * znear) / (zfar - znear);
    result[2][3] = static_cast<T>(1);
    return result;
}

template<typename T>
INLINE Matrix4<T> look_at(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up)
{
    const Vector3<T> f(normalize(center-eye));
    const Vector3<T> s(normalize(cross(up, f)));
    const Vector3<T> u(cross(f, s));

    Matrix4<T> result;
    result.identity();

    result[0][0] = s[0];
    result[0][1] = u[0];
    result[0][2] = f[0];

    result[1][0] = s[1];
    result[1][1] = u[1];
    result[1][2] = f[1];

    result[2][0] = s[2];
    result[2][1] = u[2];
    result[2][2] = f[2];

    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = -dot(f, eye);
    return result;
}
