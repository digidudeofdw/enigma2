#include <lib/base/eerror.h>
#include <lib/dvb/teletext.h>
#include <lib/dvb/idemux.h>
#include <lib/gdi/gpixmap.h>

// G0 and G2 national option table
// see table 33 in ETSI EN 300 706
// use it with (triplet 1 bits 14-11)*(ctrl bits C12-14)

unsigned char NationalOptionSubsetsLookup[16*8] =
{
	1, 4, 11, 5, 3, 8, 0, 9,
	7, 4, 11, 5, 3, 1, 0, 1,
	1, 4, 11, 5, 3, 8, 12, 1,
	1, 1, 1, 1, 1, 10, 1, 9,
	1, 4, 2, 6, 1, 1, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 12, 1,
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 3, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 1, 1, // reserved
	1, 1, 1, 1, 1, 1, 1, 1  // reserved
};

unsigned char NationalReplaceMap[128] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8,
	9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 0
};

// national option subsets (UTF8)
// see table 36 in ETSI EN 300 706

unsigned int NationalOptionSubsets[13*14] = {
	0, 0x0023, 0xc5af, 0xc48d, 0xc5a3, 0xc5be, 0xc3bd, 0xc3ad, 0xc599, 0xc3a9, 0xc3a1, 0xc49b, 0xc3ba, 0xc5a1, // Slovak/Czech
	0, 0xc2a3, 0x0024, 0x0040, 0xe28690, 0xc2bd, 0xe28692, 0xe28691, 0x0023, 0x002d, 0xc2bc, 0xc781, 0xc2be, 0xc3b7, // English
	0, 0x0023, 0xc3b5, 0xc5A0, 0xc384, 0xc396, 0xc5bd, 0xc39c, 0xc395, 0xc5a1, 0xc3a4, 0xc3b6, 0xc5be, 0xc3bc, // Estonian
	0, 0xc3a9, 0xc3af, 0xc3a0, 0xc3ab, 0xc3aa, 0xc3b9, 0xc3ae, 0x0023, 0xc3a8, 0xc3a2, 0xc3b4, 0xc3bb, 0xc3a7, // French
	0, 0x0023, 0x0024, 0xc2a7, 0xc384, 0xc396, 0xc39c, 0x005e, 0x005f, 0xcb9a, 0xc3a4, 0xc3b6, 0xc3bc, 0xc39f, // German
	0, 0xc2a3, 0x0024, 0xc3a9, 0xcb9a, 0xc3a7, 0xe28692, 0xe28691, 0x0023, 0xc3b9, 0xc3a0, 0xc3b2, 0xc3a8, 0xc3ac, // Italian
	0, 0x0023, 0x0024, 0xc5a0, 0xc497, 0xc8a9, 0xc5bd, 0xc48d, 0xc5ab, 0xc5a1, 0xc485, 0xc5b3, 0xc5be, 0xc4af/*FIXMEE*/, // Lithuanian/Lettish
	0, 0x0023, 0xc584, 0xc485, 0xc6b5, 0xc59a, 0xc581, 0xc487, 0xc3b3, 0xc499, 0xc5bc, 0xc59b, 0xc582, 0xc5ba, // Polish
	0, 0xc3a7, 0x0024, 0xc2a1, 0xc3a1, 0xc3a9, 0xc3ad, 0xc3b3, 0xc3ba, 0xc2bf, 0xc3bc, 0xc3b1, 0xc3a8, 0xc3a0, // Spanish/Portuguese
	0, 0x0023, 0xc2a4, 0xc5a2, 0xc382, 0xc59e, 0xc78d, 0xc38e, 0xc4b1, 0xc5a3, 0xc3a2, 0xc59f, 0xc78e, 0xc3ae, // Rumanian
	0, 0x0023, 0xc38b, 0xc48c, 0xc486, 0xc5bd, 0xc490, 0xc5a0, 0xc3ab, 0xc48d, 0xc487, 0xc5be, 0xc491, 0xc5a1, // Slovenian/Serbian/Croation
	0, 0x0023, 0xc2a4, 0xc389, 0xc384, 0xc396, 0xc385, 0xc39c, 0x005f, 0xc3a9, 0xc3a4, 0xc3b6, 0xc3a5, 0xc3bc, // Finnish/Hungarian/Swedish
	0, 0xee8080/*FIXME*/, 0xc7a7, 0xc4b0, 0xc59e, 0xc396, 0xc387, 0xc39c, 0xc7a6, 0xc4b1, 0xc59f, 0xc3b6, 0xc3a7, 0xc3bc  // Turkish
};

