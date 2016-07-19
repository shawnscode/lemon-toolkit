// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/system.hpp>
#include <math/matrix.hpp>
#include <ui/widget.hpp>

NS_FLOW2D_UI_BEGIN

enum class MouseButton : uint8_t
{
    LEFT    = 0,
    RIGHT   = 1,
    MIDDLE  = 2,
};

const static size_t kMaxMouseButton = 3;

enum class ButtonAction : uint8_t
{
    PRESS = 0,
    RELEASE = 1
};

struct EvtInputMouse
{
    ButtonAction action;
    MouseButton  button;
};

struct EvtInputMousePosition
{
    Vector2f position;
};

struct EvtBase
{
    void consume() { consumed = true; }
    bool is_consumed() const { return consumed; }

protected:
    bool consumed = false;
};

struct EvtMouse : public EvtBase
{
    MouseButton button      = MouseButton::LEFT;
    Vector2f    position    = {0.f, 0.f};   //position in world-space
    Vector2f    delta       = {0.f, 0.f};
    float       pressed     = 0.f;
};

// basic mouse input events, three events below should be invoked in sequence
struct EvtMouseDown : public EvtMouse {};
struct EvtMouseUp : public EvtMouse {};
struct EvtMouseMove : public EvtMouse {};
// caused if mouse actions sequence has been interrupted
struct EvtMouseCancel : public EvtMouse {};
// invoked on a widget after EvtMouseUp if the pointer is still inside its hitarea.
struct EvtMouseClick : public EvtMouse {};
// EvtMouseDrag is sent when the mouse has been moving move than threshold
struct EvtMouseDrag : public EvtMouse {};

template<size_t N> struct EventListener
{
    using closure = std::pair<void*, std::function<void(void*, void*)>>;

    EventListener& operator += (const closure& c);
    EventListener& operator -= (void* ptr);
    void operator () (void*, void*);

protected:
    closure _callbacks[N];
};

struct EventListenerGroup : public Component<kUiComponentsChunkSize>
{
    template<typename T> using func = std::function<void(Transform&, T&)>;
    template<typename T> using closure = std::pair<void*, func<T>>;

    template<typename E, typename T> void subscribe(T&);
    template<typename E, typename T> void subscribe(T&, const func<T>&);
    template<typename E, typename T> void unsubscribe(T&);

    template<typename T> void emit(Transform&, T&);

    // template<typename T> static void subscribe(EntityManager&, Entity, void*, const func<T>&);
    // template<typename T> static void dispatch(EntityManager&, Entity, void*);

protected:
    std::unordered_map<TypeID::index_type, EventListener<kMaxEventListeners>> _listeners;
};

template<size_t N> EventListener<N>& EventListener<N>::operator += (const closure& c)
{
    for( size_t i = 0; i < N; i++ )
    {
        if( _callbacks[i].first == c.first )
        {
            _callbacks[i] = c;
            return *this;
        }
    }

    for( size_t i = 0; i < N; i++ )
    {
        if( _callbacks[i].first == nullptr )
        {
            _callbacks[i] = c;
            return *this;
        }
    }

    LOGW("failed to add event listener.");
    return *this;
}

template<size_t N> EventListener<N>& EventListener<N>::operator -= (void* owner)
{
    for( size_t i = 0; i < N; i++ )
    {
        if( _callbacks[i].first == owner )
        {
            _callbacks[i].first = nullptr;
            _callbacks[i].second = nullptr;
        }
    }
    return *this;
}

template<size_t N> void EventListener<N>::operator () (void* a1, void* a2)
{
    for( size_t i = 0; i < N; i++ )
        if( _callbacks[i].first != nullptr )
            _callbacks[i].second(a1, a2);
}

template<typename E, typename T>
void EventListenerGroup::subscribe(T& owner)
{
    static_assert( std::is_base_of<EvtBase, E>::value, "E is not subtype of EvtBase." );

    auto found = _listeners.find(TypeID::value<EvtBase, E>());
    if( found == _listeners.end() )
        _listeners[TypeID::value<EvtBase, E>()] = EventListener<kMaxEventListeners>();

    _listeners[TypeID::value<EvtBase, E>()] +=
        std::make_pair(static_cast<void*>(&owner), [&](void* t, void* c)
        {
            owner.receive(*static_cast<Transform*>(t), *static_cast<E*>(c));
        });
}

template<typename E, typename T>
void EventListenerGroup::subscribe(T& owner, const func<T>& f)
{
    static_assert( std::is_base_of<EvtBase, E>::value, "E is not subtype of EvtBase." );

    auto found = _listeners.find(TypeID::value<EvtBase, E>());
    if( found == _listeners.end() )
        _listeners[TypeID::value<EvtBase, E>()] = EventListener<kMaxEventListeners>();

    _listeners[TypeID::value<EvtBase, E>()] +=
        std::make_pair(static_cast<void*>(&owner), [&](void* t, void* c)
        {
            f(*static_cast<Transform*>(t), *static_cast<E*>(c));
        });
}

template<typename E, typename T>
void EventListenerGroup::unsubscribe(T& owner)
{
    static_assert( std::is_base_of<EvtBase, E>::value, "E is not subtype of EvtBase." );

    auto found = _listeners.find(TypeID::value<EvtBase, E>());
    if( found != _listeners.end() )
        found->second -= static_cast<void*>(&owner);
}

template<typename E> void EventListenerGroup::emit(Transform& t, E& evt)
{
    static_assert( std::is_base_of<EvtBase, E>::value, "E is not subtype of EvtBase." );

    auto found = _listeners.find(TypeID::value<EvtBase, E>());
    if( found != _listeners.end() )
        found->second(static_cast<void*>(&t), static_cast<void*>(&evt));
}

NS_FLOW2D_UI_END