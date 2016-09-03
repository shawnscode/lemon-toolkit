#include <catch.hpp>
#include <lemon-toolkit.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>

USING_NS_LEMON_CORE;

const static size_t kChunkSize = 8;

struct MemoryChunksFixture : public MemoryChunks
{
    MemoryChunksFixture() : MemoryChunks(sizeof(int32_t), kChunkSize) {}

    size_t get_memory_chunks() const { return _chunks.size(); }
    size_t get_memory_capacity() const { return _total_elements; }
    size_t get_available() const { return _available; }

    void dump() const
    {
        for( size_t i = 0; i < _chunks.size(); i++ )
        {
            printf("chunk %ld:\n", i);
            for( size_t j = 0; j < _chunk_size; j++ )
                printf("%d ", *(int32_t*)get_element(_chunk_size*i+j));
            printf("\n");
        }
    }
};

TEST_CASE_METHOD(MemoryChunksFixture, "TestMemoryChunksReuse")
{
    const size_t kIterationCount = 32;

    // use current time as seed for random generator
    std::srand(std::time(0));

    //
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        std::vector<void*> ptrs;
        int holes = std::rand()%kChunkSize;

        for( size_t i = 0; i < kChunkSize - holes; i++ )
        {
            ptrs.push_back(malloc());
            *(int32_t*)ptrs.back() = kChunkSize * 1000 + i;
        }

        REQUIRE( size() == kChunkSize - holes );
        REQUIRE( get_memory_chunks() == 1 );
        REQUIRE( get_memory_capacity() == kChunkSize );
        REQUIRE( get_available() == holes );

        for( size_t i = 0; i < kChunkSize-holes; i++ )
        {
            int ra = std::rand()%ptrs.size();
            REQUIRE( *(int32_t*)ptrs[ra] >= (int32_t)(kChunkSize * 1000) );

            free(ptrs[ra]);
            REQUIRE( *(int32_t*)ptrs[ra] < (int32_t)(kChunkSize * 1000) );

            ptrs.erase(ptrs.begin()+ra);
        }
    }

    // multi-chunks
    int max_chunks = 0;
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        int chunks = std::rand()%5+1;
        int holes = std::rand()%kChunkSize;
        max_chunks = std::max(chunks, max_chunks);

        std::vector<void*> ptrs;
        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
            ptrs.push_back(malloc());

        REQUIRE( size() == kChunkSize*chunks - holes );
        REQUIRE( get_memory_chunks() == max_chunks );
        REQUIRE( get_memory_capacity() == kChunkSize*max_chunks );

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand()%ptrs.size();
            free(ptrs[ra]);
            ptrs.erase(ptrs.begin()+ra);
        }

        REQUIRE( size() == 0 );
        REQUIRE( get_memory_chunks() == max_chunks );
        REQUIRE( get_memory_capacity() == kChunkSize*max_chunks );
    }
}

struct Context;
struct IntPosition : public Component
{
    SET_CHUNK_SIZE(kChunkSize);

    IntPosition(Context&, int x, int y);
    ~IntPosition();

    int x, y;
    Context& fixture;
};

struct Context
{
    size_t spawn_count = 0;
    size_t dispose_count = 0;

    Context()
    {
        event::initialize();
        subsystem::initialize();
        ecs::initialize();
    }

    ~Context()
    {
        ecs::dispose();
        subsystem::dispose();
        event::dispose();
    }
    
    void reset()
    {
        std::srand(std::time(0));
        spawn_count = 0;
        dispose_count = 0;
    }

    size_t size() const
    {
        const auto id = TypeInfo::id<Component, IntPosition>();
        return ecs::test_mem::size(id);
    }

    size_t capacity() const
    {
        const auto id = TypeInfo::id<Component, IntPosition>();
        return ecs::test_mem::capacity(id);
    }
};

IntPosition::IntPosition(Context& f, int x, int y) : x(x), y(y), fixture(f)
{
    fixture.spawn_count ++;
}

IntPosition::~IntPosition()
{
    fixture.dispose_count ++;
}

