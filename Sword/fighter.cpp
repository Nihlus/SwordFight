#include "fighter.hpp"
#include "physics.hpp"
#include "../openclrenderer/obj_mem_manager.hpp"
#include <unordered_map>
#include "../openclrenderer/network.hpp"

#include "object_cube.hpp"

#include "particle_effect.hpp"

#include "../openclrenderer/light.hpp"

#include "../openclrenderer/obj_load.hpp"

#include "text.hpp"

/*vec3f jump_descriptor::get_absolute_jump_displacement_tick(float dt, fighter* fight)
{
    if(current_time > time_ms)
    {
        current_time = 0;
        is_jumping = false;

        return {0,0,0};
    }

    if(!is_jumping)
    {
        return {0,0,0};
    }

    vec3f offset = {0, max_height, 0};

    offset.v[0] = time_ms * dir.v[0];
    offset.v[2] = time_ms * dir.v[1];

    float frac = current_time / time_ms;

    offset.v[0] = offset.v[0] * frac;
    offset.v[2] = offset.v[2] * frac;

    offset.v[1] = offset.v[1] * sin(frac * M_PI);

    current_time += dt;

    vec3f final_pos = pre_jump_pos + offset;

    ///hmm
    //vec2f offset_2d = fight->get_wall_corrected_move({pre_jump_pos.v[0], pre_jump_pos.v[2]}, {offset.v[0], offset.v[2]});

    //vec3f final_pos = pre_jump_pos + (vec3f){offset_2d.v[0], offset.v[1], offset_2d.v[2]};

    vec3f diff = final_pos - fight->pos;

    //return (vec3f){offset_2d.v[0], offset.v[1], offset_2d.v[2]};

    vec2f offset_2d = fight->get_wall_corrected_move({fight->pos.v[0], fight->pos.v[2]}, {diff.v[0], diff.v[2]});

    ///need to include the rest of the offset too.. uuh...
    ///hmm. maybe we should have made this iterative instead ;_;
    return {offset_2d.v[0], offset.v[1], offset_2d.v[2]};
}*/

vec3f jump_descriptor::get_relative_jump_displacement_tick(float dt, fighter* fight)
{
    if(current_time > time_ms)
    {
        current_time = 0;
        is_jumping = false;

        return {0,0,0};
    }

    if(!is_jumping)
    {
        return {0,0,0};
    }

    vec3f offset = {0, 0, 0};

    offset.v[0] = dir.v[0];
    offset.v[2] = dir.v[1];

    ///this because im an idiot in walk_dir
    ///and movement speed is dt/2
    vec3f dt_struct = {dt/2.f, 0, dt/2.f};

    offset = offset * dt_struct * last_speed;

    float frac = current_time / time_ms;

    ///move in a sine curve, cos is the differential of sin
    float dh = cos(frac * M_PI);

    offset.v[1] = dh * dt;

    vec2f clamped = fight->get_wall_corrected_move({fight->pos.v[0], fight->pos.v[2]}, {offset.v[0], offset.v[2]});

    offset.v[0] = clamped.v[0];
    offset.v[2] = clamped.v[1];

    //printf("dh %f\n", dh);

    current_time += dt;

    return offset;
}

const vec3f* bodypart::init_default()
{
    using namespace bodypart;

    static vec3f pos[COUNT];

    pos[HEAD] = {0,0,0};

    float x_arm_displacement = 1;
    float y_arm_displacement = 1;

    pos[LUPPERARM] = {-x_arm_displacement, y_arm_displacement, 0.f};
    pos[RUPPERARM] = {x_arm_displacement, y_arm_displacement, 0.f};

    pos[LLOWERARM] = {-x_arm_displacement, y_arm_displacement + 1, 0.f};
    pos[RLOWERARM] = {x_arm_displacement, y_arm_displacement + 1, 0.f};

    pos[LHAND] = {-x_arm_displacement, pos[LLOWERARM].v[1] + 1, 0.f};
    pos[RHAND] = {x_arm_displacement, pos[RLOWERARM].v[1] + 1, 0.f};

    pos[BODY] = {0, 1, 0};

    float x_leg_displacement = 0.5f;

    pos[LUPPERLEG] = {-x_leg_displacement, pos[BODY].v[1] + 2, 0};
    pos[RUPPERLEG] = {x_leg_displacement, pos[BODY].v[1] + 2, 0};

    pos[LLOWERLEG] = {-x_leg_displacement, pos[LUPPERLEG].v[1] + 2, 0};
    pos[RLOWERLEG] = {x_leg_displacement, pos[RUPPERLEG].v[1] + 2, 0};

    pos[LFOOT] = {-x_leg_displacement, pos[LLOWERLEG].v[1] + 1, 0};
    pos[RFOOT] = { x_leg_displacement, pos[RLOWERLEG].v[1] + 1, 0};

    for(size_t i=0; i<COUNT; i++)
    {
        pos[i] = pos[i] * scale;

        pos[i].v[1] = -pos[i].v[1];
    }

    return pos;
}

void part::set_type(bodypart_t t)
{
    type = t;

    set_pos(bodypart::default_position[t]);

    set_hp(1.f);
}

///need to make textures unique optionally
part::part(object_context& context)
{
    //performed_death = false;

    cpu_context = &context;
    model = context.make_new();
    //hp_display = context.make_new();

    is_active = false;

    hp = 1.f;

    set_pos({0,0,0});
    set_rot({0,0,0});

    set_global_pos({0,0,0});
    set_global_rot({0,0,0});

    model->set_file("./Res/bodypart_red.obj");

    model->set_unique_textures(true);

    //model->cache = false;

    team = -1;

    quality = 0;

    ///128x128
    //tex.create(128, 128);
}

part::part(bodypart_t t, object_context& context) : part(context)
{
    set_type(t);
}

part::~part()
{
    model->set_active(false);
}

void part::set_active(bool active)
{
    model->set_active(active);
    //hp_display->set_active(active);

    is_active = active;
}

void part::scale()
{
    float amount = bodypart::scale/3.f;

    if(type != bodypart::HEAD)
        model->scale(amount);
    else
        model->scale(amount);
}

objects_container* part::obj()
{
    return model;
}

void part::set_pos(vec3f _pos)
{
    pos = _pos;
}

void part::set_rot(vec3f _rot)
{
    rot = _rot;
}

void part::set_global_pos(vec3f _global_pos)
{
    global_pos = _global_pos;
}

void part::set_global_rot(vec3f _global_rot)
{
    global_rot = _global_rot;
}

///wait, if we've got dynamic texture update
///I can just go from red -> dark red -> black
///yay!
void part::update_model()
{
    model->set_pos({global_pos.v[0], global_pos.v[1], global_pos.v[2]});
    model->set_rot({global_rot.v[0], global_rot.v[1], global_rot.v[2]});

    /*vec3f vec = {0, 20, 0};

    vec3f rvec = vec.rot(0.f, global_rot) + global_pos;

    hp_display->set_pos({rvec.v[0], rvec.v[1], rvec.v[2]});
    hp_display->set_rot(model->rot);*/
}

void part::set_team(int _team)
{
    int old = team;

    team = _team;

    if(old != team)
        load_team_model();
}

///might currently leak texture memory
void part::load_team_model()
{
    ///this is not the place to define these
    const std::string low_red = "res/low/bodypart_red.obj";
    const std::string high_red = "res/high/bodypart_red.obj";
    const std::string low_blue = "res/low/bodypart_blue.obj";
    const std::string high_blue = "res/high/bodypart_blue.obj";

    std::string to_load = low_red;

    if(quality == 0)
    {
        if(team == 0)
            to_load = low_red;
        else
            to_load = low_blue;
    }
    else
    {
        if(team == 0)
            to_load = high_red;
        else
            to_load = high_blue;
    }

    /*display_tex.type = 0;
    display_tex.set_unique();
    display_tex.set_load_func(std::bind(texture_make_blank, std::placeholders::_1, 256, 256, sf::Color(255, 255, 255)));

    hp_display->set_load_func(std::bind(obj_rect, std::placeholders::_1, display_tex, (cl_float2){100, 100}));*/

    model->set_file(to_load);

    //model->set_normal("res/norm_body.png");

    model->unload();

    set_active(true);

    cpu_context->load_active();

    model->set_specular(bodypart::specular);

    scale();
}

void part::set_quality(int _quality)
{
    int old_quality = quality;

    quality = _quality;

    if(quality != old_quality)
    {
        load_team_model();
    }
}

///a network transmission of damage will get swollowed if you are hit between the time you spawn, and the time it takes to transmit
///the hp stat to the destination. This is probably acceptable

///temp error as this class needs gpu access
void part::damage(float dam, bool do_effect)
{
    //hp -= dam;

    set_hp(hp - dam);

    //printf("%f\n", hp);

    if(is_active && hp < 0.0001f)
    {
        perform_death(do_effect);
    }

    ///so, lets do this elsewhere



    /*cl_float4 rcol = {248, 63, 95};
    cl_float4 bcol = {63, 95, 248};

    cl_float4 pcol = team == 0 ? rcol : bcol;

    if(!model->isactive || !model->isloaded)
        return;

    ///if this is async this might break
    if(hp > 0)
    {
        pcol.x *= hp;
        pcol.y *= hp;
        pcol.z *= hp;

        cl_uint tid = model->objs[0].tid;

        texture* tex = texture_manager::texture_by_id(tid);

        tex->update_gpu_texture_col(pcol, cpu_context->fetch()->tex_gpu);
    }*/


    //network_hp(dam);
}

#include <vec/vec.hpp>

