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


//
// Defaults for the local player's data. can be modified from the config
// file. This structure is passed around over the network in multiplayer
// games so that every player's information can be saved on each machine
// participating in the game.
//

cn_playerinfo_t cn_meta_local_playerinfo =
{
    "John",				// first name
    "Doe",				// last name
    "foobar",				// nickname
    "1985-04-22",			// birthdate
    "zw",				// country
    "jdoe@example.net",			// email
    "http://www.example.net/~jdoe/",	// url
};

//
// Array for holding every player's data, including the local player
// or only the local player if it's not a netgame. This is what is
// actually used when writing the metadata after recording is done.
// It is filled inside the netgame code or in the demo recording
// code if it's a single player game.
//
cn_playerinfo_t cn_meta_playerinfos[MAXPLAYERS];

#define CN_META_SIG ('C' | ('N'<<8) | ('D'<<16) | ('M'<<24))
#define CN_META_VERSION 001

static int num_tags;
static FILE *metafp;

// 
// Bind all of the common controls used by Doom and all other games.
//

void CN_BindMetaVariables(void)
{
/* not used
    M_BindVariable("cn_meta_firstname",         &cn_meta_firstname);
    M_BindVariable("cn_meta_lastname",          &cn_meta_lastname);
    M_BindVariable("cn_meta_nickname",          &cn_meta_nickname);
    M_BindVariable("cn_meta_birthdate",         &cn_meta_birthdate);
    M_BindVariable("cn_meta_country",           &cn_meta_country);
    M_BindVariable("cn_meta_email",             &cn_meta_email);
    M_BindVariable("cn_meta_url",               &cn_meta_url);
*/
    M_BindVariable("cn_meta_id",                &cn_meta_id);
}

static void CN_AddMetaTag (char *tag, int size, char *data)
{
    int len, tmp;

    // Write length of tag name + the name itself.
    len = strlen(tag);
    // convert endianness if necessary
    tmp = LONG(len);
    fwrite (&tmp, 4, 1, metafp);
    fwrite (tag, 1, len, metafp);

    // Write length of data + the data itself.
    //
    // size==0 means it's a string and we can use strlen, otherwise it's
    // binary data and it's the caller's responsibility to set size
    // correctly in advance.

    if (!size)
	len = strlen(data);
    else
	len = size;

    tmp = LONG(len);
    fwrite (&tmp, 4, 1, metafp);
    fwrite (data, 1, len, metafp);

    num_tags++;

    return;
}

void CN_WriteMetaData (char *filename)
{
    int metapos;	// location of metadata in the lmp file
    int i, tmp;
    int p;
    
    metafp = fopen(filename, "rb+");
    if (!metafp)
	return;

    // seek to end of demo first and save the location so it can be written later
    fseek (metafp, 0, SEEK_END);
    metapos = ftell(metafp);

    // write signature "CNDM"
    tmp = CN_META_SIG;
    tmp = LONG(tmp);
    fwrite (&tmp, 4, 1, metafp);

    // write version
    tmp = CN_META_VERSION;
    tmp = LONG(tmp);
    fwrite (&tmp, 4, 1, metafp);
 
    num_tags = 0;
/* not used as it's not finished
    for (i=0; i < MAXPLAYERS; i++)
    {
	char buffer[32];

	// cannot use playersingame here because a player can leave midgame,
	// but is still present in the demo
	if (players[i].mo)
	{
        
        snprintf (buffer, 32, "Player%iFirstName", i+1);
	    snprintf (buffer, 32, cn_meta_firstname, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].firstname);

	    snprintf (buffer, 32, "Player%iLastName", i+1);
        snprintf (buffer, 32, cn_meta_lastname, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].lastname);

	    snprintf (buffer, 32, "Player%iNickName", i+1);
        snprintf (buffer, 32, cn_meta_nickname, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].nickname);

	    snprintf (buffer, 32, "Player%iBirthDate", i+1);
        snprintf (buffer, 32, cn_meta_birthdate, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].birthdate);

	    snprintf (buffer, 32, "Player%iCountry", i+1);
        snprintf (buffer, 32, cn_meta_country, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].country);

	    snprintf (buffer, 32, "Player%iEMail", i+1);
        snprintf (buffer, 32, cn_meta_email, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].email);

	    snprintf (buffer, 32, "Player%iURL", i+1);
        snprintf (buffer, 32, cn_meta_url, i+1);
	    CN_AddMetaTag (buffer, 0, cn_meta_playerinfos[i].url);
	}
    }
*/
    // finally write metadata location and # of tags
    tmp = LONG(metapos);
    fwrite (&tmp, 4, 1, metafp);
    tmp = LONG(num_tags);
    fwrite (&tmp, 4, 1, metafp);

    fclose (metafp);

    return;
}

