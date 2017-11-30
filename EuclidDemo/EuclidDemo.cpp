/*
* Copyright (C) 2017 Jonathan Lidbeck
*
* Euclid's Orchard
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published
* by the Free Software Foundation; either version 2 of the License,
* or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#include <ColorUtil.h>
//#include <rsMath/rsMath.h>
#include <rsMath/rsMatrix.h>
#include <resource.h>

#include "util.h"
#include "EuclidDemo.h"

//#include <png.h>
//#include <libavcodec/avcodec.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/opt.h>
//#include <libswscale/swscale.h>

#ifdef _DEBUG
// shorten the trace spam a bit
#define TRACE ATL::CTraceFileAndLineInfo("EuclidSS", __LINE__)
#endif

using std::vector;

// Globals
#ifdef WIN32
LPCTSTR registryPath = LPCTSTR("Software\\12 Tone\\EuclidsOrchard");
#endif

#define GL_DEPTH_CLAMP 0x864F


EuclidDemo g_ss;

stringstream str;
static bool g_trace = false;

std::map<EuclidDemo::growth_enum, string> EuclidDemo::s_growth_enum_names = {
	{ SHAPE_NONE, "SHAPE_NONE" },
	{ SHAPE_TRIANGLE, "SHAPE_TRIANGLE" },
	{ SHAPE_CIRCLE, "SHAPE_CIRCLE" },
	{ SHAPE_FILL, "SHAPE_FILL" },
	{ SHAPE_POINT_SEARCH, "SHAPE_POINT_SEARCH" },
	{ SHAPE_GROW, "SHAPE_GROW" }
};

void EuclidDemo::draw() {

	m_timer.tick();

	step();

	double t = m_timer.t();

	if(t - m_lastSettingsChange > m_settings.nRandomizeTimer) {
		changeSettings();
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
		m_settings.m_fFOV,
		m_aspectRatio,
		0.1, // near clipping plane
		10000.0);	// far clipping plane
	//glOrtho(windowRect.left, windowRect.right, windowRect.bottom, windowRect.top, -20.0, 20.0);

	switch(m_settings.m_nCameraMode) {
	case 1:		// touring camera
	{
		float jitter = 0.05;
		float depth = 0;// 1.0*(1 - cos(0.3*m_timer.t()));
		rsVec vEye(
			depth + jitter*sin(t *0.5),
			depth + jitter*sin(t *0.4),
			depth + jitter*sin(t *0.4));

		gluLookAt(
			vEye[0], vEye[1], vEye[2],	// eye
			1, 1, 1,	// look
			0, 1, 0);	// up vector

		// Rotate camera
		glRotatef(t, 1, 1, 1);
		break;
	}

	case 2:		// drifty viewpoint: not mathematical but parallax good for debugging
	{
		float jitter = 0.4f;
		float speed = 0.02f * t;
		rsVec vLook(
			0.5f + jitter*(sin(speed *5)),
			0.5f + jitter*(sin(speed *3)),
			0.5f + jitter*(sin(speed *4)));
		gluLookAt(
			0, 0, 0,	// eye
			vLook[0], vLook[1], vLook[2],
			0, 1, 0);	// up vector
		break;
	}

	case 3:		// look at target
		gluLookAt(
			0, 0, 0,
			m_targetCenter.x, m_targetCenter.y, m_targetCenter.z,
			0, 1, 0);
		break;

	default:	// look at center
		gluLookAt(
			0, 0, 0,	// eye
			1, 1, 1,	// look
			0, 1, 0);	// up vector
	}

	if(g_trace) {
		rsMatrix m;
		glGetFloatv(GL_PROJECTION_MATRIX, m.get());

		str.clear();
		str << m << endl;
		TRACE("Projection:\n");
		TRACE(str.str().data());
	}

	// start drawing
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(windowedSaver) {
		m_settings.multiScreenMode = 0;
	}

	switch(m_settings.multiScreenMode) {
	case 0: {
		// ignore multiple monitors--just draw to window client
		float x0 = windowRect.left;
		float y0 = windowRect.top;
		float x1 = windowRect.right;
		float y1 = windowRect.bottom;
		drawViewport(x0, y0, x1, y1);
	}
	break;

	case 1: {
		// draw to each monitor separately
		for(unsigned int i = 0; i < monitorBounds.size(); ++i) {

			// get viewport coords for this monitor

			int flip = i & 1;	// flip alternate monitors

			RECT rc = monitorBounds[i];
			float x0 = flip ? rc.right : rc.left;
			float y0 = rc.top;
			float x1 = flip ? rc.left : rc.right;
			float y1 = rc.bottom;
			m_aspectRatio = fabs((x1-x0) / (y1-y0));

			glViewport(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);

			drawViewport(x0, y0, x1, y1);
		}	// for(i:monitors)
	}	// case 1

	}	//switch(multiscreenMode)
}

void EuclidDemo::drawViewport(float x0, float y0, float x1, float y1) {

	double t = m_timer.t();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw triangles
	//glDepthFunc(GL_GREATER);
	if(m_settings.m_bDrawTriangles || m_settings.m_nDrawEdges) {
		for(auto i = m_allTriangles.begin(); i != m_allTriangles.end(); ++i) {
			drawTriangle(i->second);
		}
	}
	
	glDepthFunc(GL_LESS);
	if(m_settings.m_nDrawVertices) {
		for(auto i = m_allTriangles.begin(); i != m_allTriangles.end(); ++i) {
			drawVertices(i->second);
		}
	}

	// draw find target
	if(m_settings.m_bDrawTarget)
	{
		float rgba[4] = { 1,1,1,0.75 };
		//hsl2rgb(0.2, 1.0, 0.75, rgba);

		if(m_growQueue.size() > 0) {
			// find is active: pulse red
			rgba[1] = rgba[2] = 0.2f;
			rgba[3] = 1.0f + cosf(3*t);	// pulse
		}
		else {
			// green
			rgba[0] = rgba[2] = 0.2f;
		}

		glColor4fv(rgba);
		glLineWidth(3.0f);

		switch(m_targetShape) 
		{
			case SHAPE_CIRCLE:
			default:
			{
				// draw circular target
				glPushMatrix();
					float sum = m_targetCenter.sum();
					glTranslatef(m_targetCenter.x / sum, m_targetCenter.y / sum, m_targetCenter.z / sum);
					glScalef(m_targetRadius, m_targetRadius, m_targetRadius);
					glCallList(LIST_TARGET);
				glPopMatrix();
			}
			break;

			case SHAPE_TRIANGLE:
			case SHAPE_POINT_SEARCH:
			{
				// draw triangular target
				glBegin(GL_LINE_LOOP);
				//glBegin(GL_TRIANGLES);
					glVertex3i(m_targetTriangle[0].x, m_targetTriangle[0].y, m_targetTriangle[0].z);
					glVertex3i(m_targetTriangle[1].x, m_targetTriangle[1].y, m_targetTriangle[1].z);
					glVertex3i(m_targetTriangle[2].x, m_targetTriangle[2].y, m_targetTriangle[2].z);
					//glVertex3i(m_targetTriangle[0].x, m_targetTriangle[0].y, m_targetTriangle[0].z);
				glEnd();
			}
			break;

		}
	}

	if(g_trace) {
		TripletTriangle<int> triBase;
		auto pLookup = m_allTriangles.find(triBase);
		TRACE("t:%.2lf evt:%u all:%u base?%d\n", 
			t, 
			42,//m_triangleQueue.size(), 
			m_allTriangles.size(), 
			pLookup != m_allTriangles.end());
		if(m_allTriangles.size() == 1) {
			TRACE("all[0]:\n");
			TriangleNode only = m_allTriangles.begin()->second;
			str.clear();
			str << only.tri << endl;
			TRACE(str.str().c_str());
			str.clear();
			str << triBase;
			TRACE(str.str().c_str());
			TRACE(" ==base ? %d <base ? %d\n", (triBase == only.tri), only.tri < triBase);
		}
	}

	if(m_settings.m_bDrawStats)
	{
		//glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(
			0.0f, 50.0f * m_aspectRatio,		// left, right
			0.0f, 50.0f,						// bottom, top
			-1.0f, 1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(1.0f, 3.0f, 0.0f);

		glColor4f(1.0f, 0.6f, 0.0f, 1.0f);
		//glBegin(GL_TRIANGLE_FAN);
		//glVertex3f(0, 0, 0);
		//glVertex3f(10, 0, 0);
		//glVertex3f(10, -10, 0);
		//glEnd();

		char buf[80];
		sprintf_s(buf, "%s %u : %u", m_ops, m_growQueue.size(), m_allTriangles.size());
		vector<string> asz = { buf, s_growth_enum_names[m_targetShape] };
		m_text.draw(asz);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	//glCopyPixels(0, 0, clientRect.right, clientRect.bottom, GL_BACK);
}

void EuclidDemo::renewAll() {
	double t = m_timer.t();
	for(auto it = m_allTriangles.begin(); it != m_allTriangles.end(); ++it) {
		it->second.m_fStartTime = t;
	}
}

void EuclidDemo::resetGrowthQueue() {
	// clear queue
	clearQueue(m_growQueue);

	// seed queue

	// create master triangle
	TriangleNode tnode(m_timer.t(), 19.5);
	m_growQueue.push(tnode);

	countMaxDepth = 0;
	countMaxSum = 0;
	countCollision = 0;
}

//
//	set growth/pruning algorithm
//
void EuclidDemo::targetFind(growth_enum shape) {
	double t = m_timer.t();

	//m_allTriangles.clear();
	int countBefore = m_allTriangles.size();

	m_targetShape = shape;

	// create master triangle
	TripletTriangle<int> masterTriangle;

	// random triangular target
	int size = 50;
	do {
		rsVec v = 1000.0f * rsVec::rsRand();
		Triplet<int> pt0(v[0]+size, v[1], v[2]);
		Triplet<int> pt1(v[0], v[1]+size, v[2]);
		Triplet<int> pt2(v[0], v[1], v[2]+size);
		m_targetTriangle.set(pt0, pt1, pt2);
		ASSERT(m_targetTriangle.isTriangle());
	} while(!masterTriangle.containsTriangle(m_targetTriangle));

	// circular target
	m_targetCenter = m_targetTriangle.centroid();
	m_targetRadius = 0;

	switch(m_targetShape) {
	case SHAPE_CIRCLE:
		m_targetRadius = 0.1f;
		break;

	case SHAPE_GROW:
	case SHAPE_FILL:
		m_targetCenter.set(1,1,1);
		m_targetRadius = 0.3f;
		break;
	}

	resetGrowthQueue();

	//while(processGrowthQueue());

	TRACE("**Find: Added %d triangles. Collisions: %d Max depth: %d Max sum: %d\n", 
		m_allTriangles.size()-countBefore, countCollision, countMaxDepth, countMaxSum);
}

void EuclidDemo::step() {
	double t = m_timer.t();

	// process triangle queues

	if(m_settings.m_alive && (m_allTriangles.empty())) {
		// seed empty queue with master triangle
		TripletTriangle<int> triBase;
		addTriangleUnique(triBase);
	}

	//	draw triangles, removing expired nodes

	auto endIt = m_allTriangles.end();
	int n = m_allTriangles.size();
	int i = 0;
	for(auto it = m_allTriangles.begin(); i < n && it != endIt; )
	{
		// take next active triangle from the queue, create some offspring from it and queue them up in the active queue.
		// add original triangle to death queue to eventually remove.

		TriangleNode tnode = it->second;
		ASSERT(it->first == it->second.tri);

		// cull nodes
		if(t > tnode.m_fStartTime + tnode.m_fLifespan) {
			// this branch has reached end of life: cull
			it = m_allTriangles.erase(it);
			continue;
		}

		it++;
	}

	// perform growth
	if(m_targetShape) {
		processGrowthQueue(100);
	}
}

bool EuclidDemo::processGrowthQueue(int count) {
	TriangleNode tnode;

	while(count-- && !m_growQueue.empty() && m_allTriangles.size() < MAX_NUM_TRIANGLES) {
		//tnode = m_growQueue.front();
		tnode = m_growQueue.top();
		ASSERT(tnode.tri.depth < 10000);
		if(m_growQueue.size() < 8) {
			TRACE("Q[%d]. LAST depth: %d sum: %d\n", m_growQueue.size(), 
				tnode.tri.depth, tnode.tri.centroid().sum());

			TRACE("All tri: %d\n", m_allTriangles.size());
		}
		m_growQueue.pop();

		//if(m_allTriangles.count(tnode.tri) > 0) {
		//	++countCollision;
		//}
		//if(tnode.tri.depth > m_settings.m_nMaxTriangleDepth) {
		//	++countMaxDepth;
		//	continue;
		//}
		//if(tnode.tri.centroid().sum() > m_settings.m_nMaxTriangleSum) {
		//	++countMaxSum;
		//	continue;
		//}

		double age = m_timer.t() - tnode.m_fStartTime;
		char ops[40];
		strcpy_s(ops, m_ops);

		bool bIntersection = false;

		switch(m_targetShape) {
			case SHAPE_FILL:
				bIntersection = true;
				break;

			case SHAPE_CIRCLE:	// circle clip
				bIntersection = ::triangleIntersectsCircle(tnode.tri, m_targetCenter, m_targetRadius);
				break;

			case SHAPE_TRIANGLE:	// triangular clip
				bIntersection = ::trianglesIntersect(tnode.tri, m_targetTriangle);
				break;

			case SHAPE_POINT_SEARCH:	// point search
				//bIntersection = tnode.tri.containsPoint(m_targetCenter);
				// all 3 points in?
				bIntersection =
					   tnode.tri.containsPoint(m_targetTriangle[0])
					&& tnode.tri.containsPoint(m_targetTriangle[1])
					&& tnode.tri.containsPoint(m_targetTriangle[2]);
				break;

			case SHAPE_GROW:
			{
				// this branch is still viable

				if(tnode.m_bFertile && age > TRI_TIME_REPRODUCE) {
					// make some babies
					float fertility = 1.2f * 5.0f / (5.0f + tnode.tri.depth);

					// select a subset of available ops
					for(int k = 0; ops[k]; ++k) {
						if(!randomBool(fertility)) {
							ops[k] = ' ';
						}
					}
					bIntersection = true;
					// no more babies
					tnode.m_bFertile = false;
				}
				else {
					//give it some more time
					bIntersection = false;
					m_growQueue.push(tnode);
				}
			}
		}	// switch(m_targetShape)

		if(!bIntersection) {
			continue;
		}

		int added = addTriangleUnique(tnode);

		// add all child triangles to search queue

		double t = tnode.m_fStartTime;
		shuffle(ops);
		//int numAdded = 0;
		for(int i = 0; ops[i]; ++i) {
			if(ops[i] == ' ') continue;

			TriangleNode childNode(tnode);
			childNode.m_bFertile = true;
			childNode.tri.operate(ops[i]);

			ASSERT(childNode.tri.depth < 10000);
			if(m_allTriangles.count(childNode.tri) > 0) {
				++countCollision;
				//continue;
			}
			if(childNode.tri.depth > m_settings.m_nMaxTriangleDepth) {
				++countMaxDepth;
				continue;
			}
			if(childNode.tri.centroid().sum() > m_settings.m_nMaxTriangleSum) {
				++countMaxSum;
				continue;
			}

			// add child
			childNode.m_fStartTime = t + (1+0.5f*::randomGaussian())*TRI_TIME_REPRODUCE;// (t += TRI_TIME_REPRODUCE * 0.0243);
			childNode.m_fLifespan = 10 + 5 * childNode.tri.depth;
			m_growQueue.push(childNode);
			//numAdded += addTriangleUnique(childNode);
		}

	}	// while (!m_growQueue.empty() ...)

	return (!m_growQueue.empty() && m_allTriangles.size() < MAX_NUM_TRIANGLES);
}

int EuclidDemo::addTriangleUnique(const TripletTriangle<int> &tri) {

	TriangleNode tnode(tri, m_timer.t());
	return addTriangleUnique(tnode);
}

int EuclidDemo::addTriangleUnique(const TriangleNode &tnode) {
	if(m_allTriangles.count(tnode.tri) > 0) {
		return 0;
	}
	if(m_allTriangles.size() % 100 == 0) {
		TRACE("%.3lf: #%d Added tri %lf-%lf\n", m_timer.t(), m_allTriangles.size(), tnode.m_fStartTime, tnode.m_fLifespan);
	}
	m_allTriangles[tnode.tri] = tnode;
	return 1;
}

void EuclidDemo::drawTriangle(const TriangleNode &tnode) {

	glDepthMask(false);	// don't modify depth buffer
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);

	double t = m_timer.t();
	double age = t - tnode.m_fStartTime;
	if(age <= 0 || age >= tnode.m_fLifespan) return;

	const TripletTriangle<int> &tri = tnode.tri;

	if(m_settings.m_bDrawTriangles) {
		// alpha: fade at end of lifespan
		float endFadeIn = 0.5f;
		float startFadeOut = 0.6f * tnode.m_fLifespan;
		float alpha = (age < endFadeIn)
			? age / endFadeIn
			: (age < startFadeOut)
				? 1.0f
				: (tnode.m_fLifespan - age) / (tnode.m_fLifespan - startFadeOut);

		//float rgba[4] = { 0,0,0,alpha };

		//auto ctr = tri.centroid();

		// draw flat triangles
		float rgba[4];
		tnode.getColor(rgba);
		rgba[3] = alpha;
		glColor4fv(rgba);
		glBegin(GL_TRIANGLES);

		for(int i = 0; i < 3; ++i) {
			Triplet<int> a = tri[i];
			glVertex3i(a.x, a.y, a.z);
		}

		glEnd();
	}

	// draw lines
	switch(m_settings.m_nDrawEdges) {
	case 0:
		return;

	case 1:
		glColor4f(0, 0, 0, 1);
		break;

	case 2:
		glColor4f(1, 1, 1, 1);
		break;
	}

	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < 3; ++i) {
		Triplet<int> a = tri[i];
		glVertex3i(a.x, a.y, a.z);
	}
	glEnd();
}

void EuclidDemo::drawVertices(const TriangleNode &tnode) {
	const TripletTriangle<int> &tri = tnode.tri;
	// draw vertex spheres

	glDepthMask(true);	// write to depth buffer
	glDisable(GL_DEPTH_TEST);

	double t = m_timer.t();
	double age = t - tnode.m_fStartTime;
	if(age < 0) return;

	// flash white (high lum, low sat)
	float sat =
		-cos(5 * t) +
		2 - 0.5 * cos(11 * t) +
		1 - 2 * cos(0.4*t - 0.5*tri.depth);
	//sat = 1;
	// flash to white at birth
	if(age < 1.3f) {
		sat = max(0, sat);
		sat += (age - 1.3f) / (1.3f);
	}
	
	// alpha: fade at end of lifespan
	float endFadeIn = 0.5f;
	float startFadeOut = 0.6f * tnode.m_fLifespan;
	float alpha = (age < endFadeIn)
		? age / endFadeIn
		: (age < startFadeOut)
			? 1.0f
			: (tnode.m_fLifespan - age) / (tnode.m_fLifespan - startFadeOut);
	float rgb[4] = { 0,0,0,alpha };
	
	// vertex size
	float s = m_settings.m_fVertexSize;

	for(int i = 0; i < 3; ++i)
	{
		const Triplet<int> &a = tri[i];
		//const Triplet<int> &a = tri.centroid();

		float distsq = (a.lengthSq());
		float dist = sqrtf(distsq);
		int sum = a.sum();

		float hue = logf(sum) * 0.36123476 + t*0.08;
		hsl2rgb(hue, sat, 1.0f, rgb);
		glColor4fv(rgb);

		glPushMatrix();
		glTranslatef(a.x, a.y, a.z);
		//glTranslatef(s*a.x, s*a.y, s*a.z);
		//float s = 0.3f * sqrt(1.5f*a.sum());

		switch(m_settings.m_nDrawVertices) {
		case 1:
			break;

		case 2:
			// size boost for distant points
			s = m_settings.m_fVertexSize * 0.1f * logf(dist + 1.0f);
			break;

		case 3:
			// stronger size boost for distant points
			s = m_settings.m_fVertexSize * 0.05f * sqrt(dist);
			break;
		}

		glScalef(s,s,s);
		glCallList(LIST_VERTEX);
		glPopMatrix();
	}
}


void idleProc() {

	if(g_ss.m_readyToDraw && !isSuspended && !checkingPassword) {

		g_ss.draw();
		g_trace = false;
		g_ss.processConsole();
	}
}


void EuclidDemo::reshape() {

	int viewport[4];
	viewport[0] = clientRect.left;
	viewport[1] = clientRect.top;
	viewport[2] = clientRect.right - clientRect.left;
	viewport[3] = clientRect.bottom - clientRect.top;

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	m_aspectRatio = float(viewport[2]) / float(viewport[3]);


}

void EuclidDemo::initSaver(HWND hwnd) {

	m_hwnd = hwnd;

	float a[3] = { -1,10,0 };
	float b[3] = { 1,10,0 };
	float ctr[3] = { 0,0,0 };
	float radius = 10;
	for(float i = -5; i <= 3; ++i) {
		a[0] = i;
		b[0] = i + 2;
		bool q = lineSegmentIntersectsCircle(a, b, ctr, radius);
		TRACE("seg-circle: %lf %lf %d\n", a[0], b[0], q);
	}

	::readRegistry(m_settings);

	srand((unsigned)time(NULL));

	// Limit memory consumption because the Windows previewer is just too darn slow
	if(doingPreview) {

	};

	// init window DC, client coords
	initWindow(hwnd);

	// init GL

	reshape();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_NORMALIZE);

	// set up front and back buffers
	for(int i = 0; i < 2; ++i) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		glEnable(GL_DEPTH);
		//glEnable(GL_TEXTURE_2D);
		//glDisable(GL_DEPTH_TEST);
		glDepthMask(true);	// write to depth buffer
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_DEPTH_CLAMP);

		//glDrawBuffer(GL_FRONT);
		glClearColor(0.5,0.5,0.5, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj *qobj;
	qobj = gluNewQuadric();

	LIST_VERTEX = glGenLists(2);
	LIST_TARGET = LIST_VERTEX + 1;

	glNewList(LIST_VERTEX, GL_COMPILE);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		// rotate sphere so poles point toward camera, that is, along x=y=z
		// angle is angle between [0,0,1] and [1,1,1], acos(sqrt(3))
		glRotatef(54.73561032f, -1, 1, 0);
		// flatten along low-resolution axis to appear more disc-like
		glScalef(1,1,0.1);
		gluSphere(qobj, 1, 16, 2);
		glPopMatrix();
	glEndList();

	glNewList(LIST_TARGET, GL_COMPILE);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		// rotate cylinder so axis points along x=y=z
		glRotatef(54.73561032f, -1, 1, 0);
		gluQuadricDrawStyle(qobj, GLU_LINE); /* flat shaded */
		gluCylinder(qobj, 1, 1, 0.1, 24, 3);
		glPopMatrix();
	glEndList();

	gluDeleteQuadric(qobj);

	m_text.init();	// ugh

	m_readyToDraw = 1;

	// init and start sim
	TriangleNode seed;
	seed.m_bFertile = true;
	m_growQueue.push(seed);

	changeSettings();
}

