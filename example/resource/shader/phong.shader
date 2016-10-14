//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 v_normal;
out vec3 v_frag_position; // in world space

void main()
{
    gl_Position = vec4(position, 1.0f) * model * view * projection;
    v_normal = (vec4(normal, 1.0f) * model).xyz;
    v_frag_position = (vec4(position, 1.0f) * model).xyz;
}
//-> }

//-> FRAGMENT_SHADER {
#version 330 core

in vec3 v_frag_position;
in vec3 v_normal;
out vec4 color;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

void main()
{
    //
    vec3 ambient = object_color * light_color;

    //
    vec3 light_dir = normalize(light_pos - v_frag_position);
    vec3 norm = normalize(v_normal);
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * light_color;

    //
    vec3 reflect_dir = reflect(-light_dir, norm);
    vec3 view_dir = normalize(view_pos - v_frag_position);
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 32) * light_color;

    //
    color = vec4(specular*0.5f + diffuse*0.8f + ambient*0.2f, 1.0f);
}
//-> }