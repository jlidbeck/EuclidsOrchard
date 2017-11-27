#pragma once

#include <stdio.h>
#include <iostream>
#include <sstream>
#define _AFXDLL 1
#include <afx.h>
#include <queue>
#include <vector>
#include <string>
#include <map>

using namespace std;

//
//	Utils
//

template<class C>
int sign(C v) {
	return (v < 0 ? -1 : (v ? 1 : 0));
}

template<class C>
char signchar(C v) {
	return (v < 0 ? '-' : (v ? '+' : '0'));
}

// Euclid's GCD algorithm
template<class I>
I gcd(I m, I n) {
	while(1) {
		I remainder;
		if(m < n) {
			if(m == 0) return n;
			remainder = n%m;
			if(0 == remainder) return m;
		}
		else {
			if(n == 0) return m;
			remainder = m%n;
			if(0 == remainder) return n;
			m = n;
		}
		n = remainder;
	}
}

// modified euclid..
// gcd(x,y,0) == gcd(x,y)
inline int gcd2(const int *pValues, int numValues) {
	vector<int> p(pValues, pValues + numValues);
	int i1, v1, i2, v2;

	bool changes = true;
	while(changes) {
		changes = false;

		std::sort(p.begin(), p.end());
		
		// replace all values with remainder(value, next smallest value)
		for(i2 = numValues - 1; i2 > 0;) {
			v2 = p[i2];
			for(i1 = i2 - 1; i1 >= 0 && (v1 = p[i1]) == v2; --i1);
			if(i1 < 0) {
				break;
			}

			// replace all v2 with v2%v1
			if(!v1) {
				break;
			}

			int newv2 = v2%v1;
			for(int j = i2; j > i1; --j) {
				p[j] = newv2;
			}
			changes = true;
			i2 = i1;
		}
	}
	return v2;
}

// brute-force method--for checking only
// gcd(x,y,0) == gcd(x,y)
inline int gcd(const int *pValues, int numValues) {
	int test = ::gcd2(pValues, numValues);

	// set k to min positive value
	int k = pValues[0];
	for(int i = 1; i < numValues; ++i) {
		if(!k || (pValues[i] && pValues[i] < k)) {
			k = pValues[i];
		}
	}

	// count k down to 2, breaking on first common factor found
	for(; k > 1; --k) {
		int i;
		for(i = 0; i < numValues; ++i) {
			if(pValues[i] % k != 0) {
				// k is not a common factor
				break;
			}
		}

		if(i == numValues) {
			// all values divided k
			return k;
		}
	}

	return k;
}

//
//	Coprime triplet of integers
//	Coprimality is not strictly enforced, but can be validated using isCoprime()
//
//	X,Y,Z can be considered a point in 3D space for some operations (addition, lengthSq)
//	but for other operations, such as the leftOf... and triangle operations, it makes more sense 
//	to imagine them projected onto a plane with the observer at (0,0,0).
//
//	While a planar projection isn't required by the underlying math, it does simplify things
//	to project the points onto the X+Y+Z=1 plane by dividing each of x, y, and z by the sum x+y+z.
//	This preserves collinearity as well as all angles from the origin.
//
//	To simplify the math even further, without loss of collinearity or the sign of angles,
//	we can then project onto the XY plane:
//	Each point (x,y,z) is mapped to (x/(x+y+z), y/(x+y+z)). This projects the limit points 
//	[1,0,0], [0,1,0], [0,0,1] to (1,0), (0,1), and the origin.
//
//	Another more aesthetic option is an equilateral triangle: ((z+0.5*y)/(x+y+z), (sqrt(3)/2*y)/(x+y+z))

template<class I>
class Triplet {
public:
	I x, y, z;
	static char _buf[4];

	Triplet()
		:x(1), y(1), z(1) {}

	Triplet(I px, I py, I pz)
		: x(px), y(py), z(pz) {}

	void set(I px, I py, I pz) {
		ASSERT(px || py || pz);
		x = px; y = py; z = pz;
	}

	inline I sum() const { return x + y + z; }
	I lengthSq() const { return x*x + y*y + z*z; }

	// returns true if this a coprime triplet, that is, GCD(x,y,z) == 1
	int isCoprime() const {
		ASSERT(::gcd(x, ::gcd(y, z)) == ::gcd(&x, 3));
		return (::gcd(x, ::gcd(y, z)) == 1);
	}

	inline void operator += (const Triplet &p) {
		x += p.x;
		y += p.y;
		z += p.z;
	}

	inline Triplet operator + (const Triplet &b) const {
		return Triplet(x + b.x, y + b.y, z + b.z);
	}

	inline bool operator == (const Triplet &p) const {
		return (x == p.x && y == p.y && z == p.z);
	}

	// returns 1 if {this} is lower priority than {p2}.
	//
	// for priority queues, or map keys.
	// operator < is strict, as it is also used for equality testing.
	// that is, if x < y, it's guaranteed that !(y<x).
	// if !(x<y) and !(y<x), then x==y.
	// This ordering assigns lower values to triplets with smaller magnitudes,
	// and if lengths are equal, to triplets that are more X- and Y- oriented.
	// so [1,2,3] < [1,3,2]
	int operator < (const Triplet &p2) const {

		// further the origin means *lower* priority
		I ldist = sum(), rdist = p2.sum();
		if(ldist > rdist) {
			return 1;
		}
		if(ldist < rdist) {
			return 0;
		}

		// to establish a strict ordering, (arbitrarily) give higher priority 
		// to the more x- or y- oriented triplet, so [1,0,0] < [0,1,0] < [0,0,1]
		if(x > p2.x) return 1;
		if(x < p2.x) return 0;
		if(y > p2.y) return 1;
		if(y < p2.y) return 0;
		if(z > p2.z) return 1;
		return 0;
	}

	// returns true if all this's elements are lte p's elements,
	// indicating this could possibly be a predecessor
	bool precedes(const Triplet &p) const {
		return (x <= p.x && y <= p.y && z <= p.z);
	}

};

