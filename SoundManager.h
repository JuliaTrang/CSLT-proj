#pragma once
#include <SFML/Audio.hpp>

class SoundManager {
private:
    sf::Music menuMusic;
    sf::Music gameMusic;
    sf::SoundBuffer clickBuffer;
    sf::Sound clickSound;
    float volume = 50.0f;
    bool isMuted = false;
    float menuVolume = 50.0f;
    float gameVolume = 50.0f;
    sf::SoundBuffer errorBuffer;
    sf::Sound errorSound;
public:
    bool load();
    void playMenuMusic();
    void playGameMusic();
    void stopMusic();
    void playClick();
    void setVolume(float vol);
    float getVolume() const;
    void toggleMute();
    bool getMuteState() const;
    float& getVolumeRef();

    float& getMenuVolumeRef();
    float& getGameVolumeRef();
    void setMenuVolume(float);
    void setGameVolume(float);
    void playError();
};






