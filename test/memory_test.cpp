#include <catch.hpp>
#include <flow2d.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>

USING_NS_FLOW2D;

const static size_t kChunkSize = 8;

struct MemoryChunksFixture : MemoryChunks<size_t>
{
    MemoryChunksFixture() : MemoryChunks<size_t>(sizeof(int32_t), kChunkSize) {}

    size_t get_memory_chunks() const
    {
        return _chunks.size();
    }

    size_t get_memory_capacity() const
    {
        return _total_elements;
    }

    size_t get_available() const
    {
        return _available;
    }

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
        std::vector<size_t> indices;
        int holes = std::rand()%kChunkSize;

        for( size_t i = 0; i < kChunkSize - holes; i++ )
        {
            size_t index = malloc();
            *(int32_t*)get_element(index) = kChunkSize * 1000 +i;
            indices.push_back(index);
        }

        REQUIRE( size() == kChunkSize - holes );
        REQUIRE( get_memory_chunks() == 1 );
        REQUIRE( get_memory_capacity() == kChunkSize );
        REQUIRE( get_available() == holes );

        for( size_t i = 0; i < kChunkSize-holes; i++ )
        {
            int ra = std::rand()%indices.size();
            REQUIRE( *(int32_t*)get_element(indices[ra]) >= (int32_t)(kChunkSize * 1000) );

            free(indices[ra%indices.size()]);
            REQUIRE( *(int32_t*)get_element(indices[ra]) < (int32_t)(kChunkSize * 1000) );

            indices.erase(indices.begin()+(ra%indices.size()));
        }
    }

    // multi-chunks
    int max_chunks = 0;
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        int chunks = std::rand()%3+1;
        int holes = std::rand()%kChunkSize;
        max_chunks = std::max(chunks, max_chunks);

        std::vector<size_t> indices;
        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            size_t index = malloc();
            *(int32_t*)get_element(index) = -1;
            indices.push_back(index);
        }

        REQUIRE( size() == kChunkSize*chunks - holes );
        REQUIRE( get_memory_chunks() == max_chunks );
        REQUIRE( get_memory_capacity() == kChunkSize*max_chunks );

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand();
            free(indices[ra%indices.size()]);
            // printf("free %ld \n", indices[ra%indices.size()]);
            indices.erase(indices.begin()+(ra%indices.size()));
        }

        REQUIRE( size() == 0 );
        REQUIRE( get_memory_chunks() == max_chunks );
        REQUIRE( get_memory_capacity() == kChunkSize*max_chunks );
    }
}

struct Position : public Component<>
{
    Position(int x, int y) : x(x), y(y) {}
    int x, y;
};

struct ObjectChunksFixture : ComponentChunksTrait<Position>
{
    EntityManager   world;
    EventManager    dispatcher;
    size_t          spawn_count = 0;
    size_t          dispose_count = 0;

    ObjectChunksFixture()
    : ComponentChunksTrait<Position>(kChunkSize)
    {
        when_spawn = [&](Entity, Position&) { spawn_count ++; };
        when_dispose = [&](Entity, Position&) { dispose_count ++; };
    }

    size_t capacity() const
    {
        return _memory_indices.capacity();
    }

    size_t get_plain_index(size_t index) const
    {
        return _memory_indices[index];
    };

    size_t get_memory_chunks() const
    {
        return _chunks.size();
    }

    size_t get_memory_capacity() const
    {
        return _total_elements;
    }
};

TEST_CASE_METHOD(ObjectChunksFixture, "TestObjectChunks")
{
    REQUIRE( capacity() == 0 );
    REQUIRE( size() == 0 );
    REQUIRE( get_memory_chunks() == 0 );
    REQUIRE( get_memory_capacity() == 0 );

    std::vector<Entity> entities;
    for( size_t i = 0; i < kChunkSize*3; i++ )
        entities.push_back(world.spawn());

    resize(kChunkSize*3); // this will changes nothing but the capacity of _memory_indices
    REQUIRE( capacity() == kChunkSize*3 );
    REQUIRE( size() == 0 );
    REQUIRE( get_memory_chunks() == 0 );
    REQUIRE( get_memory_capacity() == 0 );

    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = (Position*)spawn(entities[i], i, i*2);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = get(entities[i]);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    REQUIRE( size() == (kChunkSize-1) );
    REQUIRE( spawn_count == size() );
    REQUIRE( dispose_count == 0 );
    REQUIRE( get_memory_chunks() == 1 );
    REQUIRE( get_memory_capacity() == kChunkSize );

    for( size_t i = kChunkSize-1; i < kChunkSize*2; i++ )
        spawn(entities[i], i, i*2);

    for( size_t i = 0; i < kChunkSize*2; i++ )
    {
        auto p = get(entities[i]);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    REQUIRE( spawn_count == size() );
    REQUIRE( dispose_count == 0 );
    REQUIRE( size() == kChunkSize*2 );
    REQUIRE( get_memory_chunks() == 2 );
    REQUIRE( get_memory_capacity() == kChunkSize*2 );

    size_t holes = 0;
    std::set<size_t> removed;
    for( size_t i = 0; i < kChunkSize*2; i++ )
    {
        if( i % 2 == 0 || i % 3 == 0 || i % 7 == 0 )
        {
            dispose(entities[i]);
            removed.insert(i);
            holes ++;
        }
    }

    REQUIRE( dispose_count == holes );
    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( get_memory_chunks() == 2 );
    REQUIRE( get_memory_capacity() == kChunkSize*2 );

    // block reuse
    for( auto cursor = removed.begin(); cursor != removed.end(); cursor++ )
    {
        if( *cursor % 7 == 0 && *cursor % 2 != 0 && *cursor % 3 != 0 )
        {
            spawn(entities[*cursor], 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( get_memory_chunks() == 2 );
    REQUIRE( get_memory_capacity() == kChunkSize*2 );

    for( auto cursor = removed.begin(); cursor != removed.end(); cursor++ )
    {
        if( *cursor % 3 == 0 && *cursor % 7 != 0 && *cursor % 2 != 0 )
        {
            spawn(entities[*cursor], 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( get_memory_chunks() == 2 );
    REQUIRE( get_memory_capacity() == kChunkSize*2 );
}