unsigned short diacr_upper_cmap[26*15] = {
	0xc380, 0xc381, 0xc382, 0xc383, 0xc480, 0xc482, 0x0000, 0xc384, 0x0000, 0xc385, 0x0000, 0x0000, 0x0000, 0xc484, 0xc482,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc486, 0xc488, 0x0000, 0x0000, 0xc48c, 0xc48a, 0x0000, 0x0000, 0x0000, 0xc387, 0x0000, 0x0000, 0x0000, 0xc48c,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc48e, 0x0000,	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc48e,
	0xc388, 0xc389, 0xc38a, 0x0000, 0xc492, 0xc494, 0xc496, 0xc38b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc498, 0xc49a,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc49c, 0x0000, 0x0000, 0xc49e, 0xc4a0, 0x0000, 0x0000, 0x0000, 0xc4a2, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc4a4, 0x0000, 0xc4a6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xc38c, 0xc38d, 0xc38e, 0xc4a8, 0xc4aa, 0xc4ac, 0xc4b0, 0xc38f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc4ae, 0xc4ac,
	0x0000, 0x0000, 0xc4b4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc4b6, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc4b9, 0x0000, 0x0000, 0x0000, 0x0000, 0xc4bf, 0x0000, 0x0000, 0x0000, 0xc4bb, 0x0000, 0x0000, 0x0000, 0xc4bd,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc583, 0x0000, 0xc391, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc585, 0x0000, 0x0000, 0x0000, 0xc587,
	0xc392, 0xc393, 0xc394, 0xc395, 0xc58c, 0xc58e, 0x0000, 0xc396, 0x0000, 0x0000, 0x0000, 0x0000, 0xc590, 0x0000, 0xc58e,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc594, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc596, 0x0000, 0x0000, 0x0000, 0xc598,
	0x0000, 0xc59a, 0xc59c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc59e, 0x0000, 0x0000, 0x0000, 0xc5a0,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5a2, 0x0000, 0x0000, 0x0000, 0xc5a4,
	0xc399, 0xc39a, 0xc39b, 0xc5a8, 0xc5aa, 0xc5ac, 0x0000, 0xc39c, 0x0000, 0xc5ae, 0x0000, 0x0000, 0xc5b0, 0xc5b2, 0xc5ac,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc5b4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc39d, 0xc5b6, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5b8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc5b9, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5bb, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5bd
};

unsigned short diacr_lower_cmap[26*15] = {
	0xc3a0, 0xc3a1, 0xc3a2, 0xc3a3, 0xc481, 0xc483, 0x0000, 0xc3a4, 0x0000, 0xc3a5, 0x0000, 0x0000, 0x0000, 0xc485, 0xc483,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc487, 0xc489, 0x0000, 0x0000, 0xc48d, 0xc48b, 0x0000, 0x0000, 0x0000, 0xc3a7, 0x0000, 0x0000, 0x0000, 0xc48d,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc48f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc48f,
	0xc3a8, 0xc3a9, 0xc3aa, 0x0000, 0xc493, 0xc495, 0xc497, 0xc3ab, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc499, 0xc49b,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc49d, 0x0000, 0x0000, 0xc49f, 0xc4a1, 0x0000, 0x0000, 0x0000, 0xc4a3, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc4a5, 0x0000, 0xc4a7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xc3ac, 0xc3ad, 0xc3ae, 0xc4a9, 0xc4ab, 0xc4ad, 0xc4b1, 0xc3af, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc4af, 0xc4ad,
	0x0000, 0x0000, 0xc4b5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc4b7, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc4ba, 0x0000, 0x0000, 0x0000, 0x0000, 0xc580, 0x0000, 0x0000, 0x0000, 0xc4bc, 0x0000, 0x0000, 0x0000, 0xc4be,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc584, 0x0000, 0xc3b1, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc586, 0x0000, 0x0000, 0x0000, 0xc588,
	0xc3b2, 0xc3b3, 0xc3b4, 0xc3b5, 0xc58d, 0xc58f, 0x0000, 0xc3b6, 0x0000, 0x0000, 0x0000, 0x0000, 0xc591, 0x0000, 0xc58f,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc595, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc597, 0x0000, 0x0000, 0x0000, 0xc599,
	0x0000, 0xc59b, 0xc59d, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc59f, 0x0000, 0x0000, 0x0000, 0xc5a1,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5a3, 0x0000, 0x0000, 0x0000, 0xc5a5,
	0xc3b9, 0xc3ba, 0xc3bb, 0xc5a9, 0xc5ab, 0xc5ad, 0x0000, 0xc3bc, 0x0000, 0xc5af, 0x0000, 0x0000, 0xc5b1, 0xc5b3, 0xc5ad,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xc5b5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc3bd, 0xc5b7, 0x0000, 0x0000, 0x0000, 0x0000, 0xc3bf, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xc5ba, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5bc, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xc5be
};