TEST_CASE_METHOD(Context, "TestIndexedObjectChunks")
{
    reset();

    REQUIRE( size() == 0 );
    REQUIRE( capacity() == 0 );

    std::vector<Entity> entities;
    for( size_t i = 0; i < kChunkSize*3; i++ )
        entities.push_back(spawn());

    REQUIRE( size() == 0 );
    REQUIRE( capacity() == 0 );
    
    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = add_component<IntPosition>(entities[i], *this, i, i*2);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = get_component<IntPosition>(entities[i]);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    REQUIRE( size() == (kChunkSize-1) );
    REQUIRE( spawn_count == size() );
    REQUIRE( dispose_count == 0 );
    REQUIRE( capacity() == kChunkSize );

    for( size_t i = kChunkSize-1; i < kChunkSize*2; i++ )
        add_component<IntPosition>(entities[i], *this, i, i*2);

    for( size_t i = 0; i < kChunkSize*2; i++ )
    {
        auto p = get_component<IntPosition>(entities[i]);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    REQUIRE( spawn_count == size() );
    REQUIRE( dispose_count == 0 );
    REQUIRE( size() == kChunkSize*2 );
    REQUIRE( capacity() == kChunkSize*2 );

    size_t holes = 0;
    std::set<size_t> removed;
    for( size_t i = 0; i < kChunkSize*2; i++ )
    {
        if( i % 2 == 0 || i % 3 == 0 || i % 7 == 0 )
        {
            recycle(entities[i]);
            removed.insert(i);
            holes ++;
        }
    }

    REQUIRE( dispose_count == holes );
    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( capacity() == kChunkSize*2 );

    // block reuse
    for( auto cursor = removed.begin(); cursor != removed.end(); cursor++ )
    {
        if( *cursor % 7 == 0 && *cursor % 2 != 0 && *cursor % 3 != 0 )
        {
            add_component<IntPosition>(spawn(), *this, 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( capacity() == kChunkSize*2 );

    for( auto cursor = removed.begin(); cursor != removed.end(); cursor++ )
    {
        if( *cursor % 3 == 0 && *cursor % 7 != 0 && *cursor % 2 != 0 )
        {
            add_component<IntPosition>(spawn(), *this, 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( capacity() == kChunkSize*2 );
}

TEST_CASE_METHOD(Context, "TestIndexedObjectChunksWithRandomHoles")
{
    reset();
    const size_t kIterationCount = 32;
    // use current time as seed for random generator

    //
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        std::vector<std::pair<Entity, size_t>> indices, shuffle;
        int holes = std::rand()%kChunkSize;

        for( size_t i = 0; i < kChunkSize; i++ )
            shuffle.push_back(std::make_pair(spawn(), i));

        for( size_t i = 0; i < kChunkSize - holes; i++ )
        {
            int ra = std::rand()%shuffle.size();
            add_component<IntPosition>(shuffle[ra].first, *this, shuffle[ra].second, shuffle[ra].second);
            indices.push_back(shuffle[ra]);

            std::swap(shuffle[ra], shuffle[shuffle.size()-1]);
            shuffle.pop_back();
        };

        REQUIRE( size() == kChunkSize - holes );
        REQUIRE( capacity() == kChunkSize );

        for( size_t i = 0; i < kChunkSize-holes; i++ )
        {
            int ra = std::rand()%indices.size();

            auto object = get_component<IntPosition>(indices[ra].first);
            REQUIRE( object != nullptr );
            REQUIRE( object->x == indices[ra].second );
            REQUIRE( object->y == indices[ra].second );

            remove_component<IntPosition>(indices[ra].first);
            REQUIRE( get_component<IntPosition>(indices[ra].first) == nullptr );

            indices.erase(indices.begin()+ra);
        }
    }

    // multi-chunks
    int max_chunks = 0;
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        int chunks = std::rand()%5+1;
        int holes = std::rand()%kChunkSize;
        max_chunks = std::max(chunks, max_chunks);

        std::vector<std::pair<Entity, size_t>> indices, shuffle;
        for( size_t i = 0; i < kChunkSize*chunks; i++ )
            shuffle.push_back(std::make_pair(spawn(), i));

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand()%shuffle.size();
            add_component<IntPosition>(shuffle[ra].first, *this, shuffle[ra].second, shuffle[ra].second);
            indices.push_back(shuffle[ra]);

            std::swap(shuffle[ra], shuffle[shuffle.size()-1]);
            shuffle.pop_back();
        }

        REQUIRE( size() == kChunkSize*chunks - holes );
        REQUIRE( capacity() == kChunkSize*max_chunks );

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand()%indices.size();

            auto object = get_component<IntPosition>(indices[ra].first);
            REQUIRE( object != nullptr );
            REQUIRE( object->x == indices[ra].second );
            REQUIRE( object->y == indices[ra].second );

            remove_component<IntPosition>(indices[ra].first);
            REQUIRE( get_component<IntPosition>(indices[ra].first) == nullptr );

            indices.erase(indices.begin()+ra);
        }

        REQUIRE( size() == 0 );
        REQUIRE( capacity() == kChunkSize*max_chunks );
    }
}