void part::update_texture_by_hp()
{
    if(old_hp != hp)
    {
        old_hp = hp;

        cl_float4 rcol = {248, 63, 95};
        cl_float4 bcol = {63, 95, 248};

        cl_float4 pcol = team == 0 ? rcol : bcol;

        if(!model->isactive || !model->isloaded)
            return;

        ///if this is async this might break
        if(hp > 0 && hp != 1.f)
        {
            pcol.x *= hp;
            pcol.y *= hp;
            pcol.z *= hp;

            cl_float4 dcol = pcol;

            dcol = {20, 20, 20};

            dcol.x /= 255.f;
            dcol.y /= 255.f;
            dcol.z /= 255.f;

            cl_uint tid = model->objs[0].tid;

            texture* tex = model->parent->tex_ctx.id_to_tex(tid);

            int rnum = 10;

            for(int i=0; i<rnum; i++)
            {
                float width = tex->get_largest_dimension();

                vec2f rpos = randf<2, float>(width * 0.2f, width * 0.8f) * (vec2f){tex->c_image.getSize().x, tex->c_image.getSize().y};

                rpos = rpos / width;

                //tex->update_gpu_texture_col(pcol, cpu_context->fetch()->tex_gpu);
                /*tex->update_random_lines(5, {rpos.v[0], rpos.v[1]}, {1, 0}, cpu_context->fetch()->tex_gpu);
                tex->update_random_lines(5, {rpos.v[0], rpos.v[1]}, {-1, 1}, cpu_context->fetch()->tex_gpu);
                tex->update_random_lines(5, {rpos.v[0], rpos.v[1]}, {1, 1}, cpu_context->fetch()->tex_gpu);*/

                float angle = randf_s() * 2 * M_PI;

                vec2f dir = {cos(angle), sin(angle)};

                tex->update_random_lines(40, dcol, {rpos.v[0], rpos.v[1]}, {dir.v[0], dir.v[1]}, cpu_context->fetch()->tex_gpu_ctx);
            }

            tex->update_gpu_mipmaps(cpu_context->fetch()->tex_gpu_ctx);
        }

        if(hp == 1.f)
        {
            cl_uint tid = model->objs[0].tid;

            texture* tex = model->parent->tex_ctx.id_to_tex(tid);

            tex->update_gpu_texture_col(pcol, cpu_context->fetch()->tex_gpu_ctx);
        }
    }
}

void part::perform_death(bool do_effect)
{
    if(do_effect)
    {
        cube_effect e;

        e.make(1300, global_pos, 100.f, team, 10, *cpu_context);
        particle_effect::push(e);
    }

    set_active(false);

    cpu_context->load_active();
    cpu_context->build();
}

void part::set_hp(float h)
{
    float delta = h - hp;

    hp = h;

    network_hp(delta);
}

void part::network_hp(float delta)
{
    net.hp_dirty = true;
    //network::host_update(&hp);
    net.hp_delta += delta;
}

bool part::alive()
{
    ///hp now networked, dont need to hodge podge this with model active status
    return (hp > 0);// && model.isactive;
}

size_t movement::gid = 0;

void movement::load(int _hand, vec3f _end_pos, float _time, int _type, bodypart_t b, movement_t _move_type)
{
    end_time = _time;
    fin = _end_pos;
    type = _type;
    hand = _hand;

    limb = b;

    move_type = _move_type;
}

float movement::time_remaining()
{
    float time = clk.getElapsedTime().asMicroseconds() / 1000.f;

    return std::max(end_time - time, 0.f);
}

float movement::get_frac()
{
    float time = clk.getElapsedTime().asMicroseconds() / 1000.f;

    return (float)time / end_time;
}

void movement::fire()
{
    clk.restart();
    going = true;
}

bool movement::finished()
{
    if(going && get_frac() >= 1)
        return true;

    return false;
}

movement::movement()
{
    hit_id = -1;

    end_time = 0.f;
    start = {0,0,0};
    fin = {0,0,0};
    type = 0;
    hand = 0;
    going = false;

    /*does_damage = true;
    does_block = false;

    moves_character = false;*/

    move_type = mov::DAMAGING;

    id = gid++;
}

movement::movement(int hand, vec3f end_pos, float time, int type, bodypart_t b, movement_t _move_type) : movement()
{
    load(hand, end_pos, time, type, b, _move_type);
}

bool movement::does(movement_t t)
{
    return move_type & t;
}

void movement::set(movement_t t)
{
    move_type = (movement_t)(move_type | t);
}

void sword::set_team(int _team)
{
    int old = team;

    team = _team;

    if(old != team)
        load_team_model();
}

void sword::load_team_model()
{
    if(team == 0)
    {
        model->set_file("./Res/sword_red.obj");
    }
    else
    {
        model->set_file("./Res/sword_blue.obj");
    }

    model->unload();

    model->set_active(true);

    cpu_context->load_active();
    scale();

    model->set_specular(bodypart::specular);
}

sword::sword(object_context& cpu)
{
    cpu_context = &cpu;

    model = cpu.make_new();

    model->set_pos({0, 0, -100});
    dir = {0,0,0};
    model->set_file("./Res/sword_red.obj");
    team = -1;
}

void sword::scale()
{
    model->scale(50.f);
    model->set_specular(0.4f);

    bound = get_bbox(model);

    float sword_height = 0;

    for(triangle& t : model->objs[0].tri_list)
    {
        for(vertex& v : t.vertices)
        {
            vec3f pos = xyz_to_vec(v.get_pos());

            if(pos.v[1] > sword_height)
                sword_height = pos.v[1];
        }
    }

    length = sword_height;
}

void sword::set_pos(vec3f _pos)
{
    pos = _pos;
}

void sword::set_rot(vec3f _rot)
{
    rot = _rot;
}

link make_link(part* p1, part* p2, int team, float squish = 0.0f, float thickness = 18.f, vec3f offset = {0,0,0})
{
    vec3f dir = (p2->pos - p1->pos);

    std::string tex = "./res/red.png";

    ///should really define this in a header somewhere, rather than here in shit code
    if(team == 1)
        tex = "./res/blue.png";

    vec3f start = p1->pos + dir * squish;
    vec3f finish = p2->pos - dir * squish;

    objects_container* o = p1->cpu_context->make_new();
    o->set_load_func(std::bind(load_object_cube, std::placeholders::_1, start, finish, thickness, tex));
    o->cache = false;
    //o.set_normal("res/norm_body.png");

    link l;

    l.obj = o;

    l.p1 = p1;
    l.p2 = p2;

    l.offset = offset;

    l.squish_frac = squish;

    l.length = (finish - start).length();

    return l;
}

///need to only maintain 1 copy of this, I'm just a muppet
fighter::fighter(object_context& _cpu_context, object_context_data& _gpu_context) : weapon(_cpu_context), my_cape(_cpu_context, _gpu_context)
{
    cpu_context = &_cpu_context;
    gpu_context = &_gpu_context;

    for(int i=0; i<bodypart::COUNT; i++)
    {
        parts.push_back(part(_cpu_context));
    }

    quality = 0;

    light l1;

    my_lights.push_back(light::add_light(&l1));
    my_lights.push_back(light::add_light(&l1));
    my_lights.push_back(light::add_light(&l1));

    load();

    pos = {0,0,0};
    rot = {0,0,0};

    game_state = nullptr;
}

void fighter::load()
{
    crouch_frac = 0.f;

    momentum = {0,0};

    jump_info = jump_descriptor();

    sword_rotation_offset = {0,0,0};

    net.reported_dead = 0;

    performed_death = false;

    net.recoil = false;
    net.is_blocking = false;

    rot_diff = {0,0,0};

    look_displacement = {0,0,0};

    frametime = 0;

    my_time = 0;

    look = {0,0,0};

    left_frac = 0.f;
    right_frac = 0.f;

    idle_fired_first = -1;

    idling = false;

    team = -1;

    left_full = false;

    left_id = -1;
    right_id = -1;

    left_stage = 0;
    right_stage = 1;


    left_fired = false;
    right_fired = false;

    stance = 0;

    ///im not sure why this is a duplicate of default_position
    rest_positions = bodypart::init_default();

    for(size_t i=0; i<bodypart::COUNT; i++)
    {
        parts[i].set_type((bodypart_t)i);
        old_pos[i] = parts[i].pos;
    }

    ///this is a dirty, dirty hack to smooth the knee positions first time around
    for(int i=0; i<100; i++)
    {
        IK_foot(0, parts[bodypart::LFOOT].pos);
        IK_foot(1, parts[bodypart::RFOOT].pos);

        for(int i=0; i<bodypart::COUNT; i++)
        {
            old_pos[i] = parts[i].pos;
        }
    }

    weapon.set_pos({0, -200, -100});

    IK_hand(0, weapon.pos);
    IK_hand(1, weapon.pos);

    focus_pos = weapon.pos;

    shoulder_rotation = 0.f;

    left_foot_sound = true;
    right_foot_sound = true;
}

void fighter::respawn(vec2f _pos)
{
    int old_team = team;

    load();

    team = old_team;

    ///this doesn't work properly for some reason
    pos = {_pos.v[0],0,_pos.v[1]};
    rot = {0,0,0};

    for(auto& i : parts)
    {
        i.set_active(true);
    }

    weapon.model->set_active(true);

    for(auto& i : joint_links)
    {
        i.obj->set_active(true);
    }

    set_team(team);

    cpu_context->load_active();

    cpu_context->build();
    cpu_context->flip();
    gpu_context = cpu_context->fetch();

    //update_render_positions();

    //obj_mem_manager::g_arrange_mem();
    //obj_mem_manager::g_changeover();

    //my_cape.make_stable(this);

    //network::host_update(&net.dead);
}

void fighter::die()
{
    net.reported_dead = true;

    performed_death = true;

    //net.dead = true;

    for(auto& i : parts)
    {
        i.set_active(false);
    }

    weapon.model->set_active(false);

    for(auto& i : joint_links)
    {
        i.obj->set_active(false);
    }

    ///spawn in some kind of swanky effect here

    /*particle_effect e;
    e.make(1300, parts[bodypart::BODY].global_pos, 250.f);
    e.push();
    e.make(1300, parts[bodypart::BODY].global_pos, 150.f);
    e.push();
    e.make(1300, parts[bodypart::BODY].global_pos, 100.f);
    e.push();
    e.make(1300, parts[bodypart::BODY].global_pos, 50.f);
    e.push();*/

    /*particle_effect e;
    e.make(1300, parts[bodypart::BODY].global_pos, 50.f);
    e.push();*/

    const float death_time = 2000;

    for(auto& i : parts)
    {
        cube_effect e;

        e.make(death_time, i.global_pos, 50.f, team, 10, *cpu_context);
        particle_effect::push(e);
    }

    {
        vec3f weapon_pos = xyz_to_vec(weapon.model->pos);
        vec3f weapon_rot = xyz_to_vec(weapon.model->rot);

        vec3f weapon_dir = (vec3f){0, 1, 0}.rot({0,0,0}, weapon_rot);

        float sword_height = weapon.length;

        for(float i = 0; i < sword_height; i += 40.f)
        {
            vec3f pos = weapon_pos + i * weapon_dir.norm();

            cube_effect e;

            e.make(death_time, pos, 50.f, team, 5, *cpu_context);
            particle_effect::push(e);
        }
    }

    update_lights();

    for(auto& i : my_lights)
    {
        light_effect l;
        l.make(5000.f, i);
        particle_effect::push(l);
    }

    //network::host_update(&net.dead);

    cpu_context->load_active();

    cpu_context->build();
    cpu_context->flip();
    gpu_context = cpu_context->fetch();

    ///pipe out hp here, just to check
}