void EuclidDemo::changeSettings() {


	m_lastSettingsChange = m_timer.t();
}

void EuclidDemo::handleDestroy(HWND hwnd) {
	m_readyToDraw = 0;
	::destroyScreensaverWindow(hwnd);
}

LONG EuclidDemo::handleKeyDown(WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState) {
	switch(keyCode) {

	case '3':
	case VK_NUMPAD3:
	case '4':
	case VK_NUMPAD4:
	case '5':
	case VK_NUMPAD5:
	case '6':
	case VK_NUMPAD6:
	case '7':
	case VK_NUMPAD7:
	case '8':
	case VK_NUMPAD8:
		return 0;

	case '9':
	case VK_NUMPAD9:
		m_settings.m_bDrawTarget = m_settings.m_bDrawStats = !m_settings.m_bDrawTarget;
		return 0;

	case '0':
	case VK_NUMPAD0:
		m_settings.m_nDrawVertices = (m_settings.m_nDrawVertices + 1) % 4;
		return 0;

	case '1':
	case VK_NUMPAD1:
		m_settings.m_nDrawEdges = (m_settings.m_nDrawEdges + 1) % 3;
		return 0;

	case '2':
	case VK_NUMPAD2:
		m_settings.m_bDrawTriangles = !m_settings.m_bDrawTriangles;
		return 0;

	case VK_OEM_MINUS:	// '-': zoom out
		m_settings.m_fFOV = min(180.0, m_settings.m_fFOV + repeatCount);
		return 0;

	case VK_OEM_PLUS:	// '+': zoom in
		m_settings.m_fFOV = max(1.0, m_settings.m_fFOV - repeatCount);
		return 0;

	case VK_DELETE: 
		m_allTriangles.clear();
		m_targetShape = SHAPE_NONE;

	case VK_HOME: {
		resetGrowthQueue();
		//TriangleNode tnode(m_timer.t());
		//m_allTriangles[tnode.tri] = tnode;
	}
	return 0;

	case 'R':
		changeSettings();
		return 0;

	case 'P':
		return 0;

	case 'C':
		m_settings.m_nCameraMode = (m_settings.m_nCameraMode + 1) % 4;
		return 0;

	case 'X':
		//triangleFill();
		targetFind(SHAPE_FILL);
		return 0;

	case 'F':
		if(::GetKeyState(VK_SHIFT)) {
			targetFind(SHAPE_CIRCLE);
		}
		else {
			targetFind(SHAPE_TRIANGLE);
		}
		return 0;

	case 'G':
		targetFind(SHAPE_GROW);
		return 0;

	case 'H':
		targetFind(SHAPE_POINT_SEARCH);
		return 0;

	case 'N':
		renewAll();
		return 0;

	case 'L':
		m_settings.m_alive = !m_settings.m_alive;
		return 0;

	case 'T':
		g_trace = true;
		return 0;

	case VK_OEM_4:  //  '[': decrease vertex size
		m_settings.m_fVertexSize *= 0.9f;
		return 0;
		
	case VK_OEM_6:  //  ']': increase vertex size
		m_settings.m_fVertexSize *= 1.1f;
		return 0;
		
	case VK_OEM_PERIOD:
		m_settings.nSimulationSpeed = min(m_settings.nSimulationSpeed + 2, 25);
		return 0;

	case VK_OEM_COMMA:
		m_settings.nSimulationSpeed = max(1, m_settings.nSimulationSpeed - 2);
		return 0;

	case VK_END   :
	case VK_LEFT  :
		m_settings.m_nMaxTriangleDepth = max(1, m_settings.m_nMaxTriangleDepth - repeatCount);
		return 0;

	case VK_RIGHT :
		m_settings.m_nMaxTriangleDepth = min(10000, m_settings.m_nMaxTriangleDepth + repeatCount);
		return 0;

	case VK_UP    :
		m_settings.m_nMaxTriangleSum = min(1000, m_settings.m_nMaxTriangleSum + repeatCount);
		return 0;

	case VK_DOWN  :
		m_settings.m_nMaxTriangleSum = max(3, m_settings.m_nMaxTriangleSum - repeatCount);
		return 0;

		break;

	case VK_LWIN:
	case VK_RWIN:
		TRACE("WIN KEY: %d, %d\n", repeatCount, previousKeyState);
		return 0;

	case VK_ESCAPE:
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
		break;

	case VK_SPACE: {
		m_console.allocate();
		//processConsole();
		break;
	}

	default:
		TRACE("KEYDOWN: keycode: 0x%02x scancode: 0x%02x\n", keyCode, scanCode);
	}	// switch(wpm)

	// not handled here--use default behavior
	return 1;
}

