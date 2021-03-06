#ifndef MAP_TOOLS_HPP_INCLUDED
#define MAP_TOOLS_HPP_INCLUDED

#include "fighter.hpp"
#include <vector>
#include "../OpenCLRenderer/logging.hpp"
#include <functional>


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

struct map_cube_info;

struct world_map
{
    map_cube_info* cube_info = nullptr;
    int width = 0, height = 0;
    std::vector<std::vector<int>> map_def;

    //void init(const std::vector<int>& _map, int w, int h);
    void init(int map_id);

    int get_real_dim();

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

///I also wish this were less complex
///next up, we need to implement camera rotation movement
///so, we want to decompose the 3d camera into just the components
///relating to that plane
///in fact, we only want the y rotation component (relative to the plane)
///which should not be impossible to get
struct map_cube_info
{
    ///used for controls, this must be updated and kept in sync with the regular camera
    ///its easier to use a syncd accumulator than calculate it from the actual camera
    float accumulated_forward_angle = 0;

    vec2f current_forward = {1,1};
    vec2f pos_within_plane = {0,0};
    map_namespace::map_cube_t face = map_namespace::BOTTOM;

    ///local x, local y, is flipped z
    vec3i current_forward_with_flip = {0, 1, 1};

    static constexpr float smooth_offset = 400;

    vec3f get_current_rotation_unsmoothed(){return map_namespace::map_cube_rotations[face];};

    /*void accumulate_y_rotation(float r)
    {
        accumulated_forward_angle += r;
    }*/

    float get_y_rot()
    {
        return daccum.v[1];
    }

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

    vec2f get_residual_distance(vec2f absolute_relative_pos, int dim)
    {
        vec2f residual = {0,0};

        vec2f p = absolute_relative_pos;

        if(p.v[1] >= dim)
            residual.v[1] = p.v[1] - dim;

        if(p.v[1] < 0)
            residual.v[1] = p.v[1];

        if(p.v[0] >= dim)
            residual.v[0] = p.v[0] - dim;

        if(p.v[0] < 0)
            residual.v[0] = p.v[0];

        return residual;
    }

    int get_new_face(vec2f absolute_relative_pos, int dim)
    {
        int connection = get_connection_num(absolute_relative_pos, dim);

        if(connection == -1)
            return face;

        return map_namespace::connection_map[face][connection];
    }

