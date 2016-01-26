//#include "../openclrenderer/proj.hpp"
#include <winsock2.h>
#include "../openclrenderer/engine.hpp"
#include "../openclrenderer/ocl.h"
#include "../openclrenderer/texture_manager.hpp"

#include "../openclrenderer/text_handler.hpp"
#include <sstream>
#include <string>
#include "../openclrenderer/vec.hpp"

#include "../openclrenderer/ui_manager.hpp"

#include "fighter.hpp"
#include "text.hpp"
#include "physics.hpp"

#include "../openclrenderer/network.hpp"

#include "sound.hpp"

#include "object_cube.hpp"
#include "particle_effect.hpp"

#include "../openclrenderer/settings_loader.hpp"
#include "../openclrenderer/controls.hpp"
#include "map_tools.hpp"

#include "server_networking.hpp"
#include "../openclrenderer/game/space_manager.hpp" ///yup
#include "../openclrenderer/game/galaxy/galaxy.hpp"

#include "game_state_manager.hpp"

///has the button been pressed once, and only once
template<sf::Keyboard::Key k>
bool once()
{
    static bool last;

    sf::Keyboard key;

    if(key.isKeyPressed(k) && !last)
    {
        last = true;

        return true;
    }

    if(!key.isKeyPressed(k))
    {
        last = false;
    }

    return false;
}

template<sf::Mouse::Button b>
bool once()
{
    static bool last;

    sf::Mouse m;

    if(m.isButtonPressed(b) && !last)
    {
        last = true;

        return true;
    }

    if(!m.isButtonPressed(b))
    {
        last = false;
    }

    return false;
}

///none of these affect the camera, so engine does not care about them
///assume main is blocking
void debug_controls(fighter* my_fight, engine& window)
{
    sf::Keyboard key;

    if(once<sf::Keyboard::T>())
    {
        my_fight->queue_attack(attacks::OVERHEAD);
    }

    if(once<sf::Keyboard::Y>())
    {
        my_fight->queue_attack(attacks::SLASH);
    }

    if(once<sf::Keyboard::G>())
    {
        my_fight->queue_attack(attacks::REST);
    }

    if(once<sf::Keyboard::R>())
    {
        my_fight->queue_attack(attacks::BLOCK);
    }

    if(once<sf::Keyboard::H>())
    {
        my_fight->try_feint();
    }

    if(once<sf::Keyboard::SemiColon>())
    {
        my_fight->die();
    }

    float y_diff = 0;

    if(key.isKeyPressed(sf::Keyboard::U))
    {
        y_diff = 0.01f * window.get_frametime()/2000.f;
    }

    if(key.isKeyPressed(sf::Keyboard::O))
    {
        y_diff = -0.01f * window.get_frametime()/2000.f;
    }

    my_fight->set_rot_diff({0, y_diff, 0});

    static float look_height = 0.f;

    if(key.isKeyPressed(sf::Keyboard::Comma))
    {
        look_height += 0.01f * window.get_frametime() / 8000.f;
    }

    if(key.isKeyPressed(sf::Keyboard::Period))
    {
        look_height += -0.01f * window.get_frametime() / 8000.f;
    }

    my_fight->set_look({look_height, 0.f, 0.f});

    vec2f walk_dir = {0,0};

    if(key.isKeyPressed(sf::Keyboard::I))
        walk_dir.v[0] = -1;

    if(key.isKeyPressed(sf::Keyboard::K))
        walk_dir.v[0] = 1;

    if(key.isKeyPressed(sf::Keyboard::J))
        walk_dir.v[1] = -1;

    if(key.isKeyPressed(sf::Keyboard::L))
        walk_dir.v[1] = 1;

    if(key.isKeyPressed(sf::Keyboard::P))
        my_fight->try_jump();

    bool sprint = key.isKeyPressed(sf::Keyboard::LShift);

    my_fight->walk_dir(walk_dir, sprint);
}

