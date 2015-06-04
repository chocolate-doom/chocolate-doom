//
// CNDOOM demo metadata
//

#ifndef __CN_META_H__
#define __CN_META_H__

#include "doomdef.h" // for MAXPLAYERS

#define CN_NETMETALEN 64

typedef struct
{
    char *firstname, *lastname, *nickname,
     *birthdate, *country,  *email,
         *url;
} cn_playerinfo_t;

extern cn_playerinfo_t cn_meta_local_playerinfo;
extern cn_playerinfo_t cn_meta_playerinfos[MAXPLAYERS];

void CN_BindMetaVariables(void);

extern void CN_WriteMetaData (char *demoname);

#endif