int fighter::num_dead()
{
    int num_destroyed = 0;

    for(auto& p : parts)
    {
        ///hp_delta is predicted
        ///this is so that network stuff wont trigger multiple deaths
        ///by mistake
        if(p.hp <= 0)
        {
            //printf("%s\n", names[p.type].c_str());

            num_destroyed++;
        }
    }

    return num_destroyed;
}

///this is an awful piece of i dont even know what
int fighter::num_needed_to_die()
{
    return 3;
}

bool fighter::should_die()
{
    if(num_dead() >= num_needed_to_die() && !performed_death)
        return true;

    return false;
}

void fighter::checked_death()
{
    if(fighter::should_die())
    {
        die();
    }
}

bool fighter::dead()
{
    return (num_dead() > num_needed_to_die()) || performed_death;
}

void fighter::tick_cape()
{
    ///hmm. If cape.inert? Have cape sort its own death once parent is signalled?
    //if(dead())
    //    return;

    int ticks = 1;

    for(int i=0; i<ticks; i++)
    {
        my_cape.tick(this);
    }
}

void fighter::set_quality(int _quality)
{
    quality = _quality;

    for(auto& i : parts)
    {
        i.set_quality(quality);
    }
}

void fighter::set_gameplay_state(gameplay_state* st)
{
    game_state = st;
}

void fighter::set_look(vec3f _look)
{
    vec3f current_look = look;

    vec3f new_look = _look;

    vec3f clamps = {M_PI/8.f, M_PI/12.f, M_PI/8.f};
    new_look = clamp(new_look, -clamps, clamps);

    vec3f origin = parts[bodypart::BODY].pos;

    vec3f c2b = current_look - origin;
    vec3f n2b = new_look - origin;

    float angle_constraint = 0.004f * frametime;

    float angle = dot(c2b.norm(), n2b.norm());

    if(fabs(angle) >= angle_constraint)
    {
        new_look = mix(current_look, new_look, angle_constraint);
    }

    new_look = clamp(new_look, -clamps, clamps); /// just in case for some reason the old current_look was oob

    const float displacement = (rest_positions[bodypart::LHAND] - rest_positions[bodypart::LUPPERARM]).length();

    float height = displacement * sin(new_look.v[0]);
    float width = displacement * sin(new_look.v[1]);

    old_look_displacement = look_displacement;
    look_displacement = (vec3f){width, height, 0.f};

    look = new_look;
}

///s2 and s3 define the shoulder -> elbow, and elbow -> hand length
///need to handle double cos problem
float get_joint_angle(vec3f end_pos, vec3f start_pos, float s2, float s3)
{
    float s1 = (end_pos - start_pos).length();

    s1 = clamp(s1, 0.f, s2 + s3);

    float ic = (s2 * s2 + s3 * s3 - s1 * s1) / (2 * s2 * s3);

    ic = clamp(ic, -1, 1);

    float angle = acos ( ic );

    //printf("%f\n", angle);

    //if(angle >= M_PI/2.f)
    //    angle = M_PI/2.f - angle;

    //printf("%f\n", angle);


    return angle;
}

float get_joint_angle_foot(vec3f end_pos, vec3f start_pos, float s2, float s3)
{
    float s1 = (end_pos - start_pos).length();

    s1 = clamp(s1, 0.f, s2 + s3);

    float ic = (s2 * s2 + s3 * s3 - s1 * s1) / (2 * s2 * s3);

    ic = clamp(ic, -1, 1);

    float angle = acos ( ic );

    //printf("%f\n", angle);

    //if(angle >= M_PI/2.f)
    //    angle = M_PI/2.f - angle;


    return angle;
}

///p1 shoulder, p2 elbow, p3 hand
void inverse_kinematic(vec3f pos, vec3f p1, vec3f p2, vec3f p3, vec3f& o_p1, vec3f& o_p2, vec3f& o_p3)
{
    float s1 = (p3 - p1).length();
    float s2 = (p2 - p1).length();
    float s3 = (p3 - p2).length();

    float joint_angle = get_joint_angle(pos, p1, s2, s3);

    //o_p1 = p1;

    float max_len = (p3 - p1).length();

    float to_target = (pos - p1).length();

    float len = std::min(max_len, to_target);

    vec3f dir = (pos - p1).norm();

    o_p3 = p1 + dir * len;

    float area = 0.5f * s2 * s3 * sin(joint_angle);

    ///height of scalene triangle is 2 * area / base

    float height = 2 * area / s1;

    ///ah just fuck it
    ///we need to fekin work this out properly
    vec3f halfway = (o_p3 + p1) / 2.f;

    halfway.v[1] -= height;

    vec3f halfway_dir = (halfway - p1).norm();

    o_p2 = p1 + halfway_dir * s2;

    const float shoulder_move_amount = s2/5.f;

    o_p1 = p1 + halfway_dir * shoulder_move_amount;
}

///p1 shoulder, p2 elbow, p3 hand/foot
void inverse_kinematic_foot(vec3f pos, vec3f p1, vec3f p2, vec3f p3, vec3f off1, vec3f off2, vec3f off3, vec3f& o_p1, vec3f& o_p2, vec3f& o_p3)
{
    float s1 = (p3 - p1 - off1).length();
    float s2 = (p2 - p1).length();
    float s3 = (p3 - p2).length();


    float joint_angle = M_PI + get_joint_angle_foot(pos, p1 + off1, s2, s3);

    o_p3 = pos;

    float area = 0.5f * s2 * s3 * sin(joint_angle);

    ///height of scalene triangle is 2 * area / base

    float height = 2 * area / s1;

    vec3f d1 = (o_p3 - p1 - off1);
    vec3f d2 = {1, 0, 0};

    vec3f d3 = cross(d1, d2);

    d3 = d3.norm();

    vec3f half = (p1 + off1 + o_p3)/2.f;

    ///set this to std::max(height, 30.f) if you want beelzebub strolling around
    o_p2 = half + std::min(height, -5.f) * d3;

    vec3f d = (o_p3 - p1 - off1).norm();

    ///no wait, this is defining hip wiggle
    o_p1 = p1 + off1 + d.norm() * (vec3f){20.f, 4.f, 20.f};
}

void fighter::IK_hand(int which_hand, vec3f pos, float upper_rotation, bool arms_are_locked, bool force_positioning)
{
    using namespace bodypart;

    auto upper = which_hand ? RUPPERARM : LUPPERARM;
    auto lower = which_hand ? RLOWERARM : LLOWERARM;
    auto hand = which_hand ? RHAND : LHAND;

    //printf("%f\n", pos.v[0]);

    vec3f i1, i2, i3;

    i1 = rest_positions[upper];
    i2 = rest_positions[lower];
    i3 = rest_positions[hand];

    i1 = i1.rot({0,0,0}, {0, upper_rotation, 0});
    i2 = i2.rot({0,0,0}, {0, upper_rotation, 0});
    i3 = i3.rot({0,0,0}, {0, upper_rotation, 0});

    vec3f o1, o2, o3;

    inverse_kinematic(pos, i1, i2, i3, o1, o2, o3);

    //o1 = o1 + look_displacement;
    //o2 = o2 + look_displacement;
    //o3 = o3 + look_displacement;

    //printf("%f\n", o2.v[2]);

    //printf("%f\n", look_displacement.v[0]);

    if(arms_are_locked)
    {
        o2 = (o1 + o3) / 2.f;
    }

    if(force_positioning && (o3 - pos).length() > 1)
    {
        o3 = pos;

        o2 = (o1 + o3) / 2.f;

        ///wait. Can't I just adjust the shoulder instead, here?

        /*o3 = pos;

        float arm_len = (i3 - i2).length();

        vec3f to_elbow = (o2 - pos).norm();

        vec3f new_elbow = to_elbow * arm_len + pos;

        vec3f new_shoulder = (o1 - o3).norm() * arm_len * 2 + pos;

        o2 = new_elbow;

        o1 = new_shoulder;*/
    }

    parts[upper].set_pos(o1);
    parts[lower].set_pos(o2);
    //parts[lower].set_pos((o2 + old_pos[lower]*1)/2.f);
    parts[hand].set_pos(o3);
}

///ergh. merge offsets into inverse kinematic foot as separate arguments
///so we dont mess up length calculations
void fighter::IK_foot(int which_foot, vec3f pos, vec3f off1, vec3f off2, vec3f off3)
{
    using namespace bodypart;

    auto upper = which_foot ? RUPPERLEG : LUPPERLEG;
    auto lower = which_foot ? RLOWERLEG : LLOWERLEG;
    auto hand = which_foot ? RFOOT : LFOOT;

    //printf("%f\n", pos.v[0]);

    vec3f o1, o2, o3;

    ///put offsets into this function
    inverse_kinematic_foot(pos, rest_positions[upper], rest_positions[lower], rest_positions[hand], off1, off2, off3, o1, o2, o3);

    //printf("%f\n", o2.v[2]);

    parts[upper].set_pos(o1);
    parts[lower].set_pos((o2 + old_pos[lower]*5.f)/6.f);
    parts[hand].set_pos(o3);
}

void fighter::linear_move(int hand, vec3f pos, float time, bodypart_t b)
{
    movement m;

    m.load(hand, pos, time, 0, b, mov::MOVES);

    moves.push_back(m);
}

void fighter::spherical_move(int hand, vec3f pos, float time, bodypart_t b)
{
    movement m;

    m.load(hand, pos, time, 1, b, mov::MOVES);

    moves.push_back(m);
}

float frac_smooth(float in)
{
    return - in * (in - 2);
}

///if we exaggerate the skew, this is workable
float erf_smooth(float x)
{
    if(x < 0 || x >= 1)
        return x;

    return powf((- x * (x - 2) * (erf((x - 0.5f) * 3) + 1) / 2.f), 1.5f);
}

