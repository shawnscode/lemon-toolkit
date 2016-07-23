// @date 2016/07/20
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/action.hpp>

NS_FLOW2D_BEGIN

ActionExecutor::~ActionExecutor()
{
    if( _task != nullptr )
    {
        delete _task;
        _task = nullptr;
    }
}

void ActionExecutor::run(Action* task, bool override)
{
    if( _task != nullptr )
    {
        if( !override )
        {
            LOGW("duplicated action task.");
            return;
        }
        delete _task;
    }

    _task = task;
    _task->start(*this);
}

void ActionExecutor::stop()
{
    if( _task != nullptr )
    {
        delete _task;
        _task = nullptr;
    }
}

void ActionExecutor::update(float dt)
{
    if( _task != nullptr && !_task->is_finished() )
        _task->update(dt);
}

bool ActionExecutor::is_finished() const
{
    return _task == nullptr ? true : _task->is_finished();
}

std::unordered_map<size_t, MemoryChunks*> Action::memories;

void* Action::operator new(size_t size, MemoryChunks* chunks)
{
    ENSURE( chunks->element_size() == size + sizeof(MemoryChunks*) );
    auto ptrs = chunks->malloc();
    *static_cast<MemoryChunks**>(ptrs) = chunks;
    return static_cast<void*>((static_cast<uint8_t*>(ptrs)+sizeof(MemoryChunks*)));
}

void Action::operator delete(void* v)
{
    v = static_cast<void*>(static_cast<uint8_t*>(v)-sizeof(MemoryChunks*));
    MemoryChunks* p = *static_cast<MemoryChunks**>(v);
    p->free(v);
}

void ActionTransform::start(ActionExecutor& executor)
{
    ASSERT( executor.has_component<Transform>(),
        "trying to perform transformation action on entity without Transform component." );
    _transform = executor.get_component<Transform>();
}

void ActionSystem::update(float dt)
{
    for( auto pair : _entities )
        pair.second->update(dt);
}

NS_FLOW2D_END