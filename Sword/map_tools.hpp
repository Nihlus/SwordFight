#ifndef MAP_TOOLS_HPP_INCLUDED
#define MAP_TOOLS_HPP_INCLUDED

#include <vector>

struct objects_container;

static std::vector<int>
map_test =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static std::vector<int>
map_one
{
    1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,1,1,1,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,0,1,0,0,0,1,
    1,0,0,0,1,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,1,1,1,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,
};

void load_map(objects_container* obj, int* map_def, int width, int height);


#endif // MAP_TOOLS_HPP_INCLUDED
