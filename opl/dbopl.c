/*
 *  Copyright (C) 2002-2010  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//
// Chocolate Doom-related discussion:
//
// This is the DosBox OPL emulator code (src/hardware/dbopl.cpp) r3635,
// converted to C.  The bulk of the work was done using the minus-minus
// script in the Chocolate Doom SVN repository, then the result tweaked
// by hand until working.
//


/*
	DOSBox implementation of a combined Yamaha YMF262 and Yamaha YM3812 emulator.
	Enabling the opl3 bit will switch the emulator to stereo opl3 output instead of regular mono opl2
	Except for the table generation it's all integer math
	Can choose different types of generators, using muls and bigger tables, try different ones for slower platforms
	The generation was based on the MAME implementation but tried to have it use less memory and be faster in general
	MAME uses much bigger envelope tables and this will be the biggest cause of it sounding different at times

	//TODO Don't delay first operator 1 sample in opl3 mode
	//TODO Maybe not use class method pointers but a regular function pointers with operator as first parameter
	//TODO Fix panning for the Percussion channels, would any opl3 player use it and actually really change it though?
	//TODO Check if having the same accuracy in all frequency multipliers sounds better or not

	//DUNNO Keyon in 4op, switch to 2op without keyoff.
*/

/* $Id: dbopl.cpp,v 1.10 2009-06-10 19:54:51 harekiet Exp $ */


#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include "dosbox.h"
#include "dbopl.h"


#define GCC_UNLIKELY(x) x

#define TRUE 1
#define FALSE 0

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define OPLRATE		((double)(14318180.0 / 288.0))
#define TREMOLO_TABLE 52

//Try to use most precision for frequencies
//Else try to keep different waves in synch
//#define WAVE_PRECISION	1
#ifndef WAVE_PRECISION
//Wave bits available in the top of the 32bit range
//Original adlib uses 10.10, we use 10.22
#define WAVE_BITS	10
#else
//Need some extra bits at the top to have room for octaves and frequency multiplier
//We support to 8 times lower rate
//128 * 15 * 8 = 15350, 2^13.9, so need 14 bits
#define WAVE_BITS	14
#endif
#define WAVE_SH		( 32 - WAVE_BITS )
#define WAVE_MASK	( ( 1 << WAVE_SH ) - 1 )

//Use the same accuracy as the waves
#define LFO_SH ( WAVE_SH - 10 )
//LFO is controlled by our tremolo 256 sample limit
#define LFO_MAX ( 256 << ( LFO_SH ) )


//Maximum amount of attenuation bits
//Envelope goes to 511, 9 bits
#if (DBOPL_WAVE == WAVE_TABLEMUL )
//Uses the value directly
#define ENV_BITS	( 9 )
#else
//Add 3 bits here for more accuracy and would have to be shifted up either way
#define ENV_BITS	( 9 )
#endif
//Limits of the envelope with those bits and when the envelope goes silent
#define ENV_MIN		0
#define ENV_EXTRA	( ENV_BITS - 9 )
#define ENV_MAX		( 511 << ENV_EXTRA )
#define ENV_LIMIT	( ( 12 * 256) >> ( 3 - ENV_EXTRA ) )
#define ENV_SILENT( _X_ ) ( (_X_) >= ENV_LIMIT )

//Attack/decay/release rate counter shift
#define RATE_SH		24
#define RATE_MASK	( ( 1 << RATE_SH ) - 1 )
//Has to fit within 16bit lookuptable
#define MUL_SH		16

//Check some ranges
#if ENV_EXTRA > 3
#error Too many envelope bits
#endif

static inline void Operator__SetState(Operator *self, Bit8u s );
static inline Bit32u Chip__ForwardNoise(Chip *self);

// C++'s template<> sure is useful sometimes.

static Channel* Channel__BlockTemplate(Channel *self, Chip* chip,
                                Bit32u samples, Bit32s* output,
                                SynthMode mode );
#define BLOCK_TEMPLATE(mode) \
    static Channel* Channel__BlockTemplate_ ## mode(Channel *self, Chip* chip, \
                                             Bit32u samples, Bit32s* output) \
    { \
       return Channel__BlockTemplate(self, chip, samples, output, mode); \
    }

BLOCK_TEMPLATE(sm2AM)
BLOCK_TEMPLATE(sm2FM)
BLOCK_TEMPLATE(sm3AM)
BLOCK_TEMPLATE(sm3FM)
BLOCK_TEMPLATE(sm3FMFM)
BLOCK_TEMPLATE(sm3AMFM)
BLOCK_TEMPLATE(sm3FMAM)
BLOCK_TEMPLATE(sm3AMAM)
BLOCK_TEMPLATE(sm2Percussion)
BLOCK_TEMPLATE(sm3Percussion)

//How much to substract from the base value for the final attenuation
static const Bit8u KslCreateTable[16] = {
	//0 will always be be lower than 7 * 8
	64, 32, 24, 19, 
	16, 12, 11, 10, 
	 8,  6,  5,  4,
	 3,  2,  1,  0,
};

#define M(_X_) ((Bit8u)( (_X_) * 2))
static const Bit8u FreqCreateTable[16] = {
	M(0.5), M(1 ), M(2 ), M(3 ), M(4 ), M(5 ), M(6 ), M(7 ),
	M(8  ), M(9 ), M(10), M(10), M(12), M(12), M(15), M(15)
};
#undef M

//We're not including the highest attack rate, that gets a special value
static const Bit8u AttackSamplesTable[13] = {
	69, 55, 46, 40,
	35, 29, 23, 20,
	19, 15, 11, 10,
	9
};
//On a real opl these values take 8 samples to reach and are based upon larger tables
static const Bit8u EnvelopeIncreaseTable[13] = {
	4,  5,  6,  7,
	8, 10, 12, 14,
	16, 20, 24, 28,
	32, 
};

#if ( DBOPL_WAVE == WAVE_HANDLER ) || ( DBOPL_WAVE == WAVE_TABLELOG )
static Bit16u ExpTable[ 256 ];
#endif

#if ( DBOPL_WAVE == WAVE_HANDLER )
//PI table used by WAVEHANDLER
static Bit16u SinTable[ 512 ];
#endif

#if ( DBOPL_WAVE > WAVE_HANDLER )
//Layout of the waveform table in 512 entry intervals
//With overlapping waves we reduce the table to half it's size

//	|    |//\\|____|WAV7|//__|/\  |____|/\/\|
//	|\\//|    |    |WAV7|    |  \/|    |    |
//	|06  |0126|17  |7   |3   |4   |4 5 |5   |

//6 is just 0 shifted and masked

static Bit16s WaveTable[ 8 * 512 ];
//Distance into WaveTable the wave starts
static const Bit16u WaveBaseTable[8] = {
	0x000, 0x200, 0x200, 0x800,
	0xa00, 0xc00, 0x100, 0x400,

};
//Mask the counter with this
static const Bit16u WaveMaskTable[8] = {
	1023, 1023, 511, 511,
	1023, 1023, 512, 1023,
};

//Where to start the counter on at keyon
static const Bit16u WaveStartTable[8] = {
	512, 0, 0, 0,
	0, 512, 512, 256,
};
#endif

#if ( DBOPL_WAVE == WAVE_TABLEMUL )
static Bit16u MulTable[ 384 ];
#endif

static Bit8u KslTable[ 8 * 16 ];
static Bit8u TremoloTable[ TREMOLO_TABLE ];
//Start of a channel behind the chip struct start
static Bit16u ChanOffsetTable[32];
//Start of an operator behind the chip struct start
static Bit16u OpOffsetTable[64];