void fps_controls(fighter* my_fight, engine& window)
{
    sf::Keyboard key;

    if(key.isKeyPressed(sf::Keyboard::Escape))
        exit(0);

    vec2f walk_dir = {0,0};

    if(key.isKeyPressed(sf::Keyboard::W))
        walk_dir.v[0] = -1;

    if(key.isKeyPressed(sf::Keyboard::S))
        walk_dir.v[0] = 1;

    if(key.isKeyPressed(sf::Keyboard::A))
        walk_dir.v[1] = -1;

    if(key.isKeyPressed(sf::Keyboard::D))
        walk_dir.v[1] = 1;

    bool sprint = key.isKeyPressed(sf::Keyboard::LShift);

    my_fight->walk_dir(walk_dir, sprint);

    if(once<sf::Mouse::Left>())
        my_fight->queue_attack(attacks::SLASH);

    if(once<sf::Mouse::Middle>())
        my_fight->queue_attack(attacks::OVERHEAD);

    if(once<sf::Mouse::Right>())
        my_fight->queue_attack(attacks::BLOCK);

    if(once<sf::Keyboard::Q>())
        my_fight->try_feint();

    if(once<sf::Keyboard::Space>())
        my_fight->try_jump();

    my_fight->set_look({-window.c_rot.s[0], window.get_mouse_delta_x() / 1.f, 0});

    //part* head = &my_fight->parts[bodypart::HEAD];

    //vec3f pos = head->pos + my_fight->pos;

    //window.set_camera_pos({pos.v[0], pos.v[1], pos.v[2]});

    vec2f m;
    m.v[0] = window.get_mouse_delta_x();
    m.v[1] = window.get_mouse_delta_y();

    my_fight->set_rot_diff({0, -m.v[0] / 100.f, 0.f});

    //vec3f o_rot = xyz_to_vec(window.c_rot);

    //o_rot.v[1] = my_fight->rot.v[1];
    //o_rot.v[0] += m.v[1] / 200.f;

    //window.set_camera_rot({o_rot.v[0], -o_rot.v[1] + M_PI, o_rot.v[2]});
}

input_delta fps_camera_controls(float frametime, const input_delta& input, engine& window, const fighter* my_fight)
{
    const part* head = &my_fight->parts[bodypart::HEAD];

    vec3f pos = head->pos + my_fight->pos;

    //window.set_camera_pos({pos.v[0], pos.v[1], pos.v[2]});

    cl_float4 c_pos = {pos.v[0], pos.v[1], pos.v[2]};

    vec2f m;
    m.v[0] = window.get_mouse_delta_x();
    m.v[1] = window.get_mouse_delta_y();

    vec3f o_rot = xyz_to_vec(input.c_rot);

    o_rot.v[1] = my_fight->rot.v[1];
    o_rot.v[0] += m.v[1] / 150.f;

    //window.set_camera_rot({o_rot.v[0], -o_rot.v[1] + M_PI, o_rot.v[2]});

    cl_float4 c_rot = {o_rot.v[0], -o_rot.v[1] + M_PI, o_rot.v[2]};

    return {sub(c_pos, input.c_pos), sub(c_rot, input.c_rot)};
}

