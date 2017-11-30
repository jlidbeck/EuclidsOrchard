// Substrate Watercolor
// j.tarbell   June, 2004
// Albuquerque, New Mexico
// complexification.net

// Processing 0085 Beta syntax update
// j.tarbell   April, 2005

// C++/OpenGL port
// Jonathan Lidbeck
// 12tone.software

//
//	hooks to rendering code, point(), line(), etc.,
//	are located in GLUtil.h
//	these functions are OpenGL stand-ins
//


#include "Substrate.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <GL/gl.h>		//	Header for OpenGL
#include <GL/GLUtil.h>
#include <Rgbhsl\Rgbhsl.h>

#ifdef _DEBUG
// shorten the trace spam a bit
#define TRACE ATL::CTraceFileAndLineInfo("SubstrateSim", __LINE__)
#endif

// same as max(a,min(x,b))
#define minmax(x, low, high) ((x)<(low)?(low):((x)>(high)?(high):(x)))


//
//	class Substrate
//


// MAIN METHODS ---------------------------------------------

// allocate all memory once
void Substrate::setSize(unsigned int width, unsigned int height)
{
	ASSERT(width > 0 && height > 0 && width < 0x10000000 && height < 0x10000000 
		&& m_settings.m_maxNumCracks>0 && m_settings.m_maxNumCracks < 5000);

	dimx = width;
	dimy = height;
}

//	restarts simulation with current settings and dimensions, allocating memory as necessary
void Substrate::begin()
{
	GridCell gc;
	gc.Heading = 10001;
	gc.Weight = 0;

	// erase crack grid

	resetGrid(gc);

	// remove all cracks
	m_cracks.resize(m_settings.m_maxNumCracks);

	for(unsigned int k = 0; k<m_cracks.size(); ++k)
	{
		//m_cracks[k].reset();
		m_cracks[k].m_alive = (false);
	}

	// seed the grid and a few cracks

	// 180: inward
	// 260: radial, just inward enough to visually negate the natural outward tendency of tangents
	// 360: outward.. chunky wood texture
	float headingOffset = 360;// random(180.0f, 360.0f);

	switch(rand() % 5) {
	case 1:
		// seed regular grid with a structured pattern, and no starting cracks
		for(int x = 0-dimx / 4; x < dimx * 5 / 4; x += 10) {
			for(int y = 0-dimy / 4; y < dimy * 5 / 4; y += 10) {
				int idx = modi(x, dimx) + modi(y, dimy) * dimx;
				ASSERT(idx >= 0 && idx < dimx*dimy);
				gc.Heading = (unsigned short)(0.5f + headingOffset + 180.0f / 3.1416f * atan2(y - dimy / 2, x - dimx / 2));
				gc.Weight = 0;
				m_grid[idx] = gc;
			}
		}
		// reduce spontaneous generation to compensate for many seeds
		m_settings.m_nSpontaneousGenerationRate = 55;
		break;

	case 2:
		// seed the grid with a random scattered pattern, and no starting cracks
		for(int z = (dimx / 10)*(dimy / 10); z>0; --z) {
			const float* xy = randomGaussianPair();
			int x = dimx / 2 + round(dimx / 3 * xy[0]);
			int y = dimy / 2 + round(dimy / 3 * xy[1]);
			int idx = modi(x, dimx) + modi(y, dimy) * dimx;
			ASSERT(idx >= 0 && idx < dimx*dimy);
			gc.Heading = (unsigned short)(0.5f + headingOffset + 180.0f / 3.1416f * atan2(y - dimy / 2, x - dimx / 2));
			gc.Weight = 0;
			m_grid[idx] = gc;
		}
		// reduce spontaneous generation to compensate for many seeds
		m_settings.m_nSpontaneousGenerationRate = 55;
		break;

	default:
		// make random crack seeds
		for(int k = 0; k < 16; k++) {
			int i = (rand() % dimx) + (rand() % dimy) * dimx;
			m_grid[i].Heading = (rand() % 360);
		}

		//make just three cracks
		//for(int k = 0; k < 3; k++) {
		//	addCrack(100000);
		//}

		m_settings.m_nSpontaneousGenerationRate = 99;
	}

	m_bEraseBackgroundFlag = true;
}