///we want the hands to be slightly offset on the sword
void fighter::tick(bool is_player)
{
    float cur_time = frame_clock.getElapsedTime().asMicroseconds() / 1000.f;

    frametime = cur_time - my_time;

    my_time = cur_time;

    using namespace bodypart;

    std::vector<bodypart_t> busy_list;

    ///will be set to true if a move is currently doing a blocking action
    net.is_blocking = 0;

    if(net.recoil)
    {
        if(can_recoil())
            recoil();

        net.recoil = 0;
    }

    ///use sword rotation offset to make sword 90 degrees at blocking

    bool arms_are_locked = false;

    ///only the first move is ever executed PER LIMB
    ///the busy list is used purely to stop all the rest of the moves from activating
    ///because different limbs can have different moves activating concurrently
    for(movement& i : moves)
    {
        if(std::find(busy_list.begin(), busy_list.end(), i.limb) != busy_list.end())
        {
            continue;
        }

        action_map[i.limb] = i;

        if(!i.going)
        {
            i.fire();

            i.start = parts[i.limb].pos;

            if((i.limb == LHAND || i.limb == RHAND) && !i.does(mov::START_INDEPENDENT))
                i.start = focus_pos;

            if((i.limb == LHAND || i.limb == RHAND) && i.does(mov::START_INDEPENDENT))
                i.start = focus_pos;

        }

        float arm_len = (default_position[LHAND] - default_position[LUPPERARM]).length();

        ///this is the head vector, but we want the tip of the sword to go through the centre
        ///time for maths
        ///really this wants to dynamically find the player's LOOK vector
        ///but this combined with look displacement does an alright job
        vec3f head_vec = {0, default_position[HEAD].v[1], -arm_len};
        float sword_len = weapon.length;

        ///current sword tip
        vec3f sword_vec = (vec3f){0, sword_len, 0}.rot({0,0,0}, weapon.rot) + weapon.pos;

        vec3f desired_sword_vec = {sword_vec.v[0], head_vec.v[1], sword_vec.v[2]};

        if(i.does(mov::OVERHEAD_HACK))
            desired_sword_vec = desired_sword_vec.norm() * sword_len;

        ///this worked fine before with body because they're on the same height
        ///hmm. both are very similar in accuracy, but they're slightly stylistically different
        ///comebacktome ???
        ///????
        vec3f desired_hand_relative_sword = desired_sword_vec - (desired_sword_vec - default_position[LUPPERARM]).norm() * sword_len;

        ///really we only want the height
        float desired_hand_height = desired_hand_relative_sword.v[1];


        vec3f cx_sword_vec = {0.f, head_vec.v[1], sword_vec.v[2]};

        vec3f cx_sword_rel = cx_sword_vec - (cx_sword_vec - default_position[LUPPERARM]).norm() * sword_len;

        float desired_hand_x = cx_sword_rel.v[0];

        vec3f half_hand_vec = ((vec3f){desired_hand_x, desired_hand_height, i.fin.v[2]} + i.start)/2.f;



        busy_list.push_back(i.limb);

        float frac = i.get_frac();

        frac = clamp(frac, 0.f, 1.f);

        vec3f current_pos;

        vec3f actual_finish = i.fin;
        vec3f actual_start = i.start;

        if(i.does(mov::FINISH_INDEPENDENT))
        {
            actual_finish = actual_finish - look_displacement;
        }

        if(i.does(mov::LOCKS_ARMS))
        {
            //arms_are_locked = true;
        }

        vec3f actual_avg = (actual_start + actual_finish) / 2.f;

        ///lets use mix3
        if(i.does(mov::PASS_THROUGH_SCREEN_CENTRE))
        {
            actual_avg = head_vec;

            actual_avg.v[1] = desired_hand_height;

            //actual_avg = half_hand_vec;

            //actual_avg.v[1] = desired_hand_height;

            ///so it looks less unnatural
            //actual_finish.v[1] = desired_hand_height;
        }

        if(i.does(mov::FINISH_AT_SCREEN_CENTRE))
        {
            //actual_finish = desired_hand_relative_sword;

            //actual_finish.v[2] = -actual_finish.v[2];

            //actual_finish = head_vec;

            actual_avg = half_hand_vec;

            actual_finish.v[1] = desired_hand_height;
            actual_finish.v[0] = desired_hand_x;
        }

        if(i.does(mov::FINISH_AT_90))
        {
            float ffrac = frac_smooth(frac);

            sword_rotation_offset.v[1] = M_PI/2.f * pow(ffrac, 2.f);
        }
        else
        {
            float ffrac = frac_smooth(frac);

            if(fabs(sword_rotation_offset.v[1]) > 0.0001f)
            {
                sword_rotation_offset.v[1] = M_PI/2.f * pow((1.f - ffrac), 1.7f);
            }
            //sword_rotation_offset = sword_rotation_offset * sqrtf(1.f - frac);
        }

        ///scrap mix3 and slerp3 stuff, need to interpolate properly
        ///need to use a bitfield really, thisll get unmanageable
        if(i.type == 0)
        {
            ///apply a bit of smoothing
            frac = frac_smooth(frac);
            current_pos = mix(actual_start, actual_finish, frac); ///do a slerp3
        }
        else if(i.type == 1)
        {
            ///need to define this manually to confine it to one axis, slerp is not what i want
            frac = frac_smooth(frac);
            current_pos = slerp(actual_start, actual_finish, frac);

            float fsin = frac * M_PI;

            //float sval = sin(fsin);

            //current_pos.v[1] = current_pos.v[1] * (1.f - sval) + actual_avg.v[1] * sval;

            ///so the reason this flatttens it out anyway
            ///is because we're swappign from slerping to cosinterpolation
            if(i.does(mov::PASS_THROUGH_SCREEN_CENTRE))
            //    current_pos.v[1] = cosif3(actual_start.v[1], actual_avg.v[1], actual_finish.v[1], frac);
                current_pos.v[1] = mix3(actual_start, actual_avg, actual_finish, frac).v[1];

            if(i.does(mov::FINISH_AT_SCREEN_CENTRE))
            {
                //current_pos.v[0] = cosint3(actual_start, actual_avg, actual_finish, frac).v[0];
            }
        }
        else if(i.type == 2)
        {
            current_pos = slerp(actual_start, actual_finish, frac);
        }
        else if(i.type == 3)
        {
            frac = erf_smooth(frac);

            current_pos = slerp(actual_start, actual_finish, frac);
        }

        if(i.limb == LHAND || i.limb == RHAND)
        {
            ///focus pos is relative to player, but does NOT include look_displacement OR world anything
            vec3f old_pos = focus_pos;
            focus_pos = current_pos;

            ///losing a frame currently, FIXME
            ///if the sword hits something, not again until the next move
            ///make me a function?
            if(i.hit_id == -1 && i.does(mov::DAMAGING))
            {
                ///this is the GLOBAL move dir, current_pos could be all over the place due to interpolation, lag etc
                vec3f move_dir = (focus_pos - old_pos).norm();

                ///pass direction vector into here, then do the check
                ///returns -1 on miss
                i.hit_id = phys->sword_collides(weapon, this, move_dir, is_player);

                ///if hit, need to signal the other fighter that its been hit with its hit id, relative to part num
                if(i.hit_id != -1)
                {
                    //float damage_amount = attacks::damage_amounts[]

                    fighter* their_parent = phys->bodies[i.hit_id].parent;

                    ///this is the only time damage is applied to anything, ever
                    their_parent->damage((bodypart_t)(i.hit_id % COUNT), i.damage);

                    ///this is where the networking fighters get killed
                    ///this is no longer true, may happen here or in server_networking
                    ///probably should remove this
                    their_parent->checked_death();

                    //printf("%s\n", names[i.hit_id % COUNT].c_str());
                }
            }

            if(i.does(mov::BLOCKING))
            {
                net.is_blocking = 1;
            }
        }
    }

    for(auto it = moves.begin(); it != moves.end();)
    {
        if(it->finished())
        {
            action_map.erase(it->limb);

            it = moves.erase(it);
        }
        else
            it++;
    }

    vec3f jump_displacement = jump_info.get_relative_jump_displacement_tick(frametime, this);

    ///still jumping
    if(jump_info.is_jumping)
    {
        pos = pos + jump_displacement;
    }
    else
    {
        pos.v[1] = 0;
    }


    /*for(auto& i : parts)
    {
        if(i.hp != 1.f)
            printf("hp %f\n", i.hp);
    }

    printf("\n");*/

    //static float rot = 0.f;

    //rot += 0.01f;

    ///again needs to be made frametime independent
    const float displacement = (rest_positions[bodypart::LHAND] - rest_positions[bodypart::LUPPERARM]).length();
    float focus_rotation = 0.f;
    shoulder_rotation = shoulder_rotation * 6 + atan2((old_look_displacement.v[0] - look_displacement.v[0]) * 10.f, displacement);
    shoulder_rotation /= 7.f;

    vec3f rot_focus = (focus_pos + look_displacement).rot((vec3f){0,0,0}, (vec3f){0, focus_rotation, 0});

    /*vec2f weapon_pos = (vec2f){weapon.pos.v[0], weapon.pos.v[2]}.rot(rot.v[1]);


    ///absolute angle
    float weapon_angle = weapon_pos.angle();
    float look_angle = shoulder_rotation + rot.v[1];//(vec2f){rot_focus.v[0], rot_focus.v[2]}.angle();

    float body_diff = weapon_angle - look_angle;

    //rot_focus = rot_focus.rot({0,0,0}, {0, body_diff, 0.f});

    shoulder_rotation += body_diff/10.f;

    printf("%f sh\n", weapon_angle);*/

    vec2f weapon_pos = {weapon.pos.v[0], weapon.pos.v[2]};


    //float approx_cylinder_half_size = bodypart::scale / 4.f;


    //printf("%f %f\n", EXPAND_2(weapon_pos));

    float wangle = weapon_pos.angle() + M_PI/2.f;

    shoulder_rotation += (wangle - shoulder_rotation) + shoulder_rotation * 5.f;
    shoulder_rotation /= 6;


    IK_hand(0, rot_focus, shoulder_rotation, arms_are_locked);
    IK_hand(1, parts[LHAND].pos, shoulder_rotation, arms_are_locked, true);

    vec3f slave_to_master = parts[LHAND].pos - parts[RHAND].pos;

    ///dt smoothing doesn't work because the shoulder position is calculated
    ///dynamically from the focus position
    ///this means it probably wants to be part of our IK step?
    ///hands disconnected
    if(slave_to_master.length() > 1.f)
    {
        parts[RHAND].pos = parts[LHAND].pos;

        float arm_len = (rest_positions[LHAND] - rest_positions[LLOWERARM]).length();

        vec3f original_shoulder = parts[RUPPERARM].pos;

        /*vec3f slave_to_shoulder = original_shoulder - parts[RHAND].pos;

        vec3f elbow_pos = slave_to_shoulder.norm() * arm_len + parts[RHAND].pos;
        vec3f new_shoulder_pos = slave_to_shoulder.norm() * arm_len * 2 + parts[RHAND].pos;*/

        vec3f elbow_pos = (parts[RHAND].pos + parts[RUPPERARM].pos)/2.f;

        //float dt_tsmooth = frametime * 0.1f;

        //parts[RLOWERARM].pos = elbow_pos;
        //parts[RUPPERARM].pos = (new_shoulder_pos * dt_tsmooth + parts[RUPPERARM].pos) / (dt_tsmooth + 1);
    }


    //IK_hand(1, rot_focus, shoulder_rotation, arms_are_locked);


    ///sword render stuff updated here
    update_sword_rot();

    parts[BODY].set_pos((parts[LUPPERARM].pos + parts[RUPPERARM].pos + rest_positions[BODY]*3.f)/5.f);

    //parts[BODY].set_pos((parts[BODY].pos * 20 + parts[RUPPERLEG].pos + parts[LUPPERLEG].pos)/(20 + 2));

    parts[HEAD].set_pos((parts[BODY].pos*2.f + rest_positions[HEAD] * 32.f) / (32 + 2));




    float cdist = 2 * bodypart::scale / 2.f;
    //float cdist = 3 * bodypart::scale / 2.f;

    //float tdiff = fdiff / time_to_crouch_s;

    for(auto& i : {HEAD, BODY, LUPPERARM, RUPPERARM, LLOWERARM, RLOWERARM, LHAND, RHAND})
    {
        auto type = i;

        vec3f pos = parts[type].pos;

        pos.v[1] -= cdist * crouch_frac;

        parts[type].set_pos(pos);
    }

    /*for(auto& type : {LUPPERLEG, RUPPERLEG})
    {
        vec3f pos = parts[type].pos;

        pos.v[1] -= cdist * crouch_frac / 2.f;

        parts[type].set_pos(pos);
    }*/

    IK_foot(0, parts[LFOOT].pos, {0, -cdist * crouch_frac, 0}, {0, -cdist * crouch_frac, 0}, {0,0,0});
    IK_foot(1, parts[RFOOT].pos, {0, -cdist * crouch_frac, 0}, {0, -cdist * crouch_frac, 0}, {0,0,0});

    weapon.set_pos(parts[bodypart::LHAND].pos);

    /*float sword_len = weapon.length;

    vec3f sword_dir =  (vec3f){0, 1, 0}.rot({0,0,0}, weapon.rot);
    vec3f sword_pos = weapon.pos;

    float approx_cylinder_half_size = bodypart::scale / 4.f;

    vec3f to_sword = sword_pos - parts[RHAND].pos;

    vec3f to_sword_along = to_sword + sword_dir * approx_cylinder_half_size;

    float len = to_sword_along.length();

    if(len > 1)
    {
        IK_hand(1, to_sword_along + parts[RHAND].pos, shoulder_rotation, arms_are_locked);

        //vec3f test_euler = to_sword_along.get_euler();

        vec3f new_to_sword = sword_pos - parts[RHAND].pos;
        vec3f new_to_sword_along = new_to_sword + sword_dir * approx_cylinder_half_size;

        vec3f test_euler = new_to_sword_along.get_euler();

        if(new_to_sword_along.length() > 2 && new_to_sword.length() > 2)
            parts[RHAND].set_rot(test_euler);
    }*/

    ///process death

    ///rip
    checked_death();
    manual_check_part_death();

    /*int collide_id = phys->sword_collides(weapon);

    if(collide_id != -1)
        printf("%s %i\n", bodypart::names[collide_id % bodypart::COUNT].c_str(), collide_id);*/
}

