#include "peano.h"

// block indices:

// 0 1
// 2 3

static short rotations[4][4] = { // [cur. rot.][block #]
	{1,	-1,	0,	0},
	{-1,	0,	1,	0},
	{0,	0,	-1,	1},
	{0,	1,	0,	-1}
};

// [rot.] -> bit (0-3)
// static short x_bits[] = { 0xC, 0x6, 0x3, 0x9 };
static short x_bits[] = { 0xC, 0x6, 0x3, 0x9 };
static short y_bits[] = { 0x6, 0xC, 0x9, 0x3 };

point z1_to_z2(int z, int level, int rotation) {
	
	// printf("peano z=%i level=%i rot=%i\n",z,level,rotation);
	
	level--;
	
	rotation = (rotation + 4) % 4; // normalize -1 to 3, 4 to 0
	short x_flag, y_flag, lookup = 0; // h_flag: high on h coord
	int z_mask = (3 << level) << level; // mask the right bits of z for the level
	int apropos = ( (z & z_mask) >> level ) >> level; // make it 0-3
	int bit_mask = 1 << apropos; // mask to select from x,y_bits
	// short r_bit_h, r_bit_l;
	point ret, recur;
	ret.x = 0;
	ret.y = 0;
	
	// base case
	if (level < 0) return ret;
	
	x_flag = x_bits[rotation & 3] & bit_mask;
	y_flag = y_bits[rotation & 3] & bit_mask;
	if (x_flag) {
		ret.x |= (1 << level);
		lookup |= 1;
	}
	if (y_flag) {
		ret.y |= (1 << level);
		lookup |= 2;
	}
	recur = z1_to_z2(z, level, rotation + rotations[rotation][lookup]);
	ret.x += recur.x;
	ret.y += recur.y;
	return ret;
}