// simulation step
void Substrate::step() {
	if(m_bEraseBackgroundFlag)
	{
		// erase the drawing once, then draw points incrementally
		m_bEraseBackgroundFlag = false;
		m_screenData.assign(dimx * dimy, 0xFF000000 | m_settings.m_backgroundColor);
	}

	// step all cracks, keeping census of how many are alive at start of this frame
	int numLiving = 0;
	int numDead = 0;
	int nTotalAge = 0;
	for(int n = m_cracks.size() - 1; n >= 0; --n) {
		if(m_cracks[n].m_alive) {
			crackMove(m_cracks[n]);
			++numLiving;
			nTotalAge += m_cracks[n].m_age;
		}
		else {
			++numDead;
		}
	}

	m_nNumLivingCracks = numLiving;
	m_nNumDeadCracks = numDead;

	//if(m_nNumLivingCracks && randomBool(0.1f)) TRACE(":NUM:%d AGE:%d\n", m_nNumLivingCracks, nTotalAge / m_nNumLivingCracks);

	if(m_nNumLivingCracks <= 1) {
		// nothing is happening--try to create a new crack immediately
		TRACE("EXTINCTION!!\n");
		for(int i = 0; i < 3; ++i) {
			addCrack(100000);
		}
	}
	else {
		addCrack(m_settings.m_nSpontaneousGenerationRate);
	}
}

// CRACK METHODS --------------------------------------------------

// spawn a new crack off of some existing crack
// returns false if
// - at max num cracks (crack overflow)
// - timed out searching for new spawn point
bool Substrate::addCrack(int searchLimit) {
	if(!searchLimit) {
		searchLimit = m_settings.m_nSpontaneousGenerationRate;
	}

	Crack* cr = newCrack();
	if(!cr) {
		// all cracks active--couldn't allocate a new crack
		return false;
	}

	// make a new crack instance
	return findStart(*cr, searchLimit);
}

// add a new crack bifurcating from existing
void Substrate::addCrack(const Crack &cr) {

	ASSERT(cr.m_alive);
	Crack* pNewCrack = newCrack();
	if(pNewCrack)
	{
		// copy
		*pNewCrack = cr;
		pNewCrack->m_age = 0;
		pNewCrack->stepForward(0.85);
	}
}

Crack* Substrate::newCrack() {

	// find an unused Crack instance and recycle it
	for(unsigned int i = 0; i < m_cracks.size(); ++i) {
		if(!m_cracks[i].m_alive) {
			m_cracks[i].m_alive = true;
			m_cracks[i].reset(m_settings.m_palette.randomColor());
			return &m_cracks[i];
		}
	}

	return NULL;
}

bool Substrate::fullOfCrack() const {
	return (m_nNumLivingCracks >= m_settings.m_maxNumCracks);
}

// drawing

void Substrate::point(float fx, float fy) {
	int x = (int)round(fx), y = (int)round(fy);
	// wrap
	x = (x + dimx) % dimx;
	y = (y + dimy) % dimy;
	//
	unsigned int i = (x + (dimx * y));
	if(x < dimx && i < m_screenData.size()) {
		m_screenData[i] = mix(m_screenData[i], RGB(_r, _g, _b), _a);
	}
}



// OBJECTS -------------------------------------------------------

//
//	class Crack
//

char Crack::sz[100];

Crack::Crack(float x, float y, float hdg, unsigned short generation, COLORREF color)
{
	reset(color);

	m_generation = generation;
	m_startx = x;
	m_starty = y;

	m_x = x;
	m_y = y;
	m_heading = hdg;
	m_curvature = 0;

	m_alive = true;
	m_age = 0;

}


void Crack::reset(COLORREF color) {
	m_sp.reset(color);
	m_age = 0;
	m_generation = 1;
}

