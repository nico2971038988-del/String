#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct PlayerTextures
{
    std::vector<sf::Texture> idleFrames;
    std::vector<sf::Texture> walkFrames;
    std::vector<sf::Texture> jumpFrames;
};

bool loadIdleTextures(PlayerTextures& textures);
bool loadWalkTextures(PlayerTextures& textures);
bool loadJumpTextures(PlayerTextures& textures);