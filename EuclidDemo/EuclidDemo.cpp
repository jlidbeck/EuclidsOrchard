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


#include "EuclidDemo.h"

#include <ColorUtil.h>
#include <rsMath/rsMatrix.h>
#include <resource.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext/include/gl/glext.h>	// fresh!
#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#pragma comment(lib, "GL/glext/lib/glext.lib")

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

EuclidDemo::EuclidDemo() 
{
	m_settings.randomize();
}

void EuclidDemo::draw() 
{
	m_timer.tick();

	step();

	double t = m_timer.t();

	if(t - m_lastSettingsChange > m_settings.nRandomizeTimer)
	{
		randomizeSettings();
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
		m_settings.fFOV,
		m_fAspectRatio,
		0.1, // near clipping plane
		10000.0);	// far clipping plane
	//glOrtho(windowRect.left, windowRect.right, windowRect.bottom, windowRect.top, -20.0, 20.0);

	switch(m_settings.nCameraMode) 
	{
		case CameraMode::CENTER:
		default:	// look at center
			gluLookAt(
				0, 0, 0,	// eye
				1, 1, 1,	// look
				0, 1, 0);	// up vector
			break;

		case CameraMode::TARGET:		// look at target
			gluLookAt(
				0, 0, 0,
				m_search.m_target.x, m_search.m_target.y, m_search.m_target.z,
				0, 1, 0);
			break;

		case CameraMode::TOUR:		// touring camera
		{
			float jitter = 0.4f;
			float speed = 0.02f * t;
			rsVec vLook(
				0.5f + jitter*(sin(speed *5)),
				0.5f + jitter*(sin(speed *6)),
				0.5f + jitter*(cos(speed *4)));

			m_search.m_target.set(
				(int)(0.5f + 1000 * vLook[0]),
				(int)(0.5f + 1000 * vLook[1]),
				(int)(0.5f + 1000 * vLook[2]));
			TripletSearch<int> s;
			s.m_bVerbose = false;
			s.m_maxDepth = 50;
			s.m_maxHeight = 2000;
			s.search(m_search.m_target);
			addTrianglePath(s.m_paths.size() > 0 ? s.m_paths[0] : s.m_bestPath);

			gluLookAt(
				0, 0, 0,	// eye
				vLook[0], vLook[1], vLook[2],
				0, 1, 0);	// up vector
			break;
		}

		case CameraMode::DRIFT:		// drifty viewpoint: not mathematical but parallax.. good for debugging
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
			glRotatef((GLfloat)t, 1, 1, 1);
			break;
		}

	}

	if(g_trace) 
	{
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
			m_fAspectRatio = fabs((x1-x0) / (y1-y0));

			glViewport(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);

			drawViewport(x0, y0, x1, y1);
		}	// for(i:monitors)
	}	// case 1

	}	//switch(multiscreenMode)
}

