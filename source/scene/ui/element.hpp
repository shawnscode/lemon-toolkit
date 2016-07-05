// @date 2016/06/27
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <math/vector.hpp>
#include <math/rect.hpp>
#include <math/color.hpp>

NS_FLOW2D_BEGIN

const static size_t kUiComponentsChunkSize = 128;

struct UIElement : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

    bool is_visible(bool recursive = false) const;
    void set_visible(bool);

    void        set_fixed_size(const Vector2f&);
    bool        has_fixed_size() const;

    Vector2f    get_fixed_size() const;
    Vector2f    get_prefered_size() const;
    Vector2f    get_size() const;
    Rect2f      get_bounds() const;

    void rearrange();
    void draw(Canvas&);

protected:
    Transform*      _transform  = nullptr;
    Vector2f        _pivot      = {0.f, 0.f};
    Vector2f        _size       = {5.f, 5.f};
    Vector2f        _fixed_size = kVector2fNan;
    Rect2f          _anchor     = {0.f, 0.f, 0.f, 0.f};
    bool            _visible    = false;

    friend class ILayout;
    friend class IViewController;
    ILayout*            _layout = nullptr;
    IViewController*    _view   = nullptr;
};

struct EvtMouseDrag {};
struct EvtMouseClick {};
struct EvtFocusChanged {};
struct EvtKeyboardCharacter {};

struct IViewController : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override final;
    void on_dispose(EntityManager&, Entity) override final;

    virtual void draw(UIElement&, Canvas&) = 0;
    virtual void update(float) {};

    virtual void receive(UIElement&, EvtMouseDrag&) {}
    virtual void receive(UIElement&, EvtMouseClick&) {}
    virtual void receive(UIElement&, EvtFocusChanged&) {}
    virtual void receive(UIElement&, EvtKeyboardCharacter&) {}
};

struct ILayout : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override final;
    void on_dispose(EntityManager&, Entity) override final;

    virtual Vector2f get_prefered_size() const = 0;
    virtual void perform(Transform&) {};
};

NS_FLOW2D_END