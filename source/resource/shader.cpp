// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/shader.hpp>
#include <graphics/renderer.hpp>

#include <regex>
#include <iostream>

NS_LEMON_RESOURCE_BEGIN

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

    _program = frontend->create<graphics::Program>(_vertex.c_str(), _fragment.c_str())->handle;
    return _program.is_valid();
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

NS_LEMON_RESOURCE_END