unsigned int Latin_G2_set[6*16] = {
	0x0020, 0xc2a1, 0xc2a2, 0xc2a3, 0x0024, 0xc2a5, 0x0023, 0xc2a7, 0xc2a4, 0xc2b4, 0x0022, 0xc2ab, 0x003c, 0x005e, 0x003d, 0x0076,
	0xc2b0, 0xc2b1, 0xc2b2, 0xc2b3, 0xc397, 0xc2b5, 0xc2b6, 0xc2b7, 0xc3b7, 0xc2b4, 0x0022, 0xc2bb, 0xc2bc, 0xc2bd, 0xc2be, 0xc2bf,
	0x0020, 0x0060, 0xc2b4, 0xcb86, 0x007e, 0xcb89, 0xcb98, 0xcb99, 0xcc88, 0x002e, 0xcb9a, 0x0020, 0x005f, 0x0022, 0x0020, 0xcb98,
	0x002d, 0xc2b9, 0xc2ae, 0xc2a9, 0xc4a2, 0x002a, 0xc2ac, 0xc0b0, 0xceb1, 0x0020, 0x0020, 0x0020, 0x002a, 0x002a, 0x002a, 0x002a,
	0xcea9, 0xc386, 0xc490, 0x0061, 0xc4a6, 0x0020, 0xc4b2, 0xc4bf, 0xc581, 0xc398, 0xc592, 0x006f, 0xc39e, 0xc5a6, 0xc58a, 0xc589,
	0xc4b8, 0xc3a6, 0xc491, 0xc48f, 0xc4a7, 0xc4b1, 0xc4b3, 0xc580, 0xc582, 0xc3b8, 0xc593, 0xc39f, 0xc3be, 0xc5a7, 0xc58b, 0x0020,
};

// This is a very simple en300 706 telext decoder.
// It can only decode a single page at a time, thus it's only used
// for subtitles. And it ONLY support LATIN Charsets yet!
 
DEFINE_REF(eDVBTeletextParser);

	/* we asumme error free transmission! */
static inline unsigned char decode_odd_parity(unsigned char *b)
{
	int i;
	unsigned char res = 0;
	for (i=0; i<7; ++i)
		if (*b & (0x80 >> i))
			res |= 1<<i;
	return res;
}

static inline unsigned char decode_hamming_84(unsigned char *b)
{
	return ((*b << 3) & 8) | ((*b     ) & 4) | ((*b >> 3) & 2) | ((*b >> 6) & 1);
}

static inline unsigned long decode_hamming_2418(unsigned char *b)
{
	static const unsigned char rev[16] = {
		0x00,0x08,0x04,0x0c,
		0x02,0x0a,0x06,0x0e,
		0x01,0x09,0x05,0x0d,
		0x03,0x0b,0x07,0x0f
	};
	b[0] = rev[b[0] >> 4] | (rev[b[0] & 0xf] << 4);
	b[1] = rev[b[1] >> 4] | (rev[b[1] & 0xf] << 4);
	b[2] = rev[b[2] >> 4] | (rev[b[2] & 0xf] << 4);
	return ((b[0] & 0x04) >> 2) | ((b[0] & 0x70) >> 3) | ((b[1] & 0x7f) << 4) | ((b[2] & 0x7f) << 11);
}

static int extractPTS(pts_t &pts, unsigned char *pkt)
{
	pkt += 7;
	int flags = *pkt++;
	
	pkt++; // header length
	
	if (flags & 0x80) /* PTS present? */
	{
			/* damn gcc bug */
		pts  = ((unsigned long long)(((pkt[0] >> 1) & 7))) << 30;
		pts |=   pkt[1] << 22;
		pts |=  (pkt[2]>>1) << 15;
		pts |=   pkt[3] << 7;
		pts |=  (pkt[5]>>1);
		
		return 0;
	} else
		return -1;
}

