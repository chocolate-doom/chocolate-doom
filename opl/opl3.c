//
// Copyright (C) 2013-2016 Alexey Khokholov (Nuke.YKT)
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
//
//  Nuked OPL3 emulator.
//  Thanks:
//      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
//          Feedback and Rhythm part calculation information.
//      forums.submarine.org.uk(carbon14, opl3):
//          Tremolo and phase generator calculation information.
//      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
//          OPL2 ROMs.
//
// version: 1.6.2
//
//  Changelog:
//
//  v1.1:
//      Vibrato's sign fix.
//  v1.2:
//      Operator key fix.
//      Corrected 4-operator mode.
//      Corrected rhythm mode.
//      Some small fixes.
//  v1.2.1:
//      Small envelope generator fix.
//      Removed EX_Get function(not used)
//  v1.3:
//      Complete rewrite.
//  v1.4:
//      New envelope and waveform generator.
//      Some small fixes.
//  v1.4.1:
//      Envelope generator rate calculation fix.
//  v1.4.2:
//      Version for ZDoom.
//  v1.5:
//      Optimizations.
//  v1.6:
//      Improved emulation output.
//  v1.6.1:
//      Simple YMF289(OPL3-L) emulation.
//  v1.6.2:
//      Version for Chocolate Doom.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opl3.h"

#define RSM_FRAC    10

// Channel types

enum {
	ch_2op = 0,
	ch_4op = 1,
	ch_4op2 = 2,
	ch_drum = 3
};

// Envelope key types

enum {
	egk_norm = 0x01,
	egk_drum = 0x02
};


//
// logsin table
//

