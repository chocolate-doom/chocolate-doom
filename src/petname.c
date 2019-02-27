//
// Copyright(C) 2019 Jonathan Dowland
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Generate a randomized, private, memorable name for a Player
//

#include <stdlib.h>
#include <time.h>
#include <string.h>

static char *adjectives[] = {
    "Grumpy",
    "Ecstatic",
    "Surly",
    "Prepared",
    "Crafty",
    "Alert",
    "Sluggish",
    "Testy",
    "Reluctant",
    "Languid",
    "Passive",
    "Pacifist",
    "Aggressive",
    "Hostile",
    "Diseased",
    "Bubbly",
    "Giggly",
    "Laughing",
    "Crying",
    "Frowning",
    "Flatulent",
    "Torpid",
    "Lethargic",
    "Manic",
    "Patient",
    "Protective",
    "Philosophical",
    "Enquiring",
    "Debating",
    "Furious",
    "Laid-Back",
    "Easy-Going",
    "Cromulent",
    "Excitable",
    "Tired",
    "Exhausted",
    "Ruminating",
    "Redundant",
    "Sporty",
    "Ginger",
    "Scary",
    "Posh",
    "Baby",
};
#define NUM_ADJECTIVES (sizeof adjectives / sizeof (char *))


static char *nouns[] = {
    // Doom
    "Cacodemon",
    "Arch-Vile",
    "Cyberdemon",
    "Imp",
    "Demon",
    "Mancubus",
    "Arachnotron",
    "Baron",
    "Knight",
    "Revenant",
    // Hexen
    "Ettin",
    "Maulotaur",
    "Centaur",
    "Afrit",
    "Serpent",
    // Heretic
    "Disciple",
    "Gargoyle",
    "Golem",
    "Lich",
    // Strife
    "Sentinel",
    "Acolyte",
    "Templar",
    "Reaver",
    "Crusader",
};
#define NUM_NOUNS (sizeof nouns / sizeof (char *))

/*
 * ideally we would export this and the caller would invoke it during
 * their setup routine. But, the two callers only invoke getRandomPetName
 * once, so the initialisation might as well occur then.
 */
static void initPetName()
{
    srandom((unsigned int)time(NULL));
}

char *getRandomPetName()
{
    char *a, *n, *r;

    initPetName();

    a = adjectives[random() % NUM_ADJECTIVES];
    n = nouns[random() % NUM_NOUNS];
    r = (char *)malloc(strlen(a) + (sizeof ' ') +
                       strlen(n) + (sizeof '\0'));
    if(r)
    {
        strcpy(r,a);
        r[strlen(a)] = ' ';
        strcpy(r + strlen(a) + 1, n);
    }

    return r;
}
