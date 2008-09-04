
//**************************************************************************
//**
//** DRCoord.h : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: DRCoord.h,v $
//** $Revision: 1.1 $
//** $Date: 95/05/11 00:19:30 $
//** $Author: bgokey $
//**
//**************************************************************************

#import <appkit/appkit.h>

@interface DRCoord:Object
{
	id	players_i;
	id	console_i;
	id	skill_i;
	id	episode_i;
	id	map_i;
}

- newGame: sender;
- scale1: sender;
- scale2: sender;
- scale4: sender;

@end
