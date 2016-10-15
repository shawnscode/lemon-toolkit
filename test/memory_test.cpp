#include <catch.hpp>
#include <hayai.hpp>
#include <lemon-toolkit.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <codebase/memory/memory_pool.hpp>

USING_NS_LEMON;
USING_NS_LEMON_CORE;

const static size_t kChunkSize = 8;

struct MemoryPoolTestContext : public MemoryPoolT<int32_t, kChunkSize>
{
    void* get_element(size_t i) const
    {
        return _chunks[i/_chunk_entries_size]+(i%_chunk_entries_size)*_block_size;
    }

    void dump() const
    {
        printf("first_free_block: %zu\n", _first_free_block);
        for( size_t i = 0; i < _chunks.size(); i++ )
        {
            printf("chunk %ld:\n", i);
            for( size_t j = 0; j < _chunk_entries_size; j++ )
                printf("%d ", *(int32_t*)get_element(_chunk_entries_size*i+j));
            printf("\n");
        }
        printf("\n");
    }
};

TEST_CASE_METHOD(MemoryPoolTestContext, "TestMemoryChunksReuse")
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
       REQUIRE( _chunks.size() == 1 );
       REQUIRE( _available == holes );

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
       {
           ptrs.push_back(malloc());
           *(int32_t*)ptrs.back() = 99;
       }

       REQUIRE( size() == kChunkSize*chunks - holes );
       REQUIRE( _chunks.size() == max_chunks );

       for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
       {
           int ra = std::rand()%ptrs.size();
           free(ptrs[ra]);
           ptrs.erase(ptrs.begin()+ra);
       }

       REQUIRE( size() == 0 );
       REQUIRE( _chunks.size() == max_chunks );
   }
}

struct IndexedPoolTestContext;
struct IntPosition : public Component
{
    // SET_CHUNK_SIZE(kChunkSize);
    IntPosition(IndexedPoolTestContext&, int x, int y);
    ~IntPosition();

    int x, y;
    IndexedPoolTestContext& fixture;
};

struct IndexedPoolTestContext : public core::EntityComponentSystem
{
    IndexedPoolTestContext()
    {
        static const TypeInfo::index_t index = TypeInfo::id<Component, IntPosition>();
        _resolvers.resize(index+1, nullptr);
        _resolvers[index] = new details::ComponentResolverT<IntPosition, kChunkSize>();
    }

    size_t size()
    {
        return resolve<IntPosition>()->size();
    }

    size_t capacity()
    {
        return resolve<IntPosition>()->capacity();
    }

    size_t spawn_count = 0;
    size_t dispose_count = 0;
};

IntPosition::IntPosition(IndexedPoolTestContext& f, int x, int y) : x(x), y(y), fixture(f)
{
    fixture.spawn_count ++;
}

IntPosition::~IntPosition()
{
    fixture.dispose_count ++;
}

