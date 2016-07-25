// @date 2016/06/27
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <ui/ui.hpp>
#include <core/components.hpp>
#include <scene/transform.hpp>
#include <math/vector.hpp>
#include <math/rect.hpp>
#include <math/color.hpp>

NS_FLOW2D_UI_BEGIN

enum class WidgetEdge : uint8_t
{
    LEFT    = 0,
    RIGHT   = 1,
    TOP     = 2,
    BOTTOM  = 3
};

struct Widget : public ComponentWithEnvironment<>
{
    void on_spawn(EntityManager&, Entity) override;

    // check if position is inside this widget
    bool is_inside(const Vector2f&, TransformSpace space = TransformSpace::SELF) const;

    // calculate/return bounds of this widget
    void perform_resize(const Rect2f&);

    // get_bounds of widget in specified transform space,
    // bounds in local-space would be returned if target == nullptr.
    Rect2f get_bounds_in(Transform* target = nullptr) const;
    Rect2f get_bounds_in_world() const;

    // visibility of this widget and its children
    bool is_visible(bool recursive = false);
    void set_visible(bool);

    // location of the pivot point around which the rectangle rotates,
    // defined as a fraction of the size of the rectangle itself.
    void     set_anchor(const Vector2f&);
    Vector2f get_anchor() const;

    // custom size in local space, might be changed if on_resize happens
    void     set_custom_size(const Vector2f&);
    Vector2f get_custom_size() const;

    // positions of the rectangle’s edges relative to parent
    void set_margin(WidgetEdge, float);

protected:
    bool        _visible        = true;
    Vector2f    _custom_size    = { 0.f, 0.f };
    Vector2f    _anchor         = { 0.5f, 0.5f };
    // [optional] gives hints about positioning and sizing preferences when on_resize happens
    float       _margin[4]      = { math::nan<float>() };
};

struct Container : public VTraitComponent<Container, kUiComponentsChunkSize>
{
    virtual void on_format(Transform&, Rect2f&) = 0;
};

struct View : public VTraitComponent<View, kUiComponentsChunkSize>
{
    virtual void on_spawn(EntityManager&, Entity) override;
    virtual void on_dispose(EntityManager&, Entity) override;

    virtual void on_update(float) = 0;
    virtual void on_draw(Canvas&) = 0;

    template<typename T, typename ... Args> INLINE T* add_component(Args && ... args)
    {
        return _world->add_component(_object, std::forward<Args>(args)...);
    }

    template<typename T> INLINE T* get_component()
    {
        return _world->get_component<T>(_object);
    }

    template<typename T> INLINE void remove_component()
    {
        _world->remove_component<T>(_object);
    }

    template<typename T> INLINE bool has_component() const
    {
        return _world->has_component<T>(_object);
    }

protected:
    EntityManager*  _world;
    Entity          _object;
};

NS_FLOW2D_UI_END