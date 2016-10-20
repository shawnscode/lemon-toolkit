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
"uniform mat4 ProjectionMatrix;\n"
"uniform mat4 ViewMatrix;\n"
"uniform mat4 ModelMatrix;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position, 1.0f) * ModelMatrix * ViewMatrix * ProjectionMatrix;\n"
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

Shader::ptr Shader::create(const fs::Path& name, const char* vs, const char* fs)
{
    auto cache = core::get_subsystem<ResourceCache>();
    if( cache->is_exist(name) )
        return cache->get<Shader>(name);

    auto shader = Shader::ptr(new (std::nothrow) Shader);
    shader->_vertex = vs;
    shader->_fragment = fs;
    shader->_program = core::get_subsystem<graphics::Renderer>()->create<graphics::Program>(vs, fs);

    cache->add(name, shader);
    return shader;
}

Shader::ptr Shader::color()
{
    static const fs::Path s_path = {"_buildin_/shader/default"};
    return create(s_path, s_default_vs, s_default_fs);
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
    if( core::details::status() == core::details::Status::RUNNING )
        core::get_subsystem<graphics::Renderer>()->free(_program);
}

bool Shader::read(std::istream& in)
{
    std::regex syntax("//->.*");
    std::regex tokenize("(\\S+)");
    std::smatch pieces;

    std::string line;
    bool bracket = false;
    ShaderType type = ShaderType::VERTEX;

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
                    _vertex.clear();
                }
                else if( strcmp((*iterator).str().c_str(), Token::FragmentShader) == 0 )
                {
                    ENSURE(!bracket);
                    type = ShaderType::FRAGMENT;
                    _fragment.clear();
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
                _vertex.append(line);
                _vertex.push_back('\n');
            }
            else
            {
                _fragment.append(line);
                _fragment.push_back('\n');
            }
        }
    }

    ENSURE(!bracket);

    auto frontend = core::get_subsystem<graphics::Renderer>();
    ENSURE(frontend != nullptr);

    _program = frontend->create<graphics::Program>(_vertex.c_str(), _fragment.c_str());
    return _program != nullptr;
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

size_t Shader::get_memory_usage() const
{
    return _vertex.size() + _fragment.size();
}

NS_LEMON_RESOURCE_END