LONG screenSaverProc(HWND hwnd, UINT msg, WPARAM wpm, LPARAM lpm) {

	LONG result;

	switch(msg) {
	case WM_CREATE:
		g_ss.initSaver(hwnd);
		break;

	case WM_KEYDOWN:
		result = g_ss.handleKeyDown(
			wpm, 
			(unsigned char)(lpm >> 16),				// scan code (bits 16-23)
			(unsigned short)(lpm),			// repeat count (bits 0-15)
			(lpm >> 30) & 1					// previous key state
		);
		if(result == 0) {
			// handled--do not terminate
			return 0;
		}
		if(windowedSaver) {
			// no need to terminate--not running as true screen saver
			return 0;
		}

	case WM_DESTROY:
		g_ss.handleDestroy(hwnd);
		break;
	}	// switch(msg)

	return defScreenSaverProc(hwnd, msg, wpm, lpm);
}


void EuclidDemo::processConsole() {
	if(m_console.isAllocated()) {
		std::string sz = m_console.getCommand();
		if(!sz.empty()) {
			m_targetShape = SHAPE_NONE;
			m_settings.m_alive = false;

			if(m_allTriangles.empty()) {
				m_userNode.m_fLifespan = 9999;
				m_userNode.m_fStartTime = m_timer.t();
				//TripletTriangle<int> masterTriangle;
				//TriangleNode master(masterTriangle, m_timer.t(), 9999);// std::numeric_limits<double>::infinity());
				m_allTriangles[m_userNode.tri] = m_userNode;
			}

			if(sz.length() == 2) {
				unsigned int fromCol = toupper(sz[0]) - 'X';
				unsigned int toCol = toupper(sz[1]) - 'X';
				//TriangleNode tnode = m_allTriangles.begin()->second;
				//TriangleNode tchild(m_userNode.tri, m_timer.t(), std::numeric_limits<double>::infinity());
				if(fromCol - toCol && fromCol + toCol <= 3) {
					m_userNode.tri.merge(fromCol, toCol);
					addTriangleUnique(m_userNode);
					//m_allTriangles[m_userNode.tri] = m_userNode;
				}
				else {
					printf("? '%s'\n", sz.c_str());
				}
			}
			else if(sz.length() == 1) {
				switch(sz[0]) {
				case 'q':
					m_console.free();
					break;

				case '0':
					//m_userNode = TriangleNode(m_timer.t(), 9999);
					m_userNode.tri.reset();
					m_userNode.m_fStartTime = m_timer.t();
					//clearQueue(m_userNode);
					break;

				default:
					if(m_userNode.tri.operate(sz[0])) {
						addTriangleUnique(m_userNode);
						//m_allTriangles[m_userNode.tri] = m_userNode;
					}
				}
			}
			else if(sz == "clear") {
				clearQueue(m_allTriangles);
				clearQueue(m_userNode.tri);
				clearQueue(m_growQueue);
			}
			else if(sz == "fill") {
				this->targetFind(SHAPE_FILL);
			}
			else if(sz.find("op ") == 0) {
				strncpy_s(m_ops, sz.substr(3).c_str(), sizeof(m_ops));
			}
			else {
				printf("? '%s'\n", sz.c_str());
			}

			cout << m_userNode.tri;
		}
	}
}