    ///I think this is broken between left/right and top
    ///this corresponds to neg
    vec2f get_transition_vec(vec2f to_mod, map_namespace::axis_are_flipped mapping_type, int axis)
    {
        if(axis < 0)
            return to_mod;

        if(mapping_type == map_namespace::NO)
        {

        }

        if(mapping_type == map_namespace::NEG)
        {
            to_mod.v[axis] = -to_mod.v[axis];
            ///this is where the axis flipping metaphor breaks down
            ///only makes sense in coordinate systems
            ///and is not consistent
            ///but because its the only exception which exposes the veneer of a much more
            ///complex solution, I'm not going to fix the underlying concept
            ///as its more complicated (transforming between actual coordinate origin systems)
            ///whereas this generalisation sort of works
            ///technicaly debt hooray!
            to_mod.v[1-axis] = -to_mod.v[1-axis];
        }

        if(mapping_type == map_namespace::YES)
        {
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

    ///ffs we need a matrix to describe the movement
    ///or a 3d vector ;[
    ///hmm, YES seems broken
    vec3i get_direction_transition_vec(vec3i dir, map_namespace::axis_are_flipped mapping_type, int axis)
    {
        if(axis < 0)
            return dir;

        if(mapping_type == map_namespace::NEG)
        {
            dir.v[axis] = -dir.v[axis];
            dir.v[1-axis] = -dir.v[1-axis];
        }

        if(mapping_type == map_namespace::YES)
        {
            float intermediate = dir.v[axis];

            dir.v[axis] = -dir.v[1-axis];
            dir.v[1-axis] = intermediate;
        }

        if(mapping_type == map_namespace::YES_NEG)
        {
            float intermediate = dir.v[axis];

            dir.v[axis] = dir.v[1-axis];
            dir.v[1-axis] = -intermediate;
        }

        return dir;
    }

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

    ///I think we need a smoothed up coordinate ;_;
    vec3f get_absolute_3d_coords(vec2f loc, int dim)
    {
        float len = dim/2.;

        auto next_plane = get_new_face(loc + pos_within_plane, dim);

        vec2f next_pos = get_new_coordinates(loc + pos_within_plane, dim);

        ///relative to center of cube
        vec3f relative_to_plane = {next_pos.v[0] - dim/2., -len, next_pos.v[1] - dim/2};

        ///this is possibly not correct, I've been awake for a while
        vec3f global_pos = relative_to_plane.rot({0,0,0}, map_namespace::map_cube_rotations[next_plane]) + (vec3f){0, len, 0};

        global_pos.v[1] += FLOOR_CONST;

        vec3f smooth_up = get_smooth_up_vector(face, {smooth_offset, smooth_offset}, dim, FLOOR_CONST);

        global_pos = global_pos + smooth_up;

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

        vec2f new_dir = get_transition_vec(current_forward, mapping_type, axis);

        vec3i new_front = get_direction_transition_vec(current_forward_with_flip, mapping_type, axis);

        if(next_plane != face)
        {
            face = (map_namespace::map_cube_t)next_plane;
            pos_within_plane = next_pos;
            //angle_offset += mapping_angle;
            current_forward = new_dir;

            current_forward_with_flip = new_front;
        }
    }

    mat3f accumulated_camera = mat3f().identity();

    ///you know, its actually going to be easier to rotate the whole level
    ///fucking about with the camera is becoming pretty difficult
    ///call before above
    mat3f get_transition_camera(mat3f mat_in, int dim, vec2f offset = {0,0})
    {
        auto next_plane = get_new_face(pos_within_plane + offset, dim);

        ///0 = x, 1 = y (local, z global)
        int axis = which_axis(pos_within_plane + offset, dim);

        ///if x oob, rotate around y and vice versa
        vec3f local_axis = {axis, 0, 1-axis};

        ///this seems incorrect
        vec3f global_axis = local_axis.rot({0,0,0}, map_namespace::map_cube_rotations[face]);

        int connection = get_connection_num(pos_within_plane + offset, dim);

        float angle = 0;

        ///+ve y
        if(connection == 0)
            angle = (float)M_PI/2.0f;
        if(connection == 1)
            angle = (float)-M_PI/2.0f;
        if(connection == 2)
            angle = (float)-M_PI/2.0f;
        if(connection == 3)
            angle = (float)M_PI/2.0f;


        ///recombine rotated up and forward. We might be able to construct a proper
        ///euler thing out of a separated up and forward
        ///hmm, forward isn't perpendicular
        ///we're getting up wrong as well, we need to take the component of the vector along up, not the 1 length up vector
        ///i think its dot or cross product, or skew or something

        ///we only need to rotate z
        if(next_plane != face)
        {
            ///fine, just do axis angle about camera, need to calculate correct
            ///roll angle
            mat3f mat_aa = axis_angle_to_mat(global_axis, angle);

            mat_in = mat_in * mat_aa;
        }

        return mat_in;
    }

    void transition_camera(int dim)
    {
        accumulated_camera = get_transition_camera(accumulated_camera, dim);

        transition_if_appropriate(dim);
    }

    vec3f get_up_vector(int face)
    {
        vec3f face_rot = map_namespace::map_cube_rotations[face];

        ///pointing downwards
        vec3f center_vec = (vec3f){0, -1, 0}.rot({0,0,0}, face_rot);

        ///we want pointing to center;
        center_vec = -center_vec;

        return center_vec;
    }

    ///this should return a normalised vector
    ///stretched vector, ie 1 is normal normal, > 1 means to stretch by length of returned vector
    ///NOTHING WORTH EASY IS DOING
    vec3f get_smooth_up_vector(int face, vec2f offset, int dim, float height_offset)
    {
        height_offset = (float)fabs(height_offset);

        vec2f xoffset = {offset.v[0], 0};
        vec2f yoffset = {0, offset.v[1]};

        vec<4, vec2f> offsets = {xoffset, -xoffset, yoffset, -yoffset};
        vec<4, vec2f> absolute_offsets = {xoffset, -xoffset, yoffset, -yoffset};

        for(int i=0; i<4; i++)
        {
            absolute_offsets.v[i] = absolute_offsets.v[i] + pos_within_plane;
        }

        std::function<int(vec2f)> func = std::bind(&map_cube_info::get_new_face, this, std::placeholders::_1, dim);

        vec4i new_faces = absolute_offsets.map(func);

        ///float get_axis_frac(vec2f offset, int dim)

        std::function<float(vec2f)> axis_frac = std::bind(&map_cube_info::get_axis_frac, this, std::placeholders::_1, dim);

        vec4f residuals = offsets.map(axis_frac);

        residuals = fabs(residuals);

        vec2f selected_residuals = {residuals.v[0], residuals.v[2]};
        vec2i selected_faces = {new_faces.v[0], new_faces.v[2]};

        ///swap
        if(selected_residuals.v[0] <= 0.001f)
        {
            selected_residuals.v[0] = residuals.v[1];
            selected_faces.v[0] = new_faces.v[1];
        }

        if(selected_residuals.v[1] <= 0.001f)
        {
            selected_residuals.v[1] = residuals.v[3];
            selected_faces.v[1] = new_faces.v[3];
        }

        selected_residuals = selected_residuals / 2.f;

        ///floor const = -633, which is why this doesn't work right as its > offset
        vec<2, vec3f> up_vecs;

        for(int i=0; i<2; i++)
        {
            up_vecs.v[i] = get_up_vector(selected_faces.v[i]) * height_offset;
        }

        ///fit a bezier curve through the 4 points
        ///or maybe just fit a curve through sqrtf(floor_const*floor_const) at the corner?
        ///fit a circle?
        ///something incorrect in this function
        vec3f cur_up = get_up_vector(face) * height_offset;

        vec3f ip_x = up_vecs.v[0] * selected_residuals.v[0] + cur_up * (1.f - selected_residuals.v[0]);

        ip_x = ip_x.norm() * height_offset;

        vec3f ip_y = up_vecs.v[1] * selected_residuals.v[1] + ip_x * (1.f - selected_residuals.v[1]);

        ip_y = ip_y.norm() * height_offset;

        ///working rectangularisation of the camera movement to map it to a straight
        ///box
        ///now map to circle
        float cangle = dot(cur_up.norm(), ip_y.norm());

        float angle = (float)acos(cangle);

        float tangle = (float)tan(angle);

        float odist = height_offset * tangle;

        float vector_length = sqrtf(odist * odist + height_offset * height_offset);

        ip_y = ip_y.norm() * vector_length;


        ///to circle
        float adjacent = offset.largest_elem() - height_offset;

        float hypot = adjacent / cangle;

        float extra = hypot - adjacent;

        float total = extra + ip_y.length();

        ip_y = ip_y.norm() * total;

        return ip_y;
    }

    vec3f daccum = {0,0,0};

    mat3f get_rotation_of(mat3f camera_acc, int face, vec3f cam)
    {
        vec3f txaxis = camera_acc.transp() * (vec3f){1, 0, 0};

        vec3f yaxis = get_up_vector(face);

        mat3f XRot, YRot;

        ///work out global axis including accumulated
        XRot = axis_angle_to_mat(txaxis, cam.v[0]);
        YRot = axis_angle_to_mat(yaxis, cam.v[1]);

        mat3f test_camera = camera_acc * XRot * YRot;

        return test_camera;
    }

    mat3f get_camera_with_offset(vec2f offset, int dim)
    {
        auto plane = get_new_face(pos_within_plane + offset, dim);

        mat3f faccumulated_camera = get_transition_camera(accumulated_camera, dim, offset);

        mat3f rtest_camera = get_rotation_of(faccumulated_camera, plane, daccum);

        return rtest_camera;
    }

    mat3f get_accum_with_offset(vec2f offset, int dim)
    {
        auto plane = get_new_face(pos_within_plane + offset, dim);

        mat3f faccumulated_camera = get_transition_camera(accumulated_camera, dim, offset);

        return faccumulated_camera;

        //mat3f rtest_camera = get_rotation_of(faccumulated_camera, plane, daccum);

        //return rtest_camera;
    }

    ///inf why
    vec2f get_rfrac(vec2f offset, int dim)
    {
        vec2f residual_offset = get_residual_distance(pos_within_plane + offset, dim);

        return fabs(residual_offset) / offset.largest_elem();
    }

    float get_axis_frac(vec2f offset, int dim)
    {
        int axis = which_axis(pos_within_plane + offset, dim);

        if(axis == -1)
            return 0;

        return get_rfrac(offset, dim).v[axis];
    }

    quat get_interpolate(quat base, vec2f offset, int dim)
    {
        mat3f rtest_camera = get_camera_with_offset(offset, dim);

        float axis_frac = get_axis_frac(offset, dim);

        quat q2;

        q2.load_from_matrix(rtest_camera);

        quat qi = quat::slerp(base, q2, axis_frac);

        return qi;
    }

    ///-> top connection is broken, camera gets flipped 180 inappropriately
    ///nope, its a position error when going from left/right -> top
    std::tuple<quat, float> get_ip_camera(vec2f offset, int dim)
    {
        int rplane = get_new_face(pos_within_plane + offset, dim);
        int lrplane = get_new_face(pos_within_plane - offset, dim);

        mat3f rtest_camera = get_camera_with_offset(offset, dim);
        mat3f lrtest_camera = get_camera_with_offset(-offset, dim);

        ///if plane < rplane
        ///0 -> 0.5
        ///else 1 -> 0.5
        ///equivalent to the flip of coordinates from the other perspective
        ///so its fine for both to go 0 -> 0.5

        float axis_frac = get_axis_frac(offset, dim);
        float laxis_frac = get_axis_frac(-offset, dim);

        axis_frac = (float)fabs(axis_frac);

        if(axis_frac <= 0.001f)
        {
            axis_frac = laxis_frac;
            rtest_camera = lrtest_camera;
            rplane = lrplane;
        }

        axis_frac = (float)fabs(axis_frac);

        axis_frac /= 2.f;

        quat nquat;
        nquat.load_from_matrix(rtest_camera);

        return std::tie(nquat, axis_frac);
    }

    std::tuple<quat, float> get_ip_accumulate(vec2f offset, int dim)
    {
        int rplane = get_new_face(pos_within_plane + offset, dim);
        int lrplane = get_new_face(pos_within_plane - offset, dim);

        mat3f rtest_camera = get_accum_with_offset(offset, dim);
        mat3f lrtest_camera = get_accum_with_offset(-offset, dim);

        ///if plane < rplane
        ///0 -> 0.5
        ///else 1 -> 0.5
        ///equivalent to the flip of coordinates from the other perspective
        ///so its fine for both to go 0 -> 0.5

        float axis_frac = get_axis_frac(offset, dim);
        float laxis_frac = get_axis_frac(-offset, dim);

        axis_frac = (float)fabs(axis_frac);

        if(axis_frac <= 0.001f)
        {
            axis_frac = laxis_frac;
            rtest_camera = lrtest_camera;
            rplane = lrplane;
        }

        axis_frac = (float)fabs(axis_frac);

        axis_frac /= 2.f;

        quat nquat;
        nquat.load_from_matrix(rtest_camera);

        return std::tie(nquat, axis_frac);
    }

    ///basically euler is now daccum
    ///front y rotation is now euler.v...[1]?
    vec3f get_update_smoothed_camera_with_euler_offset(vec3f euler, int dim)
    {
        daccum = euler;

        mat3f test_camera = get_rotation_of(accumulated_camera, face, daccum);

        quat qbase;
        qbase.load_from_matrix(test_camera);

        quat rquat, lquat;
        float raxis_frac, laxis_frac;

        std::tie(rquat, raxis_frac) = get_ip_camera({smooth_offset, 0}, dim);
        std::tie(lquat, laxis_frac) = get_ip_camera({0, smooth_offset}, dim);

        quat qi;

        qi = quat::slerp(qbase, rquat, raxis_frac);

        qi = quat::slerp(qi, lquat, laxis_frac);

        mat3f ipmatrix = qi.get_rotation_matrix();

        vec3f camera = ipmatrix.get_rotation();

        return camera;
    }

    ///get_ip_camera not absolute, still getting actual camera
    ///not absolute
    ///ie using daccum :[
    mat3f get_smoothed_accumulate(int dim)
    {
        mat3f test_camera = accumulated_camera;

        quat qbase;
        qbase.load_from_matrix(test_camera);

        quat rquat, lquat;
        float raxis_frac, laxis_frac;

        std::tie(rquat, raxis_frac) = get_ip_accumulate({smooth_offset, 0}, dim);
        std::tie(lquat, laxis_frac) = get_ip_accumulate({0, smooth_offset}, dim);

        quat qi;

        qi = quat::slerp(qbase, rquat, raxis_frac);

        qi = quat::slerp(qi, lquat, laxis_frac);

        mat3f ipmatrix = qi.get_rotation_matrix();

        return ipmatrix;
    }

    vec2f transform_move_dir(vec2f mov_dir)
    {
        vec2f rvec = conv<int, float>(current_forward_with_flip.xy());

        rvec = rvec.rot(get_y_rot());

        ///so uuh. The axis axis is flipped, and im not 100% on why
        vec2f ymove = rvec;
        vec2f xmove = -perpendicular(rvec);

        vec2f nmov = {0,0};

        nmov = xmove * mov_dir.v[0];
        nmov = nmov + ymove * mov_dir.v[1];

        return nmov;
    }

    vec2f transform_move_dir_no_rot(vec2f mov_dir)
    {
        vec2f rvec = conv<int, float>(current_forward_with_flip.xy());

        vec2f ymove = rvec;
        vec2f xmove = -perpendicular(rvec);

        vec2f nmov = {0,0};

        nmov = xmove * mov_dir.v[0];
        nmov = nmov + ymove * mov_dir.v[1];

        return nmov;
    }

    ///internal local pos
    void translate_internal_pos(vec2f diff)
    {
        pos_within_plane = pos_within_plane + diff;
    }

    /*struct cube_output
    {
        vec3f pos;
        vec3f camera_rot;
    };

    cube_output tick_3d_cube(vec3f euler_offset_in, vec2f local_mov_dir, float move_speed, int dim)
    {
        cube_output ret;

        vec3f ncamera_rot = get_smoothed_camera_with_euler_offset(-euler_offset_in, dim);

        transition_camera(dim);

        vec3f global_pos = debug_map_cube.get_absolute_3d_coords((vec2f){-0, -0}, 24 * game_map::scale);

        vec2f nmov = transform_move_dir(local_mov_dir);

        translate_internal_pos(nmov * move_speed);

        return {global_pos, ncamera_rot};
    }*/

    ///mat -> euler has a nan in the obvious place
    ///frametimes etc
    ///when we transition between planes
    ///uuh
    ///uiuuuhh.. store transition matrix and apply to camera?
    /*vec3f do_keyboard_input(int dim)
    {
        using namespace map_namespace;

        sf::Keyboard key;

        vec3f rdir = {0,0,0};

        rdir.v[0] += key.isKeyPressed(sf::Keyboard::Up);
        rdir.v[0] -= key.isKeyPressed(sf::Keyboard::Down);

        rdir.v[1] += key.isKeyPressed(sf::Keyboard::Left);
        rdir.v[1] -= key.isKeyPressed(sf::Keyboard::Right);

        rdir.v[2] += key.isKeyPressed(sf::Keyboard::Insert);
        rdir.v[2] -= key.isKeyPressed(sf::Keyboard::Delete);

        rdir = rdir * 0.01f;

        daccum = daccum + rdir;

        //accumulate_y_rotation(rdir.v[1]);

        mat3f test_camera = get_rotation_of(accumulated_camera, face, daccum);

        quat qbase;
        qbase.load_from_matrix(test_camera);

        quat rquat, lquat;
        float raxis_frac, laxis_frac;

        std::tie(rquat, raxis_frac) = get_ip_camera({smooth_offset, 0}, dim);
        std::tie(lquat, laxis_frac) = get_ip_camera({0, smooth_offset}, dim);

        quat qi;

        qi = quat::slerp(qbase, rquat, raxis_frac);

        qi = quat::slerp(qi, lquat, laxis_frac);

        mat3f ipmatrix = qi.get_rotation_matrix();

        vec3f camera = ipmatrix.get_rotation();

        return camera;
    }*/
};

///what we really want is a map class
///that returns a load function
///or a loaded object

void load_map(objects_container* obj, const std::vector<int>& map_def, int width, int height);

void load_map_cube(objects_container* obj, const std::vector<std::vector<int>>& map_def, int width, int height);

///xz, where z is y in 2d space
bool is_wall(vec2f world_pos, const std::vector<int>& map_def, int width, int height);
///ok, update me to work with new cubemap system
bool rectangle_in_wall(vec2f centre, vec2f dim, const std::vector<int>& map_def, int width, int height);
bool rectangle_in_wall(vec2f centre, vec2f dim, gameplay_state* st);


#endif // MAP_TOOLS_HPP_INCLUDED
