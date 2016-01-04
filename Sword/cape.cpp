#include "cape.hpp"
#include "../openclrenderer/vertex.hpp"
#include "../openclrenderer/objects_container.hpp"
#include "../openclrenderer/texture.hpp"
#include "../openclrenderer/object.hpp"
#include "../openclrenderer/clstate.h"
#include "../openclrenderer/engine.hpp"
#include "../openclrenderer/obj_mem_manager.hpp"
#include <vec/vec.hpp>
#include "physics.hpp"
#include "fighter.hpp"
#include "../openclrenderer/vec.hpp"

///10, 30
///8, 10
#define WIDTH 8
#define HEIGHT 10

void cape::load_cape(objects_container* pobj, int team)
{
    constexpr int width = WIDTH;
    constexpr int height = HEIGHT;

    vertex verts[height][width];

    const float separation = 10.f;

    for(int j=0; j<height; j++)
    {
        for(int i=0; i<width; i++)
        {
            float xpos = i * separation;
            float ypos = j * separation;
            float zpos = 0;

            verts[j][i].set_pos({xpos, ypos, zpos});
            verts[j][i].set_vt({(float)i/width, (float)j/height});
            verts[j][i].set_normal({0, 1, 0});
        }
    }

    std::vector<triangle> tris;
    //tris.resize(width*height*2);

    for(int j=0; j<height-1; j++)
    {
        for(int i=0; i<width-1; i++)
        {
            triangle t;

            ///winding order is apparently clockwise. That or something else is wrong
            ///likely the latter
            t.vertices[0] = verts[j][i];
            t.vertices[1] = verts[j][i+1];
            t.vertices[2] = verts[j+1][i];

            tris.push_back(t);

            t.vertices[0] = verts[j][i+1];
            t.vertices[1] = verts[j+1][i+1];
            t.vertices[2] = verts[j+1][i];

            tris.push_back(t);
        }
    }

    for(int i=0; i<width-1; i++)
    {
        triangle t;

        t.vertices[0] = verts[0][0];
        t.vertices[1] = verts[0][1];
        t.vertices[2] = verts[1][0];

        tris.push_back(t);

        t.vertices[0] = verts[0][1];
        t.vertices[1] = verts[1][1];
        t.vertices[2] = verts[1][0];

        tris.push_back(t);
    }

    texture tex;
    tex.type = 0;

    if(team == 0)
        tex.set_texture_location("./res/red.png");
    else
        tex.set_texture_location("./res/blue.png");

    tex.push();

    texture normal;

    if(pobj->normal_map != "")
    {
        normal.type = 0;
        normal.set_texture_location(pobj->normal_map.c_str());
        normal.push();
    }

    object obj;
    obj.tri_list = tris;

    obj.tri_num = obj.tri_list.size();

    obj.tid = tex.id;
    obj.bid = -1;
    obj.rid = normal.id;

    obj.has_bump = 0;

    obj.pos = pobj->pos;
    obj.rot = pobj->rot;

    pobj->objs.push_back(obj);

    //pobj->set_rot({0, M_PI, 0});

    pobj->isloaded = true;
}

cape::cape(object_context& cpu, object_context_data& gpu)
{
    width = WIDTH;
    height = HEIGHT;
    depth = 1;

    loaded = false;

    cpu_context = &cpu;
    gpu_context = &gpu;
}

