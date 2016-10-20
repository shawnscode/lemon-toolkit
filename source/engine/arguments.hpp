// @date 2016/10/20
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/subsystem.hpp>
#include <resource/path.hpp>

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

NS_LEMON_BEGIN

struct Arguments : core::Subsystem
{
    bool parse(const char*);

    rapidjson::Value* fetch(const char*);
    template<typename T> rapidjson::Value& fetch(const char*, T);
    const fs::Path& get_path() const;

protected:
    fs::Path _root;
    rapidjson::Document _document;
};

INLINE rapidjson::Value* Arguments::fetch(const char* path)
{
    return rapidjson::Pointer(path).Get(_document);
}

template<typename T> rapidjson::Value& Arguments::fetch(const char* path, T value)
{
    return rapidjson::Pointer(path).GetWithDefault(_document, value);
}

INLINE const fs::Path& Arguments::get_path() const
{
    return _root;
}

NS_LEMON_END