// @date 2016/06/15
// @author Mao Jingkai(oammix@gmail.com)

INLINE Path& Path::set(const std::string& str)
{
    size_t last_pos = 0, pos = str.find_first_of("/", last_pos);
    std::vector<std::string> tokens;

    while( last_pos != std::string::npos )
    {
        if( pos != last_pos )
            tokens.push_back(str.substr(last_pos, pos - last_pos));
        last_pos = pos;
        if( last_pos == std::string::npos || last_pos + 1 == str.length() )
            break;
        pos = str.find_first_of("/", ++last_pos);
    }

    m_path = std::move(tokens);
}

INLINE Path& Path::concat(const Path& rh)
{
    m_path.insert(m_path.begin(), rh.m_path.begin(), rh.m_path.end());
}

INLINE bool Path::is_empty() const
{
    return m_path.size() == 0;
}

INLINE std::string Path::get_filename() const
{
    if( is_empty() )
        return "";

    return m_path[m_path.size()-1];
}

INLINE std::string Path::get_basename() const
{
    auto name = get_filename();
    auto pos = name.find_last_of(".");
    if( pos == std::string::npos )
        return "";
    return name.substr(0, pos);
}

INLINE std::string Path::get_extension() const
{
    auto name = get_filename();
    auto pos = name.find_last_of(".");
    if( pos == std::string::npos )
        return "";
    return name.substr(pos+1);
}

INLINE Path Path::get_parent() const
{
    Path result { *this };
    if( !result.is_empty() )
        result.m_path.pop_back();
    return result;
}

INLINE Path& Path::operator /= (const Path& rh)
{
    return concat(rh);
}

INLINE Path& Path::operator = (Path&& rh)
{
    m_path = std::move(rh.m_path);
    return *this;
}

INLINE Path Path::operator / (const Path& rh) const
{
    Path result = *this;
    return result /= rh;
}

INLINE bool Path::operator == (const Path& rh) const
{
    return m_path == rh.m_path;
}

INLINE Path::Iterator Path::begin()
{
    return m_path.begin();
}

INLINE Path::Iterator Path::end()
{
    return m_path.end();
}