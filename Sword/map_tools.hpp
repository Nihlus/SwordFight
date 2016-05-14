#ifndef MAP_TOOLS_HPP_INCLUDED
#define MAP_TOOLS_HPP_INCLUDED

#include <vector>

#include "../openclrenderer/logging.hpp"

struct objects_container;

//#include "../openclrenderer/objects_conta

#include "../sword_server/game_server/game_modes.hpp"

/*static std::vector<int>
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
};*/


struct world_map
{
    int width = 0, height = 0;
    std::vector<int> map_def;

    void init(const std::vector<int>& _map, int w, int h);
    void init(int map_id);

    std::function<void(objects_container*)> get_load_func();
};

struct game_mode_exec;

namespace menu_state
{
    enum menu_state
    {
        MENU,
        GAME,
        COUNT
    };
}

typedef menu_state::menu_state menu_t;

///make gamemode implementation a polymorphic struct
struct gameplay_state
{
    world_map current_map;
    game_mode_t current_mode = game_mode::FIRST_TO_X;
    game_mode_exec* mode_exec = nullptr;

    menu_t current_menu;

    void set_map(world_map& m);
    void start_game_mode();

    bool should_end_game_mode();
    void end_game_mode();
};

struct map_cube_info
{
    float angle_offset = 0;
    vec2f current_forward = {0,1};
    vec2f pos_within_plane = {0,0};
    map_namespace::map_cube_t face = map_namespace::BOTTOM;

    vec3f get_current_rotation_unsmoothed(){return map_namespace::map_cube_rotations[face];};

    int which_axis(vec2f absolute_relative_pos, int dim)
    {
        vec2f p = absolute_relative_pos;

        if(p.v[1] >= dim)
            return 1;

        if(p.v[1] < 0)
            return 1;

        if(p.v[0] >= dim)
            return 0;

        if(p.v[0] < 0)
            return 0;

        return -1;
    }

    ///we also need to translate our new position on the plane!
    ///position relative to my plane that I'm querying
    ///needs to handle both being oob
    int get_connection_num(vec2f absolute_relative_pos, int dim)
    {
        vec2f p = absolute_relative_pos;

        if(p.v[1] >= dim)
            return 0;

        if(p.v[1] < 0)
            return 1;

        if(p.v[0] >= dim)
            return 2;

        if(p.v[0] < 0)
            return 3;

        return -1;
    }

    int get_new_face(vec2f absolute_relative_pos, int dim)
    {
        int connection = get_connection_num(absolute_relative_pos, dim);

        if(connection == -1)
            return face;

        return map_namespace::connection_map[face][connection];
    }

    vec2f get_transition_vec(vec2f to_mod, map_namespace::axis_are_flipped mapping_type, int axis)
    {
        if(mapping_type == map_namespace::NO)
        {

        }

        if(mapping_type == map_namespace::NEG)
        {
            to_mod.v[axis] = -to_mod.v[axis];
        }

        if(mapping_type == map_namespace::YES)
        {
            //std::swap(to_mod.v[axis], to_mod.v[1-axis]);
            ///rotate 90

            float intermediate = to_mod.v[axis];

            to_mod.v[axis] = -to_mod.v[1-axis];
            to_mod.v[1-axis] = intermediate;
        }

        if(mapping_type == map_namespace::YES_NEG)
        {
            ///rotate -90?
            float intermediate = to_mod.v[axis];

            to_mod.v[axis] = to_mod.v[1-axis];
            to_mod.v[1-axis] = -intermediate;
        }

        return to_mod;
    }

    /*float get_transition_angle(map_namespace::axis_are_flipped mapping_type)
    {
        using namespace map_namespace;

        if(mapping_type == NEG)
            return M_PI;

        ///90 to the right
        if(mapping_type == YES)
            return -M_PI/2;

        if(mapping_type == YES_NEG)
            return M_PI/2;

        return 0;
    }*/

    vec2f get_new_coordinates(vec2f absolute_relative_pos, int dim)
    {
        int connection = get_connection_num(absolute_relative_pos, dim);

        if(connection == -1)
            return absolute_relative_pos;

        auto mapped_face = map_namespace::connection_map[face][connection];

        auto mapping_type = map_namespace::axis_map[face][connection];

        vec2f to_mod = absolute_relative_pos;

        int axis = which_axis(absolute_relative_pos, dim);

        to_mod = get_transition_vec(to_mod, mapping_type, axis);

        to_mod = modulus_positive(to_mod, dim);

        return to_mod;
    }