void cape::load(int team)
{
    if(loaded)
        return;

    model = cpu_context->make_new();

    model->set_load_func(std::bind(cape::load_cape, std::placeholders::_1, team));
    model->set_active(true);
    model->cache = false;
    //model->set_normal("res/norm_body.png");

    //obj_mem_manager::load_active_objects();

    cpu_context->load_active();

    model->set_two_sided(true);
    model->set_specular(0.7);

    //obj_mem_manager::g_arrange_mem();
    //obj_mem_manager::g_changeover();

    cpu_context->build();
    gpu_context = cpu_context->fetch();

    which = 0;

    in = compute::buffer(cl::context, sizeof(float)*width*height*depth*3, CL_MEM_READ_WRITE, nullptr);
    out = compute::buffer(cl::context, sizeof(float)*width*height*depth*3, CL_MEM_READ_WRITE, nullptr);

    cl_float* inmap = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), in.get(), CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float)*width*height*depth*3, 0, NULL, NULL, NULL);
    cl_float* outmap = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), out.get(), CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float)*width*height*depth*3, 0, NULL, NULL, NULL);

    const float separation = 10.f;

    for(int j=0; j<height; j++)
    {
        for(int i=0; i<width; i++)
        {
            float xpos = i * separation;
            float ypos = j * separation;
            float zpos = 0;

            inmap[(i + j*width)*3 + 0] = xpos;
            inmap[(i + j*width)*3 + 1] = ypos;
            inmap[(i + j*width)*3 + 2] = zpos;

            outmap[(i + j*width)*3 + 0] = xpos;
            outmap[(i + j*width)*3 + 1] = ypos;
            outmap[(i + j*width)*3 + 2] = zpos;
        }
    }

    clEnqueueUnmapMemObject(cl::cqueue.get(), in.get(), inmap, 0, NULL, NULL);
    clEnqueueUnmapMemObject(cl::cqueue.get(), out.get(), outmap, 0, NULL, NULL);

    loaded = true;
}

void cape::make_stable(fighter* parent)
{
    if(!loaded)
        return;

    int num = 100;

    for(int i=0; i<num; i++)
    {
        tick(parent->parts[bodypart::LUPPERARM].obj(),
                               parent->parts[bodypart::BODY].obj(),
                               parent->parts[bodypart::RUPPERARM].obj(),
                               parent
                                );
    }
}

compute::buffer cape::fighter_to_fixed(objects_container* l, objects_container* m, objects_container* r)
{
    vec3f position = xyz_to_vec(m->pos);
    vec3f rotation = xyz_to_vec(m->rot);

    vec3f lpos = xyz_to_vec(l->pos);
    vec3f rpos = xyz_to_vec(r->pos);

    bbox lbbox = get_bbox(l);
    bbox rbbox = get_bbox(r);

    float ldepth = lbbox.max.v[2] - lbbox.min.v[2];
    float rdepth = rbbox.max.v[2] - rbbox.min.v[2];

    lpos = lpos + (vec3f){0, 0, ldepth}.rot({0,0,0}, rotation);
    rpos = rpos + (vec3f){0, 0, rdepth}.rot({0,0,0}, rotation);

    vec3f dir = rpos - lpos;

    int len = width;

    vec3f step = dir / (float)len;

    vec3f current = lpos;

    /*cl_float* xmap = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), defx.get(), CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float)*w, 0, NULL, NULL, NULL);
    cl_float* ymap = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), defy.get(), CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float)*w, 0, NULL, NULL, NULL);
    cl_float* zmap = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), defz.get(), CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float)*w, 0, NULL, NULL, NULL);

    for(int i=0; i < len; i ++)
    {
        xmap[i] = current.v[0];
        ymap[i] = current.v[1];
        zmap[i] = current.v[2];

        current = current + step;
    }

    clEnqueueUnmapMemObject(cl::cqueue.get(), defx.get(), xmap, 0, NULL, NULL);
    clEnqueueUnmapMemObject(cl::cqueue.get(), defy.get(), ymap, 0, NULL, NULL);
    clEnqueueUnmapMemObject(cl::cqueue.get(), defz.get(), zmap, 0, NULL, NULL);*/

    compute::buffer buf = compute::buffer(cl::context, sizeof(float)*width*3, CL_MEM_READ_WRITE, nullptr);

    cl_float* mem_map = (cl_float*) clEnqueueMapBuffer(cl::cqueue.get(), buf.get(), CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0, sizeof(cl_float)*width*3, 0, NULL, NULL, NULL);

    for(int i=0; i<len; i++)
    {
        mem_map[i*3 + 0] = current.v[0];
        mem_map[i*3 + 1] = current.v[1];
        mem_map[i*3 + 2] = current.v[2];

        current = current + step;
    }

    clEnqueueUnmapMemObject(cl::cqueue.get(), buf.get(), mem_map, 0, NULL, NULL);

    return buf;
}