eDVBTeletextParser::eDVBTeletextParser(iDVBDemux *demux) : m_pid(-1)
{
	setStreamID(0xBD); /* as per en 300 472 */
	
	setPageAndMagazine(-1, -1);
	
	if (demux->createPESReader(eApp, m_pes_reader))
		eDebug("failed to create teletext subtitle PES reader!");
	else
		m_pes_reader->connectRead(slot(*this, &eDVBTeletextParser::processData), m_read_connection);
}

eDVBTeletextParser::~eDVBTeletextParser()
{
}

char *get_bits(int val, int count)
{
	static char buf[33];
	memset(buf, 0, 32);
	if (count < 33)
		for (int i=0; i < count; ++i)
		{
			buf[(count-i)-1]=val&1?'1':'0';
			val>>=1;
		}
	return buf;
}

void eDVBTeletextParser::processPESPacket(__u8 *pkt, int len)
{
	unsigned char *p = pkt;
	
	pts_t pts;
	int have_pts = extractPTS(pts, pkt);
	
	p += 4; len -= 4; /* start code, already be verified by pes parser */
	p += 2; len -= 2; /* length, better use the argument */	
	
	p += 3; len -= 3; /* pes header */
	
	p += 0x24; len -= 0x24; /* skip header */
	
//	eDebug("data identifier: %02x", *p);
	
	p++; len--;
	
	while (len > 2)
	{
		/*unsigned char data_unit_id = */*p++;
		unsigned char data_unit_length = *p++;
		len -= 2;
		
		if (len < data_unit_length)
		{
			eDebug("data_unit_length > len");
			break;
		}
		
		if (data_unit_length != 44)
		{
			/* eDebug("illegal data unit length %d", data_unit_length); */
			break;
		}
		
//		if (data_unit_id != 0x03)
//		{
//			/* eDebug("non subtitle data unit id %d", data_unit_id); */
//			break;
//		}
		
		/*unsigned char line_offset =*/ *p++; len--;
		unsigned char framing_code = *p++; len--;

		int magazine_and_packet_address = decode_hamming_84(p++); len--;
		magazine_and_packet_address |= decode_hamming_84(p++)<<4; len--;

		unsigned char *data = p; p += 40; len -= 40;
		
		if (framing_code != 0xe4) /* no teletxt data */
			continue;

		int M = magazine_and_packet_address & 7,
			Y = magazine_and_packet_address >> 3;
//			eDebug("line %d, framing code: %02x, M=%02x, Y=%02x", line_offset, framing_code, m_M, m_Y);

		if (Y == 0) /* page header */
		{
			int X = decode_hamming_84(data + 1) * 0x10 + decode_hamming_84(data),
//				S1 = decode_hamming_84(data + 2),
				S2C4 = decode_hamming_84(data + 3),
//				S2 = S2C4 & 7,
//				S3 = decode_hamming_84(data + 4),
				S4C5C6 = decode_hamming_84(data + 5),
//				S4 = S4C5C6 & 3,
				C = ((S2C4 & 8) ? (1<<4) : 0) |
					((S4C5C6 & 0xC) << 3) |
					(decode_hamming_84(data + 6) << 7) |
					(decode_hamming_84(data + 7) << 11),
				serial_mode = C & (1<<11);

				/* page on the same magazine? end current page. */
			if ((serial_mode || M == m_page_M) && m_page_open)
			{
				eDebug("Page End %d %lld", !have_pts, pts);
				handlePageEnd(!have_pts, pts);
				m_page_open = 0;
			}

			if ((C & (1<<6)) && (X != 0xFF) && !(C & (1<<5))) /* scan for pages with subtitle bit set */
			{
				eDVBServicePMTHandler::subtitleStream s;
				s.pid = m_pid;
				s.subtitling_type = 0x01; // ebu teletext subtitle
				s.teletext_page_number = X & 0xFF;
				s.teletext_magazine_number = M & 7;
				if (m_found_subtitle_pages.find(s) == m_found_subtitle_pages.end())
				{
					m_found_subtitle_pages.insert(s);
					m_new_subtitle_stream();
				}
			}

				/* correct page on correct magazine? open page. */
			if (M == m_page_M && X == m_page_X)
			{
				eDebug("Page Start %d %lld", !have_pts, pts);
				m_C = C;
				m_Y = Y;
				handlePageStart();
				m_page_open = 1;
				m_box_open = 0;
				handleLine(data + 8, 32);
			}
		} else if (Y < 26) // directly displayable packet
		{
			/* data for the selected page ? */
			if (M == m_page_M && m_page_open)
			{
				m_Y = Y;
				m_box_open = 0;
				handleLine(data, 40);
			}
		} else if (Y == 26 && m_page_open && M == m_page_M)
		{
//			int designation_code = decode_hamming_84(data);
			int display_row=-1, display_column=-1;
			for (int a = 1; a < 40; a+=3)
			{
				int val;
				if ((val=decode_hamming_2418(data+a)) >= 0)
				{
					unsigned char addr = val & 0x3F;
					unsigned char mode = (val >> 6) & 0x1F;
					unsigned char data = (val >> 11) & 0x7F;
					if (addr == 0x3f && mode == 0x1f) // termination marker
						break;
					if (addr >= 40)
					{
						if (mode == 4)
						{
							display_row = addr - 40;
							continue;
						}
						else
							eDebugNoNewLine("ignore unimplemented: ");
					}
					else //0..39 means column 0..39
					{
						if (display_row != -1)
						{
							display_column = addr;
//							eDebugNoNewLine("PosX(%d) ", display_column);
//							eDebugNoNewLine("PosY(%d) ", display_row);
							if (mode > 15) //char from G0 set w/ diacr.
							{
								unsigned int ch=data;
								if (!mode&0xF)
								{
									if (data == 0x2A)
										ch = '@';
								}
								else
								{
									if (ch > 96 && ch < 123)
										ch = diacr_lower_cmap[(ch-97)*15+(mode&0xF)-1];
									else if (ch > 64 && ch < 91)
										ch = diacr_upper_cmap[(ch-65)*15+(mode&0xF)-1];
								}
								if (ch)
									m_modifications[(display_row<<16)|display_column] = ch ? ch : data;
								else /* when data is 0 we set the diacr. mark later on the existing character ..
										this isn't described in the EN300706.. but i have seen this on "Das Erste" */
									m_modifications[(display_row<<16)|display_column] = (mode&0xF);
//								eDebug("char(%04x) w/ diacr. mark", ch);
								continue;
							}
							else if (mode == 15) // char from G2 set
							{
								if (data > 0x19)
								{
									unsigned int ch=Latin_G2_set[data-0x20];
									m_modifications[(display_row<<16)|display_column] = ch;
//									eDebug("char(%04x) from G2 set", ch);
									continue;
								}
								else
									eDebugNoNewLine("ignore G2 char < 0x20: ");
							}
							else
								eDebugNoNewLine("ignore unimplemented: ");
						}
						else
							eDebugNoNewLine("row is not selected.. ignore: ");
					}
					eDebugNoNewLine("triplet = %08x(%s) ", val, get_bits(val, 18));
					eDebugNoNewLine("address = %02x(%s) ", addr, get_bits(addr, 6));
					eDebugNoNewLine("mode = %02x(%s) ", mode, get_bits(mode, 5));
					eDebug("data = %02x(%s)", data, get_bits(data, 7));
				}
			}
		} else if (Y > 29)
			/*eDebug("non handled packet 30, 31", Y, decode_hamming_84(data))*/;
		else if (Y == 29 && M == m_page_M)
		{
			int designation_code = decode_hamming_84(data++);
			if (designation_code == 0) // 29/0
			{
				m_M29_t1 = decode_hamming_2418(data);
				m_M29_t2 = decode_hamming_2418(data+3);
				if ((m_M29_t1 & 0xF) == 0) // format1
					m_M29_0_valid = 1;
				else
					eDebug("non handled packet M/%d/0 format %d", Y, m_M29_t1 & 0xF);
			}
			else
				eDebug("non handled packet M/%d/%d", Y, designation_code);
		}
		else if (m_page_open && M == m_page_M)
		{
			int designation_code = decode_hamming_84(data++);
			if (Y == 28 && designation_code == 0)   // 28/0
			{
#if 1
				m_X28_t1 = decode_hamming_2418(data);
				m_X28_t2 = decode_hamming_2418(data+3);
				if ((m_X28_t1 & 0xF) == 0) // format1
					m_X28_0_valid = 1;
				else
					eDebug("non handled packet X/%d/0 format %d", Y, m_X28_t1 & 0xF);
#else
					int i=0;
					for (; i < 39; i+=3)
					{
						int tripletX = decode_hamming_2418(data+i);
						if (tripletX >= 0)
						{
							if (i == 0)
							{
								if ((m_X28_t1 & 0xF) == 0) // format1
									m_X28_0_valid = 1;
								m_X28_t1 = tripletX;
							}
							else if (i == 1)
								m_X28_t2 = tripletX;

							char *c = get_bits(tripletX, 18);
							int x=0;
							for (; x < 18; ++x)
								eDebugNoNewLine("%c", c[x]);
							eDebug("");
						}
						else
							eDebug("decode_hamming_2418 failed!\n");
						data += 3;
					}
#endif
			}
			else
				eDebug("non handled packet X/%d/%d", Y, designation_code);
		}
	}
}

