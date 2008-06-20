/*

    This file is part of libdvbcsa.

    libdvbcsa is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    libdvbcsa is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdvbcsa; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    (c) 2006-2008 Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#include "dvbcsa/dvbcsa.h"
#include "dvbcsa_pv.h"

/*
** Get nibble from A and B 40 bits registers
*/

#define	NBGET(r, n)	(r >> (n * 4))



/*
** CSA algorithm is using X, Y, Z, D, E, F 4 bits registers
** and P, Q, C 1 bits registers. these registers are stored
** in `pqzyx' and `cfed' variables to improve speed.
*/

#define GETX(x)	(x)
#define GETY(x)	((x) >> 4)
#define GETZ(x)	((x) >> 8)
#define TSTP(x)	((x) & 0x1000)
#define TSTQ(x)	((x) & 0x2000)
#define GETD(x)	(x)
#define GETE(x)	((x) >> 4)
#define GETF(x)	((x) >> 8)
#define GETC(x)	((x) >> 12)



/*
** swap nibbles in byte
*/

DVBCSA_INLINE
static inline uint8_t swap_nbl (register uint8_t byte)
{
  return ((byte >> 4) | (byte << 4));
}


/*
** conditional rotation used in stream round
*/

DVBCSA_INLINE
static inline uint8_t csa_stream_rotate (register uint32_t pqzyx,
					 register uint32_t x)
{
  return TSTP(pqzyx) ? ((x << 1) | ((x >> 3) & 1)) & 0xf : x;
}



/*
** process bit selection, permutation and sboxes
*/