void fighter::manual_check_part_death()
{
    ///num needed to die = 3
    ///if 1, one dead, fine
    ///if 2, two dead, fine
    ///if 3, 3 dead bad
    ///therefore 2 < 3
    ///so that's fine
    ///i dunno why there was a -1 here originally
    bool do_explode_effect = num_dead() < num_needed_to_die();

    for(auto& i : parts)
    {
        if(i.hp < 0.0001f && i.is_active)
        {
            i.perform_death(do_explode_effect);
        }
    }
}

void fighter::manual_check_part_alive()
{
    ///do not respawn parts if the fighter is dead!
    //if(num_dead() >= num_needed_to_die())
    if(dead())
        return;

    bool any = false;

    for(auto& i : parts)
    {
        if(i.hp > 0.0001f && !i.is_active)
        {
            i.set_active(true);

            any = true;
        }
    }

    if(any)
    {
        cpu_context->load_active();
        cpu_context->build();
        gpu_context = cpu_context->fetch(); ///I'm not sure this is necessary as gpu_context is a ptr
    }
}

int modulo_distance(int a, int b, int m)
{
    return std::min(abs(b - a), abs(m - b + a));
}

vec3f seek(vec3f cur, vec3f dest, float dist, float seek_time, float elapsed_time)
{
    float speed = elapsed_time * dist / seek_time;

    vec3f dir = (dest - cur).norm();

    float remaining = (cur - dest).length();

    if(remaining < speed)
        return dest;

    return cur + speed * dir;
}

vec2f fighter::get_wall_corrected_move(vec2f pos, vec2f move_dir)
{
    bool xw = false;
    bool yw = false;

    vec2f dir_move = move_dir;
    vec2f lpos = pos;

    if(rectangle_in_wall(lpos + (vec2f){dir_move.v[0], 0.f}, get_approx_dim(), game_state))
    {
        dir_move.v[0] = 0.f;
        xw = true;
    }
    if(rectangle_in_wall(lpos + (vec2f){0.f, dir_move.v[1]}, get_approx_dim(), game_state))
    {
        dir_move.v[1] = 0.f;
        yw = true;
    }

    ///if I move into wall, but yw and xw aren't true, stop
    ///there are some diagonal cases here which might result in funky movement
    ///but largely should be fine
    if(rectangle_in_wall(lpos + dir_move, get_approx_dim(), game_state) && !xw && !yw)
    {
        dir_move = 0.f;
    }

    return dir_move;
}