int eDVBTeletextParser::start(int pid)
{
	m_page_open = 0;

	if (m_pes_reader && pid >= 0 && pid < 0x1fff)
	{
		m_pid = pid;
		return m_pes_reader->start(pid);
	}
	else
		return -1;
}

void eDVBTeletextParser::handlePageStart()
{
	if (m_C & (1<<4)) /* erase flag set */
	{
		m_subtitle_page.clear();
		m_modifications.clear();
		m_X28_0_valid = 0;
//		eDebug("erase page!");
	}
//	else
//		eDebug("no erase flag set!");
}

void eDVBTeletextParser::handleLine(unsigned char *data, int len)
{
/* // hexdump
	for (int i=0; i<len; ++i)
		eDebugNoNewLine("%02x ", decode_odd_parity(data + i));
	eDebug(""); */
	
	m_subtitle_page.clearLine(m_Y);

	if (!m_Y) /* first line is page header, we don't need that. */
	{
		m_double_height = -1;
		return;
	}
		
	if (m_double_height == m_Y)
	{
		m_double_height = -1;
		return;
	}

	int last_was_white = 1, color = 7; /* start with whitespace. start with color=white. (that's unrelated.) */

	static unsigned char out[128];

	int outidx = 0,
		Gtriplet = 0,
		nat_opts = (m_C & (1<<14) ? 1 : 0) |
					(m_C & (1<<13) ? 2 : 0) |
					(m_C & (1<<12) ? 4 : 0),
		nat_subset_2 = NationalOptionSubsetsLookup[Gtriplet*8+nat_opts],
		nat_subset = nat_subset_2,
		second_G0_set = 0;

	if (m_X28_0_valid)
	{
		nat_subset = NationalOptionSubsetsLookup[(m_X28_t1 >> 7) & 0x7F];
		nat_subset_2 = NationalOptionSubsetsLookup[((m_X28_t1 >> 14) & 0xF) | ((m_X28_t2 & 7) << 4)];
//		eDebug("X/28/0 nat_subset %d, nat_subset2 %d", nat_subset, nat_subset_2);
	}
	else if (m_M29_0_valid)
	{
		nat_subset = NationalOptionSubsetsLookup[(m_M29_t1 >> 7) & 0x7F];
		nat_subset_2 = NationalOptionSubsetsLookup[((m_M29_t1 >> 14) & 0xF) | ((m_M29_t2 & 7) << 4)];
//		eDebug("M/29/0 nat_subset %d, nat_subset2 %d", nat_subset, nat_subset_2);
	}
/*	else
		eDebug("nat_opts = %d, nat_subset = %d, C121314 = %d%d%d",
			nat_opts, nat_subset,
			(m_C & (1<<12))?1:0,
			(m_C & (1<<13))?1:0,
			(m_C & (1<<14))?1:0);*/

//	eDebug("handle subtitle line: %d len", len);
	for (int i=0; i<len; ++i)
	{
		unsigned char b = decode_odd_parity(data + i);
		std::map<int,unsigned int>::iterator it = m_modifications.find((m_Y<<16)|i);

		if (it != m_modifications.end())
		{
			unsigned int utf8_code = it->second;
//			eDebugNoNewLine("%c[%d]", b, b);
			if (utf8_code < 0x10)
			{
				int mode = utf8_code;
				if (b > 96 && b < 123)
					utf8_code = diacr_lower_cmap[(b-97)*15+mode-1];
				else if (b > 64 && b < 91)
					utf8_code = diacr_upper_cmap[(b-65)*15+mode-1];
			}
			if (utf8_code > 0xFFFFFF)
				out[outidx++]=(utf8_code&0xFF000000)>>24;
			if (utf8_code > 0xFFFF)
				out[outidx++]=(utf8_code&0xFF0000)>>16;
			if (utf8_code > 0xFF)
				out[outidx++]=(utf8_code&0xFF00)>>8;
			if (utf8_code)
				out[outidx++]=utf8_code&0xFF;
			m_modifications.erase(it);
			continue;
		}

		if (b < 0x10) /* spacing attribute */
		{
			if (b < 8) /* colors */
			{
				if (b != color) /* new color is split into a new string */
				{
					addSubtitleString(color, std::string((const char*)out, outidx), m_Y);
					outidx = 0;
					color = b;
				}
			}
			else if (b == 0xd)
				m_double_height = m_Y + 1;
			else if (b == 0xa)  // close box
				m_box_open=0;
			else if (b == 0xb)  // open box
				++m_box_open;
			else
				eDebug("[ignore %x]", b);
				/* ignore other attributes */
		} else if (m_box_open>1)
		{
//			eDebugNoNewLine("%c(%d)", b, b);
				/* no more than one whitespace, only printable chars */
			if (((!last_was_white) || (b != ' ')) && (b >= 0x20))
			{
				int cur_nat_subset = second_G0_set ? nat_subset_2 : nat_subset;

				unsigned char offs = NationalReplaceMap[b];
				if (offs)
				{
					unsigned int utf8_code =
						NationalOptionSubsets[cur_nat_subset*14+offs];
					if (utf8_code > 0xFFFFFF)
						out[outidx++]=(utf8_code&0xFF000000)>>24;
					if (utf8_code > 0xFFFF)
						out[outidx++]=(utf8_code&0xFF0000)>>16;
					if (utf8_code > 0xFF)
						out[outidx++]=(utf8_code&0xFF00)>>8;
					out[outidx++]=utf8_code&0xFF;
				}
				else
					out[outidx++] = b;
				last_was_white = b == ' ';
			}
			else if (b == 0x1b) // ESC ... switch between default G0 and second G0 charset
				second_G0_set ^= 1;
		}
	}
//	eDebug("");
	addSubtitleString(color, std::string((const char*)out, outidx), m_Y);
}