bool Substrate::findStart(Crack &cr, int searchLimit) const
{
	// pick random point
	int px = 0;
	int py = 0;

	// shift until crack is found
	bool found = false;
	GridCell gc;
	
	for(int timeout = 0; timeout < searchLimit; ++timeout) {
		px = (rand() % (dimx));
		if(timeout%10 == 0) 
			py = (rand() % (dimy));
		gc = getGridCell(px, py);
		if(gc.Heading < 10000) {
			found = true;
			break;
		}
	}

	if(!found) {
		//TRACE("Crack::findStart(): timeout: %d\n", timeout);
		cr.m_alive = (false);
		return false;
	}

		// start crack

		// get grain angle of substrate at start point
		float parentHeading = gc.Heading;

		if(gc.Weight > 0) {
			// angle new crack off of old one
			float ba = m_settings.m_fBranchAngle 
				+ random(-m_settings.m_fHeadingVariability, m_settings.m_fHeadingVariability);	// randomize the angle just a bit

			// angle left or right off of parent
			if(rand() & 1)
				parentHeading -= (int)(ba);
			else
				parentHeading += (int)(ba);

			parentHeading = fmod(parentHeading + 360.0f, 360.0f);
		}

		crackRandomizeCurvature(cr);

		cr.startAt(px, py, parentHeading, gc.Weight + 1, m_settings.m_palette.randomColor());

		cr.m_age = 0;

		return true;
}

void Crack::startAt(float x, float y, float hdg, unsigned short generation, COLORREF color) {
	reset(color);

	m_alive = true;
	m_generation = generation;
	m_startx = x;
	m_starty = y;

	m_x = x;
	m_y = y;
	m_heading = hdg;

	// move slightly away from old crack so we don't get confused
	stepForward(0.61f);
}

void Substrate::crackRandomizeCurvature(Crack &crack) const {
	crack.m_curvature = 0.0;

	// randomly enable curvature
	if(randomBool(m_settings.m_curvatureFreq)) {
		crack.m_curvature = random(
			-m_settings.m_maxCurvature,
			m_settings.m_maxCurvature);
	}
}

void Crack::stepForward(float stepsize)
{
	ASSERT(m_alive);
	m_x += stepsize*cos(m_heading*M_PI / 180);
	m_y += stepsize*sin(m_heading*M_PI / 180);
}

void Substrate::crackMove(Crack &crack)
{
	++crack.m_age;

	// continue cracking
	crack.stepForward(0.42f);

	// draw sand painter
	crackRegionColor(crack, m_settings.m_bFillRegions);

	//float z = 0.33;
	float z = m_settings.m_fuzz;

	// draw black crack
	setColor(m_settings.m_crackColor, 85);

	point(crack.m_x + random(-z, z), crack.m_y + random(-z, z));

	// move and add fuzz
	int cx = int(crack.m_x + random(-z, z));
	int cy = int(crack.m_y + random(-z, z));
	// wrap
	cx = (cx + dimx) % dimx;
	cy = (cy + dimy) % dimy;

	// bound check
	if(validCoordinates(cx, cy)) {
		// safe to check
		GridCell gc = getGridCell(cx, cy);

		if((gc.Heading > 10000) ||
			(abs(gc.Heading - crack.m_heading) < 5.0f))
		{
			// the substrate is clear; continue cracking
			gc.Heading = (unsigned short)(crack.m_heading + 0.5f);
			gc.Weight = crack.m_generation;

			setGridCell(cx, cy, gc);

			crack.m_heading += crack.m_curvature;
			//			m_curvature *= 1.001;		// spiral inward

			// bifurcate if:
			//	- settings and randomness match;
			//	- there's room for at least one new crack (don't want half a bifurcation)
			if(!fullOfCrack() && crack.m_age > 100 && ::randomBool(m_settings.m_fBifurcationFreq))
			{
				crack.stepForward(0.81f);

				// bifurcate!

				// left branch
				Crack cr1(crack.m_x, crack.m_y, crack.m_heading + m_settings.m_fBifurcationAngle, crack.m_generation+1, m_settings.m_palette.randomColor());
				addCrack(cr1);

				// right branch
				// this crack changes heading
				Crack cr2(crack.m_x, crack.m_y, crack.m_heading - m_settings.m_fBifurcationAngle, crack.m_generation+1, m_settings.m_palette.randomColor());
				addCrack(cr2);

				crack.m_alive = (false);
			}
		}
		else
		{
			if(gc.Weight == 0) {
				// crack seed encountered
				// continue and branch in direction of seed
				Crack cr1(crack.m_x, crack.m_y, gc.Heading, crack.m_generation + 1, m_settings.m_palette.randomColor());
				addCrack(cr1);
				//m_heading = gc.Heading + theSubstrate.m_settings.m_fBranchAngle;
				crack.stepForward(0.81f);
			}
			else {
				// crack encountered (not self), stop cracking here
				crack.m_alive = false;

				if(m_settings.m_bReincarnation)
					addCrack();				// jump somewhere else
				if(m_settings.m_bAggressiveReincarnation)
					addCrack();	// start a new crack
			}
		}
	}
	else
	{
		// out of bounds, stop cracking here
		crack.m_alive = false;

		if(m_settings.m_bReincarnation)
			addCrack();					// jump somewhere else
		addCrack();		// spawn a new crack
	}
}

