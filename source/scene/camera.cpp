// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/camera.hpp>
#include <scene/transform.hpp>

NS_LEMON_BEGIN

math::Matrix4f Camera::get_view_matrix(Transform& transform)
{
    auto position = transform.get_position(TransformSpace::WORLD);
    auto center = transform.get_forward() + position;
    auto up = transform.get_up();

    return math::look_at(position, center, up);
}


NS_LEMON_END