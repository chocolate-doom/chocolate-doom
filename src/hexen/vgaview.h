
//**************************************************************************
//**
//** VGAView.h : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: VGAView.h,v $
//** $Revision: 1.1 $
//** $Date: 95/05/11 00:19:48 $
//** $Author: bgokey $
//**
//**************************************************************************

#import <appkit/appkit.h>
#import "h2def.h"

// a few globals
extern byte	*bytebuffer;


@interface VGAView:View
{
    id		game;
    int		nextpalette[256];	// color lookup table
    int		*nextimage;		// palette expanded and scaled
    unsigned	scale;
    NXWindowDepth	depth;
}

- updateView;
- (unsigned)scale;
- setPalette:(byte *)pal;
- setScale:(int)newscale;

@end
