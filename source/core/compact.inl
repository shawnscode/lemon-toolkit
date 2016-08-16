// @date 2016/08/16
// @author Mao Jingkai(oammix@gmail.com)

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
const int32_t CompactHashMap<K, V, Size>::iterator_t<T, TV>::invalid = math::max<int32_t>();

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
CompactHashMap<K, V, Size>::iterator_t<T, TV>::iterator_t(T& table, int32_t pos)
: _table(table)
{
    _cursor = invalid;
    for( int32_t i = pos; i<Size; i++ )
    {
        if( _table._keys[i] )
        {
            _cursor = i;
            break;
        }
    }
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
CompactHashMap<K, V, Size>::iterator_t<T, TV>& CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator ++ ()
{
    for( int32_t i = _cursor+1; i<Size; i++ )
    {
        if( _table._keys[i] )
        {
            _cursor = i;
            return *this;
        }
    }

    _cursor = invalid;
    return *this;
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
CompactHashMap<K, V, Size>::iterator_t<T, TV> CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator ++ (int _dummy)
{
    auto tmp = *this;
    (*this)++;
    return tmp;
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
CompactHashMap<K, V, Size>::iterator_t<T, TV>& CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator -- ()
{

    for( int32_t i = _cursor-1; i>=0; i-- )
    {
        if( _table._keys[i] )
        {
            _cursor = i;
            return *this;
        }
    }

    _cursor = invalid;
    return *this;
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
CompactHashMap<K, V, Size>::iterator_t<T, TV> CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator -- (int _dummy)
{
    auto tmp = *this;
    (*this)--;
    return tmp;
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
bool CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator == (const iterator_t<T, TV>& rhs) const
{
    return (&_table == &rhs._table) && (_cursor == rhs._cursor);
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
bool CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator != (const iterator_t<T, TV>& rhs) const
{
    return !(*this == rhs);
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
TV& CompactHashMap<K, V, Size>::iterator_t<T, TV>::operator * () const
{
    return _table._values[_cursor];
}

template<typename K, typename V, size_t Size>
template<typename T, typename TV>
int32_t CompactHashMap<K, V, Size>::iterator_t<T, TV>::get_cursor () const
{
    return _cursor;
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::iterator CompactHashMap<K, V, Size>::begin()
{
    return iterator(*this, 0);
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::iterator CompactHashMap<K, V, Size>::end()
{
    return iterator(*this);
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::const_iterator CompactHashMap<K, V, Size>::begin() const
{
    return iterator(*this, 0);
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::const_iterator CompactHashMap<K, V, Size>::end() const
{
    return iterator(*this);
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::iterator CompactHashMap<K, V, Size>::find(const K& key)
{
    for( int32_t i = 0; i < Size; i++ )
    {
        if( _keys[i] == key )
            return iterator(*this, i);
    }

    return iterator(*this);
}

template<typename K, typename V, size_t Size>
typename CompactHashMap<K, V, Size>::const_iterator CompactHashMap<K, V, Size>::find(const K& key) const
{
    for( int32_t i = 0; i < Size; i++ )
    {
        if( _keys[i] == key )
            return const_iterator(*this, i);
    }

    return const_iterator(*this);
}

template<typename K, typename V, size_t Size>
std::pair<typename CompactHashMap<K, V, Size>::iterator, bool> CompactHashMap<K, V, Size>::insert(std::pair<K, V> pair)
{
    for( int32_t i = 0; i < Size; i++ )
    {
        if( !_keys[i] )
        {
            _keys[i] = std::move(pair.first);
            _values[i] = std::move(pair.second);
            return std::make_pair(iterator(*this, i), true);
        }
    }

    return std::make_pair(iterator(*this), false);
}

template<typename K, typename V, size_t Size>
void CompactHashMap<K, V, Size>::clear()
{
    for( int32_t i = 0; i < Size; i++ )
        if( !_keys[i] )
            _keys[i] = nullptr;
}

template<typename V, size_t Size>
template<typename T, typename TV>
const int32_t CompactVector<V, Size>::iterator_t<T, TV>::invalid = math::max<int32_t>();

template<typename V, size_t Size>
template<typename T, typename TV>
CompactVector<V, Size>::iterator_t<T, TV>::iterator_t(T& vector, int32_t pos)
: _vector(vector)
{
    if( pos < _vector._size ) _cursor = pos;
    else _cursor = invalid;
}

template<typename V, size_t Size>
template<typename T, typename TV>
CompactVector<V, Size>::iterator_t<T, TV>& CompactVector<V, Size>::iterator_t<T, TV>::operator ++ ()
{
    if( _cursor < (_vector._size-1) ) _cursor++;
    else _cursor = invalid;
    return *this;
}

template<typename V, size_t Size>
template<typename T, typename TV>
CompactVector<V, Size>::iterator_t<T, TV> CompactVector<V, Size>::iterator_t<T, TV>::operator ++ (int _dummy)
{
    auto tmp = *this;
    (*this)++;
    return tmp;
}

template<typename V, size_t Size>
template<typename T, typename TV>
CompactVector<V, Size>::iterator_t<T, TV>& CompactVector<V, Size>::iterator_t<T, TV>::operator -- ()
{
    if( _cursor > 0 ) _cursor --;
    return *this;
}

template<typename V, size_t Size>
template<typename T, typename TV>
CompactVector<V, Size>::iterator_t<T, TV> CompactVector<V, Size>::iterator_t<T, TV>::operator -- (int _dummy)
{
    auto tmp = *this;
    (*this)--;
    return tmp;
}

template<typename V, size_t Size>
template<typename T, typename TV>
bool CompactVector<V, Size>::iterator_t<T, TV>::operator == (const iterator_t<T, TV>& rhs) const
{
    return (&_vector == &rhs._vector) && (_cursor == rhs._cursor);
}

template<typename V, size_t Size>
template<typename T, typename TV>
bool CompactVector<V, Size>::iterator_t<T, TV>::operator != (const iterator_t<T, TV>& rhs) const
{
    return !(*this == rhs);
}

template<typename V, size_t Size>
template<typename T, typename TV>
TV& CompactVector<V, Size>::iterator_t<T, TV>::operator * () const
{
    return _vector._values[_cursor];
}

template<typename V, size_t Size>
CompactVector<V, Size>::CompactVector(std::initializer_list<V> values)
{
    _size = 0;
    for( auto value : values )
        _values[_size++] = value;
}

template<typename V, size_t Size>
typename CompactVector<V, Size>::iterator CompactVector<V, Size>::begin()
{
    return iterator(*this, 0);
}

template<typename V, size_t Size>
typename CompactVector<V, Size>::const_iterator CompactVector<V, Size>::begin() const
{
   return const_iterator(*this, 0);
}

template<typename V, size_t Size>
typename CompactVector<V, Size>::iterator CompactVector<V, Size>::end()
{
    return iterator(*this);
}

template<typename V, size_t Size>
typename CompactVector<V, Size>::const_iterator CompactVector<V, Size>::end() const
{
   return const_iterator(*this);
}

template<typename V, size_t Size>
V& CompactVector<V, Size>::operator[] (unsigned index)
{
    return _values[index];
}

template<typename V, size_t Size>
const V& CompactVector<V, Size>::operator[] (unsigned index) const
{
    return _values[index];
}

template<typename V, size_t Size>
void CompactVector<V, Size>::push_back(V&&  value)
{
    _values[_size++] = std::move(value);
}

template<typename V, size_t Size>
void CompactVector<V, Size>::push_back(const V& value)
{
    _values[_size++] = value;
}

template<typename V, size_t Size>
void CompactVector<V, Size>::clear()
{
    _size = 0;
}

template<typename V, size_t Size>
unsigned CompactVector<V, Size>::get_size() const
{
    return _size;
}