    vec3f get_absolute_3d_coords(vec2f loc, int dim)
    {
        float len = dim/2.;

        auto next_plane = get_new_face(loc + pos_within_plane, dim);

        vec2f next_pos = get_new_coordinates(loc + pos_within_plane, dim);

        //vec3f rel_pos = (vec3f){0, -len, 0}.rot({0,0,0}, map_namespace::map_cube_rotations[next_plane]) + (vec3f){0, len, 0};

        ///relative to center of cube
        vec3f relative_to_plane = {next_pos.v[0] - dim/2., -len, next_pos.v[1] - dim/2};

        //printf("rel %f %f %f\n", relative_to_plane.v[0], relative_, relative_to_plane.v[2]);

        //lg::log("rel ", relative_to_plane.v[0], " ", relative_to_plane.v[2]);

        //lg::log("NP ", next_plane);

        ///this is possibly not correct, I've been awake for a while
        vec3f global_pos = relative_to_plane.rot({0,0,0}, map_namespace::map_cube_rotations[next_plane]) + (vec3f){0, len, 0};

        //lg::log("gpos ", global_pos.v[0], " ", global_pos.v[1], " ", global_pos.v[2]);

        return global_pos;
    }

    ///transition forward direction too
    void transition_if_appropriate(int dim)
    {
        auto next_plane = get_new_face(pos_within_plane, dim);

        vec2f next_pos = get_new_coordinates(pos_within_plane, dim);

        int axis = which_axis(pos_within_plane, dim);

        auto connection = get_connection_num(pos_within_plane, dim);

        auto mapping_type = map_namespace::axis_map[face][connection];

        //float mapping_angle = get_transition_angle(mapping_type);

        vec2f new_dir = get_transition_vec(current_forward, mapping_type, axis);

        if(next_plane != face)
        {
            face = (map_namespace::map_cube_t)next_plane;
            pos_within_plane = next_pos;
            //angle_offset += mapping_angle;
            current_forward = new_dir;
        }
    }

    ///call before above
    vec3f transition_camera(vec3f c_rot, int dim)
    {
        auto next_plane = get_new_face(pos_within_plane, dim);

        ///0 = x, 1 = y (local, z global)
        int axis = which_axis(pos_within_plane, dim);

        ///if x oob, rotate around y and vice versa
        vec3f local_axis = {axis, 0, 1-axis};

        ///this seems incorrect
        vec3f global_axis = local_axis.rot({0,0,0}, map_namespace::map_cube_rotations[face]);

        //global_axis = {0, 0, 1};

        ///lets just pretend for the moment, so we can test this all works ;_;
        float angle = M_PI/16;

        vec3f camera_dir = (vec3f){0, 0, 1}.rot({0,0,0}, c_rot);

        if(next_plane != face)
        {
            mat3f mat_aa = axis_angle_to_mat(global_axis, angle);

            mat3f current_camera;

            current_camera.load_rotation_matrix(c_rot);

            ///this combines them successfully... but im not sure thats what we want
            //mat3f rotated = mat_aa * current_camera;

            mat3f rotated = current_camera * mat_aa;

            //vec3f rotated_cdir = mat_aa * camera_dir;

            vec3f new_camera = rotated.get_rotation();

            //vec3f test_mat = mat_from_dir({0,0,0}, rotated_cdir);

            //new_camera = test_mat;

            //new_camera = rotated_cdir.get_euler();

            new_camera = rotated.get_rotation();

            return new_camera;
        }

        return c_rot;
    }
};

///what we really want is a map class
///that returns a load function
///or a loaded object

void load_map(objects_container* obj, const std::vector<int>& map_def, int width, int height);

void load_map_cube(objects_container* obj, const std::vector<std::vector<int>>& map_def, int width, int height);

///xz, where z is y in 2d space
bool is_wall(vec2f world_pos, const std::vector<int>& map_def, int width, int height);
bool rectangle_in_wall(vec2f centre, vec2f dim, const std::vector<int>& map_def, int width, int height);
bool rectangle_in_wall(vec2f centre, vec2f dim, gameplay_state* st);


#endif // MAP_TOOLS_HPP_INCLUDED
