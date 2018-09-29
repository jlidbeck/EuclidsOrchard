#pragma once

#include <afx.h>
#include "resource.h"
#include <Euclid.h>
#include <rsWin32Saver/rsWin32Saver.h>
#include <rsText/rsText.h>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <GL/gl.h>
#include <GL/glu.h>
#include "util.h"
#include <ColorUtil.h>
#include <palette.h>
#include <circular_buffer.h>
#include <ConsoleWindow.h>

using std::vector;

#ifdef _DEBUGq
//#define MAX_NUM_TRIANGLES 1555	// (6^5-1)/5
#define MAX_NUM_TRIANGLES 9331	// (6^6-1)/5
#else
//#define MAX_NUM_TRIANGLES 9331	// (6^6-1)/5
#define MAX_NUM_TRIANGLES 20000
#endif

#define TRI_TIME_REPRODUCE 0.5

typedef struct 
{
	GLfloat v[4];

	operator float*() {
		return v;
	}
	operator const float*() const {
		return v;
	}

} RGBA;

const RGBA WHITE = { 1,1,1,1 };

enum CameraMode 
{
	CENTER=0, TARGET, TOUR, DRIFT
};

// settings relating to the screensaver (not per-simulation-specific)
class EuclidDemoSettings 
{
public:
	int nDrawVertices = 1;
	float fVertexSize = 0.05f;
	int nDrawEdges = 1;
	int nDrawBranches = 0;
	bool bFillTriangles = true;
	bool bDrawTarget = true;
	bool bDrawStats = true;
	float fAlpha = 1;

	CameraMode nCameraMode = CameraMode::TARGET;
	double fFOV = 120.0;
	int multiScreenMode = 0;	// 0 = single drawing window across screens; 1 = individual window per monitor
	int nSimulationSpeed = 3;
	int nRandomizeTimer = 90;	// auto-change every 90 seconds
	int nCameraSpeed = 6;		// about 40 seconds between camera waypoints

	//weighted_palette<rgb> m_palette;
	weighted_palette<COLORREF> m_palette;

	float fRandomPaletteProbability = 0.5;

	double TRI_TIME_LIFESPAN = 30.0;
	int m_nMaxTriangleDepth = 25;
	int m_nMaxTriangleSum = 1000;
	bool m_alive = true;
	bool m_bReplaceParent = true;
	
	void resetToDefaults(int which);
	void randomize();
	void makeRandomPalette(int n);
	void usePresetPalette(int index);
};

class TriangleNode 
{
public:
	TripletTriangle<int> tri;
	double m_fStartTime, m_fLifespan;
	RGBA m_rgba;
	int m_nEvent;
	bool m_bFertile = true;
	TripletTriangle<int> parent;

	TriangleNode(const TripletTriangle<int> &tri, double startTime, double lifespan, int event = 42, bool fertile = true) 
	{
		this->tri = tri;
		m_fStartTime = startTime;
		m_fLifespan = lifespan;
		m_nEvent = event;
		m_bFertile = fertile;
		//setColor();
	}

	// creates basis/identity matrix triangle
	// uses TripletTriangle's default constructor
	TriangleNode(double startTime = 0, double lifespan=1)
	{
		m_fStartTime = startTime;
		m_fLifespan = lifespan;
		m_nEvent = 42;
		m_bFertile = true;
		//setColor();
	}

	inline void computeColor()
	{
		float hsv[3];
		getColorHSV(hsv);
		::HSVtoRGB(hsv, (float*)m_rgba);
	}

	void getColorHSV(float * hsv) const 
	{
		auto ctr = this->tri.centroid();
		hsv[0] = (float)(ctr.x) / ctr.sum() + tri.depth / 33.97f;
		hsv[1] = 0.9f;
		hsv[2] = 1.0f;
	}

	// priority comparison
	int operator < (const TriangleNode &evt) const 
	{ 
		return m_fStartTime < evt.m_fStartTime 
			|| (m_fStartTime == evt.m_fStartTime && tri < evt.tri);
	}

	int operator != (const TriangleNode &tnode) const 
	{
		return !(m_fStartTime == tnode.m_fStartTime
			&& m_nEvent == tnode.m_nEvent
			&& m_fLifespan == tnode.m_fLifespan
			&& tri == tnode.tri);
	}
};

class TNodeStartTimePriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TriangleNode& a, const TriangleNode& b) {
		return (b.m_fStartTime < a.m_fStartTime);
	}
};

class TNodeLowDepthPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TriangleNode& a, const TriangleNode& b) {
		return (a.tri < b.tri);
	}
};

//	for rendering: deep triangles have higher priority and are rendered first
class TNodeHighDepthPriority {
public:
	// returns 1 if b is higher priority than a
	int operator()(const TripletTriangle<int>& a, const TripletTriangle<int>& b) const {
		return (b < a);
	}
};

class TNodeLowSumPriority {
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

class TNodeCentralPriority {
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

interface IScreensaver 
{
	void initSaver(HWND hwnd);
	void handleDestroy(HWND hwnd);
	LONG handleKeyDown(WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState);
	void draw();
};

class EuclidDemo : public IScreensaver 
{
public:
	EuclidDemoSettings m_settings;
	rsTimer m_timer;
	rsText m_text;

private:
	// state
	int m_readyToDraw = 0;
	float m_fAspectRatio;
	double m_lastSettingsChange;

	// random
	std::random_device m_randomDevice { };
	std::mt19937 m_randomGenerator { m_randomDevice() };

	// FPS
	int m_nFrameCount;
	double m_fLastFPSTime;
	double m_fps;

	// components
	HWND m_hwnd;
	ConsoleWindow m_console;

	// Euclid data
	std::map<TripletTriangle<int>, TriangleNode, TNodeHighDepthPriority> m_allTriangles;
	
	// target find
	TripletSearch<int> m_search;
	TriangleNode m_userNode;
	//TripletTriangle<int> m_targetTriangle;
	//Triplet<int> m_targetCenter;
	//float m_targetRadius = 0;
	//char m_ops[40] =
	//	"XxYyZz";	// proper sextant enumeration
	//	//"XxYyZz123";	// complete enumeration
	//	//"uvw";		// thirds enumeration
	//	//"jkl";		// sierpinski (outer 3 quarters)
	//	//"mno";		// inner sixths
	//	//"jklmno";		// outer/inner sixths
	//	//"jklXxYyZz";
	//	//"XYZjkl";
	//	//"Xxyz";

	//enum class GrowthEnum { 
	//	NONE = 0, TRIANGLE = 1, CIRCLE, FILL, POINT_SEARCH, STOCHASTIC 
	//};
	//GrowthEnum m_growth = GrowthEnum::STOCHASTIC;
	//static std::map<GrowthEnum, string> s_growth_enum_names;
	
	//std::queue<TriangleNode>
	//std::priority_queue<TriangleNode, circular_buffer<TriangleNode, MAX_NUM_TRIANGLES>, TNodeLowSumPriority>
	//std::priority_queue<TriangleNode, vector<TriangleNode>, TNodeLowSumPriority >
	//std::priority_queue<TriangleNode, vector<TriangleNode>, TNodeLowDepthPriority >
	std::priority_queue<TriangleNode, vector<TriangleNode>, TNodeStartTimePriority >
	//std::priority_queue<TriangleNode, deque<TriangleNode>, TNodeLowSumPriority > 
		m_growQueue;

	int countMaxDepth = 0;
	int countMaxSum = 0;
	int countCollision = 0;

	//bool verbose = true;

	// GL data
private:
	vector<Triplet<GLint>> m_vTriangleVertices;
	vector<RGBA> m_vTriangleColors;
	vector<RGBA> m_vVertexColors;
	GLuint LIST_VERTEX = -1;
	GLuint LIST_TARGET = -1;

public:
	EuclidDemo();

	void draw();
	void randomizeSettings();
	void reshape();

	void initSaver(HWND hwnd);

	LONG handleKeyDown(WPARAM keyCode, unsigned char scanCode, unsigned short repeatCount, unsigned char previousKeyState);
	void handleDestroy(HWND hwnd);

	// rs hooks
	friend void idleProc();

	void processConsole();
	bool executeUserCommand(string cmd);
	bool executeUserOp(string op);
	bool addTrianglePath(string path);

private:
	void drawViewport(float x0, float y0, float x1, float y1);
	void drawTriangles();
	//void drawTriangle(const TriangleNode &evt);

	void drawVertices();
	//void drawVertices(const TriangleNode &evt);
	void drawBranches(const TriangleNode &evt);

	void step();

	void renewAll(double fExtendLifespan);
	void targetFind(GrowthEnum shape);
	void targetFind(GrowthEnum shape, const Triplet<int> &target);
	void createRandomTarget();
	bool processGrowthQueue(int count);
	void resetGrowthQueue();
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