DVBCSA_INLINE
static inline uint32_t csa_stream_sboxes(register uint64_t A)
{
  /* modified sboxes tables */
  static const uint16_t sbox[7][32] = 
    {
      {
	0x0001, 0x0000, 0x0400, 0x0400, 0x0001, 0x0401, 0x0401, 0x0000, 0x0401, 0x0001, 0x0001, 0x0000, 0x0400, 0x0400, 0x0000, 0x0401,
	0x0000, 0x0401, 0x0401, 0x0000, 0x0001, 0x0001, 0x0400, 0x0400, 0x0001, 0x0001, 0x0000, 0x0401, 0x0400, 0x0400, 0x0401, 0x0000,
      },
      {
	0x0802, 0x0800, 0x0002, 0x0800, 0x0802, 0x0002, 0x0002, 0x0800, 0x0800, 0x0802, 0x0000, 0x0002, 0x0000, 0x0000, 0x0000, 0x0802,
	0x0802, 0x0800, 0x0800, 0x0002, 0x0002, 0x0802, 0x0800, 0x0802, 0x0000, 0x0000, 0x0000, 0x0802, 0x0800, 0x0002, 0x0002, 0x0000,
      },
      {
	0x0010, 0x0000, 0x0010, 0x0010, 0x0010, 0x0014, 0x0000, 0x0004, 0x0004, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0004, 0x0000,
	0x0014, 0x0000, 0x0004, 0x0010, 0x0004, 0x0014, 0x0014, 0x0004, 0x0000, 0x0004, 0x0000, 0x0004, 0x0010, 0x0000, 0x0010, 0x0010,
      },
      {
	0x0028, 0x0008, 0x0020, 0x0028, 0x0020, 0x0000, 0x0020, 0x0008, 0x0008, 0x0020, 0x0000, 0x0008, 0x0000, 0x0028, 0x0028, 0x0000,
	0x0008, 0x0000, 0x0028, 0x0008, 0x0028, 0x0020, 0x0028, 0x0000, 0x0000, 0x0028, 0x0020, 0x0000, 0x0020, 0x0008, 0x0008, 0x0020,
      },
      {
	0x0100, 0x0000, 0x0000, 0x0040, 0x0140, 0x0100, 0x0140, 0x0100, 0x0140, 0x0140, 0x0000, 0x0040, 0x0100, 0x0040, 0x0040, 0x0000,
	0x0140, 0x0100, 0x0000, 0x0100, 0x0140, 0x0000, 0x0040, 0x0040, 0x0100, 0x0140, 0x0000, 0x0040, 0x0100, 0x0000, 0x0040, 0x0140,
      },
      {
	0x0000, 0x0080, 0x0200, 0x0280, 0x0080, 0x0200, 0x0200, 0x0000, 0x0000, 0x0080, 0x0280, 0x0000, 0x0200, 0x0280, 0x0080, 0x0280,
	0x0200, 0x0280, 0x0000, 0x0200, 0x0280, 0x0000, 0x0080, 0x0080, 0x0200, 0x0080, 0x0080, 0x0200, 0x0000, 0x0280, 0x0280, 0x0000,
      },
      {
	0x0000, 0x0000, 0x1000, 0x2000, 0x3000, 0x3000, 0x0000, 0x1000, 0x3000, 0x1000, 0x2000, 0x2000, 0x2000, 0x0000, 0x1000, 0x3000,
	0x3000, 0x1000, 0x2000, 0x2000, 0x2000, 0x3000, 0x0000, 0x0000, 0x2000, 0x1000, 0x1000, 0x3000, 0x0000, 0x0000, 0x1000, 0x3000,
      },
    };

  uint32_t		res;
  uint64_t		t;

  t = A & 0x2018004200LL;
  res =  sbox[1][ ((t >> 37) ^ (t >> 27) ^ (t >> 25) ^ (t >> 11) ^ (t >>  5)) & 0x1f ];

  t = A & 0x4201480000LL;
  res |= sbox[4][ ((t >> 38) ^ (t >> 32) ^ (t >> 22) ^ (t >> 16) ^ (t >> 18)) & 0x1f ];

  t = A & 0x8040122000LL;
  res |= sbox[5][ ((t >> 39) ^ (t >> 29) ^ (t >> 18) ^ (t >> 14) ^ (t >>  9)) & 0x1f ];

  t = A & 0x1082010040LL;
  res |= sbox[0][ ((t >> 36) ^ (t >> 30) ^ (t >> 23) ^ (t >>  3) ^ (t >> 12)) & 0x1f ];

  t = A & 0x0004a00180LL;
  res |= sbox[2][ ((t >> 26) ^ (t >> 22) ^ (t >> 19) ^ (t >>  5) ^ (t >>  3)) & 0x1f ];

  t = A & 0x0100048820LL;
  res |= sbox[3][ ((t >> 32) ^ (t >> 17) ^ (t >>  9) ^ (t >>  2) ^ (t >> 11)) & 0x1f ];

  t = A & 0x0c20001400LL;
  res |= sbox[6][ ((t >> 35) ^ (t >> 33) ^ (t >> 27) ^ (t >>  9) ^ (t >>  6)) & 0x1f ];

  return res;
}



/*
** bit selection and XOR from B
*/

DVBCSA_INLINE
static inline uint32_t csa_stream_B_sel	(register uint64_t B)
{
  uint32_t	t = B >> 9;

  /*    10000000 01000010000000000001000000000000 */
  /*   00000010 00000001000001001000000000000000 */
  /*  00000100 00000000101000010000000000000000 */
  /* 01000001 00001000000000000010000000000000 */

  return
    (((t      )                         ^ (t >> 27)) & 0x8) ^
    (((t >> 18)                                    ) & 0x9) ^
    ((            (t >> 22)             ^ (t >>  7)) & 0x4) ^
    (((t >> 4 )                                    ) & 0x5) ^
    ((            (t >> 24) ^ (t >>  6) ^ (t >> 11)) & 0x2) ^
    (((t >> 29) ^                         (t >> 23)) & 0x1) ^
    (((t >> 13)                                    ) & 0xe);
}



/*
** cfed precomputed operations
*/

