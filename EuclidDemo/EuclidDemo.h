#pragma once

#include "resource.h"
#include <ColorUtil.h>
#include <rsWin32Saver/rsWin32Saver.h>
#include <rsText/rsText.h>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "palette.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "util.h"
#include "circular_buffer.h"
#include <Euclid.h>
#include "ConsoleWindow.h"

using std::vector;

// settings relating to the screensaver (not per-simulation-specific)
class EuclidDemoSettings {
public:
	int m_nDrawVertices = 1;
	float m_fVertexSize = 0.05f;
	int m_nDrawEdges = 1;
	bool m_bDrawTriangles = true;
	bool m_bDrawTarget = true;
	bool m_bDrawStats = true;

	int m_nCameraMode = 3;
	double m_fFOV = 120.0;
	int multiScreenMode = 0;	// 0 = single drawing window across screens; 1 = individual window per monitor
	int nSimulationSpeed = 3;
	int nRandomizeTimer = 90;	// auto-change every 90 seconds
	int nCameraSpeed = 6;		// about 40 seconds between camera waypoints

	weighted_palette<COLORREF> m_palette;

	float m_fRandomPaletteProbability = 0.5;

	int m_nMaxTriangleDepth = 25;
	int m_nMaxTriangleSum = 1000;
	bool m_alive = true;
	
	void resetToDefaults(int which);
	void randomize();
	void makeRandomPalette(int n);
	void usePresetPalette(int index);
};

#ifdef _DEBUG
#define MAX_NUM_TRIANGLES 1555	// (6^5-1)/5
#else
//#define MAX_NUM_TRIANGLES 9331	// (6^6-1)/5
#define MAX_NUM_TRIANGLES 20000
#endif

#define TRI_TIME_REPRODUCE 0.35
#define TRI_TIME_LIFESPAN 19.5

class TriangleNode {
public:
	TripletTriangle<int> tri;
	double m_fStartTime, m_fLifespan;
	float m_rgba[4];
	int m_nEvent;
	bool m_bFertile = true;
	TripletTriangle<int> parent;

	TriangleNode(const TripletTriangle<int> &tri, double startTime, double lifespan = TRI_TIME_LIFESPAN, int event = 42, bool fertile = true) {
		this->tri = tri;
		m_fStartTime = startTime;
		m_fLifespan = lifespan;
		m_nEvent = event;
		m_bFertile = fertile;
		//setColor();
	}

	// creates basis/identity matrix triangle
	// uses TripletTriangle's default constructor
	TriangleNode(double startTime = 0, double lifespan = TRI_TIME_LIFESPAN) {
		m_fStartTime = startTime;
		m_fLifespan = lifespan;
		m_nEvent = 42;
		m_bFertile = true;
		//setColor();
	}

	void getColor(float * rgb) const {
		auto ctr = this->tri.centroid();
		float hue = (float)(ctr.x) / ctr.sum() + tri.depth / 33.97;
		hsl2rgb(hue, 0.9f, 1.0f, rgb);
	}

	// priority comparison
	int operator < (const TriangleNode &evt) const { 
		return m_fStartTime < evt.m_fStartTime 
			|| (m_fStartTime == evt.m_fStartTime && tri < evt.tri);
	}

	int operator != (const TriangleNode &tnode) const {
		return !(m_fStartTime == tnode.m_fStartTime
			&& m_nEvent == tnode.m_nEvent
			&& m_fLifespan == tnode.m_fLifespan
			&& tri == tnode.tri);
	}
};

class TripletTriangleLowDepthPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TriangleNode& a, const TriangleNode& b) {
		return (a.tri < b.tri);
	}
};

//	for rendering: deep triangles have higher priority and are rendered first
class TripletTriangleHighDepthPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TripletTriangle<int>& a, const TripletTriangle<int>& b) const {
		return (b < a);
	}
};

class TripletTriangleLowSumPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TriangleNode& a, const TriangleNode& b) {
		auto ca = a.tri.centroid();
		auto cb = b.tri.centroid();
		int sa = ca.sum();
		int sb = cb.sum();
		int diff = sa - sb;
		if(diff < 0) {
			// a is higher priority
			return 0;
		}
		if(diff > 0) {
			// b is higher priority
			return 1;
		}

		return (a.tri < b.tri);
	}
};

class TripletTriangleCentralPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TriangleNode& a, const TriangleNode& b) {

