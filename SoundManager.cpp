#include "SoundManager.h"
#include <algorithm>

bool SoundManager::load() {
    if (!menuMusic.openFromFile("Audio/menu.ogg")) return false;
    if (!gameMusic.openFromFile("Audio/game.ogg")) return false;
    if (!clickBuffer.loadFromFile("Audio/click.ogg")) return false;
    clickSound.setBuffer(clickBuffer);
    if (!errorBuffer.loadFromFile("Audio/error.wav")) return false;
    errorSound.setBuffer(errorBuffer);
    menuMusic.setLoop(true);
    gameMusic.setLoop(true);
    setVolume(volume);
    menuMusic.setVolume(menuVolume);
    gameMusic.setVolume(gameVolume);
    return true;
}

void SoundManager::playMenuMusic() {
    gameMusic.stop();
    if (!isMuted && menuMusic.getStatus() != sf::Music::Playing)
        menuMusic.play();
}

void SoundManager::playGameMusic() {
    menuMusic.stop();
    if (!isMuted && gameMusic.getStatus() != sf::Music::Playing)
        gameMusic.play();
}

void SoundManager::stopMusic() {
    menuMusic.stop();
    gameMusic.stop();
}

void SoundManager::playClick() {
    if (!isMuted)
        clickSound.play();
}

void SoundManager::setVolume(float vol) {
    volume = std::max(0.f, std::min(100.f, vol));
    if (!isMuted) {
        menuMusic.setVolume(volume);
        gameMusic.setVolume(volume);
        clickSound.setVolume(volume);
    }
}

float SoundManager::getVolume() const {
    return volume;
}

void SoundManager::toggleMute() {
    isMuted = !isMuted;
    if (isMuted) {
        menuMusic.setVolume(0);
        gameMusic.setVolume(0);
        clickSound.setVolume(0);
    }
    else {
        setVolume(volume);
    }
}

bool SoundManager::getMuteState() const {
    return isMuted;
}

float& SoundManager::getVolumeRef()
{
    return volume;
}


float& SoundManager::getMenuVolumeRef() { return menuVolume; }
float& SoundManager::getGameVolumeRef() { return gameVolume; }

void SoundManager::setMenuVolume(float v) {
    menuVolume = std::max(0.f, std::min(100.f, v));
    if (!isMuted) menuMusic.setVolume(menuVolume);
}

void SoundManager::setGameVolume(float v) {
    gameVolume = std::max(0.f, std::min(100.f, v));
    if (!isMuted) gameMusic.setVolume(gameVolume);
}
void SoundManager::playError() {
    if (!isMuted)
        errorSound.play();
}