// @date 2016/07/15
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>

NS_FLOW2D_BEGIN

template<typename T, size_t size> struct VTraitComponent : public Component<size>
{
    struct Trait : public Component<size>
    {
        INLINE T* operator -> () { return _instance; }
        INLINE const T* operator -> () const { return _instance; }

    protected:
        friend class VTraitComponent;
        T* _instance;
    };

    virtual void on_spawn(EntityManager& world, Entity object)
    {
        if( !world.template has_component<Trait>(object) )
            world.template add_component<Trait>(object);

        auto vt = world.template get_component<Trait>(object);
        ASSERT( vt->_instance == nullptr, "duplicated component instance inherited from T." );
        vt->_instance = static_cast<T*>(this);
    }

    virtual void on_dispose(EntityManager& world, Entity object)
    {
        auto vt = world.template get_component<Trait>(object);
        ENSURE( vt != nullptr && vt->_instance == static_cast<T*>(this) );
        vt->_instance = nullptr;
        world.template remove_component<Trait>(object);
    }
};

NS_FLOW2D_END