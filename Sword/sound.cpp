#include "sound.hpp"

#include <SFML/Audio.hpp>
#include <deque>

sf::SoundBuffer s[2];

std::deque<sf::Sound> sounds;

void sound::add(int type, vec3f pos)
{
    static int loaded = 0;

    if(!loaded)
    {
        s[0].loadFromFile("res/hitm.wav");
        s[1].loadFromFile("res/clangm.wav");

        loaded = 1;
    }

    sf::Sound sound;

    sounds.push_back(sound);


    sf::Sound& sd = sounds.back();


    sd.setBuffer(s[type]);
    sd.setPitch(randf(0.75f, 1.25f));
    sd.setRelativeToListener(true);
    sd.setAttenuation(0.005f);
    sd.setVolume(randf(80, 100));

    sd.setPosition(-pos.v[0], pos.v[1], -pos.v[2]); ///sfml is probably opengl coords. X may be backwards

    //printf("%f %f %f\n", -pos.v[0], pos.v[1], pos.v[2]);

    sd.play();


    ///sfml is probably 3d opengl
    ///which means my z is backwards to theirs

    for(int i=0; i<sounds.size(); i++)
    {
        if(sounds[i].getStatus() == sf::Sound::Status::Stopped)
        {
            sounds.erase(sounds.begin() + i);
            i--;
        }
    }
}
