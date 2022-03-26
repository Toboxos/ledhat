#pragma once

typedef struct Character {
    Character() : width(0), height(0), data(nullptr) {}
    Character(unsigned int width, unsigned int height, const char* data) : width( width ), height( height ), data( data ) {}

    unsigned int width;
    unsigned int height;
    const char* data;
} Character;

/**
 * Gets the corresponding Character entry
 *
 * @param[in] c ASCII character
 * @param[out] character Character entry
 * @returns bool Returns true if character entry was found. Otherwise false is returned
 */
bool getCharacter(const char c, Character& character);

