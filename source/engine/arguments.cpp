// @date 2016/10/20
// @author Mao Jingkai(oammix@gmail.com)

#include <engine/arguments.hpp>
#include <resource/filesystem.hpp>
#include <rapidjson/error/en.h>

NS_LEMON_BEGIN

bool Arguments::parse(const char* file)
{
    auto ifs = fs::open(file, fs::FileMode::READ);
    std::string text((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    if( _document.Parse(text.c_str()).HasParseError() )
    {
        LOGW("[%d] failed to parse %s as arguments at %d, %s",
            _document.GetParseError(),
            file,
            _document.GetErrorOffset(),
            rapidjson::GetParseError_En(_document.GetParseError()));
        return false;
    }

    // convert to absolute path
    _root = fs::get_current_directory() / _root.set(file).get_parent();
    return true;
}

NS_LEMON_END