static const Bit16u logsinrom[256] = {
	0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471, 0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
	0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd, 0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
	0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f, 0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
	0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195, 0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
	0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c, 0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
	0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8, 0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
	0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1, 0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
	0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094, 0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
	0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070, 0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
	0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052, 0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
	0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039, 0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
	0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026, 0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
	0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017, 0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
	0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c, 0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
	0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004, 0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
	0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

//
// exp table
//

static const Bit16u exprom[256] = {
	0x000, 0x003, 0x006, 0x008, 0x00b, 0x00e, 0x011, 0x014, 0x016, 0x019, 0x01c, 0x01f, 0x022, 0x025, 0x028, 0x02a,
	0x02d, 0x030, 0x033, 0x036, 0x039, 0x03c, 0x03f, 0x042, 0x045, 0x048, 0x04b, 0x04e, 0x051, 0x054, 0x057, 0x05a,
	0x05d, 0x060, 0x063, 0x066, 0x069, 0x06c, 0x06f, 0x072, 0x075, 0x078, 0x07b, 0x07e, 0x082, 0x085, 0x088, 0x08b,
	0x08e, 0x091, 0x094, 0x098, 0x09b, 0x09e, 0x0a1, 0x0a4, 0x0a8, 0x0ab, 0x0ae, 0x0b1, 0x0b5, 0x0b8, 0x0bb, 0x0be,
	0x0c2, 0x0c5, 0x0c8, 0x0cc, 0x0cf, 0x0d2, 0x0d6, 0x0d9, 0x0dc, 0x0e0, 0x0e3, 0x0e7, 0x0ea, 0x0ed, 0x0f1, 0x0f4,
	0x0f8, 0x0fb, 0x0ff, 0x102, 0x106, 0x109, 0x10c, 0x110, 0x114, 0x117, 0x11b, 0x11e, 0x122, 0x125, 0x129, 0x12c,
	0x130, 0x134, 0x137, 0x13b, 0x13e, 0x142, 0x146, 0x149, 0x14d, 0x151, 0x154, 0x158, 0x15c, 0x160, 0x163, 0x167,
	0x16b, 0x16f, 0x172, 0x176, 0x17a, 0x17e, 0x181, 0x185, 0x189, 0x18d, 0x191, 0x195, 0x199, 0x19c, 0x1a0, 0x1a4,
	0x1a8, 0x1ac, 0x1b0, 0x1b4, 0x1b8, 0x1bc, 0x1c0, 0x1c4, 0x1c8, 0x1cc, 0x1d0, 0x1d4, 0x1d8, 0x1dc, 0x1e0, 0x1e4,
	0x1e8, 0x1ec, 0x1f0, 0x1f5, 0x1f9, 0x1fd, 0x201, 0x205, 0x209, 0x20e, 0x212, 0x216, 0x21a, 0x21e, 0x223, 0x227,
	0x22b, 0x230, 0x234, 0x238, 0x23c, 0x241, 0x245, 0x249, 0x24e, 0x252, 0x257, 0x25b, 0x25f, 0x264, 0x268, 0x26d,
	0x271, 0x276, 0x27a, 0x27f, 0x283, 0x288, 0x28c, 0x291, 0x295, 0x29a, 0x29e, 0x2a3, 0x2a8, 0x2ac, 0x2b1, 0x2b5,
	0x2ba, 0x2bf, 0x2c4, 0x2c8, 0x2cd, 0x2d2, 0x2d6, 0x2db, 0x2e0, 0x2e5, 0x2e9, 0x2ee, 0x2f3, 0x2f8, 0x2fd, 0x302,
	0x306, 0x30b, 0x310, 0x315, 0x31a, 0x31f, 0x324, 0x329, 0x32e, 0x333, 0x338, 0x33d, 0x342, 0x347, 0x34c, 0x351,
	0x356, 0x35b, 0x360, 0x365, 0x36a, 0x370, 0x375, 0x37a, 0x37f, 0x384, 0x38a, 0x38f, 0x394, 0x399, 0x39f, 0x3a4,
	0x3a9, 0x3ae, 0x3b4, 0x3b9, 0x3bf, 0x3c4, 0x3c9, 0x3cf, 0x3d4, 0x3da, 0x3df, 0x3e4, 0x3ea, 0x3ef, 0x3f5, 0x3fa
};

// 
// freq mult table multiplied by 2
//
// 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15
//

static const Bit8u mt[16] = { 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30 };

//
// ksl table
//

static const Bit8u kslrom[16] = { 0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64 };

static const Bit8u kslshift[4] = { 8, 1, 2, 0 };

//
// envelope generator constants
//

static const Bit8u eg_incstep[3][4][8] = {
	{ { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ { 0, 1, 0, 1, 0, 1, 0, 1 }, { 0, 1, 0, 1, 1, 1, 0, 1 }, { 0, 1, 1, 1, 0, 1, 1, 1 }, { 0, 1, 1, 1, 1, 1, 1, 1 } },
	{ { 1, 1, 1, 1, 1, 1, 1, 1 }, { 2, 2, 1, 1, 1, 1, 1, 1 }, { 2, 2, 1, 1, 2, 2, 1, 1 }, { 2, 2, 2, 2, 2, 2, 1, 1 } }
};

static const Bit8u eg_incdesc[16] = {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2
};

static const Bit8s eg_incsh[16] = {
	0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, -1, -2
};

//
// address decoding
//

static const Bit8s ad_slot[0x20] = { 0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1, 12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static const Bit8u ch_slot[18] = { 0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32 };

//
// Envelope generator
//

typedef Bit16s(*envelope_sinfunc)(Bit16u phase, Bit16u envelope);
typedef void(*envelope_genfunc)(opl_slot *slott);

Bit16s envelope_calcexp(Bit32u level) {
    if (level > 0x1fff) {
        level = 0x1fff;
    }
    return ((exprom[(level & 0xff) ^ 0xff] | 0x400) << 1) >> (level >> 8);
}

Bit16s envelope_calcsin0(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    Bit16u neg = 0;
    phase &= 0x3ff;
    if (phase & 0x200) {
        neg = ~0;
    }
    if (phase & 0x100) {
        out = logsinrom[(phase & 0xff) ^ 0xff];
    }
    else {
        out = logsinrom[phase & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3)) ^ neg;
}

Bit16s envelope_calcsin1(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    phase &= 0x3ff;
    if (phase & 0x200) {
        out = 0x1000;
    }
    else if (phase & 0x100) {
        out = logsinrom[(phase & 0xff) ^ 0xff];
    }
    else {
        out = logsinrom[phase & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3));
}

Bit16s envelope_calcsin2(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    phase &= 0x3ff;
    if (phase & 0x100) {
        out = logsinrom[(phase & 0xff) ^ 0xff];
    }
    else {
        out = logsinrom[phase & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3));
}

Bit16s envelope_calcsin3(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    phase &= 0x3ff;
    if (phase & 0x100) {
        out = 0x1000;
    }
    else {
        out = logsinrom[phase & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3));
}

Bit16s envelope_calcsin4(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    Bit16u neg = 0;
    phase &= 0x3ff;
    if ((phase & 0x300) == 0x100) {
        neg = ~0;
    }
    if (phase & 0x200) {
        out = 0x1000;
    }
    else if (phase & 0x80) {
        out = logsinrom[((phase ^ 0xff) << 1) & 0xff];
    }
    else {
        out = logsinrom[(phase << 1) & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3)) ^ neg;
}

Bit16s envelope_calcsin5(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    phase &= 0x3ff;
    if (phase & 0x200) {
        out = 0x1000;
    }
    else if (phase & 0x80) {
        out = logsinrom[((phase ^ 0xff) << 1) & 0xff];
    }
    else {
        out = logsinrom[(phase << 1) & 0xff];
    }
    return envelope_calcexp(out + (envelope << 3));
}

Bit16s envelope_calcsin6(Bit16u phase, Bit16u envelope) {
    Bit16u neg = 0;
    phase &= 0x3ff;
    if (phase & 0x200) {
        neg = ~0;
    }
    return envelope_calcexp(envelope << 3) ^ neg;
}

Bit16s envelope_calcsin7(Bit16u phase, Bit16u envelope) {
    Bit16u out = 0;
    Bit16u neg = 0;
    phase &= 0x3ff;
    if (phase & 0x200) {
        neg = ~0;
        phase = (phase & 0x1ff) ^ 0x1ff;
    }
    out = phase << 3;
    return envelope_calcexp(out + (envelope << 3)) ^ neg;
}

envelope_sinfunc envelope_sin[8] = {
    envelope_calcsin0,
    envelope_calcsin1,
    envelope_calcsin2,
    envelope_calcsin3,
    envelope_calcsin4,
    envelope_calcsin5,
    envelope_calcsin6,
    envelope_calcsin7
};

void envelope_gen_off(opl_slot *slott);
void envelope_gen_attack(opl_slot *slott);
void envelope_gen_decay(opl_slot *slott);
void envelope_gen_sustain(opl_slot *slott);
void envelope_gen_release(opl_slot *slott);

envelope_genfunc envelope_gen[5] = {
    envelope_gen_off,
    envelope_gen_attack,
    envelope_gen_decay,
    envelope_gen_sustain,
    envelope_gen_release
};

enum envelope_gen_num {
    envelope_gen_num_off = 0,
    envelope_gen_num_attack = 1,
    envelope_gen_num_decay = 2,
    envelope_gen_num_sustain = 3,
    envelope_gen_num_release = 4
};

Bit8u envelope_calc_rate(opl_slot *slot, Bit8u reg_rate) {
    Bit8u rate;
    if (reg_rate == 0x00) {
        return 0x00;
    }
    rate = (reg_rate << 2) + (slot->reg_ksr ? slot->channel->ksv : (slot->channel->ksv >> 2));
    if (rate > 0x3c) {
        rate = 0x3c;
    }
    return rate;
}

void envelope_update_ksl(opl_slot *slot) {
    Bit16s ksl = (kslrom[slot->channel->f_num >> 6] << 2) - ((0x08 - slot->channel->block) << 5);
    if (ksl < 0) {
        ksl = 0;
    }
    slot->eg_ksl = (Bit8u)ksl;
}

void envelope_update_rate(opl_slot *slot) {
    switch (slot->eg_gen) {
    case envelope_gen_num_off:
        slot->eg_rate = 0;
        break;
    case envelope_gen_num_attack:
        slot->eg_rate = envelope_calc_rate(slot, slot->reg_ar);
        break;
    case envelope_gen_num_decay:
        slot->eg_rate = envelope_calc_rate(slot, slot->reg_dr);
        break;
    case envelope_gen_num_sustain:
    case envelope_gen_num_release:
        slot->eg_rate = envelope_calc_rate(slot, slot->reg_rr);
        break;
    }
}

void envelope_gen_off(opl_slot *slot) {
    slot->eg_rout = 0x1ff;
}

void envelope_gen_attack(opl_slot *slot) {
    if (slot->eg_rout == 0x00) {
        slot->eg_gen = envelope_gen_num_decay;
        envelope_update_rate(slot);
        return;
    }
    slot->eg_rout += ((~slot->eg_rout) * slot->eg_inc) >> 3;
    if (slot->eg_rout < 0x00) {
        slot->eg_rout = 0x00;
    }
}

void envelope_gen_decay(opl_slot *slot) {
    if (slot->eg_rout >= slot->reg_sl << 4) {
        slot->eg_gen = envelope_gen_num_sustain;
        envelope_update_rate(slot);
        return;
    }
    slot->eg_rout += slot->eg_inc;
}

void envelope_gen_sustain(opl_slot *slot) {
    if (!slot->reg_type) {
        envelope_gen_release(slot);
    }
}

void envelope_gen_release(opl_slot *slot) {
    if (slot->eg_rout >= 0x1ff) {
        slot->eg_gen = envelope_gen_num_off;
        slot->eg_rout = 0x1ff;
        envelope_update_rate(slot);
        return;
    }
    slot->eg_rout += slot->eg_inc;
}

void envelope_calc(opl_slot *slot) {
    Bit8u rate_h, rate_l;
    Bit8u inc = 0;
    rate_h = slot->eg_rate >> 2;
    rate_l = slot->eg_rate & 3;
    if (eg_incsh[rate_h] > 0) {
        if ((slot->chip->timer & ((1 << eg_incsh[rate_h]) - 1)) == 0) {
            inc = eg_incstep[eg_incdesc[rate_h]][rate_l][((slot->chip->timer) >> eg_incsh[rate_h]) & 0x07];
        }
    }
    else {
        inc = eg_incstep[eg_incdesc[rate_h]][rate_l][slot->chip->timer & 0x07] << (-eg_incsh[rate_h]);
    }
    slot->eg_inc = inc;
    slot->eg_out = slot->eg_rout + (slot->reg_tl << 2) + (slot->eg_ksl >> kslshift[slot->reg_ksl]) + *slot->trem;
    envelope_gen[slot->eg_gen](slot);
}

void eg_keyon(opl_slot *slot, Bit8u type) {
    if (!slot->key) {
        slot->eg_gen = envelope_gen_num_attack;
        envelope_update_rate(slot);
        if ((slot->eg_rate >> 2) == 0x0f) {
            slot->eg_gen = envelope_gen_num_decay;
            envelope_update_rate(slot);
            slot->eg_rout = 0x00;
        }
        slot->pg_phase = 0x00;
    }
    slot->key |= type;
}

void eg_keyoff(opl_slot *slot, Bit8u type) {
    if (slot->key) {
        slot->key &= (~type);
        if (!slot->key) {
            slot->eg_gen = envelope_gen_num_release;
            envelope_update_rate(slot);
        }
    }
}

//
// Phase Generator
//

void pg_generate(opl_slot *slot) {
    Bit16u f_num;
    Bit32u basefreq;

    f_num = slot->channel->f_num;
    if (slot->reg_vib) {
        Bit8s range;
        Bit8u vibpos;

        range = (f_num >> 7) & 7;
        vibpos = slot->chip->vibpos;

        if (!(vibpos & 3)) {
            range = 0;
        }
        else if (vibpos & 1) {
            range >>= 1;
        }
        range >>= slot->chip->vibshift;

        if (vibpos & 4) {
            range = -range;
        }
        f_num += range;
    }
    basefreq = (f_num << slot->channel->block) >> 1;
    slot->pg_phase += (basefreq * mt[slot->reg_mult]) >> 1;
}

//
// Noise Generator
//

void n_generate(opl_chip *chip) {
    if (chip->noise & 0x01) {
        chip->noise ^= 0x800302;
    }
    chip->noise >>= 1;
}

//
// Slot
//

void slot_write20(opl_slot *slot, Bit8u data) {
    if ((data >> 7) & 0x01) {
        slot->trem = &slot->chip->tremolo;
    }
    else {
        slot->trem = (Bit8u*)&slot->chip->zeromod;
    }
    slot->reg_vib = (data >> 6) & 0x01;
    slot->reg_type = (data >> 5) & 0x01;
    slot->reg_ksr = (data >> 4) & 0x01;
    slot->reg_mult = data & 0x0f;
    envelope_update_rate(slot);
}

void slot_write40(opl_slot *slot, Bit8u data) {
    slot->reg_ksl = (data >> 6) & 0x03;
    slot->reg_tl = data & 0x3f;
    envelope_update_ksl(slot);
}

void slot_write60(opl_slot *slot, Bit8u data) {
    slot->reg_ar = (data >> 4) & 0x0f;
    slot->reg_dr = data & 0x0f;
    envelope_update_rate(slot);
}

void slot_write80(opl_slot *slot, Bit8u data) {
    slot->reg_sl = (data >> 4) & 0x0f;
    if (slot->reg_sl == 0x0f) {
        slot->reg_sl = 0x1f;
    }
    slot->reg_rr = data & 0x0f;
    envelope_update_rate(slot);
}

void slot_writee0(opl_slot *slot, Bit8u data) {
    slot->reg_wf = data & 0x07;
    if (slot->chip->newm == 0x00) {
        slot->reg_wf &= 0x03;
    }
}

void slot_generatephase(opl_slot *slot, Bit16u phase) {
    slot->out = envelope_sin[slot->reg_wf](phase, slot->eg_out);
}

void slot_generate(opl_slot *slot) {
    slot_generatephase(slot, (Bit16u)(slot->pg_phase >> 9) + *slot->mod);
}

void slot_generatezm(opl_slot *slot) {
    slot_generatephase(slot, 0);
}

void slot_calcfb(opl_slot *slot) {
    if (slot->channel->fb != 0x00) {
        slot->fbmod = (slot->prout + slot->out) >> (0x09 - slot->channel->fb);
    }
    else {
        slot->fbmod = 0;
    }
    slot->prout = slot->out;
}

//
// Channel
//

void chan_setupalg(opl_channel *channel);

void chan_updaterhythm(opl_chip *chip, Bit8u data) {
    opl_channel *channel6;
    opl_channel *channel7;
    opl_channel *channel8;
    Bit8u chnum;

    chip->rhy = data & 0x3f;
    if (chip->rhy & 0x20) {
        channel6 = &chip->channel[6];
        channel7 = &chip->channel[7];
        channel8 = &chip->channel[8];
        channel6->out[0] = &channel6->slots[1]->out;
        channel6->out[1] = &channel6->slots[1]->out;
        channel6->out[2] = &chip->zeromod;
        channel6->out[3] = &chip->zeromod;
        channel7->out[0] = &channel7->slots[0]->out;
        channel7->out[1] = &channel7->slots[0]->out;
        channel7->out[2] = &channel7->slots[1]->out;
        channel7->out[3] = &channel7->slots[1]->out;
        channel8->out[0] = &channel8->slots[0]->out;
        channel8->out[1] = &channel8->slots[0]->out;
        channel8->out[2] = &channel8->slots[1]->out;
        channel8->out[3] = &channel8->slots[1]->out;
        for (chnum = 6; chnum < 9; chnum++) {
            chip->channel[chnum].chtype = ch_drum;
        }
        chan_setupalg(channel6);
        //hh
        if (chip->rhy & 0x01) {
            eg_keyon(channel7->slots[0], egk_drum);
        }
        else {
            eg_keyoff(channel7->slots[0], egk_drum);
        }
        //tc
        if (chip->rhy & 0x02) {
            eg_keyon(channel8->slots[1], egk_drum);
        }
        else {
            eg_keyoff(channel8->slots[1], egk_drum);
        }
        //tom
        if (chip->rhy & 0x04) {
            eg_keyon(channel8->slots[0], egk_drum);
        }
        else {
            eg_keyoff(channel8->slots[0], egk_drum);
        }
        //sd
        if (chip->rhy & 0x08) {
            eg_keyon(channel7->slots[1], egk_drum);
        }
        else {
            eg_keyoff(channel7->slots[1], egk_drum);
        }
        //bd
        if (chip->rhy & 0x10) {
            eg_keyon(channel6->slots[0], egk_drum);
            eg_keyon(channel6->slots[1], egk_drum);
        }
        else {
            eg_keyoff(channel6->slots[0], egk_drum);
            eg_keyoff(channel6->slots[1], egk_drum);
        }
    }
    else {
        for (chnum = 6; chnum < 9; chnum++) {
            chip->channel[chnum].chtype = ch_2op;
            chan_setupalg(&chip->channel[chnum]);
        }
    }
}

void chan_writea0(opl_channel *channel, Bit8u data) {
    if (channel->chip->newm && channel->chtype == ch_4op2) {
        return;
    }
    channel->f_num = (channel->f_num & 0x300) | data;
    channel->ksv = (channel->block << 1) | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    envelope_update_ksl(channel->slots[0]);
    envelope_update_ksl(channel->slots[1]);
    envelope_update_rate(channel->slots[0]);
    envelope_update_rate(channel->slots[1]);
    if (channel->chip->newm && channel->chtype == ch_4op) {
        channel->pair->f_num = channel->f_num;
        channel->pair->ksv = channel->ksv;
        envelope_update_ksl(channel->pair->slots[0]);
        envelope_update_ksl(channel->pair->slots[1]);
        envelope_update_rate(channel->pair->slots[0]);
        envelope_update_rate(channel->pair->slots[1]);
    }
}

void chan_writeb0(opl_channel *channel, Bit8u data) {
    if (channel->chip->newm && channel->chtype == ch_4op2) {
        return;
    }
    channel->f_num = (channel->f_num & 0xff) | ((data & 0x03) << 8);
    channel->block = (data >> 2) & 0x07;
    channel->ksv = (channel->block << 1) | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    envelope_update_ksl(channel->slots[0]);
    envelope_update_ksl(channel->slots[1]);
    envelope_update_rate(channel->slots[0]);
    envelope_update_rate(channel->slots[1]);
    if (channel->chip->newm && channel->chtype == ch_4op) {
        channel->pair->f_num = channel->f_num;
        channel->pair->block = channel->block;
        channel->pair->ksv = channel->ksv;
        envelope_update_ksl(channel->pair->slots[0]);
        envelope_update_ksl(channel->pair->slots[1]);
        envelope_update_rate(channel->pair->slots[0]);
        envelope_update_rate(channel->pair->slots[1]);
    }
}

void chan_setupalg(opl_channel *channel) {
    if (channel->chtype == ch_drum) {
        switch (channel->alg & 0x01) {
        case 0x00:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            break;
        case 0x01:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->chip->zeromod;
            break;
        }
        return;
    }
    if (channel->alg & 0x08) {
        return;
    }
    if (channel->alg & 0x04) {
        channel->pair->out[0] = &channel->chip->zeromod;
        channel->pair->out[1] = &channel->chip->zeromod;
        channel->pair->out[2] = &channel->chip->zeromod;
        channel->pair->out[3] = &channel->chip->zeromod;
        switch (channel->alg & 0x03) {
        case 0x00:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->pair->slots[0]->out;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->slots[1]->out;
            channel->out[1] = &channel->chip->zeromod;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x01:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->pair->slots[0]->out;
            channel->slots[0]->mod = &channel->chip->zeromod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->pair->slots[1]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x02:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->chip->zeromod;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->pair->slots[0]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x03:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->chip->zeromod;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->chip->zeromod;
            channel->out[0] = &channel->pair->slots[0]->out;
            channel->out[1] = &channel->slots[0]->out;
            channel->out[2] = &channel->slots[1]->out;
            channel->out[3] = &channel->chip->zeromod;
            break;
        }
    }
    else {
        switch (channel->alg & 0x01) {
        case 0x00:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->slots[1]->out;
            channel->out[1] = &channel->chip->zeromod;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x01:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->chip->zeromod;
            channel->out[0] = &channel->slots[0]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        }
    }
}

void chan_writec0(opl_channel *channel, Bit8u data) {
    channel->fb = (data & 0x0e) >> 1;
    channel->con = data & 0x01;
    channel->alg = channel->con;
    if (channel->chip->newm) {
        if (channel->chtype == ch_4op) {
            channel->pair->alg = 0x04 | (channel->con << 1) | (channel->pair->con);
            channel->alg = 0x08;
            chan_setupalg(channel->pair);
        }
        else if (channel->chtype == ch_4op2) {
            channel->alg = 0x04 | (channel->pair->con << 1) | (channel->con);
            channel->pair->alg = 0x08;
            chan_setupalg(channel);
        }
        else {
            chan_setupalg(channel);
        }
    }
    else {
        chan_setupalg(channel);
    }
    if (channel->chip->newm) {
        channel->cha = ((data >> 4) & 0x01) ? ~0 : 0;
        channel->chb = ((data >> 5) & 0x01) ? ~0 : 0;
    }
    else {
        channel->cha = channel->chb = ~0;
    }
}

void chan_generaterhythm1(opl_chip *chip) {
    opl_channel *channel6;
    opl_channel *channel7;
    opl_channel *channel8;
    Bit16u phase14;
    Bit16u phase17;
    Bit16u phase;
    Bit16u phasebit;

    channel6 = &chip->channel[6];
    channel7 = &chip->channel[7];
    channel8 = &chip->channel[8];
    slot_generate(channel6->slots[0]);
    phase14 = (channel7->slots[0]->pg_phase >> 9) & 0x3ff;
    phase17 = (channel8->slots[1]->pg_phase >> 9) & 0x3ff;
    phase = 0x00;
    //hh tc phase bit
    phasebit = ((phase14 & 0x08) | (((phase14 >> 5) ^ phase14) & 0x04) | (((phase17 >> 2) ^ phase17) & 0x08)) ? 0x01 : 0x00;
    //hh
    phase = (phasebit << 9) | (0x34 << ((phasebit ^ (chip->noise & 0x01) << 1)));
    slot_generatephase(channel7->slots[0], phase);
    //tt
    slot_generatezm(channel8->slots[0]);
}

void chan_generaterhythm2(opl_chip *chip) {
    opl_channel *channel6;
    opl_channel *channel7;
    opl_channel *channel8;
    Bit16u phase14;
    Bit16u phase17;
    Bit16u phase;
    Bit16u phasebit;

    channel6 = &chip->channel[6];
    channel7 = &chip->channel[7];
    channel8 = &chip->channel[8];
    slot_generate(channel6->slots[1]);
    phase14 = (channel7->slots[0]->pg_phase >> 9) & 0x3ff;
    phase17 = (channel8->slots[1]->pg_phase >> 9) & 0x3ff;
    phase = 0x00;
    //hh tc phase bit
    phasebit = ((phase14 & 0x08) | (((phase14 >> 5) ^ phase14) & 0x04) | (((phase17 >> 2) ^ phase17) & 0x08)) ? 0x01 : 0x00;
    //sd
    phase = (0x100 << ((phase14 >> 8) & 0x01)) ^ ((chip->noise & 0x01) << 8);
    slot_generatephase(channel7->slots[1], phase);
    //tc
    phase = 0x100 | (phasebit << 9);
    slot_generatephase(channel8->slots[1], phase);
}

void chan_enable(opl_channel *channel) {
    if (channel->chip->newm) {
        if (channel->chtype == ch_4op) {
            eg_keyon(channel->slots[0], egk_norm);
            eg_keyon(channel->slots[1], egk_norm);
            eg_keyon(channel->pair->slots[0], egk_norm);
            eg_keyon(channel->pair->slots[1], egk_norm);
        }
        else if (channel->chtype == ch_2op || channel->chtype == ch_drum) {
            eg_keyon(channel->slots[0], egk_norm);
            eg_keyon(channel->slots[1], egk_norm);
        }
    }
    else {
        eg_keyon(channel->slots[0], egk_norm);
        eg_keyon(channel->slots[1], egk_norm);
    }
}

void chan_disable(opl_channel *channel) {
    if (channel->chip->newm) {
        if (channel->chtype == ch_4op) {
            eg_keyoff(channel->slots[0], egk_norm);
            eg_keyoff(channel->slots[1], egk_norm);
            eg_keyoff(channel->pair->slots[0], egk_norm);
            eg_keyoff(channel->pair->slots[1], egk_norm);
        }
        else if (channel->chtype == ch_2op || channel->chtype == ch_drum) {
            eg_keyoff(channel->slots[0], egk_norm);
            eg_keyoff(channel->slots[1], egk_norm);
        }
    }
    else {
        eg_keyoff(channel->slots[0], egk_norm);
        eg_keyoff(channel->slots[1], egk_norm);
    }
}

void chan_set4op(opl_chip *chip, Bit8u data) {
    Bit8u bit;
    Bit8u chnum;
    for (bit = 0; bit < 6; bit++) {
        chnum = bit;
        if (bit >= 3) {
            chnum += 9 - 3;
        }
        if ((data >> bit) & 0x01) {
            chip->channel[chnum].chtype = ch_4op;
            chip->channel[chnum + 3].chtype = ch_4op2;
        }
        else {
            chip->channel[chnum].chtype = ch_2op;
            chip->channel[chnum + 3].chtype = ch_2op;
        }
    }
}

Bit16s limshort(Bit32s a) {
    if (a > 32767) {
        a = 32767;
    }
    else if (a < -32768) {
        a = -32768;
    }
    return (Bit16s)a;
}

void chip_generate(opl_chip *chip, Bit16s *buff) {
    Bit8u ii;
    Bit8u jj;
    Bit16s accm;

    buff[1] = limshort(chip->mixbuff[1]);

    for (ii = 0; ii < 12; ii++) {
        slot_calcfb(&chip->slot[ii]);
        pg_generate(&chip->slot[ii]);
        envelope_calc(&chip->slot[ii]);
        slot_generate(&chip->slot[ii]);
    }

    for (ii = 12; ii < 15; ii++) {
        slot_calcfb(&chip->slot[ii]);
        pg_generate(&chip->slot[ii]);
        envelope_calc(&chip->slot[ii]);
    }

    if (chip->rhy & 0x20) {
        chan_generaterhythm1(chip);
    }
    else {
        slot_generate(&chip->slot[12]);
        slot_generate(&chip->slot[13]);
        slot_generate(&chip->slot[14]);
    }

    chip->mixbuff[0] = 0;
    for (ii = 0; ii < 18; ii++) {
        accm = 0;
        for (jj = 0; jj < 4; jj++) {
            accm += *chip->channel[ii].out[jj];
        }
        chip->mixbuff[0] += (Bit16s)(accm & chip->channel[ii].cha);
    }

    for (ii = 15; ii < 18; ii++) {
        slot_calcfb(&chip->slot[ii]);
        pg_generate(&chip->slot[ii]);
        envelope_calc(&chip->slot[ii]);
    }

    if (chip->rhy & 0x20) {
        chan_generaterhythm2(chip);
    }
    else {
        slot_generate(&chip->slot[15]);
        slot_generate(&chip->slot[16]);
        slot_generate(&chip->slot[17]);
    }

    buff[0] = limshort(chip->mixbuff[0]);

    for (ii = 18; ii < 33; ii++) {
        slot_calcfb(&chip->slot[ii]);
        pg_generate(&chip->slot[ii]);
        envelope_calc(&chip->slot[ii]);
        slot_generate(&chip->slot[ii]);
    }

    chip->mixbuff[1] = 0;
    for (ii = 0; ii < 18; ii++) {
        accm = 0;
        for (jj = 0; jj < 4; jj++) {
            accm += *chip->channel[ii].out[jj];
        }
        chip->mixbuff[1] += (Bit16s)(accm & chip->channel[ii].chb);
    }

    for (ii = 33; ii < 36; ii++) {
        slot_calcfb(&chip->slot[ii]);
        pg_generate(&chip->slot[ii]);
        envelope_calc(&chip->slot[ii]);
        slot_generate(&chip->slot[ii]);
    }

    n_generate(chip);

    if ((chip->timer & 0x3f) == 0x3f) {
        chip->tremolopos = (chip->tremolopos + 1) % 210;
        if (chip->tremolopos < 105) {
            chip->tremolo = chip->tremolopos >> (2 + chip->tremoloshift);
        }
        else {
            chip->tremolo = (210 - chip->tremolopos) >> (2 + chip->tremoloshift);
        }
    }

    if ((chip->timer & 0x3ff) == 0x3ff) {
        chip->vibpos = (chip->vibpos + 1) & 7;
    }

    chip->timer++;
}

void chip_generate_opl3l(opl_chip *chip, Bit16s *buffer) {
    while (chip->samplecnt >= chip->rateratio) {
        chip->oldsamples[0] = chip->samples[0];
        chip->oldsamples[1] = chip->samples[1];
        chip_generate(chip, chip->samples);
        chip->samplecnt -= chip->rateratio;
    }
    buffer[0] = (Bit16s)((chip->oldsamples[0] * (chip->rateratio - chip->samplecnt) + chip->samples[0] * chip->samplecnt) / chip->rateratio);
    buffer[1] = (Bit16s)((chip->oldsamples[1] * (chip->rateratio - chip->samplecnt) + chip->samples[1] * chip->samplecnt) / chip->rateratio);
    chip->samplecnt += 1 << RSM_FRAC;
}

void chip_reset(opl_chip *chip, Bit32u samplerate) {
    Bit8u slotnum;
    Bit8u channum;

    memset(chip, 0, sizeof(opl_chip));
    for (slotnum = 0; slotnum < 36; slotnum++) {
        chip->slot[slotnum].chip = chip;
        chip->slot[slotnum].mod = &chip->zeromod;
        chip->slot[slotnum].eg_rout = 0x1ff;
        chip->slot[slotnum].eg_out = 0x1ff;
        chip->slot[slotnum].eg_gen = envelope_gen_num_off;
        chip->slot[slotnum].trem = (Bit8u*)&chip->zeromod;
    }
    for (channum = 0; channum < 18; channum++) {
        chip->channel[channum].slots[0] = &chip->slot[ch_slot[channum]];
        chip->channel[channum].slots[1] = &chip->slot[ch_slot[channum] + 3];
        chip->slot[ch_slot[channum]].channel = &chip->channel[channum];
        chip->slot[ch_slot[channum] + 3].channel = &chip->channel[channum];
        if ((channum % 9) < 3) {
            chip->channel[channum].pair = &chip->channel[channum + 3];
        }
        else if ((channum % 9) < 6) {
            chip->channel[channum].pair = &chip->channel[channum - 3];
        }
        chip->channel[channum].chip = chip;
        chip->channel[channum].out[0] = &chip->zeromod;
        chip->channel[channum].out[1] = &chip->zeromod;
        chip->channel[channum].out[2] = &chip->zeromod;
        chip->channel[channum].out[3] = &chip->zeromod;
        chip->channel[channum].chtype = ch_2op;
        chip->channel[channum].cha = ~0;
        chip->channel[channum].chb = ~0;
        chan_setupalg(&chip->channel[channum]);
    }
    chip->noise = 0x306600;
    chip->rateratio = (samplerate << RSM_FRAC) / 49716;
}

void chip_write(opl_chip *chip, Bit16u reg, Bit8u v) {
    Bit8u high = (reg >> 8) & 0x01;
    Bit8u regm = reg & 0xff;
    switch (regm & 0xf0) {
    case 0x00:
        if (high) {
            switch (regm & 0x0f) {
            case 0x04:
                chan_set4op(chip, v);
                break;
            case 0x05:
                chip->newm = v & 0x01;
                break;
            }
        }
        else {
            switch (regm & 0x0f) {
            case 0x08:
                chip->nts = (v >> 6) & 0x01;
                break;
            }
        }
        break;
    case 0x20:
    case 0x30:
        if (ad_slot[regm & 0x1f] >= 0) {
            slot_write20(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x40:
    case 0x50:
        if (ad_slot[regm & 0x1f] >= 0) {
            slot_write40(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x60:
    case 0x70:
        if (ad_slot[regm & 0x1f] >= 0) {
            slot_write60(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x80:
    case 0x90:
        if (ad_slot[regm & 0x1f] >= 0) {
            slot_write80(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);;
        }
        break;
    case 0xe0:
    case 0xf0:
        if (ad_slot[regm & 0x1f] >= 0) {
            slot_writee0(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0xa0:
        if ((regm & 0x0f) < 9) {
            chan_writea0(&chip->channel[9 * high + (regm & 0x0f)], v);
        }
        break;
    case 0xb0:
        if (regm == 0xbd && !high) {
            chip->tremoloshift = ((v >> 7) ^ 1) << 1;
            chip->vibshift = ((v >> 6) & 0x01) ^ 1;
            chan_updaterhythm(chip, v);
        }
        else if ((regm & 0x0f) < 9) {
            chan_writeb0(&chip->channel[9 * high + (regm & 0x0f)], v);
            if (v & 0x20) {
                chan_enable(&chip->channel[9 * high + (regm & 0x0f)]);
            }
            else {
                chan_disable(&chip->channel[9 * high + (regm & 0x0f)]);
            }
        }
        break;
    case 0xc0:
        if ((regm & 0x0f) < 9) {
            chan_writec0(&chip->channel[9 * high + (regm & 0x0f)], v);
        }
        break;
    }
}


void chip_update(opl_chip *chip, Bit16s* sndptr, Bit32u numsamples) {
	Bit32u i;
	
	for(i = 0; i < numsamples; i++) {
        chip_generate_opl3l(chip, sndptr);
        sndptr += 2;
	}
}