DVBCSA_INLINE
static inline uint32_t csa_stream_cfed (uint32_t pqzyx, uint32_t cfed)
{
  static const uint16_t csa_stream_cdef[0x400] =
    {    
      0x0000, 0x0101, 0x0202, 0x0303, 0x0404, 0x0505, 0x0606, 0x0707, 0x0808, 0x0909, 0x0a0a, 0x0b0b, 0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f,
      0x0001, 0x0100, 0x0203, 0x0302, 0x0405, 0x0504, 0x0607, 0x0706, 0x0809, 0x0908, 0x0a0b, 0x0b0a, 0x0c0d, 0x0d0c, 0x0e0f, 0x0f0e,
      0x0002, 0x0103, 0x0200, 0x0301, 0x0406, 0x0507, 0x0604, 0x0705, 0x080a, 0x090b, 0x0a08, 0x0b09, 0x0c0e, 0x0d0f, 0x0e0c, 0x0f0d,
      0x0003, 0x0102, 0x0201, 0x0300, 0x0407, 0x0506, 0x0605, 0x0704, 0x080b, 0x090a, 0x0a09, 0x0b08, 0x0c0f, 0x0d0e, 0x0e0d, 0x0f0c,
      0x0004, 0x0105, 0x0206, 0x0307, 0x0400, 0x0501, 0x0602, 0x0703, 0x080c, 0x090d, 0x0a0e, 0x0b0f, 0x0c08, 0x0d09, 0x0e0a, 0x0f0b,
      0x0005, 0x0104, 0x0207, 0x0306, 0x0401, 0x0500, 0x0603, 0x0702, 0x080d, 0x090c, 0x0a0f, 0x0b0e, 0x0c09, 0x0d08, 0x0e0b, 0x0f0a,
      0x0006, 0x0107, 0x0204, 0x0305, 0x0402, 0x0503, 0x0600, 0x0701, 0x080e, 0x090f, 0x0a0c, 0x0b0d, 0x0c0a, 0x0d0b, 0x0e08, 0x0f09,
      0x0007, 0x0106, 0x0205, 0x0304, 0x0403, 0x0502, 0x0601, 0x0700, 0x080f, 0x090e, 0x0a0d, 0x0b0c, 0x0c0b, 0x0d0a, 0x0e09, 0x0f08,
      0x0008, 0x0109, 0x020a, 0x030b, 0x040c, 0x050d, 0x060e, 0x070f, 0x0800, 0x0901, 0x0a02, 0x0b03, 0x0c04, 0x0d05, 0x0e06, 0x0f07,
      0x0009, 0x0108, 0x020b, 0x030a, 0x040d, 0x050c, 0x060f, 0x070e, 0x0801, 0x0900, 0x0a03, 0x0b02, 0x0c05, 0x0d04, 0x0e07, 0x0f06,
      0x000a, 0x010b, 0x0208, 0x0309, 0x040e, 0x050f, 0x060c, 0x070d, 0x0802, 0x0903, 0x0a00, 0x0b01, 0x0c06, 0x0d07, 0x0e04, 0x0f05,
      0x000b, 0x010a, 0x0209, 0x0308, 0x040f, 0x050e, 0x060d, 0x070c, 0x0803, 0x0902, 0x0a01, 0x0b00, 0x0c07, 0x0d06, 0x0e05, 0x0f04,
      0x000c, 0x010d, 0x020e, 0x030f, 0x0408, 0x0509, 0x060a, 0x070b, 0x0804, 0x0905, 0x0a06, 0x0b07, 0x0c00, 0x0d01, 0x0e02, 0x0f03,
      0x000d, 0x010c, 0x020f, 0x030e, 0x0409, 0x0508, 0x060b, 0x070a, 0x0805, 0x0904, 0x0a07, 0x0b06, 0x0c01, 0x0d00, 0x0e03, 0x0f02,
      0x000e, 0x010f, 0x020c, 0x030d, 0x040a, 0x050b, 0x0608, 0x0709, 0x0806, 0x0907, 0x0a04, 0x0b05, 0x0c02, 0x0d03, 0x0e00, 0x0f01,
      0x000f, 0x010e, 0x020d, 0x030c, 0x040b, 0x050a, 0x0609, 0x0708, 0x0807, 0x0906, 0x0a05, 0x0b04, 0x0c03, 0x0d02, 0x0e01, 0x0f00,
      0x1000, 0x1101, 0x1202, 0x1303, 0x1404, 0x1505, 0x1606, 0x1707, 0x1808, 0x1909, 0x1a0a, 0x1b0b, 0x1c0c, 0x1d0d, 0x1e0e, 0x1f0f,
      0x1001, 0x1100, 0x1203, 0x1302, 0x1405, 0x1504, 0x1607, 0x1706, 0x1809, 0x1908, 0x1a0b, 0x1b0a, 0x1c0d, 0x1d0c, 0x1e0f, 0x1f0e,
      0x1002, 0x1103, 0x1200, 0x1301, 0x1406, 0x1507, 0x1604, 0x1705, 0x180a, 0x190b, 0x1a08, 0x1b09, 0x1c0e, 0x1d0f, 0x1e0c, 0x1f0d,
      0x1003, 0x1102, 0x1201, 0x1300, 0x1407, 0x1506, 0x1605, 0x1704, 0x180b, 0x190a, 0x1a09, 0x1b08, 0x1c0f, 0x1d0e, 0x1e0d, 0x1f0c,
      0x1004, 0x1105, 0x1206, 0x1307, 0x1400, 0x1501, 0x1602, 0x1703, 0x180c, 0x190d, 0x1a0e, 0x1b0f, 0x1c08, 0x1d09, 0x1e0a, 0x1f0b,
      0x1005, 0x1104, 0x1207, 0x1306, 0x1401, 0x1500, 0x1603, 0x1702, 0x180d, 0x190c, 0x1a0f, 0x1b0e, 0x1c09, 0x1d08, 0x1e0b, 0x1f0a,
      0x1006, 0x1107, 0x1204, 0x1305, 0x1402, 0x1503, 0x1600, 0x1701, 0x180e, 0x190f, 0x1a0c, 0x1b0d, 0x1c0a, 0x1d0b, 0x1e08, 0x1f09,
      0x1007, 0x1106, 0x1205, 0x1304, 0x1403, 0x1502, 0x1601, 0x1700, 0x180f, 0x190e, 0x1a0d, 0x1b0c, 0x1c0b, 0x1d0a, 0x1e09, 0x1f08,
      0x1008, 0x1109, 0x120a, 0x130b, 0x140c, 0x150d, 0x160e, 0x170f, 0x1800, 0x1901, 0x1a02, 0x1b03, 0x1c04, 0x1d05, 0x1e06, 0x1f07,
      0x1009, 0x1108, 0x120b, 0x130a, 0x140d, 0x150c, 0x160f, 0x170e, 0x1801, 0x1900, 0x1a03, 0x1b02, 0x1c05, 0x1d04, 0x1e07, 0x1f06,
      0x100a, 0x110b, 0x1208, 0x1309, 0x140e, 0x150f, 0x160c, 0x170d, 0x1802, 0x1903, 0x1a00, 0x1b01, 0x1c06, 0x1d07, 0x1e04, 0x1f05,
      0x100b, 0x110a, 0x1209, 0x1308, 0x140f, 0x150e, 0x160d, 0x170c, 0x1803, 0x1902, 0x1a01, 0x1b00, 0x1c07, 0x1d06, 0x1e05, 0x1f04,
      0x100c, 0x110d, 0x120e, 0x130f, 0x1408, 0x1509, 0x160a, 0x170b, 0x1804, 0x1905, 0x1a06, 0x1b07, 0x1c00, 0x1d01, 0x1e02, 0x1f03,
      0x100d, 0x110c, 0x120f, 0x130e, 0x1409, 0x1508, 0x160b, 0x170a, 0x1805, 0x1904, 0x1a07, 0x1b06, 0x1c01, 0x1d00, 0x1e03, 0x1f02,
      0x100e, 0x110f, 0x120c, 0x130d, 0x140a, 0x150b, 0x1608, 0x1709, 0x1806, 0x1907, 0x1a04, 0x1b05, 0x1c02, 0x1d03, 0x1e00, 0x1f01,
      0x100f, 0x110e, 0x120d, 0x130c, 0x140b, 0x150a, 0x1609, 0x1708, 0x1807, 0x1906, 0x1a05, 0x1b04, 0x1c03, 0x1d02, 0x1e01, 0x1f00,
      0x0000, 0x0101, 0x0202, 0x0303, 0x0404, 0x0505, 0x0606, 0x0707, 0x0808, 0x0909, 0x0a0a, 0x0b0b, 0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f,
      0x0101, 0x0200, 0x0303, 0x0402, 0x0505, 0x0604, 0x0707, 0x0806, 0x0909, 0x0a08, 0x0b0b, 0x0c0a, 0x0d0d, 0x0e0c, 0x0f0f, 0x100e,
      0x0202, 0x0303, 0x0400, 0x0501, 0x0606, 0x0707, 0x0804, 0x0905, 0x0a0a, 0x0b0b, 0x0c08, 0x0d09, 0x0e0e, 0x0f0f, 0x100c, 0x110d,
      0x0303, 0x0402, 0x0501, 0x0600, 0x0707, 0x0806, 0x0905, 0x0a04, 0x0b0b, 0x0c0a, 0x0d09, 0x0e08, 0x0f0f, 0x100e, 0x110d, 0x120c,
      0x0404, 0x0505, 0x0606, 0x0707, 0x0800, 0x0901, 0x0a02, 0x0b03, 0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f, 0x1008, 0x1109, 0x120a, 0x130b,
      0x0505, 0x0604, 0x0707, 0x0806, 0x0901, 0x0a00, 0x0b03, 0x0c02, 0x0d0d, 0x0e0c, 0x0f0f, 0x100e, 0x1109, 0x1208, 0x130b, 0x140a,
      0x0606, 0x0707, 0x0804, 0x0905, 0x0a02, 0x0b03, 0x0c00, 0x0d01, 0x0e0e, 0x0f0f, 0x100c, 0x110d, 0x120a, 0x130b, 0x1408, 0x1509,
      0x0707, 0x0806, 0x0905, 0x0a04, 0x0b03, 0x0c02, 0x0d01, 0x0e00, 0x0f0f, 0x100e, 0x110d, 0x120c, 0x130b, 0x140a, 0x1509, 0x1608,
      0x0808, 0x0909, 0x0a0a, 0x0b0b, 0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f, 0x1000, 0x1101, 0x1202, 0x1303, 0x1404, 0x1505, 0x1606, 0x1707,
      0x0909, 0x0a08, 0x0b0b, 0x0c0a, 0x0d0d, 0x0e0c, 0x0f0f, 0x100e, 0x1101, 0x1200, 0x1303, 0x1402, 0x1505, 0x1604, 0x1707, 0x1806,
      0x0a0a, 0x0b0b, 0x0c08, 0x0d09, 0x0e0e, 0x0f0f, 0x100c, 0x110d, 0x1202, 0x1303, 0x1400, 0x1501, 0x1606, 0x1707, 0x1804, 0x1905,
      0x0b0b, 0x0c0a, 0x0d09, 0x0e08, 0x0f0f, 0x100e, 0x110d, 0x120c, 0x1303, 0x1402, 0x1501, 0x1600, 0x1707, 0x1806, 0x1905, 0x1a04,
      0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f, 0x1008, 0x1109, 0x120a, 0x130b, 0x1404, 0x1505, 0x1606, 0x1707, 0x1800, 0x1901, 0x1a02, 0x1b03,
      0x0d0d, 0x0e0c, 0x0f0f, 0x100e, 0x1109, 0x1208, 0x130b, 0x140a, 0x1505, 0x1604, 0x1707, 0x1806, 0x1901, 0x1a00, 0x1b03, 0x1c02,
      0x0e0e, 0x0f0f, 0x100c, 0x110d, 0x120a, 0x130b, 0x1408, 0x1509, 0x1606, 0x1707, 0x1804, 0x1905, 0x1a02, 0x1b03, 0x1c00, 0x1d01,
      0x0f0f, 0x100e, 0x110d, 0x120c, 0x130b, 0x140a, 0x1509, 0x1608, 0x1707, 0x1806, 0x1905, 0x1a04, 0x1b03, 0x1c02, 0x1d01, 0x1e00,
      0x0100, 0x0201, 0x0302, 0x0403, 0x0504, 0x0605, 0x0706, 0x0807, 0x0908, 0x0a09, 0x0b0a, 0x0c0b, 0x0d0c, 0x0e0d, 0x0f0e, 0x100f,
      0x0201, 0x0300, 0x0403, 0x0502, 0x0605, 0x0704, 0x0807, 0x0906, 0x0a09, 0x0b08, 0x0c0b, 0x0d0a, 0x0e0d, 0x0f0c, 0x100f, 0x110e,
      0x0302, 0x0403, 0x0500, 0x0601, 0x0706, 0x0807, 0x0904, 0x0a05, 0x0b0a, 0x0c0b, 0x0d08, 0x0e09, 0x0f0e, 0x100f, 0x110c, 0x120d,
      0x0403, 0x0502, 0x0601, 0x0700, 0x0807, 0x0906, 0x0a05, 0x0b04, 0x0c0b, 0x0d0a, 0x0e09, 0x0f08, 0x100f, 0x110e, 0x120d, 0x130c,
      0x0504, 0x0605, 0x0706, 0x0807, 0x0900, 0x0a01, 0x0b02, 0x0c03, 0x0d0c, 0x0e0d, 0x0f0e, 0x100f, 0x1108, 0x1209, 0x130a, 0x140b,
      0x0605, 0x0704, 0x0807, 0x0906, 0x0a01, 0x0b00, 0x0c03, 0x0d02, 0x0e0d, 0x0f0c, 0x100f, 0x110e, 0x1209, 0x1308, 0x140b, 0x150a,
      0x0706, 0x0807, 0x0904, 0x0a05, 0x0b02, 0x0c03, 0x0d00, 0x0e01, 0x0f0e, 0x100f, 0x110c, 0x120d, 0x130a, 0x140b, 0x1508, 0x1609,
      0x0807, 0x0906, 0x0a05, 0x0b04, 0x0c03, 0x0d02, 0x0e01, 0x0f00, 0x100f, 0x110e, 0x120d, 0x130c, 0x140b, 0x150a, 0x1609, 0x1708,
      0x0908, 0x0a09, 0x0b0a, 0x0c0b, 0x0d0c, 0x0e0d, 0x0f0e, 0x100f, 0x1100, 0x1201, 0x1302, 0x1403, 0x1504, 0x1605, 0x1706, 0x1807,
      0x0a09, 0x0b08, 0x0c0b, 0x0d0a, 0x0e0d, 0x0f0c, 0x100f, 0x110e, 0x1201, 0x1300, 0x1403, 0x1502, 0x1605, 0x1704, 0x1807, 0x1906,
      0x0b0a, 0x0c0b, 0x0d08, 0x0e09, 0x0f0e, 0x100f, 0x110c, 0x120d, 0x1302, 0x1403, 0x1500, 0x1601, 0x1706, 0x1807, 0x1904, 0x1a05,
      0x0c0b, 0x0d0a, 0x0e09, 0x0f08, 0x100f, 0x110e, 0x120d, 0x130c, 0x1403, 0x1502, 0x1601, 0x1700, 0x1807, 0x1906, 0x1a05, 0x1b04,
      0x0d0c, 0x0e0d, 0x0f0e, 0x100f, 0x1108, 0x1209, 0x130a, 0x140b, 0x1504, 0x1605, 0x1706, 0x1807, 0x1900, 0x1a01, 0x1b02, 0x1c03,
      0x0e0d, 0x0f0c, 0x100f, 0x110e, 0x1209, 0x1308, 0x140b, 0x150a, 0x1605, 0x1704, 0x1807, 0x1906, 0x1a01, 0x1b00, 0x1c03, 0x1d02,
      0x0f0e, 0x100f, 0x110c, 0x120d, 0x130a, 0x140b, 0x1508, 0x1609, 0x1706, 0x1807, 0x1904, 0x1a05, 0x1b02, 0x1c03, 0x1d00, 0x1e01,
      0x100f, 0x110e, 0x120d, 0x130c, 0x140b, 0x150a, 0x1609, 0x1708, 0x1807, 0x1906, 0x1a05, 0x1b04, 0x1c03, 0x1d02, 0x1e01, 0x1f00,
    };

  return ((cfed & 0x0f00) >> 4) | csa_stream_cdef[((cfed & 0x10ff) | (pqzyx & 0x2f00)) >> 4];
}