//The lower bits are the shift of the operator vibrato value
//The highest bit is right shifted to generate -1 or 0 for negation
//So taking the highest input value of 7 this gives 3, 7, 3, 0, -3, -7, -3, 0
static const Bit8s VibratoTable[ 8 ] = {	
	1 - 0x00, 0 - 0x00, 1 - 0x00, 30 - 0x00, 
	1 - 0x80, 0 - 0x80, 1 - 0x80, 30 - 0x80 
};

//Shift strength for the ksl value determined by ksl strength
static const Bit8u KslShiftTable[4] = {
	31,1,2,0
};

//Generate a table index and table shift value using input value from a selected rate
static void EnvelopeSelect( Bit8u val, Bit8u *index, Bit8u *shift ) {
	if ( val < 13 * 4 ) {				//Rate 0 - 12
		*shift = 12 - ( val >> 2 );
		*index = val & 3;
	} else if ( val < 15 * 4 ) {		//rate 13 - 14
		*shift = 0;
		*index = val - 12 * 4;
	} else {							//rate 15 and up
		*shift = 0;
		*index = 12;
	}
}

#if ( DBOPL_WAVE == WAVE_HANDLER )
/*
	Generate the different waveforms out of the sine/exponetial table using handlers
*/
static inline Bits MakeVolume( Bitu wave, Bitu volume ) {
	Bitu total = wave + volume;
	Bitu index = total & 0xff;
	Bitu sig = ExpTable[ index ];
	Bitu exp = total >> 8;
#if 0
	//Check if we overflow the 31 shift limit
	if ( exp >= 32 ) {
		LOG_MSG( "WTF %d %d", total, exp );
	}
#endif
	return (sig >> exp);
};

