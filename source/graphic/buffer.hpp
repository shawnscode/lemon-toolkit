// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <graphic/device.hpp>

NS_FLOW2D_BEGIN

template<typename T> struct VertexBuffer
{
    void force_update()
    {
        auto& device = GraphicDevice::instance();
        if( m_id == GraphicDevice::INVALID )
        {
            m_id = device.create_buffer(RenderObject::VERTEX_BUFFER, data.data(), data.size()*sizeof(T));
        }
        else
        {
            device.update_buffer(RenderObject::VERTEX_BUFFER, m_id, data.data(), data.size()*sizeof(T));
        }
    }

    Rid id() const
    {
        return m_id;
    }

    size_t stride() const
    {
        return sizeof(T);
    }

    std::vector<T>  data;

protected:
    Rid m_id = GraphicDevice::INVALID;
};

template<typename T> struct IndexBuffer
{
    void force_update()
    {
        auto& device = GraphicDevice::instance();
        if( m_id == GraphicDevice::INVALID )
        {
            m_id = device.create_buffer(RenderObject::INDEX_BUFFER, data.data(), data.size()*sizeof(T));
        }
        else
        {
            device.update_buffer(RenderObject::INDEX_BUFFER, m_id, data.data(), data.size()*sizeof(T));
        }
    }

    Rid id() const
    {
        return m_id;
    }

    size_t stride() const
    {
        return sizeof(T);
    }

    std::vector<T>  data;

protected:
    Rid m_id = GraphicDevice::INVALID;
};

NS_FLOW2D_END