/*
** stream cipher initialization rounds
*/

static inline void csa_stream_init_round(uint32_t iv,
					 uint64_t *A, uint64_t *B,
					 uint32_t *pqzyx, uint32_t *cfed)
{
  uint32_t tmp;

  *A <<= 4;
  *A |= (NBGET(*A, 10) ^ GETX(*pqzyx) ^ GETD(*cfed) ^ (iv >> 4)) & 0x0f;

  tmp = (NBGET(*B, 6) ^ NBGET(*B, 9) ^ GETY(*pqzyx) ^ (iv)) & 0x0f;
  tmp = csa_stream_rotate(*pqzyx, tmp);

  *B <<= 4;
  *B |= tmp;

  *cfed = csa_stream_cfed(*pqzyx, *cfed) ^ csa_stream_B_sel(*B);

  *pqzyx = csa_stream_sboxes(*A);
}




/*
** stream cipher stream generation rounds
*/

DVBCSA_INLINE
static inline void csa_stream_round(uint64_t *A, uint64_t *B,
				    uint32_t *pqzyx, uint32_t *cfed)
{
  uint32_t tmp;

  *A <<= 4;
  *A |= (NBGET(*A, 10) ^ GETX(*pqzyx)) & 0xf;

  tmp = (NBGET(*B, 6) ^	NBGET(*B, 9) ^ GETY(*pqzyx)) & 0xf;

  *B <<= 4;
  *B |= csa_stream_rotate(*pqzyx, tmp);

  *cfed = csa_stream_cfed(*pqzyx, *cfed) ^ csa_stream_B_sel(*B);

  *pqzyx = csa_stream_sboxes(*A);
}