int main(int argc, char *argv[])
{
    /*objects_container c1;
    c1.set_load_func(std::bind(load_object_cube, std::placeholders::_1, (vec3f){0, 0, 0}, (vec3f){0, 100, -100}, 20.f));
    c1.cache = false;
    c1.set_active(true);

    objects_container c2;
    c2.set_load_func(std::bind(load_object_cube, std::placeholders::_1, (vec3f){0, 0, 0}, (vec3f){100, -100, 100}, 20.f));
    c2.cache = false;
    c2.set_active(true);*/

    sf::Clock clk;

    object_context context;
    object_context_data* gpu_context = context.fetch();

    ///really old and wrong, ignore me
    /*objects_container* floor = context.make_new();
    floor->set_load_func(std::bind(load_object_cube, std::placeholders::_1,
                                  (vec3f){0, bodypart::default_position[bodypart::LFOOT].v[1] - bodypart::scale/3, 0},
                                  (vec3f){0, bodypart::default_position[bodypart::LFOOT].v[1] - 42.f, 0},
                                  3000.f, "./res/gray.png"));*/

    world_map default_map;
    //default_map.init(map_namespace::map_one, 11, 12);
    default_map.init(0);

    gameplay_state current_state;
    current_state.set_map(default_map);

    objects_container* floor = context.make_new();
    floor->set_load_func(default_map.get_load_func());

    ///need to extend this to textures as well
    floor->set_normal("./res/norm.png");
    floor->cache = false;
    floor->set_active(true);
    //floor->set_pos({0, bodypart::default_position[bodypart::LFOOT].v[1] - bodypart::scale/3, 0});
    floor->offset_pos({0, bodypart::default_position[bodypart::LFOOT].v[1] - bodypart::scale/3, 0});


    /*objects_container* file_map = context.make_new();
    file_map->set_file("./res/map2.obj");
    file_map->set_active(false);
    file_map->set_pos({0, bodypart::default_position[bodypart::LFOOT].v[1] - bodypart::scale/3 - 0, 0});*/
    //file_map.set_normal("res/norm_body.png");

    settings s;
    s.load("./res/settings.txt");

    engine window;
    window.load(s.width,s.height,1000, "SwordFight", "../openclrenderer/cl2.cl", true);

    window.set_camera_pos((cl_float4){-800,150,-570});

    window.window.setVerticalSyncEnabled(false);

    //window.window.setFramerateLimit(24.f);

    printf("loaded\n");

    text::set_renderwindow(window.window);

    window.set_camera_pos({-1009.17, -94.6033, -317.804});
    window.set_camera_rot({0, 1.6817, 0});

    fighter fight(context, *gpu_context);
    fight.set_team(0);
    fight.set_quality(s.quality);
    fight.set_gameplay_state(&current_state);
    //fight.my_cape.make_stable(&fight);

    fighter fight2(context, *gpu_context);
    fight2.set_team(1);
    fight2.set_pos({0, 0, -650});
    fight2.set_rot({0, M_PI, 0});
    fight2.set_quality(s.quality);
    fight2.set_gameplay_state(&current_state);


    physics phys;
    phys.load();

    printf("preload\n");

    context.load_active();

    printf("postload\n");

    fight.set_physics(&phys);
    fight2.set_physics(&phys);

    printf("loaded net fighters\n");

    ///a very high roughness is better (low spec), but then i think we need to remove the overhead lights
    ///specular component
    floor->set_specular(0.01f);
    floor->set_diffuse(4.f);

    texture_manager::allocate_textures();
    auto tex_gpu = texture_manager::build_descriptors();

    window.set_tex_data(tex_gpu);

    printf("textures\n");

    context.build();

    auto ctx = context.fetch();
    window.set_object_data(*ctx);

    printf("loaded memory\n");

    sf::Event Event;

    light l;
    //l.set_col({1.0, 1.0, 1.0, 0});
    l.set_col({1.0, 1.0, 1.0, 0});
    l.set_shadow_casting(0);
    l.set_brightness(0.215f);
    l.set_diffuse(1.f);
    l.set_pos({0, 10000, -300, 0});

    //window.add_light(&l);

    light::add_light(&l);

    auto light_data = light::build();

    window.set_light_data(light_data);

    printf("light\n");

    server_networking server;

    sf::Mouse mouse;
    sf::Keyboard key;

    vec3f original_pos = fight.parts[bodypart::LFOOT].pos;

    vec3f seek_pos = original_pos;

    vec3f rest_position = {0, -200, -100};

    fighter* my_fight = &fight;

    printf("Presspace\n");

    space_manager space_res;
    space_res.init(s.width, s.height);

    point_cloud stars = get_starmap(1);
    point_cloud_info g_star_cloud = point_cloud_manager::alloc_point_cloud(stars);

    printf("Postspace\n");

    ///debug;
    int controls_state = 0;

    printf("loop\n");

    {
        printf("%i\n", context.containers.size());
    }

    while(window.window.isOpen())
    {
        sf::Clock c;

        while(window.window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed)
                window.window.close();

            if(Event.type == sf::Event::Resized)
            {
                cl::cqueue.finish();
                cl::cqueue2.finish();

                window.load(Event.size.width, Event.size.height, 1000, "SwordFight", "../openclrenderer/cl2.cl", true);

                light_data = light::build();

                window.set_light_data(light_data);

                context.build();
                gpu_context = context.fetch();

                g_star_cloud = point_cloud_manager::alloc_point_cloud(stars);

                window.set_object_data(*gpu_context);
                window.set_light_data(light_data);

                space_res.init(window.width, window.height);

                text::set_renderwindow(window.window);

                cl::cqueue.finish();
                cl::cqueue2.finish();
            }
        }

        if(controls_state == 0)
            window.update_mouse();
        if(controls_state == 1)
            window.update_mouse(window.width/2, window.height/2, true, true);

        if(once<sf::Keyboard::X>())
        {
            controls_state = (controls_state + 1) % 2;

            ///call once to reset mouse to centre
            window.update_mouse(window.width/2, window.height/2, true, true);
            ///call again to reset mouse dx and dy to 0
            window.update_mouse(window.width/2, window.height/2, true, true);
        }

        if(controls_state == 0)
            debug_controls(my_fight, window);
        if(controls_state == 1)
            fps_controls(my_fight, window);

        control_input c_input;

        if(controls_state == 0)
            c_input = control_input();

        if(controls_state == 1)
            c_input = control_input(std::bind(fps_camera_controls, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, my_fight),
                              process_controls_empty);

        window.set_input_handler(c_input);

        server.set_my_fighter(my_fight);
        server.tick(&context, &current_state, &phys);

        ///debugging
        if(!server.joined_game)
            server.set_game_to_join(0);

        std::string display_string = server.game_info.get_display_string();

        text::add(display_string, 0, (vec2f){window.width/2.f, window.height - 20});

        if(server.game_info.game_over())
        {
            text::add(server.game_info.get_game_over_string(), 0, (vec2f){window.width/2.f, window.height/2.f});
        }

        ///network players don't die on a die
        ///because dying doesn't update part hp
        if(server.just_new_round && !my_fight->dead())
        {
            my_fight->die();
        }

        if(my_fight->dead())
        {
            std::string disp_string = server.respawn_inf.get_display_string();

            text::add(disp_string, 0, (vec2f){window.width/2.f, 20});
        }

        if(once<sf::Keyboard::B>())
        {
            my_fight->respawn();
        }

        //static float debug_look = 0;
        //my_fight->set_look({sin(debug_look), 0, 0});
        //debug_look += 0.1f;

        /*phys.tick();
        vec3f v = phys.get_pos();
        c1.set_pos({v.v[0], v.v[1], v.v[2]});
        c1.g_flush_objects();*/

        if(network::network_state == 0)
        {
            fight2.queue_attack(attacks::SLASH);
            //fight2.queue_attack(attacks::BLOCK);

            fight2.tick();
            fight2.tick_cape();

            fight2.update_render_positions();

            if(!fight2.dead())
                fight2.update_lights();

            if(once<sf::Keyboard::N>())
            {
                vec3f loc = fight2.parts[bodypart::BODY].global_pos;
                vec3f rot = fight2.parts[bodypart::BODY].global_rot;

                fight2.respawn({loc.v[0], loc.v[2]});
                fight2.set_rot(rot);
            }
        }

        /*fight3.tick();
        fight3.update_render_positions();
        fight3.update_lights();*/

        /*int hit_p = phys.sword_collides(fight.weapon, &fight, {0, 0, -1});
        if(hit_p != -1)
            printf("%s\n", bodypart::names[hit_p % (bodypart::COUNT)].c_str());*/

        my_fight->tick(true);

        my_fight->tick_cape();

        my_fight->update_render_positions();

        if(!my_fight->dead())
            my_fight->update_lights();

        particle_effect::tick();

        ///about 0.2ms slower than not doing this
        light_data = light::build();
        window.set_light_data(light_data);

        context.flush_locations();

        ///ergh
        sound::set_listener(my_fight->parts[bodypart::BODY].global_pos, my_fight->parts[bodypart::BODY].global_rot);
        sound::update_listeners();

        context.flip();
        object_context_data* cdat = context.fetch();

        window.set_object_data(*cdat);

        window.blit_to_screen();

        ///I need to reenable text drawing
        ///possibly split up window.display into display and flip
        ///then have display set a flag if its appropriate to flip the screen
        ///that way we can still keep the async rendering
        ///but also allow drawing on top of teh 3d scene
        ///we'll need to allow window querying to say should we draw
        ///otherwise in async we'll waste huge performance
        ///in synchronous that's not a problem

        text::draw();

        window.flip();

        window.render_block();

        space_res.set_depth_buffer(window.depth_buffer[window.nbuf]);
        space_res.set_screen(window.g_screen);
        space_res.update_camera(window.c_pos, window.c_rot);

        space_res.draw_galaxy_cloud_modern(g_star_cloud, (cl_float4){-5000,-8500,0});

        window.draw_bulk_objs_n();
        space_res.blit_space_to_screen();
        space_res.clear_buffers();

        if(key.isKeyPressed(sf::Keyboard::M))
            std::cout << c.getElapsedTime().asMicroseconds() << std::endl;

        /*printf("TTL %f\n", clk.getElapsedTime().asMicroseconds() / 1000.f);

        return 0;*/
    }

    cl::cqueue.finish();
}
