
//**************************************************************************
//**
//** DStrings.H
//**
//**************************************************************************

// MN_menu.c ---------------------------------------------------------------

#define PRESSKEY	"press a key."
#define PRESSYN		"press y or n."
#define TXT_PAUSED	"PAUSED"
#define QUITMSG		"are you sure you want to\nquit this great game?"
#define LOADNET		"you can't do load while in a net game!\n\n"PRESSKEY
#define QLOADNET	"you can't quickload during a netgame!\n\n"PRESSKEY
#define QSAVESPOT	"you haven't picked a quicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD 	"you can't save if you aren't playing!\n\n"PRESSKEY
#define QSPROMPT 	"quicksave over your game named\n\n'%s'?\n\n"PRESSYN
#define QLPROMPT	"do you want to quickload the game named"\
					"\n\n'%s'?\n\n"PRESSYN
#define NEWGAME		"you can't start a new game\n"\
					"while in a network game.\n\n"PRESSKEY
#define NIGHTMARE	"are you sure? this skill level\n"\
					"isn't even remotely fair.\n\n"PRESSYN
#define SWSTRING	"this is the shareware version of doom.\n\n"\
					"you need to order the entire trilogy.\n\n"PRESSKEY
#define MSGOFF		"Messages OFF"
#define MSGON		"Messages ON"
#define NETEND		"you can't end a netgame!\n\n"PRESSKEY
#define ENDGAME		"are you sure you want to end the game?\n\n"PRESSYN
#define DOSY		"(press y to quit to dos.)"
#define DETAILHI	"High detail"
#define DETAILLO	"Low detail"
#define GAMMALVL0	"Gamma correction OFF"
#define GAMMALVL1	"Gamma correction level 1"
#define GAMMALVL2	"Gamma correction level 2"
#define GAMMALVL3	"Gamma correction level 3"
#define GAMMALVL4	"Gamma correction level 4"
#define	EMPTYSTRING	"empty slot"

// P_inter.c ---------------------------------------------------------------

// Keys

#define TXT_GOTBLUEKEY			"BLUE KEY"
#define TXT_GOTYELLOWKEY		"YELLOW KEY"
#define TXT_GOTGREENKEY			"GREEN KEY"

// Artifacts

#define TXT_ARTIHEALTH			"QUARTZ FLASK"
#define TXT_ARTIFLY				"WINGS OF WRATH"
#define TXT_ARTIINVULNERABILITY	"RING OF INVINCIBILITY"
#define TXT_ARTITOMEOFPOWER		"TOME OF POWER"
#define TXT_ARTIINVISIBILITY	"SHADOWSPHERE"
#define TXT_ARTIEGG				"MORPH OVUM"
#define TXT_ARTISUPERHEALTH		"MYSTIC URN"
#define TXT_ARTITORCH			"TORCH"
#define TXT_ARTIFIREBOMB		"TIME BOMB OF THE ANCIENTS"
#define TXT_ARTITELEPORT		"CHAOS DEVICE"

// Items

#define TXT_ITEMHEALTH			"CRYSTAL VIAL"
#define TXT_ITEMBAGOFHOLDING	"BAG OF HOLDING"
#define TXT_ITEMSHIELD1			"SILVER SHIELD"
#define TXT_ITEMSHIELD2			"ENCHANTED SHIELD"
#define TXT_ITEMSUPERMAP		"MAP SCROLL"

// Ammo

#define TXT_AMMOGOLDWAND1		"WAND CRYSTAL"
#define TXT_AMMOGOLDWAND2		"CRYSTAL GEODE"
#define TXT_AMMOMACE1			"MACE SPHERES"
#define TXT_AMMOMACE2			"PILE OF MACE SPHERES"
#define TXT_AMMOCROSSBOW1		"ETHEREAL ARROWS"
#define TXT_AMMOCROSSBOW2		"QUIVER OF ETHEREAL ARROWS"
#define TXT_AMMOBLASTER1		"CLAW ORB"
#define TXT_AMMOBLASTER2		"ENERGY ORB"
#define TXT_AMMOSKULLROD1		"LESSER RUNES"
#define TXT_AMMOSKULLROD2		"GREATER RUNES"
#define TXT_AMMOPHOENIXROD1		"FLAME ORB"
#define TXT_AMMOPHOENIXROD2		"INFERNO ORB"

// Weapons

#define TXT_WPNMACE				"FIREMACE"
#define TXT_WPNCROSSBOW			"ETHEREAL CROSSBOW"
#define TXT_WPNBLASTER			"DRAGON CLAW"
#define TXT_WPNSKULLROD			"HELLSTAFF"
#define TXT_WPNPHOENIXROD		"PHOENIX ROD"
#define TXT_WPNGAUNTLETS		"GAUNTLETS OF THE NECROMANCER"

// SB_bar.c ----------------------------------------------------------------

#define TXT_CHEATGODON			"GOD MODE ON"
#define TXT_CHEATGODOFF			"GOD MODE OFF"
#define TXT_CHEATNOCLIPON		"NO CLIPPING ON"
#define TXT_CHEATNOCLIPOFF		"NO CLIPPING OFF"
#define TXT_CHEATWEAPONS		"ALL WEAPONS"
#define TXT_CHEATFLIGHTON		"FLIGHT ON"
#define TXT_CHEATFLIGHTOFF		"FLIGHT OFF"
#define TXT_CHEATPOWERON		"POWER ON"
#define TXT_CHEATPOWEROFF		"POWER OFF"
#define TXT_CHEATHEALTH			"FULL HEALTH"
#define TXT_CHEATKEYS			"ALL KEYS"
#define TXT_CHEATSOUNDON		"SOUND DEBUG ON"
#define TXT_CHEATSOUNDOFF		"SOUND DEBUG OFF"
#define TXT_CHEATTICKERON		"TICKER ON"
#define TXT_CHEATTICKEROFF		"TICKER OFF"
#define TXT_CHEATARTIFACTS1		"CHOOSE AN ARTIFACT ( A - J )"
#define TXT_CHEATARTIFACTS2		"HOW MANY ( 1 - 9 )"
#define TXT_CHEATARTIFACTS3		"YOU GOT IT"
#define TXT_CHEATARTIFACTSFAIL	"BAD INPUT"
#define TXT_CHEATWARP			"LEVEL WARP"
#define TXT_CHEATSCREENSHOT		"SCREENSHOT"
#define TXT_CHEATCHICKENON		"CHICKEN ON"
#define TXT_CHEATCHICKENOFF		"CHICKEN OFF"
#define TXT_CHEATMASSACRE		"MASSACRE"
#define TXT_CHEATIDDQD			"TRYING TO CHEAT, EH?  NOW YOU DIE!"
#define TXT_CHEATIDKFA			"CHEATER - YOU DON'T DESERVE WEAPONS"

// P_doors.c ---------------------------------------------------------------

#define TXT_NEEDBLUEKEY			"YOU NEED A BLUE KEY TO OPEN THIS DOOR"
#define TXT_NEEDGREENKEY		"YOU NEED A GREEN KEY TO OPEN THIS DOOR"
#define TXT_NEEDYELLOWKEY		"YOU NEED A YELLOW KEY TO OPEN THIS DOOR"

// G_game.c ----------------------------------------------------------------

#define TXT_GAMESAVED			"GAME SAVED"

// M_misc.c ----------------------------------------------------------------

#define HUSTR_CHATMACRO1 "I'm ready to kick butt!"
#define HUSTR_CHATMACRO2 "I'm OK."
#define HUSTR_CHATMACRO3 "I'm not looking too good!"
#define HUSTR_CHATMACRO4 "Help!"
#define HUSTR_CHATMACRO5 "You suck!"
#define HUSTR_CHATMACRO6 "Next time, scumbag..."
#define HUSTR_CHATMACRO7 "Come here!"
#define HUSTR_CHATMACRO8 "I'll take care of it."
#define HUSTR_CHATMACRO9 "Yes"
#define HUSTR_CHATMACRO0 "No"

// AM_map.c ----------------------------------------------------------------

#define AMSTR_FOLLOWON		"FOLLOW MODE ON"
#define AMSTR_FOLLOWOFF		"FOLLOW MODE OFF"

// F_finale.c --------------------------------------------------------------

#define E1TEXT		"with the destruction of the iron\n"\
					"liches and their minions, the last\n"\
					"of the undead are cleared from this\n"\
					"plane of existence.\n\n"\
					"those creatures had to come from\n"\
					"somewhere, though, and you have the\n"\
					"sneaky suspicion that the fiery\n"\
					"portal of hell's maw opens onto\n"\
					"their home dimension.\n\n"\
					"to make sure that more undead\n"\
					"(or even worse things) don't come\n"\
					"through, you'll have to seal hell's\n"\
					"maw from the other side. of course\n"\
					"this means you may get stuck in a\n"\
					"very unfriendly world, but no one\n"\
					"ever said being a Heretic was easy!"

#define E2TEXT		"the mighty maulotaurs have proved\n"\
					"to be no match for you, and as\n"\
					"their steaming corpses slide to the\n"\
					"ground you feel a sense of grim\n"\
					"satisfaction that they have been\n"\
					"destroyed.\n\n"\
					"the gateways which they guarded\n"\
					"have opened, revealing what you\n"\
					"hope is the way home. but as you\n"\
					"step through, mocking laughter\n"\
					"rings in your ears.\n\n"\
					"was some other force controlling\n"\
					"the maulotaurs? could there be even\n"\
					"more horrific beings through this\n"\
					"gate? the sweep of a crystal dome\n"\
					"overhead where the sky should be is\n"\
					"certainly not a good sign...."

#define E3TEXT		"the death of d'sparil has loosed\n"\
					"the magical bonds holding his\n"\
					"creatures on this plane, their\n"\
					"dying screams overwhelming his own\n"\
					"cries of agony.\n\n"\
					"your oath of vengeance fulfilled,\n"\
					"you enter the portal to your own\n"\
					"world, mere moments before the dome\n"\
					"shatters into a million pieces.\n\n"\
					"but if d'sparil's power is broken\n"\
					"forever, why don't you feel safe?\n"\
					"was it that last shout just before\n"\
					"his death, the one that sounded\n"\
					"like a curse? or a summoning? you\n"\
					"can't really be sure, but it might\n"\
					"just have been a scream.\n\n"\
					"then again, what about the other\n"\
					"serpent riders?"
