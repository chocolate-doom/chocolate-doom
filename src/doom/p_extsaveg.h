//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016 Fabian Greffrath
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
//	[crispy] Archiving: Extended SaveGame I/O.
//

#ifndef __P_EXTSAVEG__
#define __P_EXTSAVEG__

/* p_extsaveg.c */
extern char *savewadfilename;

extern void P_WriteExtendedSaveGameData (void);
extern void P_ReadExtendedSaveGameData (int pass);

/* p_saveg.c */
extern uint32_t P_ThinkerToIndex (thinker_t* thinker);
extern thinker_t* P_IndexToThinker (uint32_t index);

/* m_menu.c */
extern void M_ForceLoadGame (void);
extern void M_ConfirmDeleteGame (void);

#endif