/*
** xor data buffer with generated stream
*/

void dvbcsa_stream_xor (const dvbcsa_cw_t cw, const dvbcsa_block_t iv,
			uint8_t *data, unsigned int len)
{
  unsigned int		i;

  uint64_t		A, B;
  uint32_t		pqzyx = 0;
  uint32_t		cfed = 0;

  A = dvbcsa_load_le32(cw);
  B = dvbcsa_load_le32(cw + 4);

  for(i = 0; i < 8; i++)
    {
      csa_stream_init_round(iv[i]		, &A, &B, &pqzyx, &cfed);
      csa_stream_init_round(swap_nbl(iv[i])	, &A, &B, &pqzyx, &cfed);
      csa_stream_init_round(iv[i]		, &A, &B, &pqzyx, &cfed);
      csa_stream_init_round(swap_nbl(iv[i])	, &A, &B, &pqzyx, &cfed);
    }

  for(i = 0; i < len; i++)	/* 4 round = 1 stream byte */
    {
      static const uint8_t csa_stream_out[16] =
	{
	  0x00, 0x55, 0x55, 0x00, 0xaa, 0xff, 0xff, 0xaa,
	  0xaa, 0xff, 0xff, 0xaa, 0x00, 0x55, 0x55, 0x00,
	};

      csa_stream_round(&A, &B, &pqzyx, &cfed);
      data[i] ^= csa_stream_out[GETD(cfed) & 0xf] & 0xc0;

      csa_stream_round(&A, &B, &pqzyx, &cfed);
      data[i] ^= csa_stream_out[GETD(cfed) & 0xf] & 0x30;

      csa_stream_round(&A, &B, &pqzyx, &cfed);
      data[i] ^= csa_stream_out[GETD(cfed) & 0xf] & 0x0c;

      csa_stream_round(&A, &B, &pqzyx, &cfed);
      data[i] ^= csa_stream_out[GETD(cfed) & 0xf] & 0x03;
    }
}

