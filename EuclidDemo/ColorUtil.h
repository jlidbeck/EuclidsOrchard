#pragma once

/*
* Original Copyright (C) 1999-2010  Terence M. Welsh
* Additional functions and templates by Jonathan Lidbeck
*
* Rgbhsl is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* Rgbhsl is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef ASSERT
#define ASSERT(x) {}
#endif

// This library converts between colors defined with RGB values and HSL
// values.  It also finds in-between colors by moving linearly through
// HSL space.
// All functions take values for r, g, b, h, s, and l between 0.0 and 1.0
// (RGB = red, green, blue;  HSL = hue, saturation, luminosity)
//
// F should be a floating-point type.

//
// convert RGB to HSL
//

template<class F>
void rgb2hsl(F r, F g, F b, F &h, F &s, F &l)
{
	int huezone = 0;
	F rr, gg, bb;

	// find huezone
	if(r >= g) {
		huezone = 0;
		if(b > r) {
			huezone = 4;
		}
		else {
			if(b > g) {
				huezone = 5;
			}
		}
	}
	else {
		huezone = 1;
		if(b > g) {
			huezone = 2;
		}
		else {
			if(b > r) {
				huezone = 3;
			}
		}
	}

	// luminosity
	switch(huezone) {
	case 5:		// magenta .. red
	case 0:		// red .. yellow
		l = r;
		rr = 1.0;
		gg = g / l;
		bb = b / l;
		break;

	case 1:		// yellow .. green
	case 2:		// green .. cyan
		l = g;
		gg = 1.0;
		rr = r / l;
		bb = b / l;
		break;

	default:	// cyan .. blue .. magenta
		l = b;
		bb = 1.0;
		rr = r / l;
		gg = g / l;
	}

	if(l == 0.0) {
		h = 0.0;
		s = 1.0;
		return;
	}

	// saturation
	switch(huezone) {
	case 0:		// red .. yellow
	case 1:		// yellow .. green
		s = (F)1 - b;
		bb = 0.0;
		rr = (F)1 - (((F)1 - rr) / s);
		gg = (F)1 - (((F)1 - gg) / s);
		break;

	case 2:		// green .. cyan
	case 3:		// cyan .. blue
		s = (F)1 - r;
		rr = 0;
		gg = (F)1 - (((F)1 - gg) / s);
		bb = (F)1 - (((F)1 - bb) / s);
		break;

	default:	// blue .. magenta .. red
		s = (F)1 - g;
		gg = 0;
		rr = (F)1 - (((F)1 - rr) / s);
		bb = (F)1 - (((F)1 - bb) / s);
	}

	// hue
	switch(huezone) {
	case 0:		// red .. yellow
		h = g / 6;
		break;

	case 1:		// yellow .. green
		h = (((F)1 - r) / 6) + (1 / (F)(6));
		break;

	case 2:		// green .. cyan
		h = (b / 6) + (1 / (F)(3));
		break;

	case 3:		// cyan .. blue
		h = (F)(((F)1 - g) / 6) + (F)(0.5);
		break;

	case 4:		// blue .. magenta
		h = (r / 6) + ((F)(2) / (F)(3));
		break;

	default:	// magenta .. red
		h = (((F)1 - b) / 6) + ((F)(5) / (F)(6));
	}
}

template <class F>
inline void rgb2hsl(const F* rgb, F *hsl)
{
	rgb2hsl(rgb[0], rgb[1], rgb[2], hsl[0], hsl[1], hsl[2]);
}


//
// HSL to RGB
//

//	Convert HSL color triple to RGB.
//	h: hue: cyclical. 0 to 1 represents one entire hue cycle.
//	s: saturation: 0 <= s <= 1. if sat==0, color ranges from black to white.
//	l: luminance: 0 <= l <= 1
template <class F>
void hsl2rgb(F h, F s, F l, F &r, F &g, F &b)
{
	// hue influence
	// hue = hue mod 1
	h -= (F)floor(h);

	if(h < 0.5) {
		if(h < (1 / (F)6)) {
			// 0 <= h < 1/6: full red, some green
			r = 1;
			g = h * 6;
			b = 0.0;
		}
		else {
			// 1/6 <= h < 1/2: full green
			g = (F)1;
			if(h < 1 / (F)3) {
				// 1/6 <= h < 1/3: some red
				r = (F)1 - ((h - (1 / (F)(6))) * 6);
				b = 0.0;
			}
			else {
				// 1/3 <= h < 1/2: some blue
				b = (F)(h - (1 / (F)(3))) * 6;
				r = 0.0;
			}
		}
	}
	else if(h < 0.833333) {
		// 1/2 <= h < 5/6: full blue
		b = (F)1;
		if(h < ((F)(2) / (F)(3))) {
			// 1/2 <= h < 2/3: some green
			g = (F)1 - ((h - (F)(0.5)) * 6);
			r = 0.0;
		}
		else {
			// 2/3 <= h < 5/6: some red
			r = (h - ((F)(2) / (F)(3))) * 6;
			g = 0.0;
		}
	}
	else {
		// 5/6 <= h < 1: full red, some blue
		r = (F)1;
		b = (F)1 - ((h - ((F)(5) / (F)(6))) * 6);
		g = 0.0;
	}

	// saturation influence
	r = (F)1 - (s * ((F)1 - r));
	g = (F)1 - (s * ((F)1 - g));
	b = (F)1 - (s * ((F)1 - b));

	// luminosity influence
	r *= l;
	g *= l;
	b *= l;
}

template <class F>
inline void hsl2rgb(const F* hsl, F *rgb)
{
	hsl2rgb(hsl[0], hsl[1], hsl[2], rgb[0], rgb[1], rgb[2]);
}

template <class F>
inline void hsl2rgb(F h, F s, F l, F *rgb)
{
	hsl2rgb(h, s, l, rgb[0], rgb[1], rgb[2]);
}

// For these 'tween functions, a tween value of 0.0 will output the first
// color while a tween value of 1.0 will output the second color.
// A value of 0 for direction indicates a positive progression around
// the color wheel (i.e. red -> yellow -> green -> cyan...).  A value of
// 1 does the opposite.

template <class F>
inline void hslTween(
	const F* hsl1, const F* hsl2,
	F tween, int direction,
	F *hslOut)
{
	hslTween(hsl1[0], hsl1[1], hsl1[2], hsl2[0], hsl2[1], hsl2[2], tween, direction, hslOut[0], hslOut[1], hslOut[2]);
}

template<class F>
void hslTween(F h1, F s1, F l1,
	F h2, F s2, F l2, F tween, int direction,
	F &outh, F &outs, F &outl)
{
	// hue
	if(!direction) {  // forward around color wheel
		if(h2 >= h1)
			outh = h1 + (tween * (h2 - h1));
		else {
			outh = h1 + (tween * (1.0f - (h1 - h2)));
			if(outh > 1.0)
				outh -= 1.0;
		}
	}
	else {  // backward around color wheel
		if(h1 >= h2)
			outh = h1 - (tween * (h1 - h2));
		else {
			outh = h1 - (tween * (1.0f - (h2 - h1)));
			if(outh < 0.0)
				outh += 1.0;
		}
	}

	// saturation
	outs = s1 + (tween * (s2 - s1));

	// luminosity
	outl = l1 + (tween * (l2 - l1));
}

template<class F>
void rgbTween(F r1, F g1, F b1,
	F r2, F g2, F b2, F tween, int direction,
	F &outr, F &outg, F &outb) 
{
	F h1, s1, l1, h2, s2, l2, outh, outs, outl;

	rgb2hsl(r1, g1, b1, h1, s1, l1);
	rgb2hsl(r2, g2, b2, h2, s2, l2);
	hslTween(h1, s1, l1, h2, s2, l2, tween, direction, outh, outs, outl);
	hsl2rgb(outh, outs, outl, outr, outg, outb);
}

template<class F>
void rgbTween(const F* rgb1, const F* rgb2, F tween, int direction, F *rgbOut) 
{
	F hsl1[3], hsl2[3], hslOut[3];
	rgb2hsl(rgb1, hsl1);
	rgb2hsl(rgb2, hsl2);
	hslTween(hsl1, hsl2, tween, direction, hslOut);
	hsl2rgb(hslOut, rgbOut);
}


// COLORREF utils

#ifndef _WINDEF_

// As defined in windef.h
// rgb: 24-bit rgb color, stored as little-endian ABGR, 0x??BBGGRR,
// the format used by Windows GDI's COLORREF
typedef unsigned long COLORREF;

typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

#endif


template <class F>
inline void colorref2hsl(COLORREF colorref, F *hsl)
{
	F rgb[3] = {
		(colorref & 255) / 255.0f,
		((colorref >> 8) & 255) / 255.0f,
		((colorref >> 16) & 255) / 255.0f };
	rgb2hsl(rgb, hsl);
}

// converts BGR 24-bit color to RGB float (0-1).
// rgb: 24-bit rgb color, stored as little-endian ABGR, 0x??BBGGRR,
// the format used by Windows GDI's COLORREF
template <class F>
inline void colorref2rgb(COLORREF colorref, F *rgb)
{
	rgb[0] = (colorref & 255) / 255.0f;
	rgb[1] = ((colorref >> 8) & 255) / 255.0f;
	rgb[2] = ((colorref >> 16) & 255) / 255.0f;
}

// converts RGB floats (0-1) to BGR 24-bit color, 0x00BBGGRR.
template <class F>
inline COLORREF rgb2colorref(const F *rgb)
{
	return RGB(255.0f * rgb[0], 255.0f * rgb[1], 255.0f * rgb[2]);
}

// COLORREF: 24-bit rgb color, stored as little-endian ABGR, 0x??BBGGRR,
// the format used by Windows GDI's COLORREF
// The highest 8 bits are ignored, but set to 0xFF in the return value.
inline COLORREF mix(COLORREF c0, COLORREF c1, unsigned char a)
{
	const unsigned char *pb0 = (const unsigned char*)(&c0);
	const unsigned char *pb1 = (const unsigned char*)(&c1);
	return RGB(
		(a * pb1[0] + (255 - a) * pb0[0]) / 255,
		(a * pb1[1] + (255 - a) * pb0[1]) / 255,
		(a * pb1[2] + (255 - a) * pb0[2]) / 255)
		| 0xff000000;
}


//
//	HSV to RGB
//

template <class F>
bool HSVtoRGB(F fHue, F fSat, F fVal, F *rgb)
{
	if(!_finite(fVal) || fVal<0.0 || fVal>1.0)
		return false;

	if(!_finite(fHue))
		return false;
	//fHue = fmod(fHue, 360.0);
	//if(fHue < 0.0)
	//	fHue += 360.0;

	// hue = hue mod 1
	h -= (F)floor(h);

	//fHue *= 0.0166666666666666666666667;
	fHue *= (F)(6);

	//	ASSERT(fHue>=0.0 && fHue<6.0);
	
	int i = (int)fHue;
	//	ASSERT(i>=0 && i<6);
	F f = fHue - i;//(int)(fHue);
	ASSERT(f >= 0.0 && f<1.0);

	F p1 = fVal * (1 - fSat);       // V-VS
	F p2 = fVal * (1 - fSat * f);   // V-VSf
	F p3 = fVal + p1 - p2;       // V-VS+VSf

	F R, G, B;
	switch(i)
	{
	case 0:    R = fVal; G = p3;   B = p1;   break;
	case 1:    R = p2;   G = fVal; B = p1;   break;
	case 2:    R = p1;   G = fVal; B = p3;   break;
	case 3:    R = p1;   G = p2;   B = fVal; break;
	case 4:    R = p3;   G = p1;   B = fVal; break;
	default:   R = fVal; G = p1;   B = p2;
	}

	ASSERT(R >= 0.0 && R <= 1.0);
	ASSERT(G >= 0.0 && G <= 1.0);
	ASSERT(B >= 0.0 && B <= 1.0);
	rgb[0] = R;
	rgb[1] = G;
	rgb[2] = B;
}

template <class F>
COLORREF HSVtoRGB(F fHue, F fSat, F fVal)
{
	F rgb[3];
	HSVtoRGB(fHue, fSat, fVal, rgb);
	return RGB(
		(unsigned char)(rgb[0] * 255),
		(unsigned char)(rgb[1] * 255),
		(unsigned char)(rgb[2] * 255));
}