void eDVBTeletextParser::handlePageEnd(int have_pts, const pts_t &pts)
{
	m_subtitle_page.m_have_pts = have_pts;
	m_subtitle_page.m_pts = pts;
	m_subtitle_page.m_timeout = 90000 * 20; /* 20s */
	if (m_page_X != 0)
		m_new_subtitle_page(m_subtitle_page); /* send assembled subtitle page to display */
}

void eDVBTeletextParser::setPageAndMagazine(int page, int magazine)
{
	if (page > 0)
		eDebug("enable teletext subtitle page %x%02x", magazine, page);
	else
		eDebug("disable teletext subtitles");
	m_M29_0_valid = 0;
	m_page_M = magazine; /* magazine to look for */
	if (magazine != -1)
		m_page_M &= 7;
	m_page_X = page;  /* page number */
	if (page != -1)
		m_page_X &= 0xFF;
}

void eDVBTeletextParser::connectNewStream(const Slot0<void> &slot, ePtr<eConnection> &connection)
{
	connection = new eConnection(this, m_new_subtitle_stream.connect(slot));
}

void eDVBTeletextParser::connectNewPage(const Slot1<void, const eDVBTeletextSubtitlePage&> &slot, ePtr<eConnection> &connection)
{
	connection = new eConnection(this, m_new_subtitle_page.connect(slot));
}

void eDVBTeletextParser::addSubtitleString(int color, std::string string, int source_line)
{
//	eDebug("(%d)add subtitle string: %s, col %d", m_Y, string.c_str(), color);

	const gRGB pal[8] = { gRGB(102, 102, 102), gRGB(255, 0, 0), gRGB(0, 255, 0), gRGB(255, 255, 0),
		gRGB(102, 102, 255), gRGB(255, 0, 255), gRGB(0, 255, 255), gRGB(255, 255, 255) };

	if (string.empty()) return;

	m_subtitle_page.m_elements.push_back(eDVBTeletextSubtitlePageElement(pal[color], string, source_line));
}

