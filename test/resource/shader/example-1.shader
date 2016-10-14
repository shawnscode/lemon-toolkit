//-> VERTEX_SHADER {
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
uniform mat4 MVP;
out vec2 v_uv;
void main(){
    gl_Position = vec4(position, 1.0)*MVP;
    v_uv = uv;
}
//-> }


//-> FRAGMENT_SHADER {
#version 330 core
in vec2 v_uv;
out vec3 color;
uniform sampler2D sampler;
void main(){
  color = texture(sampler, v_uv).rgb;
}
//-> }