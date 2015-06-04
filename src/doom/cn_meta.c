//
// CNDOOM demo metadata
//

#include <string.h>
#include "doomtype.h"
#include "doomstat.h"
#include "d_iwad.h"
#include "i_swap.h"
#include "cn_meta.h"
#include "g_game.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_argv.h"
#include "config.h"

boolean     speedparm;
boolean     nmareparm;
boolean     maxparm;
boolean     nm100sparm;
boolean     tysonparm;
boolean     pacifistparm;
boolean     episodeparm;
boolean     movieparm;
boolean     coopparm;
boolean     dmparm;

char        cn_meta_signature[4]    = "CNDM";
char        cn_meta_version[4]      = "0002";

static FILE *metafp;

void CN_BindMetaVariables(void)
{
    M_BindStringVariable("cn_meta_id",                &cn_meta_id);
}

static void write_int(FILE *f, int v)
{
	// convert endianness if necessary
	v = LONG(v);
	fwrite(&v,4,1,f);
}

static void write_bytes(FILE *f, const void *buf, int len)
{
	fwrite(buf, 1, len, f);
}

/*
static void CN_AddMetaTag(char *tag, int size, char *data)
{
	int len;

	// Write length of tag name + the name itself.
	len = strlen(tag);
	write_int(metafp, len);
	write_bytes(metafp, tag, len);

	// Write length of data + the data itself.
	//
	// size==0 means it's a string and we can use strlen, otherwise it's
	// binary data and it's the caller's responsibility to set size
	// correctly in advance.

	len = (size) ? size : strlen(data);
	write_int(metafp, len);
	write_bytes(metafp, data, len);
}
*/

void CN_WriteMetaData(char *filename)
{
	int   metapos;	// location of metadata in the lmp file
	int   p;
	char  temp[16];

	//!
	// @category demo
	//
	// Used only when recording speed demo for Competition
	speedparm = M_CheckParm("-speed");

	//!
	// @category demo
	//
	// Used only when recording nightmare demo for Competition
	nmareparm = M_CheckParm("-nmare");

	//!
	// @category demo
	//
	// Used only when recording max demo for Competition
	maxparm = M_CheckParm("-max");

	//!
	// @category demo
	//
	// Used only when recording nightmare 100% secrets demo for Competition
	nm100sparm = M_CheckParm("-nm100s");

	//!
	// @category demo
	//
	// Used only when recording tyson demo for Competition
	tysonparm = M_CheckParm("-tyson");

	//!
	// @category demo
	//
	// Used only when recording pacifist demo for Competition
	pacifistparm = M_CheckParm("-pacifist");

	episodeparm = M_CheckParm("-episode");

	//!
	// @category demo
	//
	// Used only when recording movie demo for Competition
	movieparm = M_CheckParm("-movie");

    dmparm = M_CheckParm("-deathmatch");

    // check if game is DM or not
    if (!dmparm)
        coopparm = M_CheckParm ("-connect");

	metafp = fopen(filename, "rb+");
	if (!metafp)
		return;

	// seek to end of demo first and save the location so it can be written later
	fseek (metafp, 0, SEEK_END);
	metapos = ftell(metafp);

	// fputc (012, metafp);
    // META HEADER
    write_bytes(metafp, "*", 1);

	// write signature "CNDM"
	write_bytes(metafp, cn_meta_signature, 4);
    write_bytes(metafp, "#", 1);

	// write meta version
	write_bytes(metafp, cn_meta_version, 4);
    write_bytes(metafp, "#", 1);

	// write CNDOOM version
	write_bytes(metafp, PACKAGE_VERSION, 7);
    write_bytes(metafp, "#", 1);

	// write Competition ID
	write_bytes(metafp, cn_meta_id, 4);
    write_bytes(metafp, "#", 1);

	// player2 ID
	write_bytes(metafp, "0000", 4);
    write_bytes(metafp, "#", 1);

	// player3 ID
	write_bytes(metafp, "0000", 4);
    write_bytes(metafp, "#", 1);

	// player4 ID
	write_bytes(metafp, "0000", 4);
    write_bytes(metafp, "#", 1);

	// write game
	p = M_CheckParmWithArgs ("-file", 1);
	if (p)
	{
		while (++p != myargc && myargv[p][0] != '-')
		{
			char *wadfilename = D_TryFindWADByName(myargv[p]);
			write_bytes(metafp,
				(strcasecmp(wadfilename,"av.wad") == 0)       ? "05" :
				(strcasecmp(wadfilename,"hr.wad") == 0)       ? "06" :
				(strcasecmp(wadfilename,"scythe.wad") == 0)   ? "07" :
				(strcasecmp(wadfilename,"mm.wad") == 0)       ? "08" :
				(strcasecmp(wadfilename,"mm2.wad") == 0)      ? "09" :
				(strcasecmp(wadfilename,"requiem.wad") == 0)  ? "10" :
				(strcasecmp(wadfilename,"class_ep.wad") == 0) ? "11" :
                                                                "00" , 2);
		}
	}
	else
	{
		write_bytes(metafp,
				(gamemission == doom)      ? "01" :
				(gamemission == doom2)     ? "02" :
				(gamemission == pack_tnt)  ? "03" :
				(gamemission == pack_plut) ? "04" :
				                             "00", 2);
	}
    write_bytes(metafp, "#", 1);

    // write episode
	if (gameepisode>=1 && gameepisode<=4)   sprintf(temp, "%d", gameepisode);
	write_bytes(metafp, temp, 1);
    write_bytes(metafp, "#", 1);

	// write map
    if ((gamemission == doom) && (gameepisode == 1)) {
             if (gamemap>=1 && gamemap<=3 && !secretexit) sprintf(temp,"11%d",gamemap);
        else if (gamemap==3 && secretexit)                sprintf(temp,"114");
        else if (gamemap>=4 && gamemap<=9 && !secretexit) sprintf(temp,"%d",gamemap+111);
    }
    else if ((gamemission == doom) && (gameepisode == 2)) {
             if (gamemap>=1 && gamemap<=5 && !secretexit) sprintf(temp,"12%d",gamemap);
        else if (gamemap==5 && secretexit)                sprintf(temp,"126");
        else if (gamemap>=6 && gamemap<=9 && !secretexit) sprintf(temp,"%d",gamemap+121);
    }
    else if ((gamemission == doom) && (gameepisode == 3)) {
        if (gamemap>=1 && gamemap<=6 && !secretexit) sprintf(temp,"13%d",gamemap);
        if (gamemap==6 && secretexit)                sprintf(temp,"137");
        if (gamemap>=7 && gamemap<=9 && !secretexit) sprintf(temp,"%d",gamemap+131);
    }
    else if ((gamemission == doom) && (gameepisode == 4)) {
             if (gamemap>=1 && gamemap<=2 && !secretexit) sprintf(temp,"14%d",gamemap);
        else if (gamemap==2 && secretexit)                sprintf(temp,"143");
        else if (gamemap>=3 && gamemap<=9 && !secretexit) sprintf(temp,"%d",gamemap+141);
    }
    else
    {
         if (gamemap>=1 && gamemap<=15 && !secretexit)    sprintf(temp,"%02d",gamemap);
    else if (gamemap==15 && secretexit)                   sprintf(temp,"%02d",gamemap+1);
    else if (gamemap>=16 && gamemap<=31 && !secretexit)   sprintf(temp,"%02d",gamemap+1);
    else if (gamemap==31 && secretexit)                   sprintf(temp,"%02d",gamemap+2);
    else if (gamemap==32)                                 sprintf(temp,"%02d",gamemap+2);
    }
	write_bytes(metafp, temp, 3);
    write_bytes(metafp, "#", 1);

	// write category
	write_bytes(metafp,
		(nomonsters)   ? "31" :
		(speedparm)    ? "01" :
		(nmareparm)    ? "02" :
		(maxparm)      ? "03" :
		(nm100sparm)   ? "04" :
		(tysonparm)    ? "05" :
		(pacifistparm) ? "06" :
		(fastparm)     ? "07" :
		(respawnparm)  ? "08" :
		                 "00", 2);
    write_bytes(metafp, "#", 1);

	// check episode or movie
	write_bytes(metafp,
			(episodeparm) ? "E" :
			(movieparm)   ? "M" :
			                "D", 1);
    write_bytes(metafp, "#", 1);

	// check sp, coop, dm
	write_bytes(metafp,
			(dmparm)    ? "D" :
			(coopparm)  ? "C" :
			              "S", 1);
    write_bytes(metafp, "#", 1);

	// write skill
	write_bytes(metafp,
			(gameskill == sk_baby)      ? "1" :
			(gameskill == sk_easy)      ? "2" :
			(gameskill == sk_medium)    ? "3" :
			(gameskill == sk_hard)      ? "4" :
			(gameskill == sk_nightmare) ? "5" :
			                              "0", 1);
    write_bytes(metafp, "#", 1);

	// write game stats (valid only for one map)
	fprintf(metafp,"%04i%04i", ki, totalkills);
    write_bytes(metafp, "#", 1);
	fprintf(metafp,"%04i%04i", it, totalitems);
    write_bytes(metafp, "#", 1);
	fprintf(metafp,"%04i%04i", se, totalsecret);
    write_bytes(metafp, "#", 1);

	// write record time
	if (totaltime)
		{
        fprintf(metafp, "%02i:%02i:%05.2f",
			totaltime / TICRATE / 60 / 60,
			(totaltime / TICRATE / 60) % 60,
			(float)(totaltime % (60*TICRATE)) / TICRATE);
        write_bytes(metafp, "#", 1);
        }
	else
        {
		write_bytes(metafp, "XX:XX:XX.XX", 11);
        write_bytes(metafp, "#", 1);
        }

    // META FOOTER
    write_bytes(metafp, "*", 1);

	// finally write metadata location and # of tags
	write_int(metafp, metapos);

	fclose(metafp);
}