///use pcie instead etc
compute::buffer body_to_gpu(fighter* parent)
{
    std::vector<cl_float4> pos;

    for(auto& i : parent->parts)
    {
        vec3f p = i.global_pos;

        pos.push_back({p.v[0], p.v[1], p.v[2]});
    }

    vec3f half = (parent->parts[bodypart::LUPPERLEG].global_pos + parent->parts[bodypart::RUPPERLEG].global_pos)/2.f;
    vec3f half2 = (parent->parts[bodypart::LLOWERLEG].global_pos + parent->parts[bodypart::RLOWERLEG].global_pos)/2.f;

    vec3f half3 = (half + parent->parts[bodypart::BODY].global_pos)/2.f;

    pos.push_back({half.v[0], half.v[1], half.v[2]});
    pos.push_back({half2.v[0], half2.v[1], half2.v[2]});
    pos.push_back({half3.v[0], half3.v[1], half3.v[2]});

    compute::buffer buf = compute::buffer(cl::context, sizeof(cl_float4)*pos.size(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pos.data());

    return buf;
}

struct wind
{
    cl_float4 dir = (cl_float4){0.f, 0.f, 0.f, 0.f};
    cl_float amount = 0;

    cl_float gust = 0;

    sf::Clock clk;

    compute::buffer tick()
    {
        dir = {1, 0, 0};

        float weight = 100.f;
        amount = (amount*weight + randf_s(0.f, 0.9f)) / (1.f + weight);

        float time = clk.getElapsedTime().asMicroseconds() / 1000.f;

        std::vector<cl_float4> accel;

        for(int i=0; i<WIDTH; i++)
        {
            vec3f dir = randf<3, float>(0.f, 1.f);

            dir = dir * dir * dir;

            dir = dir * amount;

            dir = dir * 5.f;

            cl_float4 ext = {dir.v[0], dir.v[1], dir.v[2]};

            accel.push_back(ext);
        }

        return compute::buffer(cl::context, sizeof(cl_float4)*accel.size(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, accel.data());
    }
};

void cape::tick(objects_container* l, objects_container* m, objects_container* r, fighter* parent)
{
    if(!loaded)
    {
        load(parent->team);
        return;
    }

    static wind w;
    auto wind_buf = w.tick();

    cl_float4 wind_dir = w.dir;
    cl_float wind_str = w.amount;
    //cl_float wind_side = w.gust;

    auto buf = body_to_gpu(parent);
    int num = bodypart::COUNT + 3;

    arg_list cloth_args;

    cloth_args.push_back(&gpu_context->g_tri_mem);
    cloth_args.push_back(&model->objs[0].gpu_tri_start);
    cloth_args.push_back(&model->objs[0].gpu_tri_end);
    cloth_args.push_back(&width);
    cloth_args.push_back(&height);
    cloth_args.push_back(&depth);

    compute::buffer b1 = which == 0 ? in : out;
    compute::buffer b2 = which == 0 ? out : in;

    compute::buffer fixed = fighter_to_fixed(l, m, r);

    cloth_args.push_back(&b1);
    cloth_args.push_back(&b2);
    cloth_args.push_back(&fixed);
    cloth_args.push_back(&engine::g_screen);
    cloth_args.push_back(&buf);
    cloth_args.push_back(&num);
    cloth_args.push_back(&wind_dir);
    cloth_args.push_back(&wind_str);
    cloth_args.push_back(&wind_buf);

    cl_uint global_ws[1] = {width*height*depth};
    cl_uint local_ws[1] = {256};

    run_kernel_with_string("cloth_simulate", global_ws, local_ws, 1, cloth_args, cl::cqueue);

    //cl::cqueue2.finish();

    which = (which + 1) % 2;
}