void Substrate::crackRegionColor(Crack &crack, bool bStopAtCrack)
{
	// start checking one step away
	float rx = crack.m_x;
	float ry = crack.m_y;
	float dx = 0.81f*sin(crack.m_heading*M_PI / 180);
	float dy = -0.81f*cos(crack.m_heading*M_PI / 180);
	
	// stop at max dist of 1/8 of the grid height
	int maxlensq = dimy * dimy / 64;

	// find extents of open space
	bool openspace = true;
	while(openspace)
	{
		// move perpendicular to crack
		rx += dx;
		ry += dy;
		int cx = int(rx);
		int cy = int(ry);
		// wrap
		cx = (cx + dimx) % dimx;
		cy = (cy + dimy) % dimy;

		if(validCoordinates(cx, cy))
		{
			GridCell gc = getGridCell(cx, cy);

			// safe to check
			if(bStopAtCrack &&
				//theSubstrate.m_cgrid[cy*theSubstrate.dimx+cx]<=10000) 
				gc.Weight > 0)
			{
				// ran into a crack
				openspace = false;
			}
			else
			{
				// space is open

				if((rx - crack.m_x)*(rx - crack.m_x) + (ry - crack.m_y)*(ry - crack.m_y) > maxlensq) {
					openspace = false;
				}
			}
		}
		else
		{
			// ran off the grid
			openspace = false;
		}
	}
	// draw sand painter
	renderSandPainter(crack.m_sp, crack.m_x, crack.m_y, rx - crack.m_x, ry - crack.m_y);
}

//
//	class Sandpainter
//

void SandPainter::reset(COLORREF color)
{
	m_color = color;
	m_gain = random(0.01f, 0.1f);
}

// paints stroke beginning at (x,y) and going toward (x+dx, y+dy)
void Substrate::renderSandPainter(SandPainter &sp, float x, float y, float dx, float dy)
{
	// modulate gain
	sp.m_gain += random(-0.040f, 0.050f);

	const float maxg = 1.0;
	if(sp.m_gain<0) sp.m_gain = 0.050f;
	if(sp.m_gain>maxg) sp.m_gain = maxg;

	// calculate grains by distance
	//int grains = int(sqrt((dx-x)*(dx-x)+(dy-y)*(dy-y)));
	//int grains = dimx / 4;	// this is about the max.. heavy, oil paint-like
	int grains = dimx / 8;	// 
	//int grains = dimx / 16;	// medium
	//int grains = dimx / 32;	// quite thin, individual grain patterns visible

	// lay down grains of sand (transparent pixels)
	float w = sp.m_gain / (grains - 1);
	for(int i = 0; i<grains; i++) {
		float a = 0.1f * (1 - i / (float)grains);
		setColor(sp.m_color, (unsigned char)(a * 256));
		point(
			x + dx * (sin(i*w)),
			y + dy * (sin(i*w)));
	}
}


