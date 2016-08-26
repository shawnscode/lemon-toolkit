// @date 2016/08/05
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/path.hpp>

NS_LEMON_FILESYSTEM_BEGIN

std::ostream& operator << (std::ostream& os, const Path& path)
{
    return os << path.to_string();
}

std::string to_string(const Path& path)
{
    return path.to_string();
}

Path::iterator::iterator(const Path& p, size_t pos) : _target(p)
{
    _iterator = std::min(
        p._pathname.find_first_not_of(Path::sperator, pos),
        p._pathname.length());
}

Path::iterator& Path::iterator::operator ++ ()
{
    if( _iterator > 0 )
    {
        _iterator = std::min(
            _target._pathname.find_first_not_of(Path::sperator, _iterator),
            _target._pathname.length());
    }

    auto pos = _target._pathname.find_first_of(Path::sperator, _iterator);
    if( pos != std::string::npos )
        _iterator = std::min(
            _target._pathname.find_first_not_of(Path::sperator, pos+1),
            _target._pathname.length());
    else
        _iterator = _target._pathname.length();
    return *this;
}

Path::iterator Path::iterator::operator ++ (int _dummy)
{
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

Path::iterator& Path::iterator::operator -- ()
{
    auto end = _target._pathname.find_last_not_of(Path::sperator, _iterator-1);
    if( end == std::string::npos || end == 0 )
    {
        _iterator = 0;
        return *this;
    }

    auto start = _target._pathname.find_last_of(Path::sperator, end-1);
    if( start == std::string::npos )
    {
        _iterator = 0;
        return *this;
    }

    _iterator = start + 1;
    return *this;
}

Path::iterator Path::iterator::operator -- (int _dummy)
{
    iterator tmp = *this;
    --(*this);
    return tmp;
}

bool Path::iterator::operator == (const iterator& rhs) const
{
    return &_target == &rhs._target && _iterator == rhs._iterator;
}

bool Path::iterator::operator != (const iterator& rhs) const
{
    return !(*this == rhs);
}

std::string Path::iterator::operator * () const
{
    auto pos = _target._pathname.find_first_of(Path::sperator, _iterator);
    if( pos != std::string::npos )
        return _target._pathname.substr(_iterator, pos - _iterator);
    else if( _iterator < _target._pathname.length() )
        return _target._pathname.substr(_iterator, _target._pathname.length()-_iterator);
    return std::string();
}

void Path::tokenize()
{
    if( _pathname.empty() )
        _pathname = Path::empty_representation;

    if( _pathname == Path::empty_representation )
        return;

    // remove suffix sperator
    size_t pos = _pathname.find_last_not_of(Path::sperator);
    if( pos == std::string::npos )
    {
        _pathname.clear();
        _pathname.append(Path::sperator);
        return;
    }
    _pathname.erase(pos+1);

    // remove duplicated sperator between filename
    size_t end = _pathname.find_last_of(Path::sperator, _pathname.length());
    while( end != std::string::npos && end != 0 )
    {
        size_t start = _pathname.find_last_not_of(Path::sperator, end-1);
        if( start == std::string::npos )
        {
            _pathname.replace(0, end+1, Path::sperator);
            break;
        }

        if( start != end - 1 )
            _pathname.replace(start+1, end - start, Path::sperator);
        
        if( start == 0 )
            break;
        end = _pathname.find_last_of(Path::sperator, start-1);
    }

    compress();
}

void Path::compress()
{
    size_t end = _pathname.length();
    while( end != std::string::npos && end != 0 )
    {
        size_t start = _pathname.find_last_of(Path::sperator, end-1);
        if( start == std::string::npos )
        {
            if( _pathname.compare(0, end, Path::empty) == 0 )
                _pathname.erase(0, strlen(Path::empty)+1);
            break;
        }

        if( _pathname.compare(start+1, end-start-1, Path::empty) == 0 )
            _pathname.erase(end == _pathname.length() ? start : start+1,  end-start);
        end = start;
    }

    size_t previous_count = 0, eliminated = 0;
    size_t last_pos = _pathname.length(), pos = _pathname.find_last_of(Path::sperator);
    while( last_pos != std::string::npos )
    {
        if( pos != last_pos )
        {
            if( _pathname.compare(pos+1, last_pos-pos-1, Path::previous) == 0 )
            {
                previous_count ++;
            }
            else if( previous_count > 0 )
            {
                _pathname.erase(pos+1, last_pos-pos);
                previous_count --;
                eliminated ++;
            }
        }
            
        last_pos = pos;
        if( last_pos == std::string::npos || last_pos == 0 )
            break;
        pos = _pathname.find_last_of(Path::sperator, pos-1);
    }

    if( eliminated > 0 )
    {
        last_pos = _pathname.length(), pos = _pathname.find_last_of(Path::sperator);
        while( last_pos != std::string::npos && eliminated > 0 )
        {
            if( pos == std::string::npos )
            {
                _pathname.clear();
                break;
            }
            
            if( pos != last_pos )
            {
                if( _pathname.compare(pos+1, last_pos-pos-1, Path::previous) == 0 )
                {
                    eliminated --;
                    _pathname.erase(pos+1, last_pos-pos);
                }

                last_pos = pos;
                if( last_pos == std::string::npos || last_pos == 0 )
                    break;
                pos = _pathname.find_last_of(Path::sperator, pos-1);
            }
        }
        
        size_t len = strlen(Path::sperator);
        if( _pathname.length() > len && _pathname.compare(_pathname.length()-len, len, Path::sperator, len) == 0 )
            _pathname.erase(_pathname.length()-len);
    }
    
    if( is_absolute() )
    {
        last_pos = 0, pos = _pathname.find_first_of(Path::sperator, 1);
        size_t prefix = 0;
        while( last_pos != std::string::npos )
        {
            if( pos != last_pos && _pathname.compare(last_pos+1, pos - last_pos - 1, Path::previous) == 0 )
                prefix = pos;
            else
                break;

            last_pos = pos;
            pos = _pathname.find_first_of(Path::sperator, pos+1);
        }
        
        if( prefix > 0 )
            _pathname.erase(1, prefix);
    }

    if( _pathname.empty() )
        _pathname = Path::empty_representation;
}

Path& Path::concat(const Path& rhs)
{
    if( rhs.is_empty() )
        return *this;

    _pathname.reserve(rhs._pathname.length()+_pathname.length()+1);

    if( _pathname != Path::sperator && !is_empty() && !rhs.is_absolute() )
        _pathname.append(Path::sperator);
    _pathname.append(rhs._pathname);

    compress();
    return *this;
}

std::string Path::get_filename() const
{
    if( is_empty() )
        return "";

    auto spe = _pathname.find_last_of(Path::sperator);
    return _pathname.substr(spe+1);
}

std::string Path::get_basename() const
{
    if( is_empty() )
        return "";

    auto spe = _pathname.find_last_of(Path::sperator);
    auto ext = _pathname.find_last_of(Path::extension_sperator);
    return _pathname.substr(spe+1, ext-spe-1);
}

std::string Path::get_extension() const
{
    if( is_empty() )
        return "";

    auto ext = _pathname.find_last_of(Path::extension_sperator);
    return _pathname.substr(ext+1);
}

Path Path::get_parent() const
{
    if( is_empty() )
        return Path::previous;

    auto found = _pathname.find_last_of(Path::sperator);
    if( found == std::string::npos )
    {
        if( _pathname == Path::previous )
            return "../..";
        return "";
    }

    if( _pathname.compare(found+1, _pathname.length()-found, Path::previous) == 0 )
    {
        std::string tmp = _pathname;
        tmp.append(Path::sperator);
        tmp.append(Path::previous);
        return tmp;
    }

    return _pathname.substr(0, found);
}

Path Path::get_root() const
{
    if( is_empty() )
        return "";

    auto found = _pathname.find_first_of(Path::sperator);
    if( found == std::string::npos )
        return _pathname;

    return _pathname.substr(0, found+1);
}

NS_LEMON_FILESYSTEM_END