#include "server_networking.hpp"
#include "fighter.hpp"

server_networking::server_networking()
{
    ///this is a hack
    master_info.timeout_delay = 1000.f;
    //game_info.timeout_delay = 1000.f;
}

void server_networking::join_master()
{
    ///timeout 1 second
    if(master_info.owns_socket() && !master_info.within_timeout())
    {
        master_info = tcp_connect(MASTER_IP, MASTER_PORT, 1, 0);

        printf("master tcp\n");
    }

    if(master_info.valid())
    {
        to_master.close();

        to_master = tcp_sock(master_info.get());

        printf("joined master server\n");
    }
}


///we want to make this reconnect if to_game is invalid, ie we've lost connection
///?
void server_networking::join_game_tick(const std::string& address, const std::string& port)
{
    //if(game_info.owns_socket() && !game_info.within_timeout())
    //    game_info = tcp_connect(address, GAMESERVER_PORT, 5, 0);

    //if(game_info.valid())
    if(!to_game.valid())
    {
        to_game.close();

        to_game = udp_connect(address, port);

        ///send request to join
        byte_vector vec;
        vec.push_back(canary_start);
        vec.push_back(message::CLIENTJOINREQUEST);
        vec.push_back(canary_end);

        udp_send(to_game, vec.ptr);

        trying_to_join_game = false;
        joined_game = true; ///uuh. ok then, sure i guess
        have_id = false;
        my_id = -1;
        discovered_fighters.clear();

        printf("Connected gameserver %s:%s\n", address.c_str(), port.c_str());
    }
}

void server_networking::join_game_by_local_id_tick(int id)
{
    if(id < 0 || id >= (int)server_list.size())
    {
        printf("invalid joingame id %i\n", id);
        return;
    }

    game_server serv = server_list[id];

    return join_game_tick(serv.address, serv.their_host_port);
}

void server_networking::set_game_to_join(const std::string& address, const std::string& port)
{
    ///edge
    if(trying_to_join_game == false)
    {
        //game_info.retry();
        joined_game = false;
    }

    trying_to_join_game = true;
    trying_to_join_address = address;
    trying_to_join_port = port;
}

void server_networking::set_game_to_join(int id)
{
    if(id < 0 || id >= (int)server_list.size())
    {
        //printf("invalid setgame id %i\n", id);
        return;
    }

    game_server serv = server_list[id];

    return set_game_to_join(serv.address, serv.their_host_port);
}

std::vector<game_server> server_networking::get_serverlist(byte_fetch& fetch)
{
    int32_t server_num = fetch.get<int32_t>();

    std::vector<game_server> lservers;

    for(int i=0; i<server_num; i++)
    {
        int32_t len = fetch.get<int32_t>();

        std::string ip;

        for(int n=0; n<len; n++)
        {
            ip = ip + fetch.get<char>();
        }

        uint32_t port = fetch.get<uint32_t>();

        std::string their_host_port = std::to_string(port);

        game_server serv;
        serv.address = ip;
        serv.their_host_port = their_host_port;

        lservers.push_back(serv);
    }

    int32_t found_end = fetch.get<int32_t>();

    if(found_end == canary_end)
    {
        return lservers;
    }

    return std::vector<game_server>();
}

void server_networking::ping_master()
{
    if(!to_master.valid())
        return;

    pinged = true;

    byte_vector vec;
    vec.push_back(canary_start);
    vec.push_back(message::CLIENT);
    vec.push_back(canary_end);

    tcp_send(to_master, vec.ptr);
}

struct ptr_info
{
    void* ptr;
    int size;
};

template<typename T>
ptr_info get_inf(T* ptr)
{
    return {(void*)ptr, sizeof(T)};
}

std::map<int, ptr_info> build_fighter_network_stack(fighter* fight)
{
    std::map<int, ptr_info> fighter_stack;

    int c = 0;

    for(int i=0; i<fight->parts.size(); i++)
    {
        fighter_stack[c++] = get_inf(&fight->parts[i].obj()->pos);
        fighter_stack[c++] = get_inf(&fight->parts[i].obj()->rot);
        fighter_stack[c++] = get_inf(&fight->parts[i].hp);
    }

    fighter_stack[c++] = get_inf(&fight->weapon.model->pos);
    fighter_stack[c++] = get_inf(&fight->weapon.model->rot);
    fighter_stack[c++] = get_inf(&fight->net.is_blocking);
    fighter_stack[c++] = get_inf(&fight->net.recoil);

    return fighter_stack;
}

template<typename T>
int get_position_of(std::map<int, ptr_info>& stk, T* elem)
{
    for(int i=0; i<stk.size(); i++)
    {
        if(stk[i].ptr == elem)
            return i;
    }

    return -1;
}

template<typename T>
void set_map_element(std::map<int, ptr_info>& change, std::map<int, ptr_info>& stk, T* elem)
{
    int pos = get_position_of(stk, elem);

    change[pos] = get_inf(elem);
}

std::map<int, ptr_info> build_host_network_stack(fighter* fight)
{
    std::map<int, ptr_info> total_stack = build_fighter_network_stack(fight);

    std::map<int, ptr_info> to_send;

    for(int i=0; i<fight->parts.size(); i++)
    {
        set_map_element(to_send, total_stack, &fight->parts[i].obj()->pos);
        set_map_element(to_send, total_stack, &fight->parts[i].obj()->rot);
    }

    set_map_element(to_send, total_stack, &fight->weapon.model->pos);
    set_map_element(to_send, total_stack, &fight->weapon.model->rot);

    set_map_element(to_send, total_stack, &fight->net.is_blocking);

    return to_send;
}

void server_networking::tick(object_context* ctx, gameplay_state* st, physics* phys)
{
    ///tries to join
    join_master();

    if(sock_readable(to_master))
    {
        auto data = tcp_recv(to_master);

        ///actually, we only want to ping master once
        ///unless we set a big timeout and repeatedly ping master
        ///eh
        /*if(!to_master.valid())
        {
            to_master.close();
            master_info.retry();

            printf("why\n");
        }*/

        byte_fetch fetch;
        fetch.ptr.swap(data);

        int32_t found_canary = fetch.get<int32_t>();

        while(found_canary != canary_start && !fetch.finished())
        {
            found_canary = fetch.get<int32_t>();
        }

        int32_t type = fetch.get<int32_t>();

        if(type == message::CLIENTRESPONSE)
        {
            auto servs = get_serverlist(fetch);

            if(servs.size() > 0)
            {
                server_list = servs;
                has_serverlist = true;

                print_serverlist();

                ///we're done here
                to_master.close();
            }
            else
            {
                printf("Network error or 0 gameservers available\n");

                pinged = false; ///invalid response, ping again
            }
        }
    }

    bool any_recv = true;

    while(any_recv && sock_readable(to_game))
    {
        auto data = udp_receive_from(to_game, &to_game_store);
        is_init = true;

        if(data.size() > 0)
            any_recv = true;
        else
            any_recv = false;

        byte_fetch fetch;
        fetch.ptr.swap(data);

        while(!fetch.finished())
        {
            int32_t found_canary = fetch.get<int32_t>();

            while(found_canary != canary_start && !fetch.finished())
            {
                found_canary = fetch.get<int32_t>();
            }

            int32_t type = fetch.get<int32_t>();

            if(type == message::FORWARDING)
            {
                int32_t player_id = fetch.get<int32_t>();

                int32_t component_id = fetch.get<int32_t>();

                int32_t len = fetch.get<int32_t>();

                void* payload = fetch.get(len);

                int found_end = fetch.get<int32_t>();

                if(canary_end != found_end)
                {
                    printf("bad forward canary\n");

                    return;
                }

                if(discovered_fighters[player_id].id == -1)
                {
                    discovered_fighters[player_id] = make_networked_player(player_id, ctx, st, phys);

                    printf("made a new networked player\n");
                }

                network_player play = discovered_fighters[player_id];

                std::map<int, ptr_info> arg_map = build_fighter_network_stack(play.fight);

                if(component_id < 0 || component_id >= arg_map.size())
                {
                    printf("err in forwarding\n");
                    return; ///?
                }

                ptr_info comp = arg_map[component_id];

                if(len != comp.size)
                {
                    printf("err in argument size\n");
                    return;
                }

                memmove(comp.ptr, payload, comp.size);

                for(auto& i : play.fight->parts)
                {
                    i.obj()->set_pos(i.obj()->pos);
                    i.obj()->set_rot(i.obj()->rot);
                }

                play.fight->weapon.model->set_pos(play.fight->weapon.model->pos);
                play.fight->weapon.model->set_rot(play.fight->weapon.model->rot);

                play.fight->overwrite_parts_from_model();
                play.fight->network_update_render_positions();
            }

            if(type == message::CLIENTJOINACK)
            {
                int32_t recv_id = fetch.get<int32_t>();

                int32_t canary_found = fetch.get<int32_t>();

                if(canary_found == canary_end)
                {
                    have_id = true;
                    my_id = recv_id;

                    printf("Got joinack, I have id: %i\n", my_id);
                }
            }
        }
    }

    if(is_init && to_game.valid() && (time_since_last_send.getElapsedTime().asMicroseconds() / 1000.f) > time_between_sends_ms)
    {
        if(have_id && discovered_fighters[my_id].fight != nullptr)
        {
            std::map<int, ptr_info> host_stack = build_host_network_stack(discovered_fighters[my_id].fight);

            //for(int i=0; i<host_stack.size(); i++)
            for(auto& i : host_stack)
            {
                ptr_info inf = i.second;

                int32_t id = i.first;

                byte_vector vec;
                vec.push_back(canary_start);
                vec.push_back(message::FORWARDING);
                vec.push_back<int32_t>(my_id);
                vec.push_back<int32_t>(id);

                vec.push_back<int32_t>(inf.size);
                vec.push_back((uint8_t*)inf.ptr, inf.size);

                vec.push_back(canary_end);

                udp_send_to(to_game, vec.ptr, (const sockaddr*)&to_game_store);
            }

            time_since_last_send.restart();
        }
    }

    if(!has_serverlist && !pinged)
    {
        ping_master();
    }

    if(trying_to_join_game)
    {
        join_game_tick(trying_to_join_address, trying_to_join_port);
    }

    for(auto& i : discovered_fighters)
    {
        if(i.first == my_id)
            continue;

        i.second.fight->overwrite_parts_from_model();

        if(!i.second.fight->dead())
            i.second.fight->update_lights();
    }
}

void server_networking::print_serverlist()
{
    for(int i=0; i<(int)server_list.size(); i++)
    {
        printf("SN: %i %s:%s\n", i, server_list[i].address.c_str(), server_list[i].their_host_port.c_str());
    }
}

network_player server_networking::make_networked_player(int32_t id, object_context* ctx, gameplay_state* current_state, physics* phys)
{
    fighter* net_fighter = new fighter(*ctx, *ctx->fetch());

    net_fighter->set_team(1);
    net_fighter->set_pos({0, 10, 0});
    net_fighter->set_rot({0, 0, 0});
    //net_fighter->set_quality(s.quality);
    net_fighter->set_quality(1); ///???
    net_fighter->set_gameplay_state(current_state);

    ctx->load_active();

    net_fighter->set_physics(phys);
    net_fighter->update_render_positions();

    ctx->build();

    net_fighter->update_lights();

    network_player play;
    play.fight = net_fighter;
    play.id = id;

    return play;
}

void server_networking::set_my_fighter(fighter* fight)
{
    if(!have_id)
        return;

    discovered_fighters[my_id] = {fight, my_id};
}
