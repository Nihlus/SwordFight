#ifndef OBJECT_HPP_INCLUDED
#define OBJECT_HPP_INCLUDED

#include "state.hpp"
#include "renderer.hpp"

namespace team
{
    enum team : int32_t
    {
        FRIENDLY = 0,
        ENEMY,
        OBJECT,
        NONE ///skip
    };

    std::vector<vec3f> normal_cols
    {
        {0.3, 0.3, 1.f},
        {1.f, 0.3f, 0.3f},
        {0.7f, 0.7f, 0.7f},
        {1.f, 1.f, 1.f}
    };

    std::vector<vec3f> hit_cols
    {
        {0.9f, 0.9f, 1.f},
        {1.f, 0.6f, 0.6f},
        {0.9f, 0.9f, 0.9f},
        {1.f, 1.f, 1.f}
    };

    const float hit_timer_s = 0.2f;
}

typedef team::team team_t;

struct game_entity
{
    vec2f pos;
    float hp;
    bool to_remove;
    team_t my_team;
    vec2f dim;
    sf::Clock hit_timer; ///if less than hit_timer_s do hit effect

    void set_dim(vec2f _dim)
    {
        dim = _dim;
    }

    void set_pos(vec2f _pos)
    {
        pos = _pos;
    }

    void translate(vec2f _dir)
    {
        pos = pos + _dir;
    }

    void move_at_speed(vec2f _dir, float speed, float dt)
    {
        pos = pos + _dir * speed * dt;
    }

    bool is_dead()
    {
        return hp <= 0.f;
    }

    void do_damage(float frac)
    {
        hp -= frac;

        ///do hit effect
        hit_timer.restart();
    }

    void set_team(team_t _team)
    {
        my_team = _team;
    }

    virtual void tick(state& s, float dt){}

    game_entity(){hp = 1.f; to_remove = false; my_team = team::NONE;}
    virtual ~game_entity(){}
};

struct collider
{
    int get_collider_id(game_entity* me, const std::vector<game_entity*>& entities)
    {
        vec2f pos = me->pos;

        team_t my_team = me->my_team;

        for(int i=0; i<entities.size(); i++)
        {
            game_entity* en = entities[i];

            team_t their_team = en->my_team;

            ///im within his rectangle (temp)
            ///my team is not his team (no ff)
            ///and his team isn't none
            if(pos >= (en->pos - en->dim/2.f) && pos < (en->pos + en->dim/2.f) &&
               my_team != their_team && their_team != team::NONE)
            {
                return i;
            }
        }

        return -1;
    }
};

struct projectile : game_entity
{
    vec2f dir;
    float speed;
    float damage;

    void set_dir(vec2f _dir)
    {
        dir = _dir;
    }

    void set_speed(float _speed)
    {
        speed = _speed;
    }

    void set_damage(float _damage)
    {
        damage = _damage;
    }

    virtual void tick(state& s, float dt)
    {
        move_at_speed(dir.norm(), speed, dt);

        render_square sq(pos, dim, {1, 1, 0.3});

        s.render_2d->add(sq);

        collider collide;

        int id = collide.get_collider_id(this, *s.entities);

        if(id == -1)
            return;

        game_entity* en = (*s.entities)[id];

        en->do_damage(damage);

        to_remove = true;
    }

    projectile()
    {
        speed = 1.f;
        damage = 0.35f;
    }
};

struct character : game_entity
{
    float time_fired_s;
    float current_time;

    character(){time_fired_s = 0.f; current_time = 0.f;}

    virtual void tick(state& s, float dt)
    {
        vec3f normal_col = team::normal_cols[my_team];
        vec3f hurt_col = team::hit_cols[my_team];

        vec3f set_col = normal_col;

        if(hit_timer.getElapsedTime().asMilliseconds() / 1000.f < team::hit_timer_s)
        {
            set_col = hurt_col;
            printf("hi\n");
        }

        render_square sq(pos, dim, set_col);

        s.render_2d->add(sq);

        current_time += dt / 1000.f;
    }

    virtual projectile* fire(vec2f dir, float time_between_shots)
    {
        if(time_fired_s + time_between_shots > current_time)
            return nullptr;

        time_fired_s = current_time;

        projectile* new_projectile = new projectile;

        new_projectile->set_dir(dir);
        new_projectile->set_pos(pos);
        new_projectile->set_dim({3, 3});
        new_projectile->set_team(my_team);

        return new_projectile;
    }
};

struct player : character
{
    void do_keyboard_controls(float dt)
    {
        vec2f dir = {0,0};

        sf::Keyboard key;

        if(key.isKeyPressed(sf::Keyboard::W))
        {
            dir.v[1] = -1;
        }
        if(key.isKeyPressed(sf::Keyboard::S))
        {
            dir.v[1] = 1;
        }
        if(key.isKeyPressed(sf::Keyboard::A))
        {
            dir.v[0] = -1;
        }
        if(key.isKeyPressed(sf::Keyboard::D))
        {
            dir.v[0] = 1;
        }

        float speed = 0.3;

        move_at_speed(dir.norm(), speed, dt);
    }

    virtual void tick(state& s, float dt)
    {
        do_keyboard_controls(dt);

        character::tick(s, dt);
    }
};

#endif // OBJECT_HPP_INCLUDED