///do I want to do a proper dynamic timing synchronisation thing?
void fighter::walk_dir(vec2f dir, bool sprint)
{
    if(game_state == nullptr)
    {
        printf("Warning: No gameplay state for fighter\n");
    }

    if(jump_info.is_jumping)
    {
        walk_clock.restart();

        return;
    }

    jump_info.dir = {0,0};

    ///try and fix the lex stiffening up a bit, but who cares
    ///make feet average out with the ground
    bool idle = dir.v[0] == 0 && dir.v[1] == 0;

    ///Make me a member variable?
    ///the last valid direction
    static vec2f valid_dir = {-1, 0};

    if(idle)
        dir = valid_dir;
    else
        valid_dir = dir;

    ///in ms
    ///replace this with a dt
    float time_elapsed = walk_clock.getElapsedTime().asMicroseconds() / 1000.f;

    float speed_mult = fighter_stats::speed;
    time_elapsed *= speed_mult;

    jump_info.last_speed = speed_mult;

    float h = 120.f;

    if(dir.v[0] == -1 && sprint)
    {
        time_elapsed *= fighter_stats::sprint_speed;
        h *= 1.2f;

        jump_info.last_speed *= fighter_stats::sprint_speed;
    }

    float dist = 125.f;

    float up = 50.f;

    int num = 3;

    vec2f ldir = dir.norm();

    ldir.v[1] = -ldir.v[1];

    vec2f rld = ldir.rot(-rot.v[1]);

    vec3f global_dir = {rld.v[1] * time_elapsed/2.f, 0.f, rld.v[0] * time_elapsed/2.f};

    ///dont move the player if we're really idling
    ///move me into a function
    ///if we're not idling, we want to actually move the player
    ///want to make the movement animation speed based on how far we actually moved
    ///out of how far we tried to move
    if(!idle)
    {
        ///this portion of code handles collision detection
        ///as well as ensuring the leg animations don't do anything silly
        ///this code contains a lot of redundant variables
        ///particularly annoying due to a lack of swizzling
        vec3f predicted = pos + global_dir;

        vec2f lpredict = {predicted.v[0], predicted.v[2]};

        vec2f dir_move = {global_dir.v[0], global_dir.v[2]};

        vec2f lpos = {pos.v[0], pos.v[2]};

        float move_amount = dir_move.length();

        dir_move = get_wall_corrected_move(lpos, dir_move);

        ///just in case!
        ///disappearing may be because the pos is being destroyed by this
        ///hypothetical
        if(!rectangle_in_wall(lpos + dir_move, get_approx_dim(), game_state))
        {
            float real_move = dir_move.length();

            float anim_frac = 0.f;

            if(move_amount > 0.0001f)
            {
                anim_frac = real_move / move_amount;
                time_elapsed *= anim_frac;

                ///so now global_dir is my new global move direction
                ///lets translate it back into local, and then
                ///update our move estimate by that much

                vec2f inv = {dir_move.v[1], dir_move.v[0]};

                inv = inv.rot(rot.v[1]);

                ///if we cant move anywhere, prevent division by 0 and set movedir to 0
                if(time_elapsed > 0.0001f)
                    inv = inv / (time_elapsed / 2.f);
                else
                    inv = {0,0};

                ldir = inv;
            }

            ///time independent movement direction
            ///however the movement direction does not account for speed variances
            jump_info.dir = dir_move / (time_elapsed / 2.f);
            ///the animation fraction is the fraction of our speed that we keep,
            ///due to wall intersection. Eg, a very high degree of our velocity vector
            ///through the wall, results in a low anim frac
            ///really_moved_distance / wanting_to_move_distances
            jump_info.last_speed *= anim_frac;

            pos = pos + (vec3f){dir_move.v[0], 0.f, dir_move.v[1]};
        }
    }

    vec3f current_dir = (vec3f){ldir.v[1], 0.f, ldir.v[0]} * time_elapsed/2.f;

    vec3f fin = (vec3f){ldir.v[1], 0.f, ldir.v[0]}.norm() * dist;

    ///1 is push, 0 is air
    static float lmod = 1.f;
    static float frac = 0.f;

    auto foot = bodypart::LFOOT;
    auto ofoot = bodypart::RFOOT;

    vec3f lrp = {parts[foot].pos.v[0], 0.f, parts[foot].pos.v[2]};
    vec3f rrp = {parts[ofoot].pos.v[0], 0.f, parts[ofoot].pos.v[2]};


    float lfrac = (fin - lrp).length() / (dist * 2);
    float rfrac = (fin - rrp).length() / (dist * 2);

    lfrac -= 0.2f;
    rfrac -= 0.2f;

    ///dont move the feet if we're really idling in the direction of last travel
    if(!idle)
    {
        IK_foot(0, parts[foot].pos - lmod * current_dir);
        IK_foot(1, parts[bodypart::RFOOT].pos + lmod * current_dir);
    }

    lfrac /= 0.8f;
    rfrac /= 0.8f;

    lfrac = clamp(lfrac, 0.f, 1.f);
    rfrac = clamp(rfrac, 0.f, 1.f);

    //printf("%f %f\n", lfrac, rfrac);
    //printf("%f %f %f\n", fin.v[0], fin.v[1], fin.v[2]);

    if(lmod < 0 && !idle)
    {
        float xv = -lfrac * (lfrac - 1);

        parts[foot].pos.v[1] = h * xv + rest_positions[foot].v[1];
    }
    if(lmod > 0 && !idle)
    {
        float xv = -rfrac * (rfrac - 1);

        parts[ofoot].pos.v[1] = h * xv + rest_positions[ofoot].v[1];
    }

    current_dir = current_dir.norm();

    //printf("%f %f %f\n", current_dir.v[0], current_dir.v[1], current_dir.v[2]);
    //printf("%f %f %f\n", parts[foot].pos.v[0], parts[foot].pos.v[1], parts[foot].pos.v[2]);

    float real_weight = 5.f;

    ///adjust foot so that it sits on the 'correct' line

    ///works for idling, average in last direction of moving to stick the feet on ground
    ///also keeps feet in place while regular walking
    {
        foot = bodypart::LFOOT;

        vec3f rest = rest_positions[foot];
        vec3f shortest_dir = point2line_shortest(rest_positions[foot], current_dir, parts[foot].pos);

        vec3f line_point = shortest_dir + parts[foot].pos;

        IK_foot(0, (line_point + parts[foot].pos * real_weight) / (1.f + real_weight));
    }

    {
        foot = bodypart::RFOOT;

        vec3f rest = rest_positions[foot];
        vec3f shortest_dir = point2line_shortest(rest_positions[foot], current_dir, parts[foot].pos);

        vec3f line_point = shortest_dir + parts[foot].pos;

        IK_foot(1, (line_point + parts[foot].pos * real_weight) / (1.f + real_weight));
    }

    ///reflects foot when it reaches the destination
    {
        foot = bodypart::LFOOT;

        vec3f without_up = {parts[foot].pos.v[0], 0.f, parts[foot].pos.v[2]};
        vec3f without_up_rest = {rest_positions[foot].v[0], 0.f, rest_positions[foot].v[2]};

        ///current dir is the direction we are going in

        if((without_up - without_up_rest).length() > dist)
        {
            vec3f d = without_up_rest - without_up;
            d = d.norm();

            float excess = (without_up_rest - without_up).length() - dist;

            parts[foot].pos = parts[foot].pos + d * excess;

            lmod = -lmod;
        }
    }


    {
        foot = bodypart::RFOOT;

        vec3f without_up = {parts[foot].pos.v[0], 0.f, parts[foot].pos.v[2]};
        vec3f without_up_rest = {rest_positions[foot].v[0], 0.f, rest_positions[foot].v[2]};

        ///current dir is the direction we are going in

        if((without_up - without_up_rest).length() > dist)
        {
            vec3f d = without_up_rest - without_up;
            d = d.norm();

            float excess = (without_up_rest - without_up).length() - dist;

            parts[foot].pos = parts[foot].pos + d * excess;
        }
    }

    walk_clock.restart();
}

void fighter::crouch_tick(bool do_crouch)
{
    using namespace bodypart;

    ///milliseconds
    float fdiff = frametime / 1000.f;

    const float time_to_crouch_s = 0.1f;

    /*float cdist = bodypart::scale / 2.f;

    float tdiff = fdiff / time_to_crouch_s;

    for(auto& i : {HEAD, BODY, LUPPERARM, RUPPERARM, LLOWERARM, RLOWERARM, LUPPERLEG, RUPPERLEG})
    {
        auto type = i;

        vec3f pos = parts[type].pos;

        //pos.v[1] += cdist * (fdiff / time_to_crouch_s);

        parts[type].set_pos(pos);
    }*/

    if(do_crouch)
        crouch_frac += fdiff / time_to_crouch_s;
    else
        crouch_frac -= fdiff / time_to_crouch_s;

    crouch_frac = clamp(crouch_frac, 0.f, 1.f);
}

#include "sound.hpp"

void fighter::do_foot_sounds(bool is_player)
{
    if(dead())
    return;

    const int asphalt_start = 2;
    const int foot_nums = 8;

    static int current_num = 0;
    current_num %= foot_nums;

    bool is_relative = is_player;

    using namespace bodypart;

    part& lfoot = parts[LFOOT];
    part& rfoot = parts[RFOOT];

    float ldiff = fabs(lfoot.global_pos.v[1] - rest_positions[LFOOT].v[1]);
    float rdiff = fabs(rfoot.global_pos.v[1] - rest_positions[RFOOT].v[1]);

    ///so, because the sound isn't tracking
    ///after its played, itll then be exposed to the 3d audio system
    vec3f centre = (lfoot.global_pos + rfoot.global_pos) / 2.f;

    centre = parts[BODY].global_pos;

    float cutoff = 1.f;

    if(ldiff < cutoff)
    {
        if(!left_foot_sound)
        {
            sound::add(current_num + asphalt_start, centre, is_relative);

            current_num = (current_num + 1) % foot_nums;

            left_foot_sound = true;
        }
    }
    else
    {
        left_foot_sound = false;
    }

    if(rdiff < cutoff)
    {
        if(!right_foot_sound)
        {
            sound::add(current_num + asphalt_start, centre, is_relative);

            current_num = (current_num + 1) % foot_nums;

            right_foot_sound = true;
        }
    }
    else
    {
        right_foot_sound = false;
    }
}

void fighter::set_stance(int _stance)
{
    stance = _stance;
}

bool fighter::can_attack(bodypart_t type)
{
    ///find the last attack of the current type
    ///if its going, and within x ms of finishing, then fine

    int last_pos = -1;

    for(int i=0; i<moves.size(); i++)
    {
        if(moves[i].limb == type)
            last_pos = i;
    }

    if(last_pos == -1)
        return true;

    ///final move of this type not executed, probably cant attack
    ///in the future we need to sum all the times, etc. Ie do it properly
    if(!moves[last_pos].going)
        return false;

    float time_left = moves[last_pos].time_remaining();


    const float threshold = 500;

    if(time_left < threshold)
    {
        return true;
    }

    return false;
}

///this function assumes that an attack movelist keeps a consistent bodypart
void fighter::queue_attack(attack_t type)
{
    if(dead())
        return;

    attack a = attack_list[type];

    if(!can_attack(a.moves.front().limb))
        return;

    for(auto i : a.moves)
    {
        ///this is probably a mistake to initialise this here
        i.damage = attacks::damage_amounts[type];

        add_move(i);
    }
}

void fighter::add_move(const movement& m)
{
    moves.push_back(m);
}

void fighter::try_jump()
{
    if(!jump_info.is_jumping)
    {
        jump_info.is_jumping = true;

        jump_info.current_time = 0;

        jump_info.pre_jump_pos = pos;
    }
}

void fighter::update_sword_rot()
{
    using namespace bodypart;

    if(stance == 0)
    {
        ///use elbow -> hand vec
        vec3f lvec = parts[LHAND].pos - parts[LUPPERARM].pos;
        vec3f rvec = parts[RHAND].pos - parts[RUPPERARM].pos;

        float l_weight = 1.0f;
        float r_weight = 0.0f;

        float total = l_weight + r_weight;

        vec3f avg = (lvec*l_weight + rvec*r_weight) / total;

        weapon.dir = avg.norm();

        vec3f rot = avg.get_euler() + sword_rotation_offset;

        weapon.set_rot(rot);

        parts[LHAND].set_rot(rot);
        parts[RHAND].set_rot(rot);
    }
}

///flush HERE?
void fighter::set_pos(vec3f _pos)
{
    pos = _pos;

    ///stash the lights somewhere
    ///pos only gets set if we're being forcobly moved
    for(auto& i : my_lights)
    {
        i->set_pos({pos.v[0], pos.v[1], pos.v[2]});
    }
}

void fighter::set_rot(vec3f _rot)
{
    rot_diff = _rot - rot;

    rot = _rot;
}

///need to clamp this while attacking
void fighter::set_rot_diff(vec3f diff)
{
    ///check movement list
    ///if move going
    ///and does damage
    ///clamp
    float max_angle_while_damaging = 6.f * frametime / 1000.f;

    float yangle_diff = diff.v[1];

    for(auto& i : moves)
    {
        if(!i.going)
            continue;

        if(i.does(mov::DAMAGING))
        {
            yangle_diff = clamp(yangle_diff, -max_angle_while_damaging, max_angle_while_damaging);
        }
    }

    diff.v[1] = yangle_diff;

    rot_diff = diff;
    rot = rot + diff;
}

vec2f fighter::get_approx_dim()
{
    vec2f real_size = {bodypart::scale, bodypart::scale};

    real_size = real_size * 2.f;
    real_size = real_size + bodypart::scale * 2.f/5.f;

    return real_size;
}

movement* fighter::get_movement(size_t id)
{
    for(auto& i : moves)
    {
        if(id == i.id)
            return &i;
    }

    return nullptr;
}


