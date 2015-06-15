//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	Printed strings for translation.
//	English language support (default).
//

#ifndef __D_ENGLSH__
#define __D_ENGLSH__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR	"Development mode ON.\n"
#define D_CDROM	"CD-ROM Version: Accessing strife.cd\n"

//
//	M_Menu.C
//
#define PRESSKEY 	"press a key."
#define PRESSYN 	"press y or n."
#define QUITMSG	"are you sure you want to\nquit this great game?"
// [STRIFE] modified:
#define LOADNET 	"you can't load while in a net game!\n\n"PRESSKEY
#define QLOADNET	"you can't quickload during a netgame!\n\n"PRESSKEY
// [STRIFE] modified:
#define QSAVESPOT	"you haven't picked a\nquicksave slot yet!\n\n"PRESSKEY
// [STRIFE] modified:
#define SAVEDEAD 	"you're not playing a game\n\n"PRESSKEY
#define QSPROMPT 	"quicksave over your game named\n\n'%s'?\n\n"PRESSYN
// [STRIFE] modified:
#define QLPROMPT	"do you want to quickload\n\n'%s'?\n\n"PRESSYN

#define NEWGAME	\
"you can't start a new game\n"\
"while in a network game.\n\n"PRESSKEY

#define NIGHTMARE	\
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\n"PRESSYN

#define SWSTRING	\
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\n"PRESSKEY

#define MSGOFF	"Messages OFF"
#define MSGON		"Messages ON"
#define NETEND	"you can't end a netgame!\n\n"PRESSKEY
#define ENDGAME	"are you sure you want\nto end the game?\n\n"PRESSYN

// haleyjd 09/11/10: [STRIFE] No "to dos." on this
#define DOSY		"(press y to quit)" 

#define DETAILHI	"High detail"
#define DETAILLO	"Low detail"
#define GAMMALVL0	"Gamma correction OFF"
#define GAMMALVL1	"Gamma correction level 1"
#define GAMMALVL2	"Gamma correction level 2"
#define GAMMALVL3	"Gamma correction level 3"
#define GAMMALVL4	"Gamma correction level 4"
#define EMPTYSTRING	"empty slot"

//
//	P_inter.C
//
#define GOTARMOR	"Picked up the armor."
#define GOTMEGA	"Picked up the MegaArmor!"
#define GOTHTHBONUS	"Picked up a health bonus."
#define GOTARMBONUS	"Picked up an armor bonus."
#define GOTSTIM	"Picked up a stimpack."
#define GOTMEDINEED	"Picked up a medikit that you REALLY need!"
#define GOTMEDIKIT	"Picked up a medikit."
#define GOTSUPER	"Supercharge!"

#define GOTBLUECARD	"Picked up a blue keycard."
#define GOTYELWCARD	"Picked up a yellow keycard."
#define GOTREDCARD	"Picked up a red keycard."
#define GOTBLUESKUL	"Picked up a blue skull key."
#define GOTYELWSKUL	"Picked up a yellow skull key."
#define GOTREDSKULL	"Picked up a red skull key."

#define GOTINVUL	"Invulnerability!"
#define GOTBERSERK	"Berserk!"
#define GOTINVIS	"Partial Invisibility"
#define GOTSUIT	"Radiation Shielding Suit"
#define GOTMAP	"Computer Area Map"
#define GOTVISOR	"Light Amplification Visor"
#define GOTMSPHERE	"MegaSphere!"

#define GOTCLIP	"Picked up a clip."
#define GOTCLIPBOX	"Picked up a box of bullets."
#define GOTROCKET	"Picked up a rocket."
#define GOTROCKBOX	"Picked up a box of rockets."
#define GOTCELL	"Picked up an energy cell."
#define GOTCELLBOX	"Picked up an energy cell pack."
#define GOTSHELLS	"Picked up 4 shotgun shells."
#define GOTSHELLBOX	"Picked up a box of shotgun shells."
#define GOTBACKPACK	"Picked up a backpack full of ammo!"

#define GOTBFG9000	"You got the BFG9000!  Oh, yes."
#define GOTCHAINGUN	"You got the chaingun!"
#define GOTCHAINSAW	"A chainsaw!  Find some meat!"
#define GOTLAUNCHER	"You got the rocket launcher!"
#define GOTPLASMA	"You got the plasma gun!"
#define GOTSHOTGUN	"You got the shotgun!"
#define GOTSHOTGUN2	"You got the super shotgun!"

//
// P_Doors.C
//
#define PD_BLUEO	"You need a blue key to activate this object"
#define PD_REDO	"You need a red key to activate this object"
#define PD_YELLOWO	"You need a yellow key to activate this object"
#define PD_BLUEK	"You need a blue key to open this door"
#define PD_REDK	"You need a red key to open this door"
#define PD_YELLOWK	"You need a yellow key to open this door"

//
//	G_game.C
//
#define GGSAVED	"game saved."

//
//	HU_stuff.C
//
#define HUSTR_MSGU	"[Message unsent]"

// haleyjd 08/31/10: [STRIFE] Strife map names

#define HUSTR_1         "AREA  1: sanctuary"
#define HUSTR_2         "AREA  2: town"
#define HUSTR_3         "AREA  3: front base"
#define HUSTR_4         "AREA  4: power station"
#define HUSTR_5         "AREA  5: prison"
#define HUSTR_6         "AREA  6: sewers"
#define HUSTR_7         "AREA  7: castle"
#define HUSTR_8         "AREA  8: Audience Chamber"
#define HUSTR_9         "AREA  9: Castle: Programmer's Keep"

#define HUSTR_10        "AREA 10: New Front Base"
#define HUSTR_11        "AREA 11: Borderlands"
#define HUSTR_12        "AREA 12: the temple of the oracle"
#define HUSTR_13        "AREA 13: Catacombs"
#define HUSTR_14        "AREA 14: mines"
#define HUSTR_15        "AREA 15: Fortress: Administration"
#define HUSTR_16        "AREA 16: Fortress: Bishop's Tower"
#define HUSTR_17        "AREA 17: Fortress: The Bailey"
#define HUSTR_18        "AREA 18: Fortress: Stores"
#define HUSTR_19        "AREA 19: Fortress: Security Complex"

#define HUSTR_20        "AREA 20: Factory: Receiving"
#define HUSTR_21        "AREA 21: Factory: Manufacturing"
#define HUSTR_22        "AREA 22: Factory: Forge"
#define HUSTR_23        "AREA 23: Order Commons"
#define HUSTR_24        "AREA 24: Factory: Conversion Chapel"
#define HUSTR_25        "AREA 25: Catacombs: Ruined Temple"
#define HUSTR_26        "AREA 26: proving grounds"
#define HUSTR_27        "AREA 27: The Lab"
#define HUSTR_28        "AREA 28: Alien Ship"
#define HUSTR_29        "AREA 29: Entity's Lair"

#define HUSTR_30        "AREA 30: Abandoned Front Base"
#define HUSTR_31        "AREA 31: Training Facility"

#define HUSTR_32        "AREA  1: Sanctuary"
#define HUSTR_33        "AREA  2: Town"
#define HUSTR_34        "AREA  3: Movement Base"

// haleyjd 20110219: [STRIFE] replaced all with Strife chat macros:
#define HUSTR_CHATMACRO1        "Fucker!"
#define HUSTR_CHATMACRO2        "--SPLAT-- Instant wall art."
#define HUSTR_CHATMACRO3        "That had to hurt!"
#define HUSTR_CHATMACRO4        "Smackings!"
#define HUSTR_CHATMACRO5        "Gib-O-Matic baby."
#define HUSTR_CHATMACRO6        "Burn!  Yah! Yah!"
#define HUSTR_CHATMACRO7        "Buh-Bye!"
#define HUSTR_CHATMACRO8        "Sizzle chest!"
#define HUSTR_CHATMACRO9        "That sucked!"
#define HUSTR_CHATMACRO0        "Mommy?"

#define HUSTR_TALKTOSELF1	"You mumble to yourself"
#define HUSTR_TALKTOSELF2	"Who's there?"
#define HUSTR_TALKTOSELF3	"You scare yourself"
#define HUSTR_TALKTOSELF4	"You start to rave"
#define HUSTR_TALKTOSELF5	"You've lost it..."

#define HUSTR_MESSAGESENT	"[Message Sent]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

// [STRIFE]: Not used, as strings are local to hu_stuff.c
//#define HUSTR_PLRGREEN	"Green: "
//#define HUSTR_PLRINDIGO	"Indigo: "
//#define HUSTR_PLRBROWN	"Brown: "
//#define HUSTR_PLRRED		"Red: "

#define HUSTR_KEYGREEN	'g'
#define HUSTR_KEYINDIGO	'i'
#define HUSTR_KEYBROWN	'b'
#define HUSTR_KEYRED	'r'

//
//	AM_map.C
//

#define AMSTR_FOLLOWON	"Follow Mode ON"
#define AMSTR_FOLLOWOFF	"Follow Mode OFF"

#define AMSTR_GRIDON	"Grid ON"
#define AMSTR_GRIDOFF	"Grid OFF"

#define AMSTR_MARKEDSPOT        "Marked Spot"
#define AMSTR_MARKSCLEARED      "Last Mark Cleared" // [STRIFE]

//
//	ST_stuff.C
//

#define STSTR_MUS               "Music Change"
#define STSTR_NOMUS             "IMPOSSIBLE SELECTION"
#define STSTR_DQDON             "You're Invincible!"   // [STRIFE]
#define STSTR_DQDOFF            "You're a looney!"     // [STRIFE]

#define STSTR_KFAADDED          "Very Happy Ammo Added"
#define STSTR_FAADDED           "Ammo Added"           // [STRIFE]

#define STSTR_NCON              "No Clipping Mode ON"
#define STSTR_NCOFF             "No Clipping Mode OFF"

#define STSTR_BEHOLD            "Bzrk, Inviso, Mask, Health, Pack, Stats"  // [STRIFE]
#define STSTR_BEHOLDX           "Power-up Toggled"

#define STSTR_CHOPPERS          "... doesn't suck - GM"
#define STSTR_CLEV              "Changing Level..."

//
//	F_Finale.C
//
#define E1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base.  Looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"


#define E2TEXT \
"You've done it! The hideous cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But ... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to  the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's on to the final chapter of\n"\
"DOOM! -- Inferno."


#define E3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair -- for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you ..."


#define E4TEXT \
"the spider mastermind must have sent forth\n"\
"its legions of hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from hell.  but you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"besides, someone was gonna pay for what\n"\
"happened to daisy, your pet rabbit.\n"\
"\n"\
"but now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"next stop, hell on earth!"


// after level 6, put this:

#define C1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n" \
"STARPORT. BUT SOMETHING IS WRONG. THE\n" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\n" \
"\n"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" \
"OF THE STARBASE AND FIND THE CONTROLLING\n" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\n" \
"HOSTAGE."

// After level 11, put this:

#define C2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\n"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"\
"THAT YOU HAVE SAVED YOUR SPECIES.\n"\
"\n"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\n"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"\
"UP AND RETURN TO THE FRAY."


// After level 20, put this:

#define C3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"\
"TEETH AND PLUNGE THROUGH IT.\n"\
"\n"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\n"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"\
"GOT TO GO THROUGH HELL TO GET TO IT?"


// After level 29, put this:

#define C4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\n"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"\
"UP AND DIES, ITS THRASHING LIMBS\n"\
"DEVASTATING UNTOLD MILES OF HELL'S\n"\
"SURFACE.\n"\
"\n"\
"YOU'VE DONE IT. THE INVASION IS OVER.\n"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\n"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"\
"HOME. REBUILDING EARTH OUGHT TO BE A\n"\
"LOT MORE FUN THAN RUINING IT WAS.\n"



// Before level 31, put this:

#define C5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."


// Before level 32, put this:

#define C6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"


// after map 06	

#define P1TEXT  \
"You gloat over the steaming carcass of the\n"\
"Guardian.  With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell.  You relax and glance around the\n"\
"room.  Damn!  There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."


// after map 11

#define P2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."


// after map 20

#define P3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive.  Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth.  Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"

// after map 30

#define P4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place.  As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons.  You're done. Hell\n"\
"has gone back to pounding bad dead folks \n"\
"instead of good live ones.  Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up ..."

// before map 31

#define P5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32

#define P6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."


#define T1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs.   It seems that UAC has\n"\
"once again gulped it down.  With their\n"\
"high turnover, it must be hard for poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays..\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."


#define T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define T4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeeze, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."


#define T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define T6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"Maybe someone else can kick Hell's ass\n"\
"next time around. Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a cyberdemon's iron shoe."



//
// Character cast strings F_FINALE.C
//
#define CC_ZOMBIE	"ZOMBIEMAN"
#define CC_SHOTGUN	"SHOTGUN GUY"
#define CC_HEAVY	"HEAVY WEAPON DUDE"
#define CC_IMP	"IMP"
#define CC_DEMON	"DEMON"
#define CC_LOST	"LOST SOUL"
#define CC_CACO	"CACODEMON"
#define CC_HELL	"HELL KNIGHT"
#define CC_BARON	"BARON OF HELL"
#define CC_ARACH	"ARACHNOTRON"
#define CC_PAIN	"PAIN ELEMENTAL"
#define CC_REVEN	"REVENANT"
#define CC_MANCU	"MANCUBUS"
#define CC_ARCH	"ARCH-VILE"
#define CC_SPIDER	"THE SPIDER MASTERMIND"
#define CC_CYBER	"THE CYBERDEMON"
#define CC_HERO	"OUR HERO"


#endif