TEST_CASE_METHOD(IndexedPoolTestContext, "TestIndexedObjectChunks")
{
    REQUIRE( size() == 0 );
    REQUIRE( capacity() == 0 );

    std::vector<Entity*> entities;
    for( size_t i = 0; i < kChunkSize*3; i++ )
    {
        auto handle = create();
        entities.push_back(get(handle));
    }

    REQUIRE( size() == 0 );
    REQUIRE( capacity() == 0 );
    
    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = entities[i]->add_component<IntPosition>(*this, i, i*2);
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    for( size_t i = 0; i < kChunkSize-1; i++ )
    {
        auto p = entities[i]->get_component<IntPosition>();
        REQUIRE( p->x == i );
        REQUIRE( p->y == i*2 );
    }

    REQUIRE( size() == (kChunkSize-1) );
    REQUIRE( spawn_count == size() );
    REQUIRE( dispose_count == 0 );
    REQUIRE( capacity() == kChunkSize );

    for( size_t i = kChunkSize-1; i < kChunkSize*2; i++ )
    {
        entities[i]->add_component<IntPosition>(*this, i, i*2);
    }

    for( size_t i = 0; i < kChunkSize*2; i++ )
    {
        auto p = entities[i]->get_component<IntPosition>();
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
            free(entities[i]->handle);
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
            auto handle = create();
            get(handle)->add_component<IntPosition>(*this, 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( capacity() == kChunkSize*2 );

    for( auto cursor = removed.begin(); cursor != removed.end(); cursor++ )
    {
        if( *cursor % 3 == 0 && *cursor % 7 != 0 && *cursor % 2 != 0 )
        {
            auto handle = create();
            get(handle)->add_component<IntPosition>(*this, 0, 0);
            holes --;
        }
    }

    REQUIRE( size() == (kChunkSize*2 - holes) );
    REQUIRE( capacity() == kChunkSize*2 );
}

TEST_CASE_METHOD(IndexedPoolTestContext, "TestIndexedObjectChunksWithRandomHoles")
{
    const size_t kIterationCount = 32;
    // use current time as seed for random generator

    //
    for( size_t iteration = 0; iteration < kIterationCount; iteration++ )
    {
        std::vector<std::pair<Entity*, size_t>> indices, shuffle;
        int holes = std::rand()%kChunkSize;

        for( size_t i = 0; i < kChunkSize; i++ )
            shuffle.push_back(std::make_pair(get(create()), i));

        for( size_t i = 0; i < kChunkSize - holes; i++ )
        {
            int ra = std::rand()%shuffle.size();
            shuffle[ra].first->add_component<IntPosition>(*this, shuffle[ra].second, shuffle[ra].second);
            indices.push_back(shuffle[ra]);

            std::swap(shuffle[ra], shuffle[shuffle.size()-1]);
            shuffle.pop_back();
        };

        REQUIRE( size() == kChunkSize - holes );
        REQUIRE( capacity() == kChunkSize );

        for( size_t i = 0; i < kChunkSize-holes; i++ )
        {
            int ra = std::rand()%indices.size();

            auto object = indices[ra].first->get_component<IntPosition>();
            REQUIRE( object != nullptr );
            REQUIRE( object->x == indices[ra].second );
            REQUIRE( object->y == indices[ra].second );

            indices[ra].first->remove_component<IntPosition>();
            REQUIRE( indices[ra].first->get_component<IntPosition>() == nullptr );

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

        std::vector<std::pair<Entity*, size_t>> indices, shuffle;
        for( size_t i = 0; i < kChunkSize*chunks; i++ )
            shuffle.push_back(std::make_pair(get(create()), i));

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand()%shuffle.size();
            shuffle[ra].first->add_component<IntPosition>(*this, shuffle[ra].second, shuffle[ra].second);
            indices.push_back(shuffle[ra]);

            std::swap(shuffle[ra], shuffle[shuffle.size()-1]);
            shuffle.pop_back();
        }

        REQUIRE( size() == kChunkSize*chunks - holes );
        REQUIRE( capacity() == kChunkSize*max_chunks );

        for( size_t i = 0; i < kChunkSize*chunks - holes; i++ )
        {
            int ra = std::rand()%indices.size();

            auto object = indices[ra].first->get_component<IntPosition>();
            REQUIRE( object != nullptr );
            REQUIRE( object->x == indices[ra].second );
            REQUIRE( object->y == indices[ra].second );

            indices[ra].first->remove_component<IntPosition>();
            REQUIRE( indices[ra].first->get_component<IntPosition>() == nullptr );

            indices.erase(indices.begin()+ra);
        }

        REQUIRE( size() == 0 );
        REQUIRE( capacity() == kChunkSize*max_chunks );
    }
}

size_t max_elements = 100;
size_t max_operations = 100000;
size_t max_iterations = 8;

BENCHMARK(MemoryTest, Pool32, max_iterations, 1)
{
    MemoryPoolT<size_t, 32> pool;

    std::list<void*> ptrs;
    for( size_t iteration = 0; iteration < max_operations; iteration ++ )
    {
        if( iteration % 3 == 0 || ptrs.size() < max_elements )
        {
            ptrs.push_back(pool.malloc());
        }
        else
        {
            pool.free(ptrs.front());
            ptrs.pop_front();
        }
    }

    for( void* ptr : ptrs )
        pool.free(ptr);
}

BENCHMARK(MemoryTest, Pool128, max_iterations, 1)
{
    MemoryPoolT<size_t, 128> pool;

    std::list<void*> ptrs;
    for( size_t iteration = 0; iteration < max_operations; iteration ++ )
    {
        if( iteration % 3 == 0 || ptrs.size() < max_elements )
        {
            ptrs.push_back(pool.malloc());
        }
        else
        {
            pool.free(ptrs.front());
            ptrs.pop_front();
        }
    }

    for( void* ptr : ptrs )
        pool.free(ptr);
}

BENCHMARK(MemoryTest, System, max_iterations, 1)
{
    std::list<void*> ptrs;
    for( size_t iteration = 0; iteration < max_operations; iteration ++ )
    {
        if( iteration % 3 == 0 || ptrs.size() < max_elements )
        {
            ptrs.push_back(::malloc(sizeof(size_t)));
        }
        else
        {
            ::free(ptrs.front());
            ptrs.pop_front();
        }
    }

    for( void* ptr : ptrs )
        ::free(ptr);
}
