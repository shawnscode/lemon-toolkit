// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/shader.hpp>
#include <graphics/renderer.hpp>

#include <regex>
#include <iostream>

NS_LEMON_RESOURCE_BEGIN

const char* s_default_vs = 
"#version 330 core\n"
"\n"
"layout (location = 0) in vec3 position;\n"
"\n"
"uniform mat4 lm_ProjectionMatrix;\n"
"uniform mat4 lm_ViewMatrix;\n"
"uniform mat4 lm_ModelMatrix;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position, 1.0f) * lm_ModelMatrix * lm_ViewMatrix * lm_ProjectionMatrix;\n"
"}\n";

const char* s_default_fs =
"#version 330 core\n"
"\n"
"uniform vec3 ObjectColor;\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"    color = vec4(ObjectColor, 1.0f);\n"
"}\n";

Shader::ptr Shader::color()
{
    static const fs::Path s_path = {"_buildin_/shader/default"};
    if( auto cache = core::get_subsystem<ResourceCache>() )
        return cache->create<Shader>(s_path, s_default_vs, s_default_fs);
    return nullptr;
}

struct Token
{
    constexpr static char const*const VertexShader = "VERTEX_SHADER";
    constexpr static char const*const FragmentShader = "FRAGMENT_SHADER";
    constexpr static char const*const OpenBracket = "{";
    constexpr static char const*const CloseBracket = "}";
};

enum class ShaderType
{
    VERTEX,
    FRAGMENT
};

Shader::~Shader()
{
    graphics::resource::free(_program);
}

bool Shader::read(std::istream& in)
{
    std::regex syntax("//->.*");
    std::regex tokenize("(\\S+)");
    std::smatch pieces;

    std::string line;
    bool bracket = false;
    ShaderType type = ShaderType::VERTEX;

    std::string vertex, fragment;
    while( std::getline(in, line) )
    {
        if( std::regex_match(line, syntax) )
        {
            auto iterator = std::sregex_iterator(line.begin()+4, line.end(), tokenize);
            for( ; iterator != std::sregex_iterator(); iterator ++ )
            {
                if( strcmp((*iterator).str().c_str(), Token::VertexShader) == 0 )
                {
                    ENSURE(!bracket);
                    type = ShaderType::VERTEX;
                    vertex.clear();
                }
                else if( strcmp((*iterator).str().c_str(), Token::FragmentShader) == 0 )
                {
                    ENSURE(!bracket);
                    type = ShaderType::FRAGMENT;
                    fragment.clear();
                }
                else if( strcmp((*iterator).str().c_str(), Token::OpenBracket) == 0 )
                {
                    ENSURE(!bracket);
                    bracket = true;
                }
                else if( strcmp((*iterator).str().c_str(), Token::CloseBracket) == 0 )
                {
                    ENSURE(bracket);
                    bracket = false;
                }
            }
        }
        else
        {
            if( type == ShaderType::VERTEX )
            {
                vertex.append(line);
                vertex.push_back('\n');
            }
            else
            {
                fragment.append(line);
                fragment.push_back('\n');
            }
        }
    }

    ENSURE(!bracket);
    return initialize(vertex.c_str(), fragment.c_str());
}

bool Shader::save(std::ostream& out)
{
    out << "//-> VERTEX_SHADER {\n";
    out << _vertex;
    out << "//-> }\n";
    out << "//-> FRAGMENT_SHADER {\n";
    out << _fragment;
    out << "//-> }\n";
    return true;
}

bool Shader::update_video_object()
{
    if( _dirty )
    {
        if( _program )
            graphics::resource::free(_program);
        _program = graphics::resource::create<graphics::Program>(_vertex.c_str(), _fragment.c_str());
        _dirty = false;
    }

    return _program != nullptr;
}

size_t Shader::get_memory_usage() const
{
    return sizeof(Shader) + _vertex.size() + _fragment.size();
}

size_t Shader::get_video_memory_usage() const
{
    return 0;
}

bool Shader::initialize(const char* vs, const char* fs)
{
    if( vs == nullptr || fs == nullptr )
        return false;

    _vertex = vs;
    _fragment = fs;
    _dirty = true;
    return true;
}

NS_LEMON_RESOURCE_END