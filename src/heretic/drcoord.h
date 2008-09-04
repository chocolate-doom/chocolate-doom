
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
