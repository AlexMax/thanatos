//
// Copyright(C) 2017 Alex Mayfield
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
//     Console font.
//
//     This font is a patch_t conversion of the 8x8 font used by the
//     Hercules Graphics Card, which happened to be a pixel-accurate
//     recreation of the 8x8 IBM PC BIOS font.
//
//     This font was taken from fntcol16.zip by Joseph Gil.  This
//     compilation used to be available from:
//
//     ftp://ftp.simtel.net/pub/simtelnet/msdos/screen/fntcol16.zip
//
//     The font data is public domain and cannot be copyrighted.

#include <array>

#include "c_font.h"

namespace theta
{

namespace console
{

// 32 ( )
#include "resource/graphic/CONFN032.lmp.h"

// 33 (!)
#include "resource/graphic/CONFN033.lmp.h"

// 34 (")
#include "resource/graphic/CONFN034.lmp.h"

// 35 (#)
#include "resource/graphic/CONFN035.lmp.h"

// 36 ($)
#include "resource/graphic/CONFN036.lmp.h"

// 37 (%)
#include "resource/graphic/CONFN037.lmp.h"

// 38 (&)
#include "resource/graphic/CONFN038.lmp.h"

// 39 (')
#include "resource/graphic/CONFN039.lmp.h"

// 40 (()
#include "resource/graphic/CONFN040.lmp.h"

// 41 ())
#include "resource/graphic/CONFN041.lmp.h"

// 42 (*)
#include "resource/graphic/CONFN042.lmp.h"

// 43 (+)
#include "resource/graphic/CONFN043.lmp.h"

// 44 (,)
#include "resource/graphic/CONFN044.lmp.h"

// 45 (-)
#include "resource/graphic/CONFN045.lmp.h"

// 46 (.)
#include "resource/graphic/CONFN046.lmp.h"

// 47 (/)
#include "resource/graphic/CONFN047.lmp.h"

// 48 (0)
#include "resource/graphic/CONFN048.lmp.h"

// 49 (1)
#include "resource/graphic/CONFN049.lmp.h"

// 50 (2)
#include "resource/graphic/CONFN050.lmp.h"

// 51 (3)
#include "resource/graphic/CONFN051.lmp.h"

// 52 (4)
#include "resource/graphic/CONFN052.lmp.h"

// 53 (5)
#include "resource/graphic/CONFN053.lmp.h"

// 54 (6)
#include "resource/graphic/CONFN054.lmp.h"

// 55 (7)
#include "resource/graphic/CONFN055.lmp.h"

// 56 (8)
#include "resource/graphic/CONFN056.lmp.h"

// 57 (9)
#include "resource/graphic/CONFN057.lmp.h"

// 58 (:)
#include "resource/graphic/CONFN058.lmp.h"

// 59 (;)
#include "resource/graphic/CONFN059.lmp.h"

// 60 (<)
#include "resource/graphic/CONFN060.lmp.h"

// 61 (=)
#include "resource/graphic/CONFN061.lmp.h"

// 62 (>)
#include "resource/graphic/CONFN062.lmp.h"

// 63 (?)
#include "resource/graphic/CONFN063.lmp.h"

// 64 (@)
#include "resource/graphic/CONFN064.lmp.h"

// 65 (A)
#include "resource/graphic/CONFN065.lmp.h"

// 66 (B)
#include "resource/graphic/CONFN066.lmp.h"

// 67 (C)
#include "resource/graphic/CONFN067.lmp.h"

// 68 (D)
#include "resource/graphic/CONFN068.lmp.h"

// 69 (E)
#include "resource/graphic/CONFN069.lmp.h"

// 70 (F)
#include "resource/graphic/CONFN070.lmp.h"

// 71 (G)
#include "resource/graphic/CONFN071.lmp.h"

// 72 (H)
#include "resource/graphic/CONFN072.lmp.h"

// 73 (I)
#include "resource/graphic/CONFN073.lmp.h"

// 74 (J)
#include "resource/graphic/CONFN074.lmp.h"

// 75 (K)
#include "resource/graphic/CONFN075.lmp.h"

// 76 (L)
#include "resource/graphic/CONFN076.lmp.h"

// 77 (M)
#include "resource/graphic/CONFN077.lmp.h"

// 78 (N)
#include "resource/graphic/CONFN078.lmp.h"

// 79 (O)
#include "resource/graphic/CONFN079.lmp.h"

// 80 (P)
#include "resource/graphic/CONFN080.lmp.h"

// 81 (Q)
#include "resource/graphic/CONFN081.lmp.h"

// 82 (R)
#include "resource/graphic/CONFN082.lmp.h"

// 83 (S)
#include "resource/graphic/CONFN083.lmp.h"

// 84 (T)
#include "resource/graphic/CONFN084.lmp.h"

// 85 (U)
#include "resource/graphic/CONFN085.lmp.h"

// 86 (V)
#include "resource/graphic/CONFN086.lmp.h"

// 87 (W)
#include "resource/graphic/CONFN087.lmp.h"

// 88 (X)
#include "resource/graphic/CONFN088.lmp.h"

// 89 (Y)
#include "resource/graphic/CONFN089.lmp.h"

// 90 (Z)
#include "resource/graphic/CONFN090.lmp.h"

// 91 ([)
#include "resource/graphic/CONFN091.lmp.h"

// 92 (\)
#include "resource/graphic/CONFN092.lmp.h"

// 93 (])
#include "resource/graphic/CONFN093.lmp.h"

// 94 (^)
#include "resource/graphic/CONFN094.lmp.h"

// 95 (_)
#include "resource/graphic/CONFN095.lmp.h"

// 96 (`)
#include "resource/graphic/CONFN096.lmp.h"

// 97 (a)
#include "resource/graphic/CONFN097.lmp.h"

// 98 (b)
#include "resource/graphic/CONFN098.lmp.h"

// 99 (c)
#include "resource/graphic/CONFN099.lmp.h"

// 100 (d)
#include "resource/graphic/CONFN100.lmp.h"

// 101 (e)
#include "resource/graphic/CONFN101.lmp.h"

// 102 (f)
#include "resource/graphic/CONFN102.lmp.h"

// 103 (g)
#include "resource/graphic/CONFN103.lmp.h"

// 104 (h)
#include "resource/graphic/CONFN104.lmp.h"

// 105 (i)
#include "resource/graphic/CONFN105.lmp.h"

// 106 (j)
#include "resource/graphic/CONFN106.lmp.h"

// 107 (k)
#include "resource/graphic/CONFN107.lmp.h"

// 108 (l)
#include "resource/graphic/CONFN108.lmp.h"

// 109 (m)
#include "resource/graphic/CONFN109.lmp.h"

// 110 (n)
#include "resource/graphic/CONFN110.lmp.h"

// 111 (o)
#include "resource/graphic/CONFN111.lmp.h"

// 112 (p)
#include "resource/graphic/CONFN112.lmp.h"

// 113 (q)
#include "resource/graphic/CONFN113.lmp.h"

// 114 (r)
#include "resource/graphic/CONFN114.lmp.h"

// 115 (s)
#include "resource/graphic/CONFN115.lmp.h"

// 116 (t)
#include "resource/graphic/CONFN116.lmp.h"

// 117 (u)
#include "resource/graphic/CONFN117.lmp.h"

// 118 (v)
#include "resource/graphic/CONFN118.lmp.h"

// 119 (w)
#include "resource/graphic/CONFN119.lmp.h"

// 120 (x)
#include "resource/graphic/CONFN120.lmp.h"

// 121 (y)
#include "resource/graphic/CONFN121.lmp.h"

// 122 (z)
#include "resource/graphic/CONFN122.lmp.h"

// 123 ({)
#include "resource/graphic/CONFN123.lmp.h"

// 124 (|)
#include "resource/graphic/CONFN124.lmp.h"

// 125 (})
#include "resource/graphic/CONFN125.lmp.h"

// 126 (~)
#include "resource/graphic/CONFN126.lmp.h"

// A fixed-width console font.
Font ConsoleFont{
    { 32, gsl::make_span(CONFN032) },
    { 33, gsl::make_span(CONFN033) },
    { 34, gsl::make_span(CONFN034) },
    { 35, gsl::make_span(CONFN035) },
    { 36, gsl::make_span(CONFN036) },
    { 37, gsl::make_span(CONFN037) },
    { 38, gsl::make_span(CONFN038) },
    { 39, gsl::make_span(CONFN039) },
    { 40, gsl::make_span(CONFN040) },
    { 41, gsl::make_span(CONFN041) },
    { 42, gsl::make_span(CONFN042) },
    { 43, gsl::make_span(CONFN043) },
    { 44, gsl::make_span(CONFN044) },
    { 45, gsl::make_span(CONFN045) },
    { 46, gsl::make_span(CONFN046) },
    { 47, gsl::make_span(CONFN047) },
    { 48, gsl::make_span(CONFN048) },
    { 49, gsl::make_span(CONFN049) },
    { 50, gsl::make_span(CONFN050) },
    { 51, gsl::make_span(CONFN051) },
    { 52, gsl::make_span(CONFN052) },
    { 53, gsl::make_span(CONFN053) },
    { 54, gsl::make_span(CONFN054) },
    { 55, gsl::make_span(CONFN055) },
    { 56, gsl::make_span(CONFN056) },
    { 57, gsl::make_span(CONFN057) },
    { 58, gsl::make_span(CONFN058) },
    { 59, gsl::make_span(CONFN059) },
    { 60, gsl::make_span(CONFN060) },
    { 61, gsl::make_span(CONFN061) },
    { 62, gsl::make_span(CONFN062) },
    { 63, gsl::make_span(CONFN063) },
    { 64, gsl::make_span(CONFN064) },
    { 65, gsl::make_span(CONFN065) },
    { 66, gsl::make_span(CONFN066) },
    { 67, gsl::make_span(CONFN067) },
    { 68, gsl::make_span(CONFN068) },
    { 69, gsl::make_span(CONFN069) },
    { 70, gsl::make_span(CONFN070) },
    { 71, gsl::make_span(CONFN071) },
    { 72, gsl::make_span(CONFN072) },
    { 73, gsl::make_span(CONFN073) },
    { 74, gsl::make_span(CONFN074) },
    { 75, gsl::make_span(CONFN075) },
    { 76, gsl::make_span(CONFN076) },
    { 77, gsl::make_span(CONFN077) },
    { 78, gsl::make_span(CONFN078) },
    { 79, gsl::make_span(CONFN079) },
    { 80, gsl::make_span(CONFN080) },
    { 81, gsl::make_span(CONFN081) },
    { 82, gsl::make_span(CONFN082) },
    { 83, gsl::make_span(CONFN083) },
    { 84, gsl::make_span(CONFN084) },
    { 85, gsl::make_span(CONFN085) },
    { 86, gsl::make_span(CONFN086) },
    { 87, gsl::make_span(CONFN087) },
    { 88, gsl::make_span(CONFN088) },
    { 89, gsl::make_span(CONFN089) },
    { 90, gsl::make_span(CONFN090) },
    { 91, gsl::make_span(CONFN091) },
    { 92, gsl::make_span(CONFN092) },
    { 93, gsl::make_span(CONFN093) },
    { 94, gsl::make_span(CONFN094) },
    { 95, gsl::make_span(CONFN095) },
    { 96, gsl::make_span(CONFN096) },
    { 97, gsl::make_span(CONFN097) },
    { 98, gsl::make_span(CONFN098) },
    { 99, gsl::make_span(CONFN099) },
    { 100, gsl::make_span(CONFN100) },
    { 101, gsl::make_span(CONFN101) },
    { 102, gsl::make_span(CONFN102) },
    { 103, gsl::make_span(CONFN103) },
    { 104, gsl::make_span(CONFN104) },
    { 105, gsl::make_span(CONFN105) },
    { 106, gsl::make_span(CONFN106) },
    { 107, gsl::make_span(CONFN107) },
    { 108, gsl::make_span(CONFN108) },
    { 109, gsl::make_span(CONFN109) },
    { 110, gsl::make_span(CONFN110) },
    { 111, gsl::make_span(CONFN111) },
    { 112, gsl::make_span(CONFN112) },
    { 113, gsl::make_span(CONFN113) },
    { 114, gsl::make_span(CONFN114) },
    { 115, gsl::make_span(CONFN115) },
    { 116, gsl::make_span(CONFN116) },
    { 117, gsl::make_span(CONFN117) },
    { 118, gsl::make_span(CONFN118) },
    { 119, gsl::make_span(CONFN119) },
    { 120, gsl::make_span(CONFN120) },
    { 121, gsl::make_span(CONFN121) },
    { 122, gsl::make_span(CONFN122) },
    { 123, gsl::make_span(CONFN123) },
    { 124, gsl::make_span(CONFN124) },
    { 125, gsl::make_span(CONFN125) },
    { 126, gsl::make_span(CONFN126) }
};

}

}