pos_rot to_world_space(vec3f world_pos, vec3f world_rot, vec3f local_pos, vec3f local_rot)
{
    vec3f relative_pos = local_pos.rot({0,0,0}, world_rot);

    vec3f total_rot = world_rot + local_rot;

    vec3f n_pos = relative_pos + world_pos;

    /*vec3f relative_pos = local_pos.back_rot({0,0,0}, local_rot);

    vec3f n_pos = relative_pos.back_rot(world_pos, world_rot);

    vec3f total_rot = world_rot + local_rot;*/

    return {n_pos, total_rot};
}

void smooth(vec3f& in, vec3f old, float dt)
{
    float diff = (in - old).length();

    if(diff > 100.f)
    {
        return;
    }

    vec3f vec_frac = {40.f, 35.f, 40.f};

    vec3f diff_indep = (in - old);

    diff_indep = (diff_indep / vec_frac) * dt;

    //in = (in + old) / 2.f;

    in = old + diff_indep;
}

///note to self, make this not full of shit
///smoothing still isn't good
void fighter::update_render_positions()
{
    using namespace bodypart;

    /*float dt_smooth = 0.1f * frametime;// *frametime* 0.1f;

    float dt_shoulder =  0.05f * frametime;// *frametime* 0.05f;*/

    smooth(parts[LLOWERARM].pos, old_pos[LLOWERARM], frametime);
    smooth(parts[RLOWERARM].pos, old_pos[RLOWERARM], frametime);

    smooth(parts[RUPPERARM].pos, old_pos[RUPPERARM], frametime);
    smooth(parts[LUPPERARM].pos, old_pos[LUPPERARM], frametime);

    std::map<int, float> foot_heights;

    ///bob OPPOSITE side of body
    float r_bob = parts[LFOOT].pos.v[1] - rest_positions[LFOOT].v[1];
    float l_bob = parts[RFOOT].pos.v[1] - rest_positions[RFOOT].v[1];

    foot_heights[0] = l_bob * 0.7 + r_bob * 0.3;
    foot_heights[1] = l_bob * 0.3 + r_bob * 0.7;
    foot_heights[2] = (foot_heights[0] + foot_heights[1]) / 2.f;

    for(part& i : parts)
    {
        vec3f t_pos = i.pos;

        t_pos.v[1] += foot_heights[which_side[i.type]] * foot_modifiers[i.type] * overall_bob_modifier;

        float idle_parameter = sin(idle_offsets[i.type] + idle_speed * my_time / 1000.f);

        t_pos.v[1] += idle_modifiers[i.type] * idle_height * idle_parameter;

        float twist_extra = shoulder_rotation * waggle_modifiers[i.type];

        t_pos = t_pos.rot({0,0,0}, {0, twist_extra, 0});

        auto r = to_world_space(pos, rot, t_pos, i.rot);

        i.set_global_pos({r.pos.v[0], r.pos.v[1], r.pos.v[2]});
        i.set_global_rot({r.rot.v[0], r.rot.v[1], r.rot.v[2]});

        i.update_model();
    }

    /*///do head look
    {
        part& p = parts[HEAD];

        ///extrinsic xyz
        vec3f rotation = p.global_rot;

        vec3f rvec = p.global_pos - parts[BODY].global_pos;

        ///want to rotate about x
        float look_angle = look.v[0];

        vec3f rotated_vec = rvec.rot({0,0,0}, {-look_angle/2.f, 0, 0});

        ///i think the - is  because of the character's retarded
        ///z reflection
        rotation.v[0] = -look_angle;

        vec3f new_pos = p.global_pos;

        new_pos = new_pos + rotated_vec - rvec;

        p.set_global_pos(new_pos);
        p.set_global_rot(rotation);

        p.update_model();
    }*/

    ///the above headlook never worked

    {
        part& p = parts[HEAD];

        /*///extrinsic xyz
        vec3f rotation = p.global_rot;

        vec3f rvec = p.global_pos - parts[BODY].global_pos;

        ///want to rotate about x
        float look_angle = look.v[0];

        vec3f rotated_vec = rvec.rot({0,0,0}, {-look_angle/2.f, 0, 0});

        ///i think the - is  because of the character's retarded
        ///z reflection
        rotation.v[0] = -look_angle;

        vec3f new_pos = p.global_pos;

        new_pos = new_pos + rotated_vec - rvec;

        p.set_global_pos(new_pos);
        p.set_global_rot(rotation);*/

        //vec3f rvec = p.pos - parts[BODY].pos;

        vec3f rvec = default_position[HEAD] - default_position[BODY];

        vec3f wvec = (p.pos - parts[BODY].pos);

        rvec = wvec * 0.1f + rvec * 0.9f;

        float look_angle = look.v[0];

        ///-look_angle/2.f

        vec3f r1 = rvec.rot({0,0,0}, {-look_angle/2.f, 0, 0});

        vec3f total_rot = {rot.v[0], rot.v[1], rot.v[2]};

        vec3f rotated_vec = r1.rot({0,0,0}, total_rot);

        vec3f angles = rotated_vec.get_euler();


        p.set_global_pos(rotated_vec + pos - rvec + p.pos);
        p.set_global_rot(angles);


        p.update_model();
    }

    //parts[RHAND].set_global_pos(parts[LHAND].global_pos);
    //parts[RHAND].update_model();


    auto r = to_world_space(pos, rot, weapon.pos, weapon.rot);

    weapon.model->set_pos({r.pos.v[0], r.pos.v[1], r.pos.v[2]});
    weapon.model->set_rot({r.rot.v[0], r.rot.v[1], r.rot.v[2]});

    ///calculate distance between links, dynamically adjust positioning
    ///so there's equal slack on both sides
    for(auto& i : joint_links)
    {
        bool active = i.p1->is_active && i.p2->is_active;

        objects_container* obj = i.obj;

        vec3f start = i.p1->global_pos;
        vec3f fin = i.p2->global_pos;

        float desired_len = (fin - start).length();
        float real_len = i.length;

        float diff = 0.f;

        if(desired_len > real_len)
            diff = desired_len - real_len;

        start = start + i.offset;
        fin = fin + i.offset;

        vec3f rot = (fin - start).get_euler();

        vec3f dir = (fin - start);

        //start = start + dir * i.squish_frac;
        start = start + dir.norm() * diff/2.f;

        obj->set_pos({start.v[0], start.v[1], start.v[2]});
        obj->set_rot({rot.v[0], rot.v[1], rot.v[2]});
    }

    ///sent rhand to point towards the weapon centre from its centre

    /*vec3f to_centre = xyz_to_vec(weapon.model->pos) - parts[RHAND].global_pos;

    vec3f sword_vec = weapon.pos

    float len = to_centre.length();

    if(len > 1.f)
    {
        parts[RHAND].set_global_rot(to_centre.get_euler());
        parts[RHAND].update_model();
    }*/

    for(int i=0; i<bodypart::COUNT; i++)
    {
        old_pos[i] = parts[i].pos;
    }
}

void fighter::network_update_render_positions()
{
    using namespace bodypart;

    for(auto& i : joint_links)
    {
        bool active = i.p1->is_active && i.p2->is_active;

        objects_container* obj = i.obj;

        cl_float4 op1 = i.p1->obj()->pos;
        cl_float4 op2 = i.p2->obj()->pos;

        vec3f start = xyz_to_vec(op1);
        vec3f fin = xyz_to_vec(op2);

        start = start + i.offset;
        fin = fin + i.offset;

        vec3f rot = (fin - start).get_euler();

        vec3f dir = (fin - start);

        start = start + dir * i.squish_frac;

        obj->set_pos({start.v[0], start.v[1], start.v[2]});
        obj->set_rot({rot.v[0], rot.v[1], rot.v[2]});
    }
}

void fighter::update_lights()
{
    vec3f lpos = (parts[bodypart::LFOOT].global_pos + parts[bodypart::RFOOT].global_pos) / 2.f;

    my_lights[0]->set_pos({lpos.v[0], lpos.v[1] + 40.f, lpos.v[2]});

    vec3f bpos = (vec3f){0, parts[bodypart::BODY].global_pos.v[1] + 50.f, -150.f}.rot({0,0,0}, rot) + (vec3f){pos.v[0], 0.f, pos.v[2]};

    my_lights[1]->set_pos({bpos.v[0], bpos.v[1], bpos.v[2]});

    vec3f sword_tip = xyz_to_vec(weapon.model->pos) + (vec3f){0, weapon.length, 0}.rot({0,0,0}, xyz_to_vec(weapon.model->rot));

    my_lights[2]->set_pos({sword_tip.v[0], sword_tip.v[1], sword_tip.v[2]});

    for(auto& i : my_lights)
    {
        i->set_radius(1000.f);
        i->set_shadow_casting(0);
        i->set_brightness(1.0f);
        i->set_diffuse(1.0f);
    }

    my_lights[1]->set_radius(700.f);
    my_lights[2]->set_radius(400.f);

    for(auto& i : my_lights)
    {
        if(team == 0)
            i->set_col({1.f, 0.f, 0.f, 0.f});
        else
            i->set_col({0.f, 0.f, 1.f, 0.f});

        //i->set_col({1.f, 1.f, 1.f});
    }

    for(auto& i : my_lights)
    {
        vec3f pos = xyz_to_vec(i->pos);

        ///dirty hack of course
        ///ideally we'd use the alive status for this
        ///but that'd break the death effects
        if(pos.length() > 10000.f)
        {
            i->set_active(false);
        }
        else
        {
            i->set_active(true);
        }
    }
}

void fighter::respawn_if_appropriate()
{
    //printf("respawn %i %i\n", num_dead(), num_needed_to_die());

    ///only respawns if on full health
    ///easiest way to get around some of the delayed dying bugs
    ///could probably still cause issues if we go from full health to 0
    ///in almost no time
    for(auto& i : parts)
    {
        if(i.hp < 0.9999f)
        {
            return;
        }
    }

    //if(num_dead() < num_needed_to_die())
    {
        if(performed_death)
        {
            ///this will still bloody network all the hp changes
            respawn();

            ///hack to stop it from networking the hp change
            ///from respawning the network fighter
            for(auto& i : parts)
            {
                i.net.hp_delta = 0.f;
                i.net.hp_dirty = false;
            }

            //printf("respawning other playern\n");
        }
    }

    //printf("post_respawn\n");
}