//	returns sign of AB cross AC.
//	this indicates whether point C lies to the left of segment AB,
//	equivalently, whether A is left of BC, or B is left of CA.
//	this can be interpreted as the sign of the triangle ABC: 
//	1 indicates the triangle ABC is left-winding;
//	-1 indicates the triangle ABC is right-winding;
//	0 indicates A,B,C are collinear.
//
//	Note that only the sign is computed. This reduces the issue of integer overflow
//	and also avoids extra operations.
template<class I>
int leftOf(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &c) {
	// to expand the domain of this function, we take advantage of the fact 
	// that only the sign needs to be computed. 
	// otherwise the actual dot/cross calculations would require dividing by the sums--
	// here we only need to calculate their signs and multiply them.
	int abm = sign(c.sum()) * sign(a.sum()) * sign(b.sum());
	return abm * sign(
		  a.x * (b.y*c.z - b.z*c.y)
		+ a.y * (b.z*c.x - b.x*c.z)
		+ a.z * (b.x*c.y - b.y*c.x));
}

template<class I>
ostream& operator << (ostream &out, const Triplet<I> &p) {
	out << "[" << p.x << ", " << p.y << ", " << p.z << "]";
	return out;
}

template<class I>
istream& operator >> (istream &in, Triplet<I> &p) {
	char c = ',';
	in >> p.x >> c >> p.y >> c >> p.z;
	return in;
}

template<class I>
char Triplet<I>::_buf[] = "***";

//
//	3x3 integer matrix
//

template<class I>
class TripletTriangle {
	Triplet<I> m[3] = { { 1,0,0 },{ 0,1,0 },{ 0,0,1 } };
public:
	unsigned int depth = 0;

	TripletTriangle() {}

	TripletTriangle(const Triplet<I> &p0, const Triplet<I> &p1, const Triplet<I> &p2) {
		m[0] = p0;
		m[1] = p1;
		m[2] = p2;
	}

	TripletTriangle(char op, const TripletTriangle<I> &src) {
		operate(op, src);
	}

	void set(const Triplet<I> &p0, const Triplet<I> &p1, const Triplet<I> &p2) {
		m[0] = p0;
		m[1] = p1;
		m[2] = p2;
	}

	// fetch column / triplet
	const Triplet<I>& operator[](int idx) const {
		ASSERT(idx >= 0 && idx < 3);
		return m[idx];
	}

	// fetch column / triplet
	Triplet<I>& operator[](int idx) {
		ASSERT(idx >= 0 && idx < 3);
		return m[idx];
	}

	I determinant() const {
		return
			  m[0].x * (m[1].y * m[2].z - m[1].z * m[2].y)
			+ m[0].y * (m[1].z * m[2].x - m[1].x * m[2].z)
			+ m[0].z * (m[1].x * m[2].y - m[1].y * m[2].x);
	}

	Triplet<I> centroid() const {
		return m[0] + m[1] + m[2];
	}

	//	Check whether all 3 columns are coprime and the row sums are coprime.
	bool isCoprime() const {

		// can this be replaced with a simple check that the determinant is 1? "unimodular"

		if(!m[0].isCoprime() || !m[1].isCoprime() || !m[2].isCoprime()) {
			return false;
		}
		if(!centroid().isCoprime()) {
			return false;
		}

		ASSERT(determinant() == 1);
		return true;
	}

	//	matrix forms proper, non-collinear, left-winding triangle
	inline bool isTriangle() const {
		return leftOf(m[0], m[1], m[2]) == 1;
	}

	/*/	DEPRECATE
	// returns true if the given point is inside this triangle.
	// points on the edge of the triangle are considered to be inside.
	bool isPointInTriangle1(const Triplet<I> &pt) const {

		// compute the point's side relative to the three triangle sides.
		// here we're computing AB x AP, BC x BP, and CA x CP.
		// a simple 2D projection is used: the point [Ax, Ay, Az] is mapped to 
		// (Ax / (Ax+Ay+Az), Ay / (Ax+Ay+Az)).
		// This projection preserves straight line properties... there's probably
		// a mathematically simpler way to do this without projection.

		// Any negative cross product means the point lies outside the triangle.
		// If none are negative, a zero cross product means the point lies on one of the edges.

		// 2d projections
		I psum = pt.sum(), asum = m[0].sum(), bsum = m[1].sum(), csum = m[2].sum();
		// double ax = m[0].x / asum, ay = m[0].y / asum;
		// double bx = m[1].x / bsum, by = m[1].y / bsum;
		// double cx = m[2].x / csum, cy = m[2].y / csum;
		// double px = pt.x / psum, py = pt.y / psum;
		I ax = m[0].x, ay = m[0].y;
		I bx = m[1].x, by = m[1].y;
		I cx = m[2].x, cy = m[2].y;
		I px = pt.x, py = pt.y;
		// cross products
		I abs = (asum*bx - bsum*ax)*(asum*py - psum*ay) - (asum*by - bsum*ay)*(asum*px - psum*ax);
		I bcs = (bsum*cx - csum*bx)*(bsum*py - psum*by) - (bsum*cy - csum*by)*(bsum*px - psum*bx);
		I cas = (csum*ax - asum*cx)*(csum*py - psum*cy) - (csum*ay - asum*cy)*(csum*px - psum*cx);

		//cerr << "-- " << pt << " AB: " << sign(abs) << " BC: " << sign(bcs) << " CA: " << sign(cas) << endl;

		return (abs >= 0 && bcs >= 0 && cas >= 0);
	}//*/

	bool isPointInTriangle(const Triplet<I> &pt) const {
		return leftOf(pt, m[0], m[1]) >= 0
			&& leftOf(pt, m[1], m[2]) >= 0
			&& leftOf(pt, m[2], m[0]) >= 0;
	}

	//	returns true if {tri} is contained entirely within this triangle.
	//	edges and verti
	bool isTriangleInTriangle(const TripletTriangle<I> &tri) const {
		return isPointInTriangle(tri[0])
			&& isPointInTriangle(tri[1])
			&& isPointInTriangle(tri[2]);
	}

	//
	//	merge transforms
	//	these preserve the coprime triangle properties:
	//	- determinant = 1
	//	- rows, columns, row sums are all coprime triples
	//	- columns form left-winding triangle
	//