void EuclidDemo::drawViewport(float x0, float y0, float x1, float y1) 
{

	double t = m_timer.t();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// enumerate vertices

		m_vTriangleVertices.reserve(3 * m_allTriangles.size());
		m_vTriangleVertices.resize(0);
		m_vTriangleColors.reserve(3 * m_allTriangles.size());
		m_vTriangleColors.resize(0);
		m_vVertexColors.reserve(3 * m_allTriangles.size());
		m_vVertexColors.resize(0);

		drawTriangles();

	// draw triangles
	if(m_settings.bFillTriangles || m_settings.nDrawEdges) 
	{
		glDepthMask(false);	// don't modify depth buffer
		//glDepthFunc(GL_GREATER);
		//glEnable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);

		glVertexPointer(3, GL_INT, 0, m_vTriangleVertices.data());
		glColorPointer(4, GL_FLOAT, 0, m_vTriangleColors.data());
		//glColorPointer(4, GL_FLOAT, 0, m_vVertexColors.data());
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		if(m_settings.bFillTriangles) {
			glDrawArrays(GL_TRIANGLES, 0, m_vTriangleVertices.size());
		}

		if(m_settings.nDrawEdges) 
		{
			glLineWidth(2.0f);
			glDisableClientState(GL_COLOR_ARRAY);
			switch(m_settings.nDrawEdges) {
				case 1:
					glColor4f(0, 0, 0, 1);
					break;

				case 2:
					glColor4f(1, 1, 1, 1);
					break;
			}
			glVertexPointer(3, GL_INT, 0, m_vTriangleVertices.data());
			glDrawArrays(GL_LINES, 0, m_allTriangles.size() * 3);
			//for(int i = 0; i < 3; ++i) {
			//	glDrawArrays(GL_LINES, 0, m_allTriangles.size() * 3);
			//	glDrawArrays(GL_LINES, 1, m_allTriangles.size() * 3);
			//}
		}

		//glDrawArrays(GL_LINES, 0, m_allTriangles.size() * 3);
	}
	
	if(m_settings.nDrawBranches)
	{
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);

		for(auto i = m_allTriangles.begin(); i != m_allTriangles.end(); ++i) {
			drawBranches(i->second);
		}
	}

	if(m_settings.nDrawVertices) {
		drawVertices();
	}

	// draw find target
	if(m_settings.bDrawTarget)
	{
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);

		float rgba[4] = { 1,1,1,0.75 };
		//HSLtoRGB(0.2, 1.0, 0.75, rgba);

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

		switch(m_search.m_growth) 
		{
			case GrowthEnum::CIRCLE:
			default:
			{
				// draw circular target
				glPushMatrix();
					float sum = m_search.m_target.sum();
					glTranslatef(m_search.m_target.x / sum, m_search.m_target.y / sum, m_search.m_target.z / sum);
					glScalef(m_search.m_targetRadius, m_search.m_targetRadius, m_search.m_targetRadius);
					glCallList(LIST_TARGET);
				glPopMatrix();
			}
			break;

			case GrowthEnum::TRIANGLE:
			case GrowthEnum::POINT_SEARCH:
			{
				// draw triangular target
				glBegin(GL_LINE_LOOP);
				//glBegin(GL_TRIANGLES);
					glVertex3i(m_search.m_clip[0].x, m_search.m_clip[0].y, m_search.m_clip[0].z);
					glVertex3i(m_search.m_clip[1].x, m_search.m_clip[1].y, m_search.m_clip[1].z);
					glVertex3i(m_search.m_clip[2].x, m_search.m_clip[2].y, m_search.m_clip[2].z);
					//glVertex3i(m_search.m_clip[0].x, m_search.m_clip[0].y, m_search.m_clip[0].z);
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

	if(m_settings.bDrawStats)
	{
		// FPS
		++m_nFrameCount;
		if(t > m_fLastFPSTime + 1) {
			m_fps = m_nFrameCount / (t - m_fLastFPSTime);
			m_fLastFPSTime = t;
			m_nFrameCount = 0;
		}

		//glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(
			0.0f, 50.0f * m_fAspectRatio,		// left, right
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

		char buf[80], buf2[80];
		sprintf_s(buf2, "%.2lf fps", m_fps);
		sprintf_s(buf, "%s %s queue: %u tri: %u collision: %d", 
			GrowthEnumString(m_search.m_growth),
			m_search.m_operations.c_str(), 
			m_growQueue.size(), m_allTriangles.size(),
			m_search.m_countCollision);
		vector<string> asz = { buf2, buf };
		m_text.draw(asz);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	//glCopyPixels(0, 0, clientRect.right, clientRect.bottom, GL_BACK);
}

//	Renews all nodes as if they've just started.
//	Optionally extends lifespan as well.
void EuclidDemo::renewAll(double fExtendLifespan) 
{
	double t = m_timer.t();
	for(auto it = m_allTriangles.begin(); it != m_allTriangles.end(); ++it) {
		it->second.m_fStartTime = t;
		it->second.m_fLifespan += fExtendLifespan;
	}
}

void EuclidDemo::resetGrowthQueue() 
{
	// clear queue
	clearQueue(m_growQueue);

	// seed queue

	// create master triangle
	TriangleNode tnode(m_timer.t(), 19.5);
	m_growQueue.push(tnode);

	m_search.m_countMaxDepth = 0;
	m_search.m_countMaxSum = 0;
	m_search.m_countCollision = 0;
}

//	search for a specific target
void EuclidDemo::targetFind(GrowthEnum shape, const Triplet<int> &target)
{
	m_search.m_growth = shape;
	m_search.m_target = target;
	m_search.m_targetRadius = 0.1f;

    cout << "FIND: " << m_search.m_target << " " << GrowthEnumString(m_search.m_growth)
        << " radius: " << m_search.m_targetRadius << " ops: " << m_search.m_operations << endl
        << " max depth: " << m_search.m_maxDepth << " max sum: " << m_search.m_nMaxTriangleSum << endl;

	resetGrowthQueue();
}

//
//	search for a random target
//
void EuclidDemo::targetFind(GrowthEnum shape)
{
	m_search.m_growth = shape;

	createRandomTarget();
	resetGrowthQueue();

    cout << "FIND: " << m_search.m_target << " " << GrowthEnumString(m_search.m_growth)
        << " radius: " << m_search.m_targetRadius << " ops: " << m_search.m_operations << endl
        << " max depth: " << m_search.m_maxDepth << " max sum: " << m_search.m_nMaxTriangleSum << endl;

	//TRACE("**Find: Added %d triangles. Collisions: %d Max depth: %d Max sum: %d\n", 
	//	m_allTriangles.size()-countBefore, m_search.m_countCollision, m_search.m_countMaxDepth, m_search.m_countMaxSum);
}

void EuclidDemo::createRandomTarget()
{
	// create master triangle
	TripletTriangle<int> masterTriangle;

	// random triangular target
	int size = 50;
	do {
		rsVec v = 1000.0f * rsVec::rsRand();
		Triplet<int> pt0(v[0] + size, v[1], v[2]);
		Triplet<int> pt1(v[0], v[1] + size, v[2]);
		Triplet<int> pt2(v[0], v[1], v[2] + size);
		m_search.m_clip.set(pt0, pt1, pt2);
		ASSERT(m_search.m_clip.isTriangle(), "clip is not a triangle");
	} while(!masterTriangle.containsTriangle(m_search.m_clip));

	m_search.m_target = m_search.m_clip.centroid();
	m_search.m_targetRadius = 0;

	switch(m_search.m_growth) 
	{
	case GrowthEnum::CIRCLE:
		m_search.m_targetRadius = 0.1f;
		break;

	case GrowthEnum::STOCHASTIC:
	case GrowthEnum::FILL:
		m_search.m_target.set(1, 1, 1);
		m_search.m_targetRadius = 0.3f;
		break;
	}

}

void EuclidDemo::step() 
{
	double t = m_timer.t();

	if(m_settings.nCameraMode == CameraMode::DRIFT) 
	{
	}

	// process triangle queues

	if(m_settings.m_alive && (m_allTriangles.empty())) 
	{
		// seed empty queue with master triangle
		TripletTriangle<int> triBase;
		addTriangleUnique(triBase);
	}

	//	first pass: remove expired triangles

	auto endIt = m_allTriangles.end();
	int n = m_allTriangles.size();
	int i = 0;
	for(auto it = m_allTriangles.begin(); i < n && it != endIt; )
	{
		// take next active triangle from the queue, create some offspring from it and queue them up in the active queue.
		// add original triangle to death queue to eventually remove.

		TriangleNode tnode = it->second;
		ASSERT(it->first == it->second.tri, "check failed");

		// cull nodes
		if(t > tnode.m_fStartTime + tnode.m_fLifespan) 
		{
			// this branch has reached end of life: cull
			it = m_allTriangles.erase(it);
			continue;
		}

		it++;
	}

	// perform growth
	if(m_search.m_growth != GrowthEnum::NONE) 
	{
		processGrowthQueue(100);
	}
}

bool EuclidDemo::processGrowthQueue(int count) 
{
	TriangleNode tnode;
	double t = m_timer.t();

	// random time about one second from now
	std::normal_distribution<> randomChildTimer { t + TRI_TIME_REPRODUCE, 0.5 * TRI_TIME_REPRODUCE };

	// pull all high-priority nodes from the growth queue (nodes whose startTime has passed)
	// and add them to the drawing queue;
	// also generate offspring according to current growth algorithm

	while(count-- && !m_growQueue.empty() && m_allTriangles.size() < MAX_NUM_TRIANGLES) 
	{
		tnode = m_growQueue.top();
		
		// sanity checks
		ASSERT(tnode.tri.depth < 10000, "depth out of range:");
		//if(m_growQueue.size() < 8) {
		//	TRACE("Q[%d]. LAST depth: %d sum: %d\n", m_growQueue.size(), 
		//		tnode.tri.depth, tnode.tri.centroid().sum());

		//	TRACE("All tri: %d\n", m_allTriangles.size());
		//}

		double age = t - tnode.m_fStartTime;
		if(tnode.m_bFertile && age < TRI_TIME_REPRODUCE) 
		{
			// this and all subsequent nodes are too new to draw or process
			break;
		}

		m_growQueue.pop();

		// select set of operations from settings--these will be modified/randomized by the growth algorithm
		char ops[40];
		strcpy_s(ops, m_search.m_operations.c_str());

		bool bGenerateChildren = false;

		switch(m_search.m_growth) 
		{
			case GrowthEnum::FILL:
				bGenerateChildren = true;
				break;

			case GrowthEnum::CIRCLE:		// circle clip
				bGenerateChildren = ::triangleIntersectsCircle(tnode.tri, m_search.m_target, m_search.m_targetRadius);
				break;

			case GrowthEnum::TRIANGLE:	// triangular clip
				bGenerateChildren = ::trianglesIntersect(tnode.tri, m_search.m_clip);
				break;

			case GrowthEnum::POINT_SEARCH:	// point search
            {
                auto c = tnode.tri.centroid();
                if (c == m_search.m_target)
                {
                    if (tnode.tri.numColumns() == 1)
                    {
                        cout << "FOUND:  " << tnode.tri.path << endl << tnode.tri << endl;
                        m_search.m_paths.push_back(tnode.tri.path);
                        bGenerateChildren = false;
                    }
                    else
                    {
                        //cou?t << "ALMOST! waiting for dim collapse\n" << tnode.tri << "\nat: " << tnode.tri.path << endl;
                        bGenerateChildren = true;
                    }
                }
                else if (tnode.tri.numColumns() == 1)
                {
                    //cout << "dead end: ";
                    bGenerateChildren = false;
                }
                else 
                {
                    bool b0 = c.x >= m_search.m_target.x;
                    bool b1 = c.y >= m_search.m_target.y;
                    bool b2 = c.z >= m_search.m_target.z;

                    bGenerateChildren = tnode.tri.containsPoint(m_search.m_target);

                    if (b0 || b1 || b2)
                        bGenerateChildren = false;

                    //if (bGenerateChildren)
                    //{
                    //    if (tnode.tri.numColumns() == 2)
                    //    {
                    //        cout << "contains target, searching edge " << tnode.tri.path << ":\n" << tnode.tri;
                    //    }
                    //}
                }

                //// all 3 points in?
                //bGenerateChildren =
                //	   tnode.tri.containsPoint(m_search.m_clip[0])
                //	&& tnode.tri.containsPoint(m_search.m_clip[1])
                //	&& tnode.tri.containsPoint(m_search.m_clip[2]);
                break;
            }

			case GrowthEnum::STOCHASTIC:
			{
				if(age > TRI_TIME_REPRODUCE) 
				{
					// randomly select operations to generate children
					// make some babies
					float fertility = 0.7f;// 1.2f * 5.0f / (5.0f + tnode.tri.depth);

					// select a subset of available ops
					for(int k = 0; ops[k]; ++k) 
					{
						if(!randomBool(fertility)) 
						{
							ops[k] = ' ';
						}
					}
					bGenerateChildren = true;
				}
				else 
				{
					//give it some more time:
					// don't add any child nodes, but re-add the parent
					bGenerateChildren = false;
					m_growQueue.push(tnode);
				}
			}
			break;

		}	// switch(m_search.m_growth)

		if(!bGenerateChildren) 
		{
			continue;
		}

		// add to drawing
		int added = 0;
		added += addTriangleUnique(tnode);

		// remove parent
		if(m_settings.m_bReplaceParent)
		{
			auto pnode = m_allTriangles.find(tnode.parent);
			if(pnode != m_allTriangles.end())
			{
				m_allTriangles.erase(pnode);
			}
		}

		// add child triangles to grow queue

		shuffle(ops);
		for(int i = 0; ops[i]; ++i) {
			if(ops[i] == ' ') continue;

			TriangleNode childNode(tnode);
			childNode.m_bFertile = true;
			if(!childNode.tri.operate(ops[i])) {
				// operation was not valid
				continue;
			}

			ASSERT(childNode.tri.depth < 10000, "depth out");
			if(m_allTriangles.count(childNode.tri) > 0) {
				++m_search.m_countCollision;
				//continue;
			}
			if(childNode.tri.depth > m_search.m_maxDepth) {
				++m_search.m_countMaxDepth;
				continue;
			}
			if(childNode.tri.centroid().sum() > m_search.m_nMaxTriangleSum) {
				++m_search.m_countMaxSum;
				continue;
			}

			// add child
			childNode.m_fStartTime =
				//t + (1 + ::randomGaussian(0.5))*TRI_TIME_REPRODUCE;
				randomChildTimer(m_randomGenerator);
				// (t += TRI_TIME_REPRODUCE * 0.0243);
				//tnode.m_fStartTime + (childNode.tri.centroid().sum() - tnode.tri.centroid().sum())*0.1*TRI_TIME_REPRODUCE;

			childNode.m_fLifespan = m_settings.TRI_TIME_LIFESPAN;
				//10 + 5 * childNode.tri.depth;
			childNode.parent = tnode.tri;
			m_growQueue.push(childNode);
			//numAdded += addTriangleUnique(childNode);
		}

	}	// while (!m_growQueue.empty() ...)

	return (!m_growQueue.empty() && m_allTriangles.size() < MAX_NUM_TRIANGLES);
}

//	Add triangle to display list, if it doesn't already exist
//	Start time is set to the current time
int EuclidDemo::addTriangleUnique(const TripletTriangle<int> &tri)
{
	TriangleNode tnode(tri, m_timer.t(), m_settings.TRI_TIME_LIFESPAN);
	return addTriangleUnique(tnode);
}

//	Add triangle to display list, if it doesn't already exist
int EuclidDemo::addTriangleUnique(const TriangleNode &tnode)
{
	//if(m_settings.m_bReplaceParent && m_allTriangles.count(tnode.parent) > 0) {
	//	m_allTriangles.erase(tnode.parent);
	//}

	if(m_allTriangles.count(tnode.tri) > 0) 
	{
		return 0;
	}
	TriangleNode t = tnode;
	t.computeColor();
	//if(m_allTriangles.size() % 100 == 0) {
	//	TRACE("%.3lf: #%d Added tri %lf-%lf\n", m_timer.t(), m_allTriangles.size(), tnode.m_fStartTime, tnode.m_fLifespan);
	//}
	m_allTriangles[tnode.tri] = t;
	return 1;
}

//void EuclidDemo::drawTriangle(const TriangleNode &tnode)
void EuclidDemo::drawTriangles()
{
	double t = m_timer.t();
	// accumulate 3 arrays, all the same size:
	// - triangle vertex coords
	// - triangle color (each repeated 3x)
	// - triangle vertex colors
	//for(auto i = m_allTriangles.begin(); i != m_allTriangles.end(); ++i)
	for(auto i : m_allTriangles)
	{
		const TriangleNode &tnode = i.second;

		double age = t - tnode.m_fStartTime;
		if(age <= 0 || age >= tnode.m_fLifespan) continue;

		const TripletTriangle<int> &tri = tnode.tri;
		auto ctr = tri.centroid();

		// compute triangle fill color
		//if(m_settings.bFillTriangles) 
		{
			// flash white (high lum, low sat)
			float sat =
				//-cos(5 * t) +
				//2 - 0.5 * cos(11 * t) +
				//1 - 2 * cos(0.4*t - 0.5*tri.depth);
				//0;
				1;
			// flash to white at birth
			if(age < 1.3f) {
				sat = max(0, sat);
				sat += (age - 1.3f) / (1.3f);
			}

			// alpha: fade in and out at beginning and end of lifespan
			float endFadeIn = 0.5f;
			float startFadeOut = 0.6f * tnode.m_fLifespan;
			float alpha = (age < endFadeIn)
				? age / endFadeIn
				: (age < startFadeOut)
					? 1.0f
					: (tnode.m_fLifespan - age) / (tnode.m_fLifespan - startFadeOut);

			RGBA rgbFace;
			RGBA rgba1, rgba2, rgba;
			//float hsv[3];
			//tnode.getColor(rgba2);
			//tnode.getColorHSV(hsv);
			//{
			//	auto ctr = tnode.tri.centroid();
			//	auto c = m_settings.m_palette.blendAt((float)ctr.x / ctr.sum());
			//	COLORREFtoRGB(c, rgba1.v);
			//}
			//mix3(rgba1.v, rgba2.v, 0.5f, rgba.v);
			rgbFace[3] = rgba1[3] = rgba2[3] =rgba[3]= m_settings.fAlpha * alpha;
			//glColor4fv(rgba);
			//glBegin(GL_TRIANGLES);
			mix3(WHITE.v, tnode.m_rgba.v, sat, rgbFace.v);
			for(int i = 0; i < 3; ++i) 
			{
				const Triplet<int> &a = tri[i];
				//glVertex3i(a.x, a.y, a.z);
				m_vTriangleVertices.push_back(a);
				m_vTriangleColors.push_back(rgbFace);

				int sum = a.sum();
				// per- vertex color
				float hue =
					//logf(sum) * 0.36123476 + t*0.08;
					//((pt.x & 1) * 4 + (pt.y & 1) * 2 + (pt.z & 1)) / 8.0f;	// even/odd: 7 possible hues... highlights edges
					//(sum % 8) / 8.0f;	// some order: repetitions, lines, spectra
					//a.isPythagoreanTriple() ? 0.15 : 0.9;
					(a.x == a.y || a.y == a.z || a.z == a.x) ? 0.15 : 0.9;	// edges
					//a.z ? (((a.x + a.y) % a.z) % 8) / 8.0f : 0;	// color: order and chaos
					//(sum % 5) / 5.0f;
					//(sum % 6) / 6.0f;
					//(isPrime(a.x) + isPrime(a.y) + isPrime(a.z)) / 4.0f;
					//(sign(a.y*a.y - 4 * a.x*a.z)+1)/3.0f;	// quadratic determinant
					//0;


										//sat = 2 - 2 * cos(0.7f*t + 0.5*sum);
				sat = 1;
				HSVtoRGB(hue, sat, 1.0f, rgba.v);

				//rgb[0] = ((pt.y % 2 == 0) + (pt.x % 2 == 0) + (pt.z % 2 == 0)) / 2.0f;
				//rgb[1] = ((pt.y % 3 == 0) + (pt.x % 3 == 0) + (pt.z % 3 == 0)) / 2.0f;
				//rgb[2] = ((pt.x % 5 == 0) + (pt.y % 5 == 0) + (pt.z % 5 == 0)) / 2.0f;
				// highlight primes
				//rgb[0] = 1.0f * isPrime(pt.x);
				//rgb[1] = 1.0f * isPrime(pt.y);
				//rgb[2] = 1.0f * isPrime(pt.z);
				//rgb[0] = (pt.x%16)/15.0f;
				//rgb[1] = (pt.y%16)/15.0f;
				//rgb[2] = (pt.z%16)/15.0f;}

				m_vVertexColors.push_back(rgba);
			}

			//glEnd();
		}	// for(all triangles)
	}

	//// draw lines
	//switch(m_settings.nDrawEdges) {
	//case 0:
	//	return;

	//case 1:
	//	glColor4f(0, 0, 0, 1);
	//	break;

	//case 2:
	//	glColor4f(1, 1, 1, 1);
	//	break;
	//}

	//glLineWidth(2.0f);
	//glBegin(GL_LINE_LOOP);
	//for(int i = 0; i < 3; ++i) {
	//	Triplet<int> a = tri[i];
	//	glVertex3i(a.x, a.y, a.z);
	//}
	//glEnd();
}

void EuclidDemo::drawVertices()
{
	//double t = m_timer.t();
	// vertex size
	float s = m_settings.fVertexSize;

	//glDepthMask(true);	// write to depth buffer
	glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

	// draw vertex dots
	//for(auto i = m_allTriangles.begin(); i != m_allTriangles.end(); ++i) {
	int i = 0;
	for(auto pt : m_vTriangleVertices)
	{
		/*const TriangleNode &tnode = (i->second);
		const TripletTriangle<int> &tri = tnode.tri;

		double age = t - tnode.m_fStartTime;
		if(age < 0) continue;

		// flash white (high lum, low sat)
		float sat =
			//-cos(5 * t) +
			//2 - 0.5 * cos(11 * t) +
			//1 - 2 * cos(0.4*t - 0.5*tri.depth);
			//0;
			1;
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
		RGBA rgb = { 0,0,0,alpha };
		*/

		//for(int i = 0; i < 3; ++i)
		{
			//const Triplet<int> &pt = tri[i];

			float distsq = (pt.lengthSq());

			const RGBA &rgba = m_vVertexColors[i++];
			//getVertexColor(pt, rgb);
			glColor4fv(rgba);

			glPushMatrix();
			glTranslatef(pt.x, pt.y, pt.z);
			//glTranslatef(s*a.x, s*a.y, s*a.z);
			//float s = 0.3f * sqrt(1.5f*a.sum());

			switch(m_settings.nDrawVertices) {
				case 1:
					// scaling by perspective only
					break;

				case 2:
					// size boost for distant points
					s = m_settings.fVertexSize * 0.1f * logf(sqrt(distsq) + 1.0f);
					break;

				case 3:
					// stronger size boost for distant points
					s = m_settings.fVertexSize * 0.05f * sqrt(sqrt(distsq));
					break;
			}

			glScalef(s, s, s);
			glCallList(LIST_VERTEX);
			glPopMatrix();
		}
	}

}

void EuclidDemo::drawBranches(const TriangleNode &tnode)
{
	const TripletTriangle<int> &tri = tnode.tri;

	double t = m_timer.t();
	double age = t - tnode.m_fStartTime;
	if(age < 0) return;

	auto parentPoint = tnode.parent.centroid();
	if(parentPoint.sum() >= 3) 
	{
		glLineWidth(1.5f);
		glColor4f(1, 1, 1, 1);
		auto centerPoint = tri.centroid();
		glBegin(GL_LINES);
			glVertex3i(centerPoint.x, centerPoint.y, centerPoint.z);
			glVertex3i(parentPoint.x, parentPoint.y, parentPoint.z);
		glEnd();
	}
}

void idleProc() 
{

	if(g_ss.m_readyToDraw && !isSuspended && !checkingPassword) {

		g_ss.draw();
		g_trace = false;
		g_ss.processConsole();
	}
}


void EuclidDemo::reshape() 
{

	int viewport[4];
	viewport[0] = clientRect.left;
	viewport[1] = clientRect.top;
	viewport[2] = clientRect.right - clientRect.left;
	viewport[3] = clientRect.bottom - clientRect.top;

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	m_fAspectRatio = float(viewport[2]) / float(viewport[3]);


}

void EuclidDemo::initSaver(HWND hwnd) 
{
	{
		::runColorUtilTests<float>();
		// new
	}

	TRACE("size: %d %d\n", sizeof(int), sizeof(Triplet<int>));

	m_hwnd = hwnd;

	float a[3] = { -1,10,0 };
	float b[3] = { 1,10,0 };
	float ctr[3] = { 0,0,0 };
	float radius = 10;
	for(float i = -5; i <= 3; ++i) 
	{
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
	//initWindow(hwnd);

	// init GL

	reshape();

	GLint dims[4];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
	GLuint id;
	glBindFramebuffer(1, 1);
	glCheckFramebufferStatus(1);
	glGenFramebuffers(1, &id);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_NORMALIZE);

	// set up front and back buffers
	for(int i = 0; i < 2; ++i) 
	{
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
		//glScalef(1,1,0.1);
		//gluSphere(qobj, 1, 32, 2);
		//glScalef(-1, -1, -1);
		gluDisk(qobj, 0, 1, 24, 1);	// on z=0 plane
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

	m_fLastFPSTime = m_timer.t();
	m_nFrameCount = 0;

	// init and start sim
	TriangleNode seed;
	seed.m_bFertile = true;
	m_growQueue.push(seed);

	randomizeSettings();
}

void EuclidDemo::randomizeSettings() 
{
	m_settings.randomize();

	m_lastSettingsChange = m_timer.t();
}

void EuclidDemo::handleDestroy(HWND hwnd) 
{
	m_readyToDraw = 0;
	::destroyGLContext(hwnd);
}

LONG EuclidDemo::handleKeyDown(
	WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState) 
{
	switch(keyCode) {

	case '0':
	case VK_NUMPAD0:
		m_settings.nDrawVertices = (m_settings.nDrawVertices + 1) % 4;
		return 0;

	case '1':
	case VK_NUMPAD1:
		m_settings.nDrawEdges = (m_settings.nDrawEdges + 1) % 3;
		return 0;

	case '2':
	case VK_NUMPAD2:
		m_settings.bFillTriangles = !m_settings.bFillTriangles;
		return 0;

	case '3':
	case VK_NUMPAD3:
		m_settings.nDrawBranches = (m_settings.nDrawBranches + 1) % 2;
		return 0;

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
		m_settings.bDrawTarget = m_settings.bDrawStats = !m_settings.bDrawTarget;
		return 0;

	case VK_OEM_MINUS:	// '-': zoom out
		m_settings.fFOV = min(180.0, m_settings.fFOV + repeatCount);
		return 0;

	case VK_OEM_PLUS:	// '+': zoom in
		m_settings.fFOV = max(1.0, m_settings.fFOV - repeatCount);
		return 0;

	case VK_DELETE: 
		m_allTriangles.clear();
		m_search.m_growth = GrowthEnum::NONE;

	case VK_HOME: {
		resetGrowthQueue();
		//TriangleNode tnode(m_timer.t());
		//m_allTriangles[tnode.tri] = tnode;
		return 0;
	}

	case 'R':
		randomizeSettings();
		return 0;

	case 'C':
		m_settings.nCameraMode = (CameraMode)((m_settings.nCameraMode + 1) % 4);
		return 0;

	case 'P':
		m_settings.m_bReplaceParent = !m_settings.m_bReplaceParent;
		return 0;

	case 'X':
		targetFind(GrowthEnum::FILL);
		return 0;

	case 'F':
		if(::GetKeyState(VK_SHIFT)) {
			targetFind(GrowthEnum::CIRCLE);
		}
		else {
			targetFind(GrowthEnum::TRIANGLE);
		}
		return 0;

	case 'G':
		targetFind(GrowthEnum::STOCHASTIC);
		return 0;

	case 'H':
		targetFind(GrowthEnum::POINT_SEARCH);
		return 0;

	case 'N':
		if(::GetKeyState(VK_SHIFT)) {
			renewAll(1e10);
			m_settings.TRI_TIME_LIFESPAN = 1e10;
		}
		else {
			renewAll(1e10);
		}
		return 0;

	case 'L':
		m_settings.m_alive = !m_settings.m_alive;
		return 0;

	case 'T':
		g_trace = true;
		return 0;

	case VK_OEM_4:  //  '[': decrease vertex size
		m_settings.fVertexSize *= 0.9f;
		return 0;
		
	case VK_OEM_6:  //  ']': increase vertex size
		m_settings.fVertexSize *= 1.1f;
		return 0;
		
	case VK_OEM_PERIOD:
		m_settings.nSimulationSpeed = min(m_settings.nSimulationSpeed + 2, 25);
		return 0;

	case VK_OEM_COMMA:
		m_settings.nSimulationSpeed = max(1, m_settings.nSimulationSpeed - 2);
		return 0;

	case VK_END   :
	case VK_LEFT  :
		m_search.m_maxDepth = max(1, m_search.m_maxDepth - repeatCount);
		return 0;

	case VK_RIGHT :
		m_search.m_maxDepth = min(10000, m_search.m_maxDepth + repeatCount);
		return 0;

	case VK_UP    :
		m_search.m_nMaxTriangleSum = min(1000, m_search.m_nMaxTriangleSum + repeatCount);
		return 0;

	case VK_DOWN  :
		m_search.m_nMaxTriangleSum = max(3, m_search.m_nMaxTriangleSum - repeatCount);
		return 0;

		break;

	case VK_LWIN:
	case VK_RWIN:
		TRACE("WIN KEY: %d, %d\n", repeatCount, previousKeyState);
		return 0;

	case VK_SPACE:
		m_console.allocate("Euclid's Orchard: Console");
		break;

	default:
		TRACE("KEYDOWN: keycode: 0x%02x scancode: 0x%02x\n", keyCode, scanCode);
	}	// switch(wparam)

	// not handled here--use default behavior
	return 1;
}

LONG screenSaverProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	LONG result;

	switch(msg) {
	case WM_CREATE:
		g_ss.initSaver(hwnd);
		break;

	case WM_MOVE:
		TRACE("MOVE: %d, %d\n", LOWORD(lparam), HIWORD(lparam));
		break;

	case WM_SIZE:
		TRACE("WM_SIZE: %d x %d\n", LOWORD(lparam), HIWORD(lparam));
		// note: ::initWindow destroys the current GL context. so initSaver is necessary to re-initialize it.
		::initGLContext(hwnd);
		g_ss.initSaver(hwnd);
		return 0;	// no default behavior

	case WM_KEYDOWN:
		result = g_ss.handleKeyDown(
			wparam, 
			(unsigned char)(lparam >> 16),				// scan code (bits 16-23)
			(unsigned short)(lparam),			// repeat count (bits 0-15)
			(lparam >> 30) & 1					// previous key state
		);
		if(result == 0) {
			// handled--do not terminate
			return 0;
		}
		//if(windowedSaver) {
		//	// no need to terminate--not running as true screen saver
		//	return 0;
		//}
		break;	// default SS behavior

	case WM_DESTROY:
		g_ss.handleDestroy(hwnd);
		break;
	}	// switch(msg)

	return defScreenSaverProc(hwnd, msg, wparam, lparam);
}

bool EuclidDemo::addTrianglePath(string path)
{
	double t = m_timer.t();
	m_userNode.tri.reset();
	m_userNode.m_fStartTime = t;
	m_userNode.m_fLifespan = m_settings.TRI_TIME_LIFESPAN;
	addTriangleUnique(m_userNode);
	for(char c : path)
	{
		auto parentTriangle = m_userNode.tri;
		if(!m_userNode.tri.operate(c))
		{
			return true;
		}
		m_userNode.parent = parentTriangle;
		addTriangleUnique(m_userNode);
	}
	return true;
}

bool EuclidDemo::executeUserOp(string sz)
{
	// stop all auto growth
	m_search.m_growth = GrowthEnum::NONE;
	m_settings.m_alive = false;

	// if clear, set user node to basis triangle
	if(m_allTriangles.empty()) 
	{
		m_userNode.m_fLifespan = 9999;
		m_userNode.m_fStartTime = m_timer.t();
		//TripletTriangle<int> masterTriangle;
		//TriangleNode master(masterTriangle, m_timer.t(), 9999);// std::numeric_limits<double>::infinity());
		m_allTriangles[m_userNode.tri] = m_userNode;
	}

	if(sz.length() == 1)
	{
		if(sz == "0")
		{
			m_userNode.tri.reset();
			m_userNode.m_fStartTime = m_timer.t();
			addTriangleUnique(m_userNode);
			return true;
		}

		// if preset op code, execute it
		auto parentTriangle = m_userNode.tri;
		if(m_userNode.tri.operate(sz[0]))
		{
			m_userNode.parent = parentTriangle;
			addTriangleUnique(m_userNode);
			return true;
		}

	}

	// if valid column pair, transform
	// commands like "xy" define a single operation
	if(sz.length() == 2)
	{
		unsigned int fromCol = toupper(sz[0]) - 'X';
		unsigned int toCol = toupper(sz[1]) - 'X';
		if(fromCol == toCol || fromCol >= 3 || toCol >= 3)
		{
			return false;
		}

		m_userNode.tri.merge(fromCol, toCol);
		addTriangleUnique(m_userNode);
		return true;
	}

	return false;
}

void EuclidDemo::processConsole()
{
	if(m_console.isAllocated())
	{
		if(m_console.userActivityPending())
		{
			// since getCommand is synchronous, pause the simulation timer to prevent a big jump when app continues
			double t = m_timer.t();
			m_timer.pause();

			std::string sz = m_console.getCommand();

			executeUserCommand(sz);

			m_timer.start();
		}	// if(m_console.userActiviyPending)
	}
}

bool EuclidDemo::executeUserCommand(string sz)
{
	if(!sz.empty())
	{
		std::vector<std::string> asz = split(sz, ' ');

		if(asz.size() == 1)
		{
			// single-word commands

			if(executeUserOp(sz))
			{
				cout << "ok\n";
			}
			else if(sz == "q")
			{
				m_console.free();
			}
			else if(sz == "clear")
			{
				clearQueue(m_allTriangles);
				clearQueue(m_userNode.tri);
				clearQueue(m_growQueue);
			}
			else if(sz == "fill")
			{
				this->targetFind(GrowthEnum::FILL);
			}
			else if(asz[0] == "pal")
			{
				m_settings.randomize();
			}
			else if(asz[0] == "verbose")
			{
				m_search.m_bVerbose = !m_search.m_bVerbose;
				cout << "verbose: " << m_search.m_bVerbose << endl;
			}
			else
			{
				printf("? '%s'\n", sz.c_str());
				printf("OPS: 0=reset, {xyzXYZjklmnoabcABC123}: abcABC=half merges, xyzXYZ=cascade merges, 123=dimensionality collapse\n");
				printf("clear, fill, pal, verbose,\n");
				printf("op {ops}, find {x y z}, look {x y z}, best {x y z}, alpha x\n");
			}
		}
		else
		{
			// multi-word command
			if(asz[0] == "op")
			{
				m_search.m_operations = asz[1];
			}
			else if(asz[0] == "find")
			{
				stringstream str(asz[1]);
				Triplet<int> target;
				str >> target;
				targetFind(GrowthEnum::POINT_SEARCH, target);
			}
			else if(asz[0] == "best")
			{
				for(size_t i = 1; i < asz.size(); ++i)
				{
					stringstream str1(asz[i]);
					Triplet<int> a;
					str1 >> a;
					cout << "BEST: " << a << endl;
					TripletSearch<int> s;
					s.m_operations = m_search.m_operations;
					s.m_bVerbose = m_search.m_bVerbose;
					s.m_maxDepth = 100;
					s.m_maxHeight = 5000;
					s.search(a);
					cout << "Paths found: " << s.m_paths.size()
						<< " MaxDepthCount: " << s.m_countMaxDepth
						<< (s.m_paths.size() > 0 ? string("\nfirst: ") + s.m_paths[0] : "")
						<< "\nbest:  " << s.m_bestPath << endl;
					addTrianglePath(s.m_paths.size() > 0 ? s.m_paths[0] : s.m_bestPath);
				}
			}
			else if(asz[0] == "look")
			{
				Triplet<int> x;
				stringstream str(asz[1]);
				str >> x;
				m_search.m_target = x;
			}
			else if(asz[0] == "alpha")
			{
				sscanf_s(asz[1].c_str(), "%f", &m_settings.fAlpha);
			}
			else
			{
				printf("? '%s'\n", sz.c_str());
			}
		}

	}
	cout << m_userNode.tri;
	return true;
}