		// projected distance from 1/3,1/3,1/3
		auto ca = a.tri.centroid();
		auto cb = b.tri.centroid();
		int sa = ca.sum();
		int sb = cb.sum();
		//int diff = sa - sb;
		//int diff = (int)a.tri.depth - (int)b.tri.depth;
		int diff = (ca.lengthSq() * (sb*sb) - cb.lengthSq() * (sa*sa));
		//int diff = ca.x*sb - cb.x*sa;
		if(diff < 0) {	// A is more central
			// a is higher priority
			return 0;
		}
		if(diff > 0) {
			// b is higher priority
			return 1;
		}

		return (a.tri < b.tri);
	}
};

class TripletTriangleHasher {
public:
	size_t operator()(const TripletTriangle<int>& tri) const {
		size_t f = 1;
		size_t h = 0;
		for(int i = 0; i < 3; ++i) {
			h += (f * tri[i].x);
			f *= 3;
			h += (f * tri[i].y);
			f *= 4;
			h += (f * tri[i].z);
			f *= 3;
		}
		return h;
	}

	size_t operator()(const TriangleNode& evt) const {
		return this->operator()(evt.tri);
	}
};

interface IScreensaver {
	void initSaver(HWND hwnd);
	void handleDestroy(HWND hwnd);
	LONG handleKeyDown(WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState);
	void draw();
};

class EuclidDemo : public IScreensaver {

	// state
	int m_readyToDraw = 0;
	float m_aspectRatio;
	double m_lastSettingsChange;
	HWND m_hwnd;
	ConsoleWindow m_console;

	// OpenGL objects
	rsTimer m_timer;
	rsText m_text;

	// Euclid data
	std::map<TripletTriangle<int>, TriangleNode, TripletTriangleHighDepthPriority> m_allTriangles;
	
	// target find
	TriangleNode m_userNode;
	TripletTriangle<int> m_targetTriangle;
	Triplet<int> m_targetCenter;
	float m_targetRadius = 0;
	char m_ops[40] =
		// "XxYyZz";			//"jklXxYyZz";
		//"XYZjkl";
		//"jkl";	// sierpinski
		//"jklmno";	// outer/inner sixths
		"mno";	// inner sixths
		//"Xxyz";
	typedef enum { SHAPE_NONE = 0, SHAPE_TRIANGLE = 1, SHAPE_CIRCLE, SHAPE_FILL, SHAPE_POINT_SEARCH, SHAPE_GROW } growth_enum;
	growth_enum m_targetShape = SHAPE_GROW;
	static std::map<growth_enum, string> s_growth_enum_names;
	
	//std::queue<TriangleNode>
	//std::priority_queue<TriangleNode, circular_buffer<TriangleNode, MAX_NUM_TRIANGLES>, TripletTriangleLowSumPriority>
	//std::priority_queue<TriangleNode, vector<TriangleNode>, TripletTriangleLowSumPriority >
	std::priority_queue<TriangleNode, vector<TriangleNode>, TripletTriangleLowDepthPriority > 
	//std::priority_queue<TriangleNode, deque<TriangleNode>, TripletTriangleLowSumPriority > 
		m_growQueue;

	int countMaxDepth = 0;
	int countMaxSum = 0;
	int countCollision = 0;

public:
	EuclidDemoSettings m_settings;

private:	
	GLuint LIST_VERTEX = -1;
	GLuint LIST_TARGET = -1;

public:
	void draw();
	void changeSettings();
	void reshape();

	void initSaver(HWND hwnd);

	LONG handleKeyDown(WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState);
	void handleDestroy(HWND hwnd);

	// rs hooks
	friend void idleProc();

	void processConsole();

private:
	void drawViewport(float x0, float y0, float x1, float y1);
	void drawTriangle(const TriangleNode &evt);
	void drawVertices(const TriangleNode &evt);

	void step();

	void renewAll();
	void targetFind(growth_enum shape);
	bool processGrowthQueue(int count);
	void resetGrowthQueue();
	//void triangleFill();
	int addTriangleUnique(const TripletTriangle<int> &tri);
	int addTriangleUnique(const TriangleNode &evt);
};

class CSettingsDialog {
public:
	HWND hdlg;
	EuclidDemoSettings m_settings;

	CSettingsDialog() : hdlg(NULL) { }

	void initSettingsDialog(HWND hdlg, const EuclidDemoSettings& ssSettings);
	BOOL handleMessage(HWND hdlg, UINT msg, WPARAM wpm, LPARAM lpm);
	void readDialogSettings();
};

extern CSettingsDialog g_settingsDialog;

void readRegistry(EuclidDemoSettings& settings);
void writeRegistry(const EuclidDemoSettings& settings);

