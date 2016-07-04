// @date 2016/06/27
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <math/vector.hpp>
#include <math/rect.hpp>

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

    void rearrange();
    void draw(Canvas&);

protected:
    void set_size(const Vector2f&);

    Transform*      _transform  = nullptr;
    Vector2f        _pivot      = {0.f, 0.f};
    Vector2f        _size       = {0.f, 0.f};
    Vector2f        _fixed_size = kVector2fNan;
    Rect2f          _anchor     = {0.f, 0.f, 0.f, 0.f};
    bool            _visible    = false;

    friend class ILayout;
    friend class IVisual;
    friend class IInteraction;

    ILayout*        _layout         = nullptr;
    IVisual*        _visual         = nullptr;
    IInteraction*   _interaction    = nullptr;
};

struct IVisual : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

    virtual void draw(Canvas&) = 0;
};

struct IInteraction : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;
};

struct ILayout : public Component<kUiComponentsChunkSize>
{
    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

    virtual Vector2f get_prefered_size() const = 0;
    virtual void perform(Transform&);
};

NS_FLOW2D_END