// @date 2016/07/15
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>

NS_FLOW2D_BEGIN

template<typename T, size_t size = kEntPoolChunkSize> struct VTraitComponent : public Component<size>
{
    struct Trait : public Component<size>
    {
        INLINE T* operator -> () { return _instance; }
        INLINE const T* operator -> () const { return _instance; }

    protected:
        friend class VTraitComponent;
        T* _instance;
    };

    virtual void on_spawn(EntityManager& world, Entity object) override
    {
        if( !world.template has_component<Trait>(object) )
            world.template add_component<Trait>(object);

        auto vt = world.template get_component<Trait>(object);
        ASSERT( vt->_instance == nullptr, "duplicated component instance inherited from T." );
        vt->_instance = static_cast<T*>(this);
    }

    virtual void on_dispose(EntityManager& world, Entity object) override
    {
        auto vt = world.template get_component<Trait>(object);
        ENSURE( vt != nullptr && vt->_instance == static_cast<T*>(this) );
        vt->_instance = nullptr;
        world.template remove_component<Trait>(object);
    }
};

template<size_t size = kEntPoolChunkSize> struct ComponentWithEnvironment : public Component<size>
{
    virtual void on_spawn(EntityManager& world, Entity object) override
    {
        _world = &world;
        _object = object;
    }

    virtual void on_dispose(EntityManager& world, Entity object) override
    {
        _world = nullptr;
        _object = Entity();
    }

    INLINE Entity get_object() const
    {
        return _object;
    }

    template<typename T, typename ... Args>
    INLINE T* add_component(Args && ... args)
    {
        return _world->add_component(_object, std::forward<Args>(args)...);
    }

    template<typename T>
    INLINE T* get_component()
    {
        return _world->get_component<T>(_object);
    }

    template<typename T>
    INLINE void remove_component()
    {
        _world->remove_component<T>(_object);
    }

    template<typename T>
    INLINE bool has_component() const
    {
        return _world->has_component<T>(_object);
    }

protected:
    Entity          _object;
    EntityManager*  _world = nullptr;
};

NS_FLOW2D_END