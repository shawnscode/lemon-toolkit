// @date 2016/07/04
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/ui/button.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_BEGIN

void UIButton::update(float dt)
{
    if( _fade_ticker > 0 )
    {
        _fade_ticker -= dt;
        _fade_ticker = std::max(_fade_ticker, 0.f);
    }
}

Vector2f UIButton::get_prefered_size() const
{
    return {20.f, 10.f};
}

void UIButton::set_fade_duration(float duration)
{
    _fade_duration = duration;
}

void UIButton::draw(UIElement& element, Canvas& canvas)
{
    canvas.begin_path();

    canvas.save();

    auto bounds = element.get_bounds();
    canvas.move_to(bounds.position());
    canvas.line_to({bounds[2], bounds[1]});
    canvas.line_to(bounds.corner());
    canvas.line_to({bounds[0], bounds[3]});
    canvas.close_path();

    auto gt = Color::BLUE;
    auto gb = Color::YELLOW;

    auto paint = CanvasPaint()
        .as_linear_gradient(bounds.position(), {bounds[0], bounds[3]}, gb, gt);
    canvas.set_fill_paint(paint);
    canvas.fill();

    canvas.restore();
}

NS_FLOW2D_END