static Bits DB_FASTCALL WaveForm0( Bitu i, Bitu volume ) {
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	Bitu wave = SinTable[i & 511];
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm1( Bitu i, Bitu volume ) {
	Bit32u wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm2( Bitu i, Bitu volume ) {
	Bitu wave = SinTable[i & 511];
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm3( Bitu i, Bitu volume ) {
	Bitu wave = SinTable[i & 255];
	wave |= ( ( (i ^ 256 ) & 256) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm4( Bitu i, Bitu volume ) {
	//Twice as fast
	i <<= 1;
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	Bitu wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm5( Bitu i, Bitu volume ) {
	//Twice as fast
	i <<= 1;
	Bitu wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm6( Bitu i, Bitu volume ) {
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	return (MakeVolume( 0, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm7( Bitu i, Bitu volume ) {
	//Negative is reversed here
	Bits neg = (( i >> 9) & 1) - 1;
	Bitu wave = (i << 3);
	//When negative the volume also runs backwards
	wave = ((wave ^ neg) - neg) & 4095;
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}

static const WaveHandler WaveHandlerTable[8] = {
	WaveForm0, WaveForm1, WaveForm2, WaveForm3,
	WaveForm4, WaveForm5, WaveForm6, WaveForm7
};

#endif

/*
	Operator
*/

//We zero out when rate == 0
static inline void Operator__UpdateAttack(Operator *self, const Chip* chip ) {
	Bit8u rate = self->reg60 >> 4;
	if ( rate ) {
		Bit8u val = (rate << 2) + self->ksr;
		self->attackAdd = chip->attackRates[ val ];
		self->rateZero &= ~(1 << ATTACK);
	} else {
		self->attackAdd = 0;
		self->rateZero |= (1 << ATTACK);
	}
}
static inline void Operator__UpdateDecay(Operator *self, const Chip* chip ) {
	Bit8u rate = self->reg60 & 0xf;
	if ( rate ) {
		Bit8u val = (rate << 2) + self->ksr;
		self->decayAdd = chip->linearRates[ val ];
		self->rateZero &= ~(1 << DECAY);
	} else {
		self->decayAdd = 0;
		self->rateZero |= (1 << DECAY);
	}
}
static inline void Operator__UpdateRelease(Operator *self, const Chip* chip ) {
	Bit8u rate = self->reg80 & 0xf;
	if ( rate ) {
		Bit8u val = (rate << 2) + self->ksr;
		self->releaseAdd = chip->linearRates[ val ];
		self->rateZero &= ~(1 << RELEASE);
		if ( !(self->reg20 & MASK_SUSTAIN ) ) {
			self->rateZero &= ~( 1 << SUSTAIN );
		}	
	} else {
		self->rateZero |= (1 << RELEASE);
		self->releaseAdd = 0;
		if ( !(self->reg20 & MASK_SUSTAIN ) ) {
			self->rateZero |= ( 1 << SUSTAIN );
		}	
	}
}

static inline void Operator__UpdateAttenuation(Operator *self) {
	Bit8u kslBase = (Bit8u)((self->chanData >> SHIFT_KSLBASE) & 0xff);
	Bit32u tl = self->reg40 & 0x3f;
	Bit8u kslShift = KslShiftTable[ self->reg40 >> 6 ];
	//Make sure the attenuation goes to the right bits
	self->totalLevel = tl << ( ENV_BITS - 7 );	//Total level goes 2 bits below max
	self->totalLevel += ( kslBase << ENV_EXTRA ) >> kslShift;
}

static void Operator__UpdateFrequency(Operator *self) {
	Bit32u freq = self->chanData & (( 1 << 10 ) - 1);
	Bit32u block = (self->chanData >> 10) & 0xff;
#ifdef WAVE_PRECISION
	block = 7 - block;
	self->waveAdd = ( freq * self->freqMul ) >> block;
#else
	self->waveAdd = ( freq << block ) * self->freqMul;
#endif
	if ( self->reg20 & MASK_VIBRATO ) {
		self->vibStrength = (Bit8u)(freq >> 7);

#ifdef WAVE_PRECISION
		self->vibrato = ( self->vibStrength * self->freqMul ) >> block;
#else
		self->vibrato = ( self->vibStrength << block ) * self->freqMul;
#endif
	} else {
		self->vibStrength = 0;
		self->vibrato = 0;
	}
}

static void Operator__UpdateRates(Operator *self, const Chip* chip ) {
	//Mame seems to reverse this where enabling ksr actually lowers
	//the rate, but pdf manuals says otherwise?
	Bit8u newKsr = (Bit8u)((self->chanData >> SHIFT_KEYCODE) & 0xff);
	if ( !( self->reg20 & MASK_KSR ) ) {
		newKsr >>= 2;
	}
	if ( self->ksr == newKsr )
		return;
	self->ksr = newKsr;
	Operator__UpdateAttack( self, chip );
	Operator__UpdateDecay( self, chip );
	Operator__UpdateRelease( self, chip );
}

static inline Bit32s Operator__RateForward(Operator *self, Bit32u add ) {
	Bit32s ret; // haleyjd: GNUisms out!
	self->rateIndex += add;
	ret = self->rateIndex >> RATE_SH;
	self->rateIndex = self->rateIndex & RATE_MASK;
	return ret;
}

static Bits Operator__TemplateVolume(Operator *self, OperatorState yes) {
	Bit32s vol = self->volume;
	Bit32s change;
	switch ( yes ) {
	case OFF:
		return ENV_MAX;
	case ATTACK:
		change = Operator__RateForward( self, self->attackAdd );
		if ( !change )
			return vol;
		vol += ( (~vol) * change ) >> 3;
		if ( vol < ENV_MIN ) {
			self->volume = ENV_MIN;
			self->rateIndex = 0;
			Operator__SetState( self, DECAY );
			return ENV_MIN;
		}
		break;
	case DECAY:
		vol += Operator__RateForward( self, self->decayAdd );
		if ( GCC_UNLIKELY(vol >= self->sustainLevel) ) {
			//Check if we didn't overshoot max attenuation, then just go off
			if ( GCC_UNLIKELY(vol >= ENV_MAX) ) {
				self->volume = ENV_MAX;
				Operator__SetState( self, OFF );
				return ENV_MAX;
			}
			//Continue as sustain
			self->rateIndex = 0;
			Operator__SetState( self, SUSTAIN );
		}
		break;
	case SUSTAIN:
		if ( self->reg20 & MASK_SUSTAIN ) {
			return vol;
		}
		//In sustain phase, but not sustaining, do regular release
	case RELEASE: 
		vol += Operator__RateForward( self, self->releaseAdd );;
		if ( GCC_UNLIKELY(vol >= ENV_MAX) ) {
			self->volume = ENV_MAX;
			Operator__SetState( self, OFF );
			return ENV_MAX;
		}
		break;
	}
	self->volume = vol;
	return vol;
}

#define TEMPLATE_VOLUME(mode) \
    static Bits Operator__TemplateVolume ## mode(Operator *self) \
    { \
        return Operator__TemplateVolume(self, mode); \
    }

TEMPLATE_VOLUME(OFF)
TEMPLATE_VOLUME(RELEASE)
TEMPLATE_VOLUME(SUSTAIN)
TEMPLATE_VOLUME(ATTACK)
TEMPLATE_VOLUME(DECAY)

static const VolumeHandler VolumeHandlerTable[5] = {
        &Operator__TemplateVolumeOFF,
        &Operator__TemplateVolumeRELEASE,
        &Operator__TemplateVolumeSUSTAIN,
        &Operator__TemplateVolumeDECAY,
        &Operator__TemplateVolumeATTACK,
};

static inline Bitu Operator__ForwardVolume(Operator *self) {
	return self->currentLevel + (self->volHandler)(self);
}


static inline Bitu Operator__ForwardWave(Operator *self) {
	self->waveIndex += self->waveCurrent;	
	return self->waveIndex >> WAVE_SH;
}

static void Operator__Write20(Operator *self, const Chip* chip, Bit8u val ) {
	Bit8u change = (self->reg20 ^ val );
	if ( !change ) 
		return;
	self->reg20 = val;
	//Shift the tremolo bit over the entire register, saved a branch, YES!
	self->tremoloMask = (Bit8s)(val) >> 7;
	self->tremoloMask &= ~(( 1 << ENV_EXTRA ) -1);
	//Update specific features based on changes
	if ( change & MASK_KSR ) {
		Operator__UpdateRates( self, chip );
	}
	//With sustain enable the volume doesn't change
	if ( self->reg20 & MASK_SUSTAIN || ( !self->releaseAdd ) ) {
		self->rateZero |= ( 1 << SUSTAIN );
	} else {
		self->rateZero &= ~( 1 << SUSTAIN );
	}
	//Frequency multiplier or vibrato changed
	if ( change & (0xf | MASK_VIBRATO) ) {
		self->freqMul = chip->freqMul[ val & 0xf ];
		Operator__UpdateFrequency(self);
	}
}

static void Operator__Write40(Operator *self, const Chip *chip, Bit8u val ) {
	if (!(self->reg40 ^ val )) 
		return;
	self->reg40 = val;
	Operator__UpdateAttenuation( self );
}

static void Operator__Write60(Operator *self, const Chip* chip, Bit8u val ) {
	Bit8u change = self->reg60 ^ val;
	self->reg60 = val;
	if ( change & 0x0f ) {
		Operator__UpdateDecay( self, chip );
	}
	if ( change & 0xf0 ) {
		Operator__UpdateAttack( self, chip );
	}
}

static void Operator__Write80(Operator *self, const Chip* chip, Bit8u val ) {
	Bit8u change = (self->reg80 ^ val );
	Bit8u sustain; // haleyjd 09/09/10: GNUisms out!
	if ( !change ) 
		return;
	self->reg80 = val;
	sustain = val >> 4;
	//Turn 0xf into 0x1f
	sustain |= ( sustain + 1) & 0x10;
	self->sustainLevel = sustain << ( ENV_BITS - 5 );
	if ( change & 0x0f ) {
		Operator__UpdateRelease( self, chip );
	}
}

static void Operator__WriteE0(Operator *self, const Chip* chip, Bit8u val ) {
	Bit8u waveForm; // haleyjd 09/09/10: GNUisms out!
	if ( !(self->regE0 ^ val) ) 
		return;
	//in opl3 mode you can always selet 7 waveforms regardless of waveformselect
	waveForm = val & ( ( 0x3 & chip->waveFormMask ) | (0x7 & chip->opl3Active ) );
	self->regE0 = val;
#if( DBOPL_WAVE == WAVE_HANDLER )
	self->waveHandler = WaveHandlerTable[ waveForm ];
#else
	self->waveBase = WaveTable + WaveBaseTable[ waveForm ];
	self->waveStart = WaveStartTable[ waveForm ] << WAVE_SH;
	self->waveMask = WaveMaskTable[ waveForm ];
#endif
}

static inline void Operator__SetState(Operator *self, Bit8u s ) {
	self->state = s;
	self->volHandler = VolumeHandlerTable[ s ];
}

static inline int Operator__Silent(Operator *self) {
	if ( !ENV_SILENT( self->totalLevel + self->volume ) )
		return FALSE;
	if ( !(self->rateZero & ( 1 << self->state ) ) )
		return FALSE;
	return TRUE;
}

static inline void Operator__Prepare(Operator *self, const Chip* chip )  {
	self->currentLevel = self->totalLevel + (chip->tremoloValue & self->tremoloMask);
	self->waveCurrent = self->waveAdd;
	if ( self->vibStrength >> chip->vibratoShift ) {
		Bit32s add = self->vibrato >> chip->vibratoShift;
		//Sign extend over the shift value
		Bit32s neg = chip->vibratoSign;
		//Negate the add with -1 or 0
		add = ( add ^ neg ) - neg; 
		self->waveCurrent += add;
	}
}

static void Operator__KeyOn(Operator *self, Bit8u mask ) {
	if ( !self->keyOn ) {
		//Restart the frequency generator
#if( DBOPL_WAVE > WAVE_HANDLER )
		self->waveIndex = self->waveStart;
#else
		self->waveIndex = 0;
#endif
		self->rateIndex = 0;
		Operator__SetState( self, ATTACK );
	}
	self->keyOn |= mask;
}

static void Operator__KeyOff(Operator *self, Bit8u mask ) {
	self->keyOn &= ~mask;
	if ( !self->keyOn ) {
		if ( self->state != OFF ) {
			Operator__SetState( self, RELEASE );
		}
	}
}

static inline Bits Operator__GetWave(Operator *self, Bitu index, Bitu vol ) {
#if( DBOPL_WAVE == WAVE_HANDLER )
	return self->waveHandler( index, vol << ( 3 - ENV_EXTRA ) );
#elif( DBOPL_WAVE == WAVE_TABLEMUL )
	return(self->waveBase[ index & self->waveMask ] * MulTable[ vol >> ENV_EXTRA ]) >> MUL_SH;
#elif( DBOPL_WAVE == WAVE_TABLELOG )
	Bit32s wave = self->waveBase[ index & self->waveMask ];
	Bit32u total = ( wave & 0x7fff ) + vol << ( 3 - ENV_EXTRA );
	Bit32s sig = ExpTable[ total & 0xff ];
	Bit32u exp = total >> 8;
	Bit32s neg = wave >> 16;
	return((sig ^ neg) - neg) >> exp;
#else
#error "No valid wave routine"
#endif
}

static inline Bits Operator__GetSample(Operator *self, Bits modulation ) {
	Bitu vol = Operator__ForwardVolume(self);
	if ( ENV_SILENT( vol ) ) {
		//Simply forward the wave
		self->waveIndex += self->waveCurrent;
		return 0;
	} else {
		Bitu index = Operator__ForwardWave(self);
		index += modulation;
		return Operator__GetWave( self, index, vol );
	}
}

static void Operator__Operator(Operator *self) {
	self->chanData = 0;
	self->freqMul = 0;
	self->waveIndex = 0;
	self->waveAdd = 0;
	self->waveCurrent = 0;
	self->keyOn = 0;
	self->ksr = 0;
	self->reg20 = 0;
	self->reg40 = 0;
	self->reg60 = 0;
	self->reg80 = 0;
	self->regE0 = 0;
	Operator__SetState( self, OFF );
	self->rateZero = (1 << OFF);
	self->sustainLevel = ENV_MAX;
	self->currentLevel = ENV_MAX;
	self->totalLevel = ENV_MAX;
	self->volume = ENV_MAX;
	self->releaseAdd = 0;
}

/*
	Channel
*/

static void Channel__Channel(Channel *self) {
        Operator__Operator(&self->op[0]);
        Operator__Operator(&self->op[1]);
	self->old[0] = self->old[1] = 0;
	self->chanData = 0;
	self->regB0 = 0;
	self->regC0 = 0;
	self->maskLeft = -1;
	self->maskRight = -1;
	self->feedback = 31;
	self->fourMask = 0;
	self->synthHandler = Channel__BlockTemplate_sm2FM;
};

static inline Operator* Channel__Op( Channel *self, Bitu index ) {
        return &( ( self + (index >> 1) )->op[ index & 1 ]);
}

static void Channel__SetChanData(Channel *self, const Chip* chip, Bit32u data ) {
	Bit32u change = self->chanData ^ data;
	self->chanData = data;
	Channel__Op( self, 0 )->chanData = data;
	Channel__Op( self, 1 )->chanData = data;
	//Since a frequency update triggered this, always update frequency
        Operator__UpdateFrequency(Channel__Op( self, 0 ));
        Operator__UpdateFrequency(Channel__Op( self, 1 ));
	if ( change & ( 0xff << SHIFT_KSLBASE ) ) {
                Operator__UpdateAttenuation(Channel__Op( self, 0 ));
                Operator__UpdateAttenuation(Channel__Op( self, 1 ));
	}
	if ( change & ( 0xff << SHIFT_KEYCODE ) ) {
                Operator__UpdateRates(Channel__Op( self, 0 ), chip);
                Operator__UpdateRates(Channel__Op( self, 1 ), chip);
	}
}

static void Channel__UpdateFrequency(Channel *self, const Chip* chip, Bit8u fourOp ) {
	//Extrace the frequency bits
	Bit32u data = self->chanData & 0xffff;
	Bit32u kslBase = KslTable[ data >> 6 ];
	Bit32u keyCode = ( data & 0x1c00) >> 9;
	if ( chip->reg08 & 0x40 ) {
		keyCode |= ( data & 0x100)>>8;	/* notesel == 1 */
	} else {
		keyCode |= ( data & 0x200)>>9;	/* notesel == 0 */
	}
	//Add the keycode and ksl into the highest bits of chanData
	data |= (keyCode << SHIFT_KEYCODE) | ( kslBase << SHIFT_KSLBASE );
        Channel__SetChanData( self + 0, chip, data );
	if ( fourOp & 0x3f ) {
                Channel__SetChanData( self + 1, chip, data );
	}
}

static void Channel__WriteA0(Channel *self, const Chip* chip, Bit8u val ) {
	Bit8u fourOp = chip->reg104 & chip->opl3Active & self->fourMask;
	Bit32u change; // haleyjd 09/09/10: GNUisms out!
	//Don't handle writes to silent fourop channels
	if ( fourOp > 0x80 )
		return;
	change = (self->chanData ^ val ) & 0xff;
	if ( change ) {
		self->chanData ^= change;
		Channel__UpdateFrequency( self, chip, fourOp );
	}
}

static void Channel__WriteB0(Channel *self, const Chip* chip, Bit8u val ) {
	Bit8u fourOp = chip->reg104 & chip->opl3Active & self->fourMask;
	Bitu change; // haleyjd 09/09/10: GNUisms out!
	//Don't handle writes to silent fourop channels
	if ( fourOp > 0x80 )
		return;
	change = (self->chanData ^ ( val << 8 ) ) & 0x1f00;
	if ( change ) {
		self->chanData ^= change;
		Channel__UpdateFrequency( self, chip, fourOp );
	}
	//Check for a change in the keyon/off state
	if ( !(( val ^ self->regB0) & 0x20))
		return;
	self->regB0 = val;
	if ( val & 0x20 ) {
                Operator__KeyOn( Channel__Op(self, 0), 0x1 );
                Operator__KeyOn( Channel__Op(self, 1), 0x1 );
		if ( fourOp & 0x3f ) {
                        Operator__KeyOn( Channel__Op(self + 1, 0), 1 );
                        Operator__KeyOn( Channel__Op(self + 1, 1), 1 );
		}
	} else {
                Operator__KeyOff( Channel__Op(self, 0), 0x1 );
                Operator__KeyOff( Channel__Op(self, 1), 0x1 );
		if ( fourOp & 0x3f ) {
                        Operator__KeyOff( Channel__Op(self + 1, 0), 1 );
                        Operator__KeyOff( Channel__Op(self + 1, 1), 1 );
		}
	}
}

static void Channel__WriteC0(Channel *self, const Chip* chip, Bit8u val ) {
	Bit8u change = val ^ self->regC0;
	if ( !change )
		return;
	self->regC0 = val;
	self->feedback = ( val >> 1 ) & 7;
	if ( self->feedback ) {
		//We shift the input to the right 10 bit wave index value
		self->feedback = 9 - self->feedback;
	} else {
		self->feedback = 31;
	}
	//Select the new synth mode
	if ( chip->opl3Active ) {
		//4-op mode enabled for this channel
		if ( (chip->reg104 & self->fourMask) & 0x3f ) {
			Channel* chan0, *chan1;
			Bit8u synth; // haleyjd 09/09/10: GNUisms out!
			//Check if it's the 2nd channel in a 4-op
			if ( !(self->fourMask & 0x80 ) ) {
				chan0 = self;
				chan1 = self + 1;
			} else {
				chan0 = self - 1;
				chan1 = self;
			}

			synth = ( (chan0->regC0 & 1) << 0 )| (( chan1->regC0 & 1) << 1 );
			switch ( synth ) {
			case 0:
				chan0->synthHandler = Channel__BlockTemplate_sm3FMFM;
				break;
			case 1:
				chan0->synthHandler = Channel__BlockTemplate_sm3AMFM;
				break;
			case 2:
				chan0->synthHandler = Channel__BlockTemplate_sm3FMAM ;
				break;
			case 3:
				chan0->synthHandler = Channel__BlockTemplate_sm3AMAM ;
				break;
			}
		//Disable updating percussion channels
		} else if ((self->fourMask & 0x40) && ( chip->regBD & 0x20) ) {

		//Regular dual op, am or fm
		} else if ( val & 1 ) {
			self->synthHandler = Channel__BlockTemplate_sm3AM;
		} else {
			self->synthHandler = Channel__BlockTemplate_sm3FM;
		}
		self->maskLeft = ( val & 0x10 ) ? -1 : 0;
		self->maskRight = ( val & 0x20 ) ? -1 : 0;
	//opl2 active
	} else { 
		//Disable updating percussion channels
		if ( (self->fourMask & 0x40) && ( chip->regBD & 0x20 ) ) {

		//Regular dual op, am or fm
		} else if ( val & 1 ) {
			self->synthHandler = Channel__BlockTemplate_sm2AM;
		} else {
			self->synthHandler = Channel__BlockTemplate_sm2FM;
		}
	}
}

static void Channel__ResetC0(Channel *self, const Chip* chip ) {
	Bit8u val = self->regC0;
	self->regC0 ^= 0xff;
	Channel__WriteC0( self, chip, val );
};

static inline void Channel__GeneratePercussion(Channel *self, Chip* chip,
                                               Bit32s* output, int opl3Mode ) {
	Channel* chan = self;

	//BassDrum
	Bit32s mod = (Bit32u)((self->old[0] + self->old[1])) >> self->feedback;
	Bit32s sample; // haleyjd 09/09/10
	Bit32u noiseBit;
	Bit32u c2;
	Bit32u c5;
	Bit32u phaseBit;
	Bit32u hhVol;
	Bit32u sdVol;
	Bit32u tcVol;

	self->old[0] = self->old[1];
	self->old[1] = Operator__GetSample( Channel__Op(self, 0), mod ); 

	//When bassdrum is in AM mode first operator is ignoed
	if ( chan->regC0 & 1 ) {
		mod = 0;
	} else {
		mod = self->old[0];
	}
	sample = Operator__GetSample( Channel__Op(self, 1), mod ); 

	//Precalculate stuff used by other outputs
	noiseBit = Chip__ForwardNoise(chip) & 0x1;
	c2 = Operator__ForwardWave(Channel__Op(self, 2));
	c5 = Operator__ForwardWave(Channel__Op(self, 5));
	phaseBit = (((c2 & 0x88) ^ ((c2<<5) & 0x80)) | ((c5 ^ (c5<<2)) & 0x20)) ? 0x02 : 0x00;

	//Hi-Hat
	hhVol = Operator__ForwardVolume(Channel__Op(self, 2));
	if ( !ENV_SILENT( hhVol ) ) {
		Bit32u hhIndex = (phaseBit<<8) | (0x34 << ( phaseBit ^ (noiseBit << 1 )));
		sample += Operator__GetWave( Channel__Op(self, 2), hhIndex, hhVol );
	}
	//Snare Drum
	sdVol = Operator__ForwardVolume( Channel__Op(self, 3) );
	if ( !ENV_SILENT( sdVol ) ) {
		Bit32u sdIndex = ( 0x100 + (c2 & 0x100) ) ^ ( noiseBit << 8 );
		sample += Operator__GetWave( Channel__Op(self, 3), sdIndex, sdVol );
	}
	//Tom-tom
	sample += Operator__GetSample( Channel__Op(self, 4), 0 );

	//Top-Cymbal
	tcVol = Operator__ForwardVolume(Channel__Op(self, 5));
	if ( !ENV_SILENT( tcVol ) ) {
		Bit32u tcIndex = (1 + phaseBit) << 8;
		sample += Operator__GetWave( Channel__Op(self, 5), tcIndex, tcVol );
	}
	sample <<= 1;
	if ( opl3Mode ) {
		output[0] += sample;
		output[1] += sample;
	} else {
		output[0] += sample;
	}
}

Channel* Channel__BlockTemplate(Channel *self, Chip* chip,
                                Bit32u samples, Bit32s* output,
                                SynthMode mode ) {
        Bitu i;

	switch( mode ) {
	case sm2AM:
	case sm3AM:
		if ( Operator__Silent(Channel__Op(self, 0))
                 && Operator__Silent(Channel__Op(self, 1))) {
			self->old[0] = self->old[1] = 0;
			return(self + 1);
		}
		break;
	case sm2FM:
	case sm3FM:
		if ( Operator__Silent(Channel__Op(self, 1))) {
			self->old[0] = self->old[1] = 0;
			return (self + 1);
		}
		break;
	case sm3FMFM:
		if ( Operator__Silent(Channel__Op(self, 3))) {
			self->old[0] = self->old[1] = 0;
			return (self + 2);
		}
		break;
	case sm3AMFM:
		if ( Operator__Silent( Channel__Op(self, 0) )
                 && Operator__Silent( Channel__Op(self, 3) )) {
			self->old[0] = self->old[1] = 0;
			return (self + 2);
		}
		break;
	case sm3FMAM:
		if ( Operator__Silent( Channel__Op(self, 1))
                 && Operator__Silent( Channel__Op(self, 3))) {
			self->old[0] = self->old[1] = 0;
			return (self + 2);
		}
		break;
	case sm3AMAM:
		if ( Operator__Silent( Channel__Op(self, 0) )
                 && Operator__Silent( Channel__Op(self, 2) )
                 && Operator__Silent( Channel__Op(self, 3) )) {
			self->old[0] = self->old[1] = 0;
			return (self + 2);
		}
		break;

        default:
                abort();
	}
	//Init the operators with the the current vibrato and tremolo values
        Operator__Prepare( Channel__Op( self, 0 ), chip );
        Operator__Prepare( Channel__Op( self, 1 ), chip );
	if ( mode > sm4Start ) {
                Operator__Prepare( Channel__Op( self, 2 ), chip );
                Operator__Prepare( Channel__Op( self, 3 ), chip );
	}
	if ( mode > sm6Start ) {
                Operator__Prepare( Channel__Op( self, 4 ), chip );
                Operator__Prepare( Channel__Op( self, 5 ), chip );
	}
	for ( i = 0; i < samples; i++ ) {
		Bit32s mod; // haleyjd 09/09/10: GNUisms out!
		Bit32s sample;
		Bit32s out0;

		//Early out for percussion handlers
		if ( mode == sm2Percussion ) {
			Channel__GeneratePercussion( self, chip, output + i, FALSE );
			continue;	//Prevent some unitialized value bitching
		} else if ( mode == sm3Percussion ) {
			Channel__GeneratePercussion( self, chip, output + i * 2, TRUE );
			continue;	//Prevent some unitialized value bitching
		}

		//Do unsigned shift so we can shift out all bits but still stay in 10 bit range otherwise
		mod = (Bit32u)((self->old[0] + self->old[1])) >> self->feedback;
		self->old[0] = self->old[1];
		self->old[1] = Operator__GetSample( Channel__Op(self, 0), mod );
		sample = 0;
		out0 = self->old[0];
		if ( mode == sm2AM || mode == sm3AM ) {
			sample = out0 + Operator__GetSample( Channel__Op(self, 1), 0 );
		} else if ( mode == sm2FM || mode == sm3FM ) {
			sample = Operator__GetSample( Channel__Op(self, 1), out0 );
		} else if ( mode == sm3FMFM ) {
			Bits next = Operator__GetSample( Channel__Op(self, 1), out0 );
			next = Operator__GetSample( Channel__Op(self, 2), next );
			sample = Operator__GetSample( Channel__Op(self, 3), next );
		} else if ( mode == sm3AMFM ) {
			Bits next; // haleyjd 09/09/10: GNUisms out!
			sample = out0;
			next = Operator__GetSample( Channel__Op(self, 1), 0 );
			next = Operator__GetSample( Channel__Op(self, 2), next );
			sample += Operator__GetSample( Channel__Op(self, 3), next );
		} else if ( mode == sm3FMAM ) {
			Bits next; // haleyjd 09/09/10: GNUisms out!
			sample = Operator__GetSample( Channel__Op(self, 1), out0 );
			next = Operator__GetSample( Channel__Op(self, 2), 0 );
			sample += Operator__GetSample( Channel__Op(self, 3), next );
		} else if ( mode == sm3AMAM ) {
			Bits next; // haleyjd 09/09/10: GNUisms out!
			sample = out0;
			next = Operator__GetSample( Channel__Op(self, 1), 0 );
			sample += Operator__GetSample( Channel__Op(self, 2), next );
			sample += Operator__GetSample( Channel__Op(self, 3), 0 );
		}
		switch( mode ) {
		case sm2AM:
		case sm2FM:
			output[ i ] += sample;
			break;
		case sm3AM:
		case sm3FM:
		case sm3FMFM:
		case sm3AMFM:
		case sm3FMAM:
		case sm3AMAM:
			output[ i * 2 + 0 ] += sample & self->maskLeft;
			output[ i * 2 + 1 ] += sample & self->maskRight;
			break;
                default:
                        abort();
		}
	}
	switch( mode ) {
	case sm2AM:
	case sm2FM:
	case sm3AM:
	case sm3FM:
		return ( self + 1 );
	case sm3FMFM:
	case sm3AMFM:
	case sm3FMAM:
	case sm3AMAM:
		return ( self + 2 );
	case sm2Percussion:
	case sm3Percussion:
		return( self + 3 );
        default:
                abort();
	}
	return 0;
}

/*
	Chip
*/

void Chip__Chip(Chip *self) {
        int i;

        for (i=0; i<18; ++i) {
                Channel__Channel(&self->chan[i]);
        }

	self->reg08 = 0;
	self->reg04 = 0;
	self->regBD = 0;
	self->reg104 = 0;
	self->opl3Active = 0;
}

static inline Bit32u Chip__ForwardNoise(Chip *self) {
	Bitu count;
	self->noiseCounter += self->noiseAdd;
	count = self->noiseCounter >> LFO_SH;
	self->noiseCounter &= WAVE_MASK;
	for ( ; count > 0; --count ) {
		//Noise calculation from mame
		self->noiseValue ^= ( 0x800302 ) & ( 0 - (self->noiseValue & 1 ) );
		self->noiseValue >>= 1;
	}
	return self->noiseValue;
}

static inline Bit32u Chip__ForwardLFO(Chip *self, Bit32u samples ) {
	Bit32u todo; // haleyjd 09/09/10: GNUisms out!!!!!!
	Bit32u count;

	//Current vibrato value, runs 4x slower than tremolo
	self->vibratoSign = ( VibratoTable[ self->vibratoIndex >> 2] ) >> 7;
	self->vibratoShift = ( VibratoTable[ self->vibratoIndex >> 2] & 7) + self->vibratoStrength; 
	self->tremoloValue = TremoloTable[ self->tremoloIndex ] >> self->tremoloStrength;

	//Check hom many samples there can be done before the value changes
	todo = LFO_MAX - self->lfoCounter;
	count = (todo + self->lfoAdd - 1) / self->lfoAdd;
	if ( count > samples ) {
		count = samples;
		self->lfoCounter += count * self->lfoAdd;
	} else {
		self->lfoCounter += count * self->lfoAdd;
		self->lfoCounter &= (LFO_MAX - 1);
		//Maximum of 7 vibrato value * 4
		self->vibratoIndex = ( self->vibratoIndex + 1 ) & 31;
		//Clip tremolo to the the table size
		if ( self->tremoloIndex + 1 < TREMOLO_TABLE  )
			++self->tremoloIndex;
		else
			self->tremoloIndex = 0;
	}
	return count;
}


static void Chip__WriteBD(Chip *self, Bit8u val ) {
	Bit8u change = self->regBD ^ val;
	if ( !change )
		return;
	self->regBD = val;
	//TODO could do this with shift and xor?
	self->vibratoStrength = (val & 0x40) ? 0x00 : 0x01;
	self->tremoloStrength = (val & 0x80) ? 0x00 : 0x02;
	if ( val & 0x20 ) {
		//Drum was just enabled, make sure channel 6 has the right synth
		if ( change & 0x20 ) {
			if ( self->opl3Active ) {
				self->chan[6].synthHandler
                                    = Channel__BlockTemplate_sm3Percussion; 
			} else {
				self->chan[6].synthHandler
                                    = Channel__BlockTemplate_sm2Percussion;
			}
		}
		//Bass Drum
		if ( val & 0x10 ) {
                        Operator__KeyOn( &self->chan[6].op[0], 0x2 );
                        Operator__KeyOn( &self->chan[6].op[1], 0x2 );
		} else {
                        Operator__KeyOff( &self->chan[6].op[0], 0x2 );
                        Operator__KeyOff( &self->chan[6].op[1], 0x2 );
		}
		//Hi-Hat
		if ( val & 0x1 ) {
                        Operator__KeyOn( &self->chan[7].op[0], 0x2 );
		} else {
                        Operator__KeyOff( &self->chan[7].op[0], 0x2 );
		}
		//Snare
		if ( val & 0x8 ) {
                        Operator__KeyOn( &self->chan[7].op[1], 0x2 );
		} else {
                        Operator__KeyOff( &self->chan[7].op[1], 0x2 );
		}
		//Tom-Tom
		if ( val & 0x4 ) {
                        Operator__KeyOn( &self->chan[8].op[0], 0x2 );
		} else {
                        Operator__KeyOff( &self->chan[8].op[0], 0x2 );
		}
		//Top Cymbal
		if ( val & 0x2 ) {
                        Operator__KeyOn( &self->chan[8].op[1], 0x2 );
		} else {
                        Operator__KeyOff( &self->chan[8].op[1], 0x2 );
		}
	//Toggle keyoffs when we turn off the percussion
	} else if ( change & 0x20 ) {
		//Trigger a reset to setup the original synth handler
                Channel__ResetC0( &self->chan[6], self );
                Operator__KeyOff( &self->chan[6].op[0], 0x2 );
                Operator__KeyOff( &self->chan[6].op[1], 0x2 );
                Operator__KeyOff( &self->chan[7].op[0], 0x2 );
                Operator__KeyOff( &self->chan[7].op[1], 0x2 );
                Operator__KeyOff( &self->chan[8].op[0], 0x2 );
                Operator__KeyOff( &self->chan[8].op[1], 0x2 );
	}
}


#define REGOP( _FUNC_ )															\
	index = ( ( reg >> 3) & 0x20 ) | ( reg & 0x1f );								\
	if ( OpOffsetTable[ index ] ) {													\
		Operator* regOp = (Operator*)( ((char *)self ) + OpOffsetTable[ index ] );	\
                Operator__ ## _FUNC_ (regOp, self, val); \
	}

#define REGCHAN( _FUNC_ )																\
	index = ( ( reg >> 4) & 0x10 ) | ( reg & 0xf );										\
	if ( ChanOffsetTable[ index ] ) {													\
		Channel* regChan = (Channel*)( ((char *)self ) + ChanOffsetTable[ index ] );	\
                Channel__ ## _FUNC_ (regChan, self, val); \
	}

void Chip__WriteReg(Chip *self, Bit32u reg, Bit8u val ) {
	Bitu index;
	switch ( (reg & 0xf0) >> 4 ) {
	case 0x00 >> 4:
		if ( reg == 0x01 ) {
			self->waveFormMask = ( val & 0x20 ) ? 0x7 : 0x0; 
		} else if ( reg == 0x104 ) {
			//Only detect changes in lowest 6 bits
			if ( !((self->reg104 ^ val) & 0x3f) )
				return;
			//Always keep the highest bit enabled, for checking > 0x80
			self->reg104 = 0x80 | ( val & 0x3f );
		} else if ( reg == 0x105 ) {
                        int i;

			//MAME says the real opl3 doesn't reset anything on opl3 disable/enable till the next write in another register
			if ( !((self->opl3Active ^ val) & 1 ) )
				return;
			self->opl3Active = ( val & 1 ) ? 0xff : 0;
			//Update the 0xc0 register for all channels to signal the switch to mono/stereo handlers
			for ( i = 0; i < 18;i++ ) {
                                Channel__ResetC0( &self->chan[i], self );
			}
		} else if ( reg == 0x08 ) {
			self->reg08 = val;
		}
	case 0x10 >> 4:
		break;
	case 0x20 >> 4:
	case 0x30 >> 4:
		REGOP( Write20 );
		break;
	case 0x40 >> 4:
	case 0x50 >> 4:
		REGOP( Write40 );
		break;
	case 0x60 >> 4:
	case 0x70 >> 4:
		REGOP( Write60 );
		break;
	case 0x80 >> 4:
	case 0x90 >> 4:
		REGOP( Write80 );
		break;
	case 0xa0 >> 4:
		REGCHAN( WriteA0 );
		break;
	case 0xb0 >> 4:
		if ( reg == 0xbd ) {
			Chip__WriteBD( self, val );
		} else {
			REGCHAN( WriteB0 );
		}
		break;
	case 0xc0 >> 4:
		REGCHAN( WriteC0 );
	case 0xd0 >> 4:
		break;
	case 0xe0 >> 4:
	case 0xf0 >> 4:
		REGOP( WriteE0 );
		break;
	}
}

Bit32u Chip__WriteAddr(Chip *self, Bit32u port, Bit8u val ) {
	switch ( port & 3 ) {
	case 0:
		return val;
	case 2:
		if ( self->opl3Active || (val == 0x05) )
			return 0x100 | val;
		else
			return val;
	}
	return 0;
}

void Chip__GenerateBlock2(Chip *self, Bitu total, Bit32s* output ) {
	while ( total > 0 ) {
                Channel *ch;
		int count;

		Bit32u samples = Chip__ForwardLFO( self, total );
		memset(output, 0, sizeof(Bit32s) * samples);
		count = 0;
		for ( ch = self->chan; ch < self->chan + 9; ) {
			count++;
			ch = (ch->synthHandler)( ch, self, samples, output );
		}
		total -= samples;
		output += samples;
	}
}

void Chip__GenerateBlock3(Chip *self, Bitu total, Bit32s* output  ) {
	while ( total > 0 ) {
                int count;
                Channel *ch;

		Bit32u samples = Chip__ForwardLFO( self, total );
		memset(output, 0, sizeof(Bit32s) * samples *2);
		count = 0;
		for ( ch = self->chan; ch < self->chan + 18; ) {
			count++;
			ch = (ch->synthHandler)( ch, self, samples, output );
		}
		total -= samples;
		output += samples * 2;
	}
}

void Chip__Setup(Chip *self, Bit32u rate ) {
	double original = OPLRATE;
	Bit32u i;
	Bit32u freqScale; // haleyjd 09/09/10: GNUisms out!
//	double original = rate;
	double scale = original / (double)rate;

	//Noise counter is run at the same precision as general waves
	self->noiseAdd = (Bit32u)( 0.5 + scale * ( 1 << LFO_SH ) );
	self->noiseCounter = 0;
	self->noiseValue = 1;	//Make sure it triggers the noise xor the first time
	//The low frequency oscillation counter
	//Every time his overflows vibrato and tremoloindex are increased
	self->lfoAdd = (Bit32u)( 0.5 + scale * ( 1 << LFO_SH ) );
	self->lfoCounter = 0;
	self->vibratoIndex = 0;
	self->tremoloIndex = 0;

	//With higher octave this gets shifted up
	//-1 since the freqCreateTable = *2
#ifdef WAVE_PRECISION
	double freqScale = ( 1 << 7 ) * scale * ( 1 << ( WAVE_SH - 1 - 10));
	for ( i = 0; i < 16; i++ ) {
		self->freqMul[i] = (Bit32u)( 0.5 + freqScale * FreqCreateTable[ i ] );
	}
#else
	freqScale = (Bit32u)( 0.5 + scale * ( 1 << ( WAVE_SH - 1 - 10)));
	for ( i = 0; i < 16; i++ ) {
		self->freqMul[i] = freqScale * FreqCreateTable[ i ];
	}
#endif

	//-3 since the real envelope takes 8 steps to reach the single value we supply
	for ( i = 0; i < 76; i++ ) {
		Bit8u index, shift;
		EnvelopeSelect( i, &index, &shift );
		self->linearRates[i] = (Bit32u)( scale * (EnvelopeIncreaseTable[ index ] << ( RATE_SH + ENV_EXTRA - shift - 3 )));
	}
	//Generate the best matching attack rate
	for ( i = 0; i < 62; i++ ) {
		Bit8u index, shift;
		Bit32s original; // haleyjd 09/09/10: GNUisms out!
		Bit32s guessAdd;
		Bit32s bestAdd;
		Bit32u bestDiff;
		Bit32u passes;
		EnvelopeSelect( i, &index, &shift );
		//Original amount of samples the attack would take
		original = (Bit32u)( (AttackSamplesTable[ index ] << shift) / scale);
		 
		guessAdd = (Bit32u)( scale * (EnvelopeIncreaseTable[ index ] << ( RATE_SH - shift - 3 )));
		bestAdd = guessAdd;
		bestDiff = 1 << 30;

		for ( passes = 0; passes < 16; passes ++ ) {
			Bit32s volume = ENV_MAX;
			Bit32s samples = 0;
			Bit32u count = 0;
			Bit32s diff;
			Bit32u lDiff;
			while ( volume > 0 && samples < original * 2 ) {
				Bit32s change; // haleyjd 09/09/10
				count += guessAdd;
				change = count >> RATE_SH;
				count &= RATE_MASK;
				if ( GCC_UNLIKELY(change) ) { // less than 1 % 
					volume += ( ~volume * change ) >> 3;
				}
				samples++;

			}
			diff = original - samples;
			lDiff = labs( diff );
			//Init last on first pass
			if ( lDiff < bestDiff ) {
				bestDiff = lDiff;
				bestAdd = guessAdd;
				if ( !bestDiff )
					break;
			}
			//Below our target
			if ( diff < 0 ) {
				//Better than the last time
				Bit32s mul = ((original - diff) << 12) / original;
				guessAdd = ((guessAdd * mul) >> 12);
				guessAdd++;
			} else if ( diff > 0 ) {
				Bit32s mul = ((original - diff) << 12) / original;
				guessAdd = (guessAdd * mul) >> 12;
				guessAdd--;
			}
		}
		self->attackRates[i] = bestAdd;
	}
	for ( i = 62; i < 76; i++ ) {
		//This should provide instant volume maximizing
		self->attackRates[i] = 8 << RATE_SH;
	}
	//Setup the channels with the correct four op flags
	//Channels are accessed through a table so they appear linear here
	self->chan[ 0].fourMask = 0x00 | ( 1 << 0 );
	self->chan[ 1].fourMask = 0x80 | ( 1 << 0 );
	self->chan[ 2].fourMask = 0x00 | ( 1 << 1 );
	self->chan[ 3].fourMask = 0x80 | ( 1 << 1 );
	self->chan[ 4].fourMask = 0x00 | ( 1 << 2 );
	self->chan[ 5].fourMask = 0x80 | ( 1 << 2 );

	self->chan[ 9].fourMask = 0x00 | ( 1 << 3 );
	self->chan[10].fourMask = 0x80 | ( 1 << 3 );
	self->chan[11].fourMask = 0x00 | ( 1 << 4 );
	self->chan[12].fourMask = 0x80 | ( 1 << 4 );
	self->chan[13].fourMask = 0x00 | ( 1 << 5 );
	self->chan[14].fourMask = 0x80 | ( 1 << 5 );

	//mark the percussion channels
	self->chan[ 6].fourMask = 0x40;
	self->chan[ 7].fourMask = 0x40;
	self->chan[ 8].fourMask = 0x40;

	//Clear Everything in opl3 mode
	Chip__WriteReg( self, 0x105, 0x1 );
	for ( i = 0; i < 512; i++ ) {
		if ( i == 0x105 )
			continue;
		Chip__WriteReg( self, i, 0xff );
		Chip__WriteReg( self, i, 0x0 );
	}
	Chip__WriteReg( self, 0x105, 0x0 );
	//Clear everything in opl2 mode
	for ( i = 0; i < 255; i++ ) {
		Chip__WriteReg( self, i, 0xff );
		Chip__WriteReg( self, i, 0x0 );
	}
}

static int doneTables = FALSE;
void DBOPL_InitTables( void ) {
        int i, oct;

	if ( doneTables )
		return;
	doneTables = TRUE;
#if ( DBOPL_WAVE == WAVE_HANDLER ) || ( DBOPL_WAVE == WAVE_TABLELOG )
	//Exponential volume table, same as the real adlib
	for ( i = 0; i < 256; i++ ) {
		//Save them in reverse
		ExpTable[i] = (int)( 0.5 + ( pow(2.0, ( 255 - i) * ( 1.0 /256 ) )-1) * 1024 );
		ExpTable[i] += 1024; //or remove the -1 oh well :)
		//Preshift to the left once so the final volume can shift to the right
		ExpTable[i] *= 2;
	}
#endif
#if ( DBOPL_WAVE == WAVE_HANDLER )
	//Add 0.5 for the trunc rounding of the integer cast
	//Do a PI sinetable instead of the original 0.5 PI
	for ( i = 0; i < 512; i++ ) {
		SinTable[i] = (Bit16s)( 0.5 - log10( sin( (i + 0.5) * (PI / 512.0) ) ) / log10(2.0)*256 );
	}
#endif
#if ( DBOPL_WAVE == WAVE_TABLEMUL )
	//Multiplication based tables
	for ( i = 0; i < 384; i++ ) {
		int s = i * 8;
		//TODO maybe keep some of the precision errors of the original table?
		double val = ( 0.5 + ( pow(2.0, -1.0 + ( 255 - s) * ( 1.0 /256 ) )) * ( 1 << MUL_SH ));
		MulTable[i] = (Bit16u)(val);
	}

	//Sine Wave Base
	for ( i = 0; i < 512; i++ ) {
		WaveTable[ 0x0200 + i ] = (Bit16s)(sin( (i + 0.5) * (PI / 512.0) ) * 4084);
		WaveTable[ 0x0000 + i ] = -WaveTable[ 0x200 + i ];
	}
	//Exponential wave
	for ( i = 0; i < 256; i++ ) {
		WaveTable[ 0x700 + i ] = (Bit16s)( 0.5 + ( pow(2.0, -1.0 + ( 255 - i * 8) * ( 1.0 /256 ) ) ) * 4085 );
		WaveTable[ 0x6ff - i ] = -WaveTable[ 0x700 + i ];
	}
#endif
#if ( DBOPL_WAVE == WAVE_TABLELOG )
	//Sine Wave Base
	for ( i = 0; i < 512; i++ ) {
		WaveTable[ 0x0200 + i ] = (Bit16s)( 0.5 - log10( sin( (i + 0.5) * (PI / 512.0) ) ) / log10(2.0)*256 );
		WaveTable[ 0x0000 + i ] = ((Bit16s)0x8000) | WaveTable[ 0x200 + i];
	}
	//Exponential wave
	for ( i = 0; i < 256; i++ ) {
		WaveTable[ 0x700 + i ] = i * 8;
		WaveTable[ 0x6ff - i ] = ((Bit16s)0x8000) | i * 8;
	} 
#endif

	//	|    |//\\|____|WAV7|//__|/\  |____|/\/\|
	//	|\\//|    |    |WAV7|    |  \/|    |    |
	//	|06  |0126|27  |7   |3   |4   |4 5 |5   |

#if (( DBOPL_WAVE == WAVE_TABLELOG ) || ( DBOPL_WAVE == WAVE_TABLEMUL ))
	for ( i = 0; i < 256; i++ ) {
		//Fill silence gaps
		WaveTable[ 0x400 + i ] = WaveTable[0];
		WaveTable[ 0x500 + i ] = WaveTable[0];
		WaveTable[ 0x900 + i ] = WaveTable[0];
		WaveTable[ 0xc00 + i ] = WaveTable[0];
		WaveTable[ 0xd00 + i ] = WaveTable[0];
		//Replicate sines in other pieces
		WaveTable[ 0x800 + i ] = WaveTable[ 0x200 + i ];
		//double speed sines
		WaveTable[ 0xa00 + i ] = WaveTable[ 0x200 + i * 2 ];
		WaveTable[ 0xb00 + i ] = WaveTable[ 0x000 + i * 2 ];
		WaveTable[ 0xe00 + i ] = WaveTable[ 0x200 + i * 2 ];
		WaveTable[ 0xf00 + i ] = WaveTable[ 0x200 + i * 2 ];
	} 
#endif

	//Create the ksl table
	for ( oct = 0; oct < 8; oct++ ) {
		int base = oct * 8;
		for ( i = 0; i < 16; i++ ) {
			int val = base - KslCreateTable[i];
			if ( val < 0 )
				val = 0;
			//*4 for the final range to match attenuation range
			KslTable[ oct * 16 + i ] = val * 4;
		}
	}
	//Create the Tremolo table, just increase and decrease a triangle wave
	for ( i = 0; i < TREMOLO_TABLE / 2; i++ ) {
		Bit8u val = i << ENV_EXTRA;
		TremoloTable[i] = val;
		TremoloTable[TREMOLO_TABLE - 1 - i] = val;
	}
	//Create a table with offsets of the channels from the start of the chip
	{ // haleyjd 09/09/10: Driving me #$%^@ insane
		Chip *chip = NULL;
		for ( i = 0; i < 32; i++ ) {
			Bitu index = i & 0xf;
			Bitu blah; // haleyjd 09/09/10
			if ( index >= 9 ) {
				ChanOffsetTable[i] = 0;
				continue;
			}
			//Make sure the four op channels follow eachother
			if ( index < 6 ) {
				index = (index % 3) * 2 + ( index / 3 );
			}
			//Add back the bits for highest ones
			if ( i >= 16 )
				index += 9;
			blah = (Bitu) ( &(chip->chan[ index ]) );
			ChanOffsetTable[i] = blah;
		}
		//Same for operators
		for ( i = 0; i < 64; i++ ) {
			Bitu chNum; // haleyjd 09/09/10
			Bitu opNum;
			Bitu blah;
			Channel* chan = NULL;
			if ( i % 8 >= 6 || ( (i / 8) % 4 == 3 ) ) {
				OpOffsetTable[i] = 0;
				continue;
			}
			chNum = (i / 8) * 3 + (i % 8) % 3;
			//Make sure we use 16 and up for the 2nd range to match the chanoffset gap
			if ( chNum >= 12 )
				chNum += 16 - 12;
			opNum = ( i % 8 ) / 3;
			blah = (Bitu) ( &(chan->op[opNum]) );
			OpOffsetTable[i] = ChanOffsetTable[ chNum ] + blah;
		}
#if 0
		//Stupid checks if table's are correct
		for ( Bitu i = 0; i < 18; i++ ) {
			Bit32u find = (Bit16u)( &(chip->chan[ i ]) );
			for ( Bitu c = 0; c < 32; c++ ) {
				if ( ChanOffsetTable[c] == find ) {
					find = 0;
					break;
				}
			}
			if ( find ) {
				find = find;
			}
		}
		for ( Bitu i = 0; i < 36; i++ ) {
			Bit32u find = (Bit16u)( &(chip->chan[ i / 2 ].op[i % 2]) );
			for ( Bitu c = 0; c < 64; c++ ) {
				if ( OpOffsetTable[c] == find ) {
					find = 0;
					break;
				}
			}
			if ( find ) {
				find = find;
			}
		}
#endif
	}
}

/*

Bit32u Handler::WriteAddr( Bit32u port, Bit8u val ) {
	return chip.WriteAddr( port, val );

}
void Handler::WriteReg( Bit32u addr, Bit8u val ) {
	chip.WriteReg( addr, val );
}

void Handler::Generate( MixerChannel* chan, Bitu samples ) {
	Bit32s buffer[ 512 * 2 ];
	if ( GCC_UNLIKELY(samples > 512) )
		samples = 512;
	if ( !chip.opl3Active ) {
		chip.GenerateBlock2( samples, buffer );
		chan->AddSamples_m32( samples, buffer );
	} else {
		chip.GenerateBlock3( samples, buffer );
		chan->AddSamples_s32( samples, buffer );
	}
}

void Handler::Init( Bitu rate ) {
	InitTables();
	chip.Setup( rate );
}
*/