///net-fighters ONLY
void fighter::overwrite_parts_from_model()
{
    for(part& i : parts)
    {
        cl_float4 pos = i.obj()->pos;
        cl_float4 rot = i.obj()->rot;

        i.set_global_pos(xyz_to_vec(pos));
        i.set_global_rot(xyz_to_vec(rot));

        i.set_active(i.obj()->isactive);

        i.update_model();
    }

    ///pos.v[1] is not correct I don't think
    ///but im not sure that matters
    pos = parts[bodypart::BODY].global_pos;
    rot = parts[bodypart::BODY].global_rot;

    ///this is so that if the remote fighter is actually alive but we don't know
    ///it can die again
    ///we could avoid this by networking the death state
    ///but that requries more networking. We already have the info, so do it clientside
    ///hp for parts are also 'reliably' delivered (every frame)
    /*if(!should_die())
    {
        performed_death = false;
    }*/

    ///do not need to update weapon because it does not currently have a global position stored (not a part)
}

void fighter::update_texture_by_part_hp()
{
    for(auto& i : parts)
    {
        i.update_texture_by_hp();
    }
}

void fighter::set_team(int _team)
{
    int old = team;

    team = _team;

    for(part& i : parts)
    {
        i.set_team(team);
    }

    weapon.set_team(team);

    if(old == team)
        return;

    for(auto& i : joint_links)
    {
        i.obj->set_active(false);
    }

    joint_links.clear();

    float squish = 0.1f;//-0.2f;

    ///now we know the team, we can add the joint parts
    using namespace bodypart;

    joint_links.push_back(make_link(&parts[LUPPERARM], &parts[LLOWERARM], team, squish));
    joint_links.push_back(make_link(&parts[LLOWERARM], &parts[LHAND], team, squish));

    joint_links.push_back(make_link(&parts[RUPPERARM], &parts[RLOWERARM], team, squish));
    joint_links.push_back(make_link(&parts[RLOWERARM], &parts[RHAND], team, squish));

    joint_links.push_back(make_link(&parts[LUPPERARM], &parts[BODY], team, squish));
    joint_links.push_back(make_link(&parts[RUPPERARM], &parts[BODY], team, squish));

    /*joint_links.push_back(make_link(&parts[LUPPERARM], &parts[RUPPERARM], 0.1f, 25.f, {0, -bodypart::scale * 0.2, 0}));
    joint_links.push_back(make_link(&parts[LUPPERARM], &parts[RUPPERARM], 0.1f, 23.f, {0, -bodypart::scale * 0.8, 0}));
    joint_links.push_back(make_link(&parts[LUPPERLEG], &parts[RUPPERLEG], -0.2f, 21.f, {0, bodypart::scale * 0.2, 0}));*/

    //joint_links.push_back(make_link(&parts[LUPPERARM], &parts[BODY], 0.2f, {0, -bodypart::scale * 0.75f, 0}));
    //joint_links.push_back(make_link(&parts[RUPPERARM], &parts[BODY], 0.2f, {0, -bodypart::scale * 0.75f, 0}));

    //joint_links.push_back(make_link(&parts[LUPPERARM], &parts[BODY], 0.2f, {0, -bodypart::scale * 1.5f, 0}));
    //joint_links.push_back(make_link(&parts[RUPPERARM], &parts[BODY], 0.2f, {0, -bodypart::scale * 1.5f, 0}));

    /*joint_links.push_back(make_link(&parts[LUPPERLEG], &parts[RUPPERLEG], 0.2f));*/

    joint_links.push_back(make_link(&parts[LUPPERLEG], &parts[LLOWERLEG], team, squish));
    joint_links.push_back(make_link(&parts[RUPPERLEG], &parts[RLOWERLEG], team, squish));

    joint_links.push_back(make_link(&parts[LLOWERLEG], &parts[LFOOT], team, squish));
    joint_links.push_back(make_link(&parts[RLOWERLEG], &parts[RFOOT], team, squish));


    for(auto& i : joint_links)
    {
        i.obj->set_active(true);
    }

    cpu_context->load_active();

    for(auto& i : joint_links)
    {
        i.obj->set_specular(bodypart::specular);
    }

    cpu_context->build();
    cpu_context->flip();
    gpu_context = cpu_context->fetch();
}

void fighter::set_physics(physics* _phys)
{
    phys = _phys;

    for(part& i : parts)
    {
        phys->add_objects_container(&i, this);
    }
}

void fighter::cancel(bodypart_t type)
{
    for(auto it = moves.begin(); it != moves.end();)
    {
        if(it->limb == type)
        {
            action_map.erase(it->limb);

            it = moves.erase(it);
        }
        else
            it++;
    }
}

void fighter::cancel_hands()
{
    cancel(bodypart::LHAND);
    cancel(bodypart::RHAND);
}

///wont recoil more than once, because recoil is not a kind of windup
void fighter::checked_recoil()
{
    movement lhand = action_map[bodypart::LHAND];
    movement rhand = action_map[bodypart::RHAND];

    if(lhand.does(mov::WINDUP) || rhand.does(mov::WINDUP))
    {
        recoil();
    }
}

bool fighter::can_recoil()
{
    movement lhand = action_map[bodypart::LHAND];
    movement rhand = action_map[bodypart::RHAND];

    if(lhand.does(mov::WINDUP) || rhand.does(mov::WINDUP))
    {
        return true;
    }

    return false;
}

void fighter::recoil()
{
    cancel(bodypart::LHAND);
    cancel(bodypart::RHAND);
    queue_attack(attacks::RECOIL);
}

void fighter::try_feint()
{
    const float unfeintable_time = attacks::unfeintable_time;

    movement lhand = action_map[bodypart::LHAND];
    movement rhand = action_map[bodypart::RHAND];

    bool lfeint = lhand.does(mov::WINDUP) && (lhand.time_remaining() > unfeintable_time);
    bool rfeint = rhand.does(mov::WINDUP) && (rhand.time_remaining() > unfeintable_time);

    if(lfeint || rfeint)
    {
        cancel(bodypart::LHAND);
        cancel(bodypart::RHAND);

        queue_attack(attacks::FEINT);
    }
}


///i've taken damage. If im during the windup phase of an attack, recoil
void fighter::damage(bodypart_t type, float d)
{
    using namespace bodypart;

    bool do_explode_effect = num_dead() < num_needed_to_die() - 1;

    parts[type].damage(d, do_explode_effect);

    net.recoil = 1;
    net.recoil_dirty = true;
    //network::host_update(&net.recoil);
}

void fighter::set_contexts(object_context* _cpu, object_context_data* _gpu)
{
    if(_cpu)
        cpu_context = _cpu;

    if(_gpu)
        gpu_context = _gpu;
}

///problem is, whenever someone's name gets updated, everyone else's old name gets overwritten
///we need to update all fighters whenever there's an update, or a new fighter created
void fighter::set_name(std::string name)
{
    //texture* tex = texture_manager::texture_by_id(name_tex_gpu.id);

    if(!name_tex_gpu)
        return;

    local_name = name;

    vec2f fname = {name_dim.x, name_dim.y};

    name_tex.clear(sf::Color(0,0,0,0));
    name_tex.display();

    name_tex.setActive(true);

    name_tex_gpu->update_gpu_texture(name_tex.getTexture(), transparency_context->fetch()->tex_gpu_ctx, false);
    name_tex_gpu->update_gpu_mipmaps(transparency_context->fetch()->tex_gpu_ctx);

    name_tex.setActive(false);

    text::immediate(&name_tex, local_name, fname/2.f, 16, true);
    name_tex.display();

    //tex->update_gpu_texture_col({0.f, 255.f, 0.f, 255.f}, transparency_context->fetch()->tex_gpu);

    name_tex.setActive(true);

    name_tex_gpu->update_gpu_texture(name_tex.getTexture(), transparency_context->fetch()->tex_gpu_ctx, false);
    name_tex_gpu->update_gpu_mipmaps(transparency_context->fetch()->tex_gpu_ctx);

    name_tex.setActive(false);
}

void fighter::set_secondary_context(object_context* _transparency_context)
{
    if(_transparency_context == nullptr)
    {
        printf("massive error in set_secondary_context\n");

        exit(44334);
    }

    if(name_container)
    {
        name_container->set_active(false);
    }

    transparency_context = _transparency_context;

    name_tex.create(name_dim.x, name_dim.y);
    name_tex.clear(sf::Color(0, 0, 0, 0));
    name_tex.display();

    /*name_tex_gpu.set_texture_location("Res/128x128.png");
    name_tex_gpu.set_unique();
    name_tex_gpu.push();*/

    ///destroy name_tex_gpu

    name_tex_gpu = _transparency_context->tex_ctx.make_new();
    name_tex_gpu->set_location("Res/128x128.png");
    //name_tex_gpu->set_unique();

    name_container = transparency_context->make_new();
    name_container->set_load_func(std::bind(obj_rect, std::placeholders::_1, *name_tex_gpu, name_obj_dim));

    name_container->set_unique_textures(true);
    name_container->cache = false;
    name_container->set_active(true);

    transparency_context->load_active();

    name_container->set_two_sided(true);
    name_container->set_diffuse(10.f);

    transparency_context->build();
    transparency_context->flip();

    //name_tex_gpu.update_gpu_texture(name_tex.getTexture(), transparency_context->fetch()->tex_gpu);
    //name_tex_gpu.update_gpu_mipmaps(transparency_context->fetch()->tex_gpu);
}

void fighter::update_name_info(bool networked_fighter)
{
    if(!name_container)
        return;

    vec3f head_pos = parts[bodypart::HEAD].global_pos;

    float offset = bodypart::scale;

    name_container->set_pos({head_pos.v[0], head_pos.v[1] + offset, head_pos.v[2]});
    name_container->set_rot({rot.v[0], rot.v[1], rot.v[2]});

    if(!name_tex_gpu)
        return;

    if(name_reset_timer.getElapsedTime().asMilliseconds() > 5000.f)
    {
        ///we've got the correct local name, but it wont blit for some reason
        if(!networked_fighter)
            set_name(local_name);
        else
        {
            std::string str;

            for(int i=0; i<MAX_NAME_LENGTH && net.net_name.v[i] != 0; i++)
            {
                str.push_back(net.net_name.v[i]);
            }

            str.push_back(0);

            printf("fighter network name %s\n", str.c_str());

            ///turns out the hack was just disguising the real problem (rendering an invalid string)
            set_name(str);
        }

        name_reset_timer.restart();
    }
}
