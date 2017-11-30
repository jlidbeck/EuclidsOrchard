#pragma once

/*
* Copyright (C) 2017 Jonathan Lidbeck
*
* PALETTE is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* PALETTE is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <vector>

template<class _COLORTYPE>
struct weighted_palette_entry 
{
	_COLORTYPE color = 0;
	float fraction = 0;
	const char *name = NULL;
	float cumulativeDistribution = 0;
};

template<class _COLORTYPE>
class weighted_palette 
{
protected:
	std::vector<weighted_palette_entry<_COLORTYPE> > m_pal;

public:

	weighted_palette() {}
	weighted_palette(const weighted_palette_entry<_COLORTYPE> *pal, int numEntries) 
	{
		this->create(pal, numEntries);
	}

	void create(const weighted_palette_entry<_COLORTYPE> *pal, int numEntries) 
	{
		m_pal.resize(numEntries);

		float cumulativeProbability = 0;
		for(int i = 0; i < numEntries; ++i) {
			m_pal[i] = pal[i];
		}
		// normalize
		this->normalize();
	}

	inline void create(std::vector<_COLORTYPE> buf) 
	{
		create(buf.data(), buf.size());
	}

	void create(const _COLORTYPE *buf, int numEntries) 
	{
		m_pal.resize(numEntries);
		for(int i = 0; i < numEntries; ++i) {
			m_pal[i].color = buf[i];
			m_pal[i].fraction = 1.0f;
			m_pal[i].name = "";
		}
		normalize();
	}

	// calculates cumulativeProbability for all colors
	// scales cumulativeProbability so they sum to 1.0
	void normalize() 
	{
		const float equalizeBoost = 0.1f;
		float cumulativeProbability = 0;
		for(unsigned int i = 0; i < m_pal.size(); ++i) {
			cumulativeProbability += (m_pal[i].fraction += equalizeBoost);
			m_pal[i].cumulativeDistribution = cumulativeProbability;
		}

		if(cumulativeProbability > 0) {
			for(unsigned int i = 0; i < m_pal.size(); ++i) {
				m_pal[i].cumulativeDistribution /= cumulativeProbability;
			}
		}
	}

	void removeColor(int index) 
	{
		m_pal.erase(m_pal.begin() + index);
		this->normalize();
	}

	inline _COLORTYPE randomColor() const {
		return colorAt(randomFloat());
	}

	inline _COLORTYPE colorAtIndex(int index)  const {
		return m_pal[index].color;
	}

	inline _COLORTYPE colorAt(float x) const {
		//x = 1.0f - (x*x*x);
		for(unsigned int i = 0; i < m_pal.size(); ++i) {
			if(m_pal[i].cumulativeDistribution > x) {
				return m_pal[i].color;
			}
		}
		return (_COLORTYPE)(-1);
	}

	inline _COLORTYPE& operator[](int index) 
	{
		return m_pal[index];
	}
};

