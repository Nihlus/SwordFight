#include "object_cube.hpp"
#include "../openclrenderer/objects_container.hpp"
#include "../openclrenderer/vec.hpp"
#include "../openclrenderer/texture.hpp"
#include "vec.hpp"
#include <vector>
#include <array>

void rect_to_tris(std::array<cl_float4, 4> p, cl_float4 out[2][3])
{
    out[0][0] = p[0];
    out[0][1] = p[1];
    out[0][2] = p[3];

    out[1][0] = p[1];
    out[1][1] = p[2];
    out[1][2] = p[3];
}

triangle points_to_tri(cl_float4 in[3], float size, float len, int which_side) ///0 top and bottom, 1 front/back, 2 left right
{
    triangle t;

    for(int i=0; i<3; i++)
        t.vertices[i].set_pos(in[i]);


    ///should probs fix this being slightly wrong, could eliminate edge boundaries
    ///and then just use the twice sized texture
    for(int i=0; i<3; i++)
    {
        float mx = 0;
        float mz = 0;

        if(which_side == 0)
        {
            mx = in[i].x / (size * 2);
            mz = in[i].z / (size * 2);

            mx += 0.5f;
            mz += 0.5f;
        }
        else if(which_side == 1)
        {
            mx = in[i].x / (size * 2);
            mz = in[i].y / (len * 2);

            mx += 0.5f;
        }
        else if(which_side == 2)
        {
            mx = in[i].z / (size * 2);
            mz = in[i].y / (len * 2);

            mx += 0.5f;
        }

        t.vertices[i].set_vt({mx, mz});
    }


    for(int i=0; i<3; i++)
        t.vertices[i].set_normal(cross(sub(in[1], in[0]), sub(in[2], in[0])));

        return t;
}

///fin - start is the object direction
void load_object_cube(objects_container* pobj, vec3f start, vec3f fin, float size, std::string tex_name)
{
    start = start + xyz_to_vec(pobj->pos);
    fin = fin + xyz_to_vec(pobj->pos);

    float len = (fin - start).length();

    ///make it point straight up

    std::vector<cl_float4> p;

    p.push_back({-1, 0, 1});
    p.push_back({1, 0, 1});
    p.push_back({1, 0, -1});
    p.push_back({-1, 0, -1});

    p.push_back({-1, len, 1});
    p.push_back({1, len, 1});
    p.push_back({1, len, -1});
    p.push_back({-1, len, -1});

    for(auto& i : p)
    {
        i.s[0] *= size;
        i.s[2] *= size;
    }


    vec3f dir = fin - start;

    vec3f rot = dir.get_euler();

    std::vector<triangle> tris;
    tris.resize(12);


    cl_float4 out[2][3];

    rect_to_tris({p[3], p[2], p[1], p[0]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 0));
    tris.push_back(points_to_tri(out[1], size, len, 0));

    rect_to_tris({p[4], p[5], p[6], p[7]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 0));
    tris.push_back(points_to_tri(out[1], size, len, 0));

    ///left
    rect_to_tris({p[0], p[4], p[7], p[3]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 2));
    tris.push_back(points_to_tri(out[1], size, len, 2));

    ///right
    rect_to_tris({p[2], p[6], p[5], p[1]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 2));
    tris.push_back(points_to_tri(out[1], size, len, 2));

    ///forward
    rect_to_tris({p[0], p[1], p[5], p[4]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 1));
    tris.push_back(points_to_tri(out[1], size, len, 1));

    ///back
    rect_to_tris({p[2], p[3], p[7], p[6]}, out);

    tris.push_back(points_to_tri(out[0], size, len, 1));
    tris.push_back(points_to_tri(out[1], size, len, 1));

    texture tex;
    tex.type = 0;
    tex.set_texture_location(tex_name.c_str());
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

    ///???
    //pobj->objs.clear();
    pobj->objs.push_back(obj);

    pobj->set_pos({start.v[0], start.v[1], start.v[2]});
    pobj->set_rot({rot.v[0], rot.v[1], rot.v[2]});

    pobj->isloaded = true;

    pobj->set_specular(0.9f);
}
