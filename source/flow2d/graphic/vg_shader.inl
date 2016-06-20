const static char* vg_vertex = 
"#version 330 core\n"
"uniform mat3 transform;\n"
"layout(location = 0) in vec2 in_position;\n"
"out vec2 position;\n"
"void main() {\n"
"  position = in_position;\n"
"  gl_Position = vec4((transform * vec3(in_position, 1.0f)).xy, 0.f, 1.f);\n"
"}\n";

const static char* vg_fragment =
"#version 330 core\n"
"uniform mat3  paint;\n"
"uniform vec4  inner_color;\n"
"uniform vec4  outer_color;\n"
"uniform vec2  extent;\n"
"uniform float radius;\n"
"uniform float feather;\n"
"in vec2 position;\n"
"out vec4 color;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec2 pt = (paint*vec3(position, 1.0f)).xy;\n"
"    vec2 extent2 = extent - vec2(radius, radius);\n"
"    vec2 d = abs(pt) - extent2;\n"
"    float ratio = min(max(d.x, d.y), 0.f) + length(max(d, 0.0)) - radius;\n"
"    color = mix( inner_color, outer_color, clamp((ratio + feather*0.5)/feather, 0, 1) );\n"
"}\n";

const static char* vg_uniforms[] = {
    "transform", "paint", "inner_color", "outer_color", "extent", "radius", "feather"
};
