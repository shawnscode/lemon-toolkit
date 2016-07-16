// @date 2016/07/04
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/ui/button.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

void Button::on_update(float dt)
{
    if( _fade_ticker > 0 )
    {
        _fade_ticker -= dt;
        _fade_ticker = std::max(_fade_ticker, 0.f);
    }
}

void Button::set_fade_duration(float duration)
{
    _fade_duration = duration;
}

void Button::on_draw(Canvas& canvas, const Rect2f& bounds)
{
    canvas.save();

    canvas.begin_path();
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

NS_FLOW2D_UI_END