#include "characters.h"
#include "characters_definition.h"


bool getCharacter(const char c, Character& character) {
    auto characterIndex = c - ' ';

    // check boundaries
    if( characterIndex < 0 || characterIndex >= sizeof(characters) / sizeof(Character) ) {
        return false;
    }

    character = characters[ characterIndex ];
    return true;
}
