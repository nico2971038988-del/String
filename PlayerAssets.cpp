#include "PlayerAssets.hpp"

#include <iostream>
#include <string>

bool loadIdleTextures(PlayerTextures& textures)
{
    constexpr int frameCount = 2;

    textures.idleFrames.clear();
    textures.idleFrames.reserve(frameCount);


    for (int i = 1; i <= frameCount; ++i)
    {
        sf::Texture texture;

        const std::string path =
            "Assets/Arts/player/run"+
            std::to_string(i) + ".png";

        if (!texture.loadFromFile(path))
        {
            std::cerr << "Failed to load: " << path << '\n';
            return false;
        }

        textures.idleFrames.push_back(std::move(texture));
    }

    return true;
}

bool loadWalkTextures(PlayerTextures& textures)
{
    constexpr int frameCount = 2;

    textures.walkFrames.clear();
    textures.walkFrames.reserve(frameCount);

    for (int i = 1; i <= frameCount; ++i)
    {
        sf::Texture texture;

        const std::string path =
            "Assets/Arts/player/run" +
            std::to_string(i) + ".png";

        if (!texture.loadFromFile(path))
        {
            std::cerr << "Failed to load: " << path << '\n';
            return false;
        }

        textures.walkFrames.push_back(std::move(texture));
    }

    return true;
}

bool loadJumpTextures(PlayerTextures& textures)
{
    constexpr int frameCount = 3;

    textures.jumpFrames.clear();
    textures.jumpFrames.reserve(frameCount);

    for (int i = 1; i <= frameCount; ++i)
    {
        sf::Texture texture;

        const std::string path =
            "Assets/Arts/player/run" +
            std::to_string(i) + ".png";

        if (!texture.loadFromFile(path))
        {
            std::cerr << "Failed to load: " << path << '\n';
            return false;
        }

        textures.jumpFrames.push_back(std::move(texture));
    }

    return true;
}