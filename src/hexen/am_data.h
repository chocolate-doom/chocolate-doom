//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#ifndef __AMDATA_H__
#define __AMDATA_H__

// a line drawing of the player pointing right, starting from the middle.

#define R ((8*PLAYERRADIUS)/7)

mline_t player_arrow[] = {
  { { -R+R/4, 0 }, { 0, 0} }, // center line.
  { { -R+R/4, R/8 }, { R, 0} }, // blade
  { { -R+R/4, -R/8 }, { R, 0 } },
  { { -R+R/4, -R/4 }, { -R+R/4, R/4 } }, // crosspiece
  { { -R+R/8, -R/4 }, { -R+R/8, R/4 } },
  { { -R+R/8, -R/4 }, { -R+R/4, -R/4} }, //crosspiece connectors
  { { -R+R/8, R/4 }, { -R+R/4, R/4} },
  { { -R-R/4, R/8 }, { -R-R/4, -R/8 } }, //pommel
  { { -R-R/4, R/8 }, { -R+R/8, R/8 } },
  { { -R-R/4, -R/8}, { -R+R/8, -R/8 } }
  };

/*
mline_t keysquare[] = {
	{ { 0, 0 }, { R/4, -R/2 } },
	{ { R/4, -R/2 }, { R/2, -R/2 } },
	{ { R/2, -R/2 }, { R/2, R/2 } },
	{ { R/2, R/2 }, { R/4, R/2 } },
	{ { R/4, R/2 }, { 0, 0 } }, // handle part type thing
	{ { 0, 0 }, { -R, 0 } }, // stem
	{ { -R, 0 }, { -R, -R/2 } }, // end lockpick part
	{ { -3*R/4, 0 }, { -3*R/4, -R/4 } }
	};
*/

/*mline_t player_arrow[] = {
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
  };
*/
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))
#define NUMKEYSQUARELINES (sizeof(keysquare)/sizeof(mline_t))

/*
#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/6 } },  // ----->
  { { R, 0 }, { R-R/2, -R/6 } },
  { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
  { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
  { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
  { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
  { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
  { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
  { { -R/6, -R/6 }, { 0, -R/6 } },
  { { 0, -R/6 }, { 0, R/4 } },
  { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
  { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
  { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
  };
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))
*/


/*
#define R (FRACUNIT)
mline_t triangle_guy[] = {
  { { -.867*R, -.5*R }, { .867*R, -.5*R } },
  { { .867*R, -.5*R } , { 0, R } },
  { { 0, R }, { -.867*R, -.5*R } }
  };
#undef R
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))
*/

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
  { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
  { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
  { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
  };
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))

#endif