	inline void merge(int fromCol, int toCol) {
		ASSERT(fromCol != toCol);
		m[toCol] += m[fromCol];
		++depth;
		ASSERT(isCoprime());
		ASSERT(isTriangle());
		ASSERT(determinant() == 1);
	}

	inline void mergeAll(int toCol) {
		depth += 2;
		m[toCol] = centroid();
	}

	// replaces this triangle with one of its sextants.
	// - vertex[primary] is preserved; 
	// - vertex[secondary] moves to primary+secondary midpoint;
	// - remaining vertex moves to centroid.
	//
	// same as merge(p, s); merge(s, t);
	inline void merge3(int primary, int secondary) {
		// before: x, y, z
		// after: x, x+y, x+y+z
		merge(primary, secondary);
		merge(secondary, 3 - primary - secondary);
	}

	inline bool operate(char op, const TripletTriangle<I> &src) {
		*this = src;
		return operate(op);
	}

	bool operate(char op) {
		switch(op) {
		case 'a': this->merge(0, 1); break;
		case 'b': this->merge(1, 2); break;
		case 'c': this->merge(2, 0); break;
		case 'A': this->merge(1, 0); break;
		case 'B': this->merge(2, 1); break;
		case 'C': this->merge(0, 2); break;
		case 'x': this->merge3(0, 1); break;
		case 'y': this->merge3(1, 2); break;
		case 'z': this->merge3(2, 0); break;
		case 'X': this->merge3(0, 2); break;
		case 'Y': this->merge3(1, 0); break;
		case 'Z': this->merge3(2, 1); break;
		case '1': this->mergeAll(0); break;
		case '2': this->mergeAll(1); break;
		case '3': this->mergeAll(2); break;
		default:
			return false;
		}

		return true;
	}

	// returns sextant signature of point.
	// characterizes point as left of (-1), right of (+1), or on (0)
	// each of the 3 median lines.
	int getSextant(const Triplet<I> &pt) const {

		// compute the point's side relative to the three triangle medians.
		//
		// here we're computing AB x AP, BC x BP, and CA x CP.
		// a simple 2D projection is used: the point [Ax, Ay, Az] is mapped to 
		// (Ax / (Ax+Ay+Az), Ay / (Ax+Ay+Az)).
		// This projection preserves straight line properties... there's probably
		// a mathematically simpler way to do this without projection.

		// centroid
		Triplet<I> ctr = this->centroid();

		// reciprocals for 2d projections
		I psum = pt.sum(), asum = m[0].sum(), bsum = m[1].sum(), csum = m[2].sum(), msum = ctr.sum();

		// project to (x/(x+y+z), y/(x+y+z))
		// division is delayed, so these values are scaled by ?sum
		I ax = m[0].x, ay = m[0].y;
		I bx = m[1].x, by = m[1].y;
		I cx = m[2].x, cy = m[2].y;
		I px = pt.x, py = pt.y;
		I mx = ctr.x, my = ctr.y;
		I temp;

		// calculate cross-products to characterize P by edges MA, MB, MC
		temp = (msum*ax - asum*mx)*(msum*py - psum*my) - (msum*ay - asum*my)*(msum*px - psum*mx);
		// ((mx+my+mz)*ax-(ax+ay+az)*mx)*((mx+my+mz)*py-(px+py+pz)*my) - ((mx+my+mz)*ay-(ax+ay+az)*my)*((mx+my+mz)*px-(px+py+pz)*mx)
		// ((my+mz)*ax-(ay+az)*mx)*((mx+mz)*py-(px+pz)*my) - ((mx+mz)*ay-(ax+az)*my)*((my+mz)*px-(py+pz)*mx)
		// ((my+mz)*ax)*((mx+mz)*py-(px+pz)*my) - ((ay+az)*mx)*((mx+mz)*py-(px+pz)*my) - ((mx+mz)*ay)*((my+mz)*px-(py+pz)*mx) + ((ax+az)*my)*((my+mz)*px-(py+pz)*mx)
		// (my+mz)*ax*(mx+mz)*py - (my+mz)*ax*(px+pz)*my - (ay+az)*mx*(mx+mz)*py - (ay+az)*mx*(px+pz)*my - (mx+mz)*ay*(my+mz)*px - (mx+mz)*ay*(py+pz)*mx + (ax+az)*my*(my+mz)*px - (ax+az)*my*(py+pz)*mx
		//  - (mx+my+mz)*mx*py*az 
		//  - (mx + my + mz)*mx*pz*ay
		//	- (mx + my + mz)*my*pz*ax
		//	+ (-mx + my + mz)*my*px*az
		//	+ (mx + my + mz)*mz*py*ax
		//	- (mx + my + mz)*mz*px*ay
		//	- 2 * mx*mx*py*ay
		//	- 2 * mz*mx*py*ay
		//	- 2 * mx*my*pz*az
		//	- 2 * mx*my*px*ay
		int maxmp = leftOf(ctr, m[0], pt);
		ASSERT(sign(temp) == maxmp);

		temp = (msum*bx - bsum*mx)*(msum*py - psum*my) - (msum*by - bsum*my)*(msum*px - psum*mx);
		int mbxmp = leftOf(ctr, m[1], pt);
		ASSERT(sign(temp) == mbxmp);

		temp = (msum*cx - csum*mx)*(msum*py - psum*my) - (msum*cy - csum*my)*(msum*px - psum*mx);
		int mcxmp = leftOf(ctr, m[2], pt);
		ASSERT(sign(temp) == mcxmp);

		//cerr << " {" << signchar(maxmp) << signchar(mbxmp) << signchar(mcxmp) << "}\n";

		char sex[4] = { 0,0,0,0 };
		sex[0] = signchar(maxmp);
		sex[1] = signchar(mbxmp);
		sex[2] = signchar(mcxmp);
		return *((int*)(sex));
	}

	// matrix functions..

	TripletTriangle transpose() const {
		TripletTriangle tri(*this);
		tri.m[0].y = m[1].x;
		tri.m[0].z = m[2].x;
		tri.m[1].x = m[0].y;
		tri.m[1].z = m[2].y;
		tri.m[2].x = m[0].z;
		tri.m[2].y = m[1].z;
		return tri;
	}

	void transpose(const TripletTriangle &tri) {
		*this = tri;
		m[0].y = tri.m[1].x;
		m[0].z = tri.m[2].x;
		m[1].x = tri.m[0].y;
		m[1].z = tri.m[2].y;
		m[2].x = tri.m[0].z;
		m[2].y = tri.m[1].z;
	}

	// comparators

	int operator == (const TripletTriangle &tri) const {

		return m[0] == tri.m[0] && m[1] == tri.m[1] && m[2] == tri.m[2];
	}

	// returns 1 if {this} is lower priority than {tri}.
	//
	// for priority queues, or map keys.
	// operator < is strict, as it is also used for equality testing.
	// that is, if x < y, it's guaranteed that !(y<x).
	// if !(x<y) and !(y<x), then x==y.
	// This ordering assigns lower values to ...
	int operator < (const TripletTriangle &tri) const {

		// smaller depth means higher priority
		if(depth < tri.depth) {
			return 0;
		}
		if(tri.depth < depth) {
			return 1;	// this is lower priority than tri
		}

		auto l = this->centroid();
		auto r = tri.centroid();
		if(l < r) return 1;
		if(r < l) return 0;
		for(int i = 0; i < 3; ++i) {
			if(m[i] < tri.m[i]) return 1;
			if(tri.m[i] < m[i]) return 0;
		}
		// they are equal
		return 0;
	}
};		// class TripletTriangle

// returns true if the two segments intersect, including endpoint intersections
template<class I>
bool segmentsIntersect(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &c, const Triplet<I> &d) {
	int abc = leftOf(a, b, c);
	int abd = leftOf(a, b, d);
	if(abc*abd == 1) {
		// both positive:
		// C and D lie on the same side of line AB -- no intersection
		return false;
	}

	// C and D lie on opposite sides of AB
	// now characterize A and B relative to CD

	int cda = leftOf(c, d, a);
	int cdb = leftOf(c, d, b);

	if(cda*cdb == 1) {
		// both positive or both negative:
		// A and B lie on the same side of line CD
		return false;
	}

	// C and D lie on opposite sides of AB, and A and B lie on opposite sides of CD
	return true;
}

template<class I>
bool trianglesIntersect(const TripletTriangle<I> &a, const TripletTriangle<I> &b) {
	// if any vertex is inside the other triangle, the triangles overlap

	if(    b.isPointInTriangle(a[0])
		|| b.isPointInTriangle(a[1])
		|| b.isPointInTriangle(a[2])
		|| a.isPointInTriangle(b[0])
		|| a.isPointInTriangle(b[1])
		|| a.isPointInTriangle(b[2])) {
		return true;
	}

	// check for edge intersections

	for(int i = 0; i < 3; ++i) {
		for(int j = 0; j < 3; ++j) {
			if(segmentsIntersect(a[i], a[(i + 1) % 3], b[j], b[(j + 1) % 3])) {
				return true;
			}
		}
	}

	return false;
}

// actually a line-cone intersect: does the line thru point and origin
// intersect the cone whose axis is the line thru ctr and origin,
// and whose radius is {radius} at its intersection with the plane x+y+z=1
// radius
template<class I>
bool pointInCircle(const Triplet<I> &point, const Triplet<I> &ctr, float radius) {
	// project to x+y+z=1
	float ctrSum = ctr.sum();
	float vCtr[3] = { ctr.x / ctrSum, ctr.y / ctrSum, ctr.z / ctrSum };

	float pointSum = point.sum();
	float v[3] = { 
		point.x / pointSum - vCtr[0], 
		point.y / pointSum - vCtr[1], 
		point.z / pointSum - vCtr[2] };
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] <= (float)radius * radius);
}

template<class F>
inline F dot3(const F *a, const F *b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template<class F>
inline F sumsq3(const F *a) {
	return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
}

template<class F>
inline F distsq3(const F *a, const F *b) {
	F ab[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
	return sumsq3(ab);
}

template<class F>
bool lineSegmentIntersectsCircle(const F *a, const F *b, const F *ctr, F radius) {

	F rsq = radius * radius;
	
	// test both endpoints
	F ap[3] = { ctr[0] - a[0], ctr[1] - a[1], ctr[2] - a[2] };
	if(sumsq3(ap) <= rsq) {
		return true;
	}

	F bp[3] = { ctr[0] - b[0], ctr[1] - b[1], ctr[2] - b[2] };
	if(sumsq3(bp) <= rsq) {
		return true;
	}

	F ab[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
	F abls = sumsq3(ab);

	// distsq(t) = t^2(|AB|^2) + 2t(AB dot PA) + |PA|^2
	// distsq'(t) = 2t(|AB|^2) + 2(AB dot PA)
	// min at t=(AB dot PA) / |AB|^2
	F abdotap = dot3(ab, ap);
	F t = abdotap / abls;

	if(t < 0 || t > 1) {
		return false;
	}

	// project ctr onto AB
	float c[3] = { a[0] + t*ab[0], a[1] + t*ab[1], a[2] + t*ab[2] };
	return (distsq3(c, ctr) <= rsq);
}

template<class I>
bool lineSegmentIntersectsCircle(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &ctr, float radius) {
	// distsq = |ta + (1-t)b - ctr|^2
	// = (t(a-b) + b - ctr)^2
	// = sum[.=x,y,z] t^2 (a-b).^2 + 2t (a-b).(b-ctr). + (b-ctr).^2
	// distsq' = 2t(a-b). + 2(a-b).(b-ctr).
	// closest point at t = (b-a)(ctr-b) / (b-a)
	// min distsq = ((b-a)(b-ctr) + b-ctr)^2

	// point-to-line distance: AB x AP / |AB|
	// project to x+y+z=1
	float pSum = ctr.sum();
	float vp[3] = { ctr.x / pSum, ctr.y / pSum, ctr.z / pSum };
	float aSum = a.sum();
	float va[3] = { a.x / aSum, a.y / aSum, a.z / aSum };
	float bSum = b.sum();
	float vb[3] = { b.x / bSum, b.y / bSum, b.z / bSum };

	return lineSegmentIntersectsCircle(va, vb, vp, radius);
}

template<class I>
bool triangleIntersectsCircle(const TripletTriangle<I> &tri, const Triplet<I> &ctr, float radius) {
	// if any vertex is inside the circle
	//if(    pointInCircle(tri[0], ctr, radius)
	//	|| pointInCircle(tri[1], ctr, radius)
	//	|| pointInCircle(tri[2], ctr, radius)) {
	//	return true;
	//}

	// if circle's center is inside triangle, they intersect
	if(tri.isPointInTriangle(ctr)) {
		return true;
	}

	// if any triangle vertex lies in circle or any edge intersects circle..
	if(    lineSegmentIntersectsCircle(tri[0], tri[1], ctr, radius)
		|| lineSegmentIntersectsCircle(tri[1], tri[2], ctr, radius)
		|| lineSegmentIntersectsCircle(tri[2], tri[0], ctr, radius)) {
		return true;
	}

	return false;
}

template<class I>
ostream& operator << (ostream &out, const TripletTriangle<I> &m) {
	Triplet<I> merge = m.centroid();

	bool good = m.isCoprime();
	char buf[80];

	sprintf_s(buf, "%5d %5d %5d   %5d\n", m[0].x, m[1].x, m[2].x, merge.x);
	out << buf;
	sprintf_s(buf, "%5d %5d %5d = %5d\n", m[0].y, m[1].y, m[2].y, merge.y);
	out << buf;
	sprintf_s(buf, "%5d %5d %5d   %5d   det: %d\n", m[0].z, m[1].z, m[2].z, merge.z, m.determinant());
	out << buf;

	//printf("  %s\n", good ? "ok" : "---- BAD!! ----");
	return out;
}



//

// 13 possible sextant classifications:
// 00: centroid: single point
// X0, Y0, Z0: edge between centroid and vertex
// YZ, ZX, XY: edge opposite vertex X, Y, and Z, respectively
// XP, XN: triangular regions left and right of edge between centroid and vertex X
#define SEXX0 *((const int*)("0-+"))
#define SEXXN *((const int*)("+-+"))
#define SEXXY *((const int*)("+-0"))
#define SEXYP *((const int*)("+--"))
#define SEXY0 *((const int*)("+0-"))
#define SEXYN *((const int*)("++-"))
#define SEXYZ *((const int*)("0+-"))
#define SEXZP *((const int*)("-+-"))
#define SEXZ0 *((const int*)("-+0"))
#define SEXZN *((const int*)("-++"))
#define SEXZX *((const int*)("-0+"))
#define SEXXP *((const int*)("--+"))
#define SEX00 *((const int*)("000"))


template<class I>
class TripletSearch {

public:
	// search params
	Triplet<I> m_target;
	unsigned int m_maxDepth;
	unsigned int m_maxHeight;
	unsigned int m_maxNumEnumerationResults;
	TripletTriangle<I> m_clip;
	string m_operations;

	// search results
	vector<string> m_paths;
	int m_countMaxDepth;
	int m_countMaxHeight;
	int m_countDeadEnds;
	bool m_bVerbose;

	// enumeration results
	map<Triplet<I>, string> m_enumeration;

	TripletSearch() {
		m_maxDepth = 18;
		m_maxHeight = 0x00010000;
		m_maxNumEnumerationResults = 100000;
		m_operations = "xyzXYZ";// "468";// "456789";// "xyzXYZ";
								//m_operations = operations;
		m_bVerbose = true;
	}

	bool findAll(const Triplet<I> &target) {
		m_target = target;
		m_paths.clear();
		m_countMaxDepth = 0;
		m_countMaxHeight = 0;
		m_countDeadEnds = 0;

		//cout << "--- Searching for " << m_target << " Ops: " << m_operations << " Depth: " << m_maxDepth << endl;

		TripletTriangle<I> m;
		findAllR(m, "", 0);

		//cout << "" << m_paths.size() << " paths. Maxdepth: " << m_countMaxDepth << " Deadends: " << m_countDeadEnds << endl;
		return (m_paths.size() > 0);
	}

private:
	bool findAllR(const TripletTriangle<I>& triangle, string path, unsigned int depth) {
		Triplet<I> ctr = triangle.centroid();
		ASSERT(ctr.isCoprime());
		if(ctr == m_target) {
			path = path + "-ctr";
			if(std::find(m_paths.begin(), m_paths.end(), path) == m_paths.end()) {
				m_paths.push_back(path);
				if(m_bVerbose) {
					cout << "Found path: [" << path << "]\n";
				}
			}
			return true;
		}

		// compute the triangle edge midpoints
		Triplet<I> p01 = triangle[0] + triangle[1];
		Triplet<I> p12 = triangle[1] + triangle[2];
		Triplet<I> p02 = triangle[0] + triangle[2];

		if(p01 == m_target) { m_paths.push_back(path + "-p01"); return true; }
		if(p12 == m_target) { m_paths.push_back(path + "-p12"); return true; }
		if(p02 == m_target) { m_paths.push_back(path + "-p02"); return true; }

		if(depth >= m_maxDepth) {
			++m_countMaxDepth;
			return false;
		}

		if(!ctr.precedes(m_target)) {
			++m_countDeadEnds;
			return false;
		}

		const char *ops = m_operations.data();
		for(; *ops; ++ops) {
			TripletTriangle<I> m2(*ops, triangle);
			findAllR(m2, path + *ops, depth + 1);
		}

		return false;
	}

public:
	const int ERROR_INTERNAL = -9;

	// sextant tree search. each recursion drills down into 1/6 of the parent search area.
	int search(const Triplet<I> &target) {
		m_target = target;
		m_paths.clear();
		m_countMaxDepth = 0;
		m_countMaxHeight = 0;
		m_countDeadEnds = 0;

		if(m_bVerbose) {
			cout << "--- Sextant search for " << m_target << " Depth: " << m_maxDepth << " ---\n";
		}

		TripletTriangle<I> m;
		bool found = searchR(m, "", 0);

		//cout << endl;
		std::sort(m_paths.begin(), m_paths.end());
		return (m_paths.size() > 0);
	}

private:
	int searchR(const TripletTriangle<I> &triangle, string path, unsigned int depth) {
		Triplet<I> ctr = triangle.centroid();
		ASSERT(ctr.isCoprime());
		if(!(ctr.isCoprime())) {
			cerr << "\n -- TEST FAILED: \n" << triangle << endl;
			return ERROR_INTERNAL;
		}

		if(!triangle.isPointInTriangle(m_target)) {
			cerr << ".. Rejecting[" << depth << "]..\n";
			cerr << triangle;
			return ERROR_INTERNAL;
		}

		if(!triangle[0].precedes(m_target) &&
			!triangle[1].precedes(m_target) &&
			!triangle[2].precedes(m_target)) {
			cerr << " ** wrong turn somewhere. no vertex precedes " << m_target << endl;
			cerr << triangle;
			return ERROR_INTERNAL;
		}

		if(m_bVerbose) {
			cout << ".. Searching[" << depth << "]..\n";
			cout << triangle;
		}

		int sextant = triangle.getSextant(m_target);

		int foundCount = 0;

		// compute the triangle edge midpoints
		Triplet<I> p01 = triangle[0] + triangle[1];
		Triplet<I> p12 = triangle[1] + triangle[2];
		Triplet<I> p02 = triangle[0] + triangle[2];

		bool found = false;		// shortcut... unnecessary?
		if(p01 == m_target) { found = true; m_paths.push_back(path + "-p01"); }
		if(p12 == m_target) { found = true; m_paths.push_back(path + "-p12"); }
		if(p02 == m_target) { found = true; m_paths.push_back(path + "-p02"); }

		//if(ctr == m_target) { 
		if(sextant == SEX00) {
			found = true;
			m_paths.push_back(path + "-ctr");
			return 1;
		}

		if(!found)
		{
			if(depth >= m_maxDepth) {
				++m_countMaxDepth;
				return 0;
			}
			/*
			// build 6 subregions, characterize point: deprecate
			TripletTriangle<I> mx('x', triangle); int inmx = mx.isPointInTriangle(m_target);
			TripletTriangle<I> my('y', triangle); int inmy = my.isPointInTriangle(m_target);
			TripletTriangle<I> mz('z', triangle); int inmz = mz.isPointInTriangle(m_target);
			TripletTriangle<I> mX('X', triangle); int inmX = mX.isPointInTriangle(m_target);
			TripletTriangle<I> mY('Y', triangle); int inmY = mY.isPointInTriangle(m_target);
			TripletTriangle<I> mZ('Z', triangle); int inmZ = mZ.isPointInTriangle(m_target);
			int regionCount = inmx + inmy + inmz + inmX + inmY + inmZ;
			*/

			if(m_bVerbose) {
				cout << "Sextant: " << ((char*)(&sextant)) << endl;
			}

			// note that mirror symmetry is broken here, to avoid duplicate paths to edge points:
			// points on edges (other than the centroid) are resolved to neighboring regions in a
			// counterclockwise fashion.
			// points on the X median nearer to X, for example, are resolved 
			// as if the edge is part of the X- region;
			// points on the X median nearer the YZ edge are resolved as if in the Z+ region.
			// This breaks mirror symmetry but preserves a sort of rotational symmetry.
			// Not sure how else this could be resolved: 
			// - accept that there would be up to 2 paths to a triplet?
			// - on reaching a median, collapse to 2 points--exclude one of the other vertices 
			//   from all further calculations. this reduces to a modified rational tree traversal.

			if(sextant == SEXX0 || sextant == SEXXN) {
				foundCount += searchR(TripletTriangle<I>('x', triangle), path + 'x', depth + 1);
			}
			else if(sextant == SEXXY || sextant == SEXYP) {
				foundCount += searchR(TripletTriangle<I>('Y', triangle), path + 'Y', depth + 1);
			}
			else if(sextant == SEXY0 || sextant == SEXYN) {
				foundCount += searchR(TripletTriangle<I>('y', triangle), path + 'y', depth + 1);
			}
			else if(sextant == SEXYZ || sextant == SEXZP) {
				foundCount += searchR(TripletTriangle<I>('Z', triangle), path + 'Z', depth + 1);
			}
			else if(sextant == SEXZ0 || sextant == SEXZN) {
				foundCount += searchR(TripletTriangle<I>('z', triangle), path + 'z', depth + 1);
			}
			else if(sextant == SEXZX || sextant == SEXXP) {
				foundCount += searchR(TripletTriangle<I>('X', triangle), path + 'X', depth + 1);
			}
			else {
				cerr << "!!??\n";
			}
		}

		if(foundCount && m_bVerbose) {
			cout << "** found in " << path << " **\n" << triangle;
		}

		return foundCount;
	}

public:

	void dumpSearchResults() const {
		if(m_paths.size() == 1) {
			cout << "Path to " << m_target << ": " << m_paths[0] << endl;
		}
		else if(m_paths.size() > 1) {
			cout << " MULTIPLE PATHS!!" << endl;
			for(unsigned int i = 0; i < m_paths.size(); ++i) {
				cout << m_paths[i] << ", ";
			}
			cout << endl;
		}
		else {
			cout << "\n**** NO PATHS FOUND ****\n";
		}
	}

	//
	//	Traversal / Enumeration
	//

	bool enumerate() {

		ASSERT(m_maxDepth >= 0 && m_maxDepth < 1000);
		ASSERT(m_maxHeight > 1);
		//ASSERT(m_clip.isCoprime());
		ASSERT(m_clip.isTriangle());

		cerr << "Enumerating depth: " << m_maxDepth << " max height: " << m_maxHeight
			<< " max results: " << m_maxNumEnumerationResults
			<< "\nClip triangle:\n" << m_clip;

		m_enumeration.clear();
		m_countMaxDepth = 0;
		m_countMaxHeight = 0;
		m_countDeadEnds = 0;
		TripletTriangle<I> m;

		// enumerate the 3 parent vertices
		enumerateCheck(m[0], "-P0", 0);
		enumerateCheck(m[1], "-P1", 0);
		enumerateCheck(m[2], "-P2", 0);

		// recursively enumerate all descendent vertices
		enumerateR(m, "", 0);

		return true;
	}

	void dumpEnumerationResults() const {
		for(auto i = m_enumeration.begin(); i != m_enumeration.end(); ++i) {
			// key is the triplet, "[1,2,3]"
			//cout << "\"" << i->first << "\",";
			cout << i->second << endl;
		}

		cerr << "Enumerated " << m_enumeration.size() << " unique triplets.\n";
		cerr << "Max depth: " << m_countMaxDepth << " Max height: " << m_countMaxHeight << endl;

		for(auto i = m_depthCounts.begin(); i != m_depthCounts.end(); ++i) {
			cerr << "Depth " << i->first << ": " << i->second << endl;
		}
	}

private:

	bool enumerateCheck(const Triplet<I> &p, string path, unsigned int depth) {
		if(abs(p.sum()) > m_maxHeight) {
			return false;
		}

		if(m_enumeration.size() >= m_maxNumEnumerationResults) {
			return false;
		}

		if(!m_clip.isPointInTriangle(p)) {
			return false;
		}

		if(m_enumeration.find(p) == m_enumeration.end()) {
			// add CSV fields: x,y,z,path
			stringstream str;
			str << p.x << "," << p.y << "," << p.z << ",\"" << path << "\"," << depth;
			m_enumeration[p] = str.str();
			return true;
		}

		return false;
	}

	map<unsigned int, unsigned int> m_depthCounts;

	void enumerateR(const TripletTriangle<I> &triangle, string path, unsigned int depth) {

		if(depth > m_maxDepth) {
			++m_countMaxDepth;
			return;
		}

		if(m_enumeration.size() >= m_maxNumEnumerationResults) {
			//cerr << ".. max results reached: " << m_enumeration.size() << " ..";
			return;
		}

		if(!trianglesIntersect(triangle, m_clip)) {
			return;
		}

		cerr << "Intersection: depth=" << depth << endl << triangle;
		++m_depthCounts[depth];

		// compute the triangle edge midpoints and centroid
		Triplet<I> p01 = triangle[0] + triangle[1];
		Triplet<I> p12 = triangle[1] + triangle[2];
		Triplet<I> p02 = triangle[0] + triangle[2];
		Triplet<I> ctr = triangle.centroid();
		ASSERT(ctr.isCoprime());

		enumerateCheck(p01, path + "-p01", depth);
		enumerateCheck(p12, path + "-p12", depth);
		enumerateCheck(p02, path + "-p02", depth);
		enumerateCheck(ctr, path + "-ctr", depth);

		if(abs(ctr.sum()) > m_maxHeight) {
			++m_countMaxHeight;
			return;
		}

		enumerateR(TripletTriangle<I>('x', triangle), path + 'x', depth + 1);
		enumerateR(TripletTriangle<I>('y', triangle), path + 'y', depth + 1);
		enumerateR(TripletTriangle<I>('z', triangle), path + 'z', depth + 1);
		enumerateR(TripletTriangle<I>('X', triangle), path + 'X', depth + 1);
		enumerateR(TripletTriangle<I>('Y', triangle), path + 'Y', depth + 1);
		enumerateR(TripletTriangle<I>('Z', triangle), path + 'Z', depth + 1);

	}

	public:

	static int runTests() {
		// GCD algs
		{
			int nn[12] = { 16, 16, 16, 27, 27, 9, 0, 0, 0, 8, 8, 8 };
			ASSERT(::gcd(nn, 12) == 1);
			int mm[12] = { 48, 15, 12, 27, 27, 9, 0, 0, 0, 90, 24, 21 };
			ASSERT(::gcd(mm, 12) == 3);

			ASSERT(::gcd(8, 1) == 1);
			ASSERT(::gcd(0, 1) == 1);
			ASSERT(::gcd(0, 8) == 8);
			ASSERT(::gcd(40, 32) == 8);
			ASSERT(::gcd(0, 0) == 0);

			int n[3] = { 8, 8, 8 };
			ASSERT(::gcd(n, 3) == 8);
			n[2] = 5;
			ASSERT(::gcd(n, 3) == 1);
			n[2] = 0;
			ASSERT(::gcd(n, 3) == 8);
			n[1] = 0;
			ASSERT(::gcd(n, 3) == 8);
			n[0] = 0;
			ASSERT(::gcd(n, 3) == 0);
		}

		// triangle algs

		TripletTriangle<int> tri;

		for(int i = 0; i < 1000; ++i) {
			tri = TripletTriangle<int>();
			for(int j = 0; j < 20; ++j) {
				int c = rand() % 3;
				int d = (c + 1 + rand() % 2) % 3;
				tri.merge(c, d);
				ASSERT(tri.isCoprime());
				tri = tri.transpose();
				c = rand() % 3;
				d = (c + 1 + rand() % 2) % 3;
				tri.merge(c, d);
				ASSERT(tri.isCoprime());
			}
		}

		// math: any matrix with det == 1 is coprime? (yes--all rows and columns)
		// however the converse is not true
		for(int count = 0; count < 100; ) {
			for(int i = 0; i < 3; ++i) {
				tri[i].x = rand() % 17;
				tri[i].y = rand() % 16;
				tri[i].z = rand() % 20;
				if(tri.determinant() == 1) {
					stringstream str;
					str << tri << " iscoprime: " << tri.isCoprime() << endl;
					TRACE(str.str().c_str());
					ASSERT(tri.isCoprime());
					ASSERT(tri.transpose().isCoprime());
					++count;
				}
			}
		}

		TripletTriangle<I> mBase;
		ASSERT(mBase.isCoprime());
		ASSERT(mBase.isTriangle());
		ASSERT(leftOf(mBase[0], mBase[1], mBase[2]));

		TripletTriangle<I> mTest1(vec3(15, 10, 8), vec3(6, 4, 3), vec3(25, 17, 14));
		TripletTriangle<I> mTest2(vec3(15, 10, 8), vec3(21, 14, 11), vec3(46, 31, 25));
		bool bb;

		cout << "Test 1\n" << mTest1;
		ASSERT(mTest1.isCoprime());
		ASSERT(mTest1.isTriangle());
		bb = mTest1.isPointInTriangle(vec3(36, 24, 19));
		ASSERT(bb);
		bb = mTest1.isPointInTriangle1(vec3(36, 24, 19));
		ASSERT(bb);

		cout << "Test 2\n" << mTest2;
		ASSERT(mTest2.isCoprime());
		ASSERT(mTest2.isTriangle());
		bb = mTest2.isPointInTriangle(vec3(36, 24, 19));
		ASSERT(bb);
		bb = mTest2.isPointInTriangle1(vec3(36, 24, 19));
		ASSERT(bb);

		int signabc;
		vec3 a(1, 1, 1), b(3, 1, 2), c(3, 2, 1), temp;
		signabc = leftOf(a, b, c);
		ASSERT(signabc == 1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		temp = a; a = b; b = temp;
		signabc = leftOf(a, b, c);
		ASSERT(signabc == -1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		a.set(1, 0, 0); b.set(0, 1, 0); c.set(0, 0, 1);
		signabc = leftOf(a, b, c);
		ASSERT(signabc == 1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		// collinear test
		a.set(3, 3, 4); b.set(3, 4, 6); c.set(3, 5, 8);
		signabc = leftOf(a, b, c);
		ASSERT(signabc == 0);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;

		TripletSearch<int> s;
		s.m_bVerbose = false;
		TripletTriangle<I> t;
		for(int i = 1; i <= 3; ++i) {
			for(int j = 1; j <= 3; ++j) {
				for(int k = 1; k <= 3; ++k) {
					vec3 pt(i, j, k);
					if(pt.isCoprime()) {
						int sex = t.getSextant(pt);
						cout << pt << " Sex: " << ((char*)(&sex));

						s.search(pt);
						cout << " search: " << s.m_paths.size() << " paths";

						ASSERT(s.m_paths.size() == 1);

						s.findAll(pt);
						cout << " findAll: " << s.m_paths.size() << " paths found.\n";
					}
				}
			}
		}
		ASSERT(t.getSextant(vec3(1, 1, 1)) == *((int*)("000")));
		ASSERT(t.getSextant(vec3(1, 1, 2)) == *((int*)("-+0")));
		ASSERT(t.getSextant(vec3(1, 1, 3)) == *((int*)("-+0")));
		ASSERT(t.getSextant(vec3(1, 2, 1)) == *((int*)("+0-")));
		ASSERT(t.getSextant(vec3(1, 2, 2)) == *((int*)("0+-")));
		ASSERT(t.getSextant(vec3(1, 2, 3)) == *((int*)("-+-")));
		ASSERT(t.getSextant(vec3(1, 3, 1)) == *((int*)("+0-")));
		ASSERT(t.getSextant(vec3(1, 3, 2)) == *((int*)("++-")));
		ASSERT(t.getSextant(vec3(1, 3, 3)) == *((int*)("0+-")));
		ASSERT(t.getSextant(vec3(2, 1, 1)) == *((int*)("0-+")));
		ASSERT(t.getSextant(vec3(2, 1, 2)) == *((int*)("-0+")));
		ASSERT(t.getSextant(vec3(2, 1, 3)) == *((int*)("-++")));
		ASSERT(t.getSextant(vec3(2, 2, 1)) == *((int*)("+-0")));
		ASSERT(t.getSextant(vec3(2, 2, 3)) == *((int*)("-+0")));
		ASSERT(t.getSextant(vec3(2, 3, 1)) == *((int*)("+--")));
		ASSERT(t.getSextant(vec3(2, 3, 2)) == *((int*)("+0-")));
		ASSERT(t.getSextant(vec3(2, 3, 3)) == *((int*)("0+-")));
		ASSERT(t.getSextant(vec3(3, 1, 1)) == *((int*)("0-+")));
		ASSERT(t.getSextant(vec3(3, 1, 2)) == *((int*)("--+")));
		ASSERT(t.getSextant(vec3(3, 1, 3)) == *((int*)("-0+")));
		ASSERT(t.getSextant(vec3(3, 2, 1)) == *((int*)("+-+")));
		ASSERT(t.getSextant(vec3(3, 2, 2)) == *((int*)("0-+")));
		ASSERT(t.getSextant(vec3(3, 2, 3)) == *((int*)("-0+")));
		ASSERT(t.getSextant(vec3(3, 3, 1)) == *((int*)("+-0")));
		ASSERT(t.getSextant(vec3(3, 3, 2)) == *((int*)("+-0")));

		// test 1: generate all coprime triples up to a limit, and ensure a unique path exists to each

		std::priority_queue<vec3> q;
		// fill queue with all coprime triples up to a limit
		int n = 7;
		for(int x = 1; x <= n; ++x) {
			for(int y = 1; y <= n; ++y) {
				for(int z = 1; z <= n; ++z) {
					vec3 p = { x, y, z };
					if(p.isCoprime()) {
						// symmetry is established... restrict to one sextant
						if(p.x <= p.y && p.y <= p.z) {
							q.push(p);
						}
					}
				}
			}
		}


		while(q.size() > 0) {
			vec3 p = q.top();
			q.pop();
			TripletSearch<int> search;
			search.m_operations = "xyzXYZ123";
			search.m_maxDepth = 6;
			search.m_bVerbose = false;
			//cout << "Searching for " << p << ": ";

			search.search(p);

			search.dumpSearchResults();
		}

		cout << "Testing merge coprimality\n";

		for(int i = 0; i < 100; ++i) {
			TripletTriangle<I> x;
			for(int j = 0; j < 50; ++j) {
				int fromCol = rand() % 3;
				int toCol = (fromCol + 1+rand() % 1) % 3;
				x.merge(fromCol, toCol);
				ASSERT(x.centroid().isCoprime());
			}
		}

		cout << "--- Completed tests ---\n>";
		char dummy;
		cin >> dummy;

		return 1;
	}
};		// class TripletSearch

