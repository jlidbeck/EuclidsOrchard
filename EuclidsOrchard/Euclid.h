#pragma once

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <vector>
#include <string>
#include <map>
#include <numeric>


#ifndef TRACE
#define TRACE(x) ((void)0)
#endif

using std::string;
using std::exception;
using std::vector;
using std::istream;
using std::ostream;
using std::cerr;
using std::endl;
using std::cout;
using namespace std;


#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            __debugbreak(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

//
//	Utils
//

template<class T>
T dot(const vector<T> &a, const vector<T> &b)
{
    return std::inner_product(a.begin(), a.end(), b.begin(), (T)0);
}

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
inline int gcd2(const int *pValues, int numValues) 
{
	vector<int> p(pValues, pValues + numValues);
	int i1, v1, v2;

	bool changes = true;
	while(changes) 
	{
		changes = false;

		std::sort(p.begin(), p.end());
		
		// replace all values with remainder(value, next smallest value)
		for(int i2 = numValues - 1; i2 > 0;) 
		{
			// find all values equal to [i2]

			v2 = p[i2];
			for(i1 = i2 - 1; i1 >= 0 && (v1 = p[i1]) == v2; --i1);
			if(i1 < 0) 
			{
				break;
			}

			// replace all v2 with v2%v1

			if(v1 == 0)
			{
				break;
			}

			int newv2 = v2%v1;
			for(int j = i2; j > i1; --j) 
			{
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
inline int gcd(const int *pValues, int numValues) 
{
	int test = ::gcd2(pValues, numValues);

	// set k to min positive value
	int k = pValues[0];
	for(int i = 1; i < numValues; ++i) {
		if(!k || (pValues[i] && pValues[i] < k)) {
			k = pValues[i];
		}
	}

	// count k down to 2, breaking on first common factor found
	for(; k > 1; --k) 
	{
		int i;
		for(i = 0; i < numValues; ++i)
		{
			if(pValues[i] % k != 0) 
			{
				// k is not a common factor
				break;
			}
		}

		if(i == numValues) 
		{
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
//
template<class I>
class Triplet 
{
public:
	I x, y, z;
	static char _buf[4];

	Triplet()
		:x(1), y(1), z(1) {}

	Triplet(I px, I py, I pz)
		: x(px), y(py), z(pz) {}

	void set(I px, I py, I pz) 
	{
		//assert(px || py || pz);
		x = px; y = py; z = pz;
	}

	inline I sum() const { return x + y + z; }

	vector<float> project() const
	{
		float sum = (float)this->sum();
		return vector<float>({ (float)x / sum, (float)y / sum, (float)z / sum });
	}

	I lengthSq() const { return x*x + y*y + z*z; }

	// vector is all zeros
	bool operator!() const { return !x && !y && !z; }

	Triplet operator-(const Triplet &rhs) const
	{
		return Triplet(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	int dot(const Triplet &rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	// returns true if this a coprime triplet, that is, GCD(x,y,z) == 1
	int isCoprime() const 
	{
		ASSERT(::gcd(x, ::gcd(y, z)) == ::gcd(&x, 3), string("non-transitive gcd")+std::to_string(*this));
		return (::gcd(x, ::gcd(y, z)) == 1);
	}

	bool isPythagoreanTriple() const
	{
		int a2 = x*x;
		int b2 = y*y;
		int c2 = z*z;
		return ((a2 + b2 == c2) || (b2 + c2 == a2) || (c2 + a2 == b2));
	}

	inline void operator += (const Triplet &p) 
	{
		x += p.x;
		y += p.y;
		z += p.z;
	}

	inline Triplet operator + (const Triplet &b) const 
	{
		return Triplet(x + b.x, y + b.y, z + b.z);
	}

	inline bool operator == (const Triplet &p) const 
	{
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

//
//	Returns the scalar triple product of 3 vectors, or equivalently,
//	the orientation of the triangle ABC.
//
//	The scalar triple product is defined as A dot (B cross C) and is
//	invariant under a circle-shift of the parameters. 
//
//	this indicates whether point C lies to the left of segment AB,
//	or equivalently, whether A is left of BC, or B is left of CA.
//	The function is invariant under a circle-shift of the parameters.
//
//	this can be interpreted as 
//	- the sign of the volume of the parallelopiped defined by 3 vectors
//	- whether A lies to the left of the line BC
//	- the orientation of the triangle ABC: 
//		1 indicates the triangle ABC is left-winding;
//		-1 indicates the triangle ABC is right-winding;
//		0 indicates A,B,C are collinear.
//
//	Note that only the sign is computed. This avoids extra operations
//	and also reduces the issue of integer overflow.
//
template<class I>
int leftOf(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &c) 
{
	int abm = sign(c.sum()) * sign(a.sum()) * sign(b.sum());
	return abm * sign(
		  a.x * (b.y*c.z - b.z*c.y)
		+ a.y * (b.z*c.x - b.x*c.z)
		+ a.z * (b.x*c.y - b.y*c.x));
}

template<class I>
ostream& operator << (ostream &out, const Triplet<I> &p) 
{
	out << "[" << p.x << ", " << p.y << ", " << p.z << "]";
	return out;
}

template<class I>
istream& operator >> (istream &in, Triplet<I> &p) 
{
	char c = ',';
	in >> p.x >> c >> p.y >> c >> p.z;
	return in;
}

namespace std
{
    std::string to_string(const Triplet<int> &tri)
    {
        std::ostringstream strstr;
        strstr << tri;
        return strstr.str();
    }

    void from_string(std::string const & str, Triplet<int>& _Value) noexcept
    {
        stringstream sstr(str);
        sstr >> _Value;
    }
}


union SexClass
{
	char asChar[4];
	int asInt = 0;

	constexpr SexClass(int i) : asInt(i) { };
	constexpr SexClass(const char sz[4]) : asInt(*((const int*)(&sz[0]))) { };
	bool operator==(int i) const { return (asInt == i); }
};

// 13 possible sextant classifications:
// 00: centroid: single point (0 d.f.)
// X0, Y0, Z0: edge between centroid and vertex (1 d.f.)
// YZ, ZX, XY: edge opposite vertex X, Y, and Z, respectively (1 d.f.)
// XP, XN: triangular regions left (P) and right (N) of edge between centroid and vertex X (2 d.f.)
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
char Triplet<I>::_buf[] = "***";

//
//	3x3 integer matrix
//
template<class I>
class TripletTriangle 
{
public:
	Triplet<I> m[3] = { { 1,0,0 },{ 0,1,0 },{ 0,0,1 } };
public:
	string path = "";
	unsigned int depth = 0;

	TripletTriangle() {}

	TripletTriangle(const Triplet<I> &p0, const Triplet<I> &p1, const Triplet<I> &p2) 
	{
		m[0] = p0;
		m[1] = p1;
		m[2] = p2;
	}

	TripletTriangle(char op, const TripletTriangle<I> &src) 
	{
		operate(op, src);
	}

	bool isZero() const
	{
		return (!m[0] && !m[1] && !m[2]);
	}

	TripletTriangle operator+(char op) const
	{
		TripletTriangle result(*this);
		if(result.operate(op))
			return result;
		result.m[0].set(0, 0, 0);
		result.m[1].set(0, 0, 0);
		result.m[2].set(0, 0, 0);
		result.depth = 0;
		return result;
	}

	//	Reset to identity matrix/basis triangle
	void reset() 
	{
		m[0].set(1, 0, 0);
		m[1].set(0, 1, 0);
		m[2].set(0, 0, 1);
		depth = 0;
	}

	void set(const Triplet<I> &p0, const Triplet<I> &p1, const Triplet<I> &p2) 
	{
		m[0] = p0;
		m[1] = p1;
		m[2] = p2;
	}

	int numColumns() const
	{
		return !!m[0] + !!m[1] + !!m[2];
	}

	// fetch column / triplet
	const Triplet<I>& operator [](int idx) const 
	{
		assert(idx >= 0 && idx < 3);
		return m[idx];
	}

	// fetch column / triplet
	Triplet<I>& operator [](int idx) 
	{
		assert(idx >= 0 && idx < 3);
		return m[idx];
	}

	I determinant() const 
	{
		return
			  m[0].x * (m[1].y * m[2].z - m[1].z * m[2].y)
			+ m[0].y * (m[1].z * m[2].x - m[1].x * m[2].z)
			+ m[0].z * (m[1].x * m[2].y - m[1].y * m[2].x);
	}

	Triplet<I> centroid() const 
	{
		return m[0] + m[1] + m[2];
	}

	//	Check whether all 3 columns are coprime and the row sums are coprime.
	bool isCoprime() const 
	{
		// if a column has a common factor greater than 1,
		// then the determinant will be a multiple of that factor.
		if(determinant() != 1) 
		{
			return false;
		}
		assert(m[0].isCoprime() && m[1].isCoprime() && m[2].isCoprime());

		// can this be replaced with a simple check that the determinant is 1? "unimodular"

		auto ctr = centroid();
		if(!ctr.isCoprime()) 
		{
			return false;
		}

		assert(determinant() == 1);
		return true;
	}

	//	matrix forms proper, non-collinear, left-winding triangle
	inline bool isTriangle() const 
	{
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

	//
	//	AKA pointInTriangle
	//	returns true if the point is inside this triangle or on an edge or vertex
    //  correct even if dimensions have been collapsed
	//
	bool containsPoint(const Triplet<I> &pt) const
	{
		return leftOf(pt, m[0], m[1]) >= 0
			&& leftOf(pt, m[1], m[2]) >= 0
			&& leftOf(pt, m[2], m[0]) >= 0;
	}

	//
	//	AKA triangleInTriangle
	//	returns true if {tri} is contained entirely within this triangle.
	//	edges and vertices may touch, so x.containsTriangle(x) would return true.
	//
	bool containsTriangle(const TripletTriangle<I> &tri) const
	{
		return containsPoint(tri[0])
			&& containsPoint(tri[1])
			&& containsPoint(tri[2]);
	}

	//
	//	merge transforms
	//	these preserve the coprime triangle properties:
	//	- determinant = 1
	//	- rows, columns, row sums are all coprime triples
	//	- columns form left-winding triangle
	//

    //  if src and dest cols are nonzero,
    //  toCol += fromCol
	inline bool merge(int fromCol, int toCol) 
	{
		assert(fromCol != toCol);
		assert(fromCol >= 0 && fromCol < 3 && toCol >= 0 && toCol < 3);
		if(!m[fromCol]) return false;
		if(!m[toCol]) return false;
		m[toCol] += m[fromCol];
		++depth;
		//assert(isCoprime());
		//assert(isTriangle());
		return true;
	}

	//inline void mergeAll(int toCol)
	//{
	//	depth += 2;
	//	m[toCol] = centroid();
	//}

	//	collapse: aggregates two columns by adding; sets a column to zeros
	//	decreases dimensionality, leaving centroid unchanged
	inline bool collapse(int ai, int bi)
	{
		Triplet<I> before = this->centroid();
		assert(before.isCoprime());

		assert(ai != bi);
		assert(ai >= 0 && ai < 3 && bi >= 0 && bi < 3);
		if(!m[ai] && !m[bi])
			return false;

		auto otherCol = m[3 - ai - bi];
		// [0]=other, [1]=[from]+[to]
		auto colsum = m[ai] + m[bi];
		m[0] = colsum;
		m[1] = otherCol;
		m[2] = { 0,0,0 };

		Triplet<I> ctr = this->centroid();
        assert(ctr == before);

        ++depth;
        return true;
	}

	inline void mergeAll()
	{
		depth += 1;
		m[0] = centroid();
	}

    // 3-dimensional cascade merge
	// replaces this triangle with one of its sextants.
	// - vertex[primary] is preserved; 
	// - vertex[secondary] moves to primary+secondary midpoint;
	// - remaining vertex moves to centroid.
	//
	// same as merge(p, s); merge(s, t);
	inline bool merge3(int primary, int secondary) 
	{
		// before: x, y, z
		// after: x, x+y, x+y+z
        if (!merge(primary, secondary))
            return false;   // temporary logic here: consider it legal (return true) as long as the first of the merges is complete.. 
        // this allows more lax code once dimensions have collapsed

        merge(secondary, 3 - primary - secondary);
        return true;
	}

	// creates triangle from midpoints--central quadrant
	// this is a counterexample--breaking the sequential column-merge restriction
	// causes the centroid to become non-coprime
	void mergeCenter() 
	{
		auto col0 = m[0];
		m[0] += m[1];
		m[1] += m[2];
		m[2] += col0;
		++depth;
		assert(isCoprime());
		assert(isTriangle());
	}

	// rotate columns one to the right. preserves triangle invariants.
	bool rotate() 
	{
		auto temp = m[2];
		m[2] = m[1];
		m[1] = m[0];
		m[0] = temp;
        return true;
	}

	inline bool operate(char op, const TripletTriangle<I> &src) 
	{
		*this = src;
		return operate(op);
	}

	bool operate(char op) 
	{
		// append to path string
		switch(op)
		{
			case '1': this->path += "{XY}"; break;
			case '2': this->path += "{YZ}"; break;
			case '3': this->path += "{ZX}"; break;

			default:
				this->path += op;
		}

		// apply operations
		switch(op)
		{
			// the 6 basic transforms: each divides the parent triangle in half
			case 'a': return this->merge(0, 1);
			case 'b': return this->merge(1, 2);
			case 'c': return this->merge(2, 0);
			case 'A': return this->merge(1, 0);
			case 'B': return this->merge(2, 1);
			case 'C': return this->merge(0, 2);
			// cascade merges
			// transforms triangle to one of its parent's sextants
			// the 6 aggregate transforms: each defines one sixth of the parent triangle
			case 'x': return this->merge3(0, 1);
			case 'y': return this->merge3(1, 2);
			case 'z': return this->merge3(2, 0);
			case 'X': return this->merge3(0, 2);
			case 'Y': return this->merge3(1, 0);
			case 'Z': return this->merge3(2, 1);
			// alternate sixth division: 3 corner quadrants (like a Sierpinski triangle), 3 inner triangle thirds
			case 'j': this->merge(0, 1); this->merge(0, 2); break; // X corner quadrant
			case 'k': this->merge(1, 0); this->merge(1, 2); break; // Y corner quadrant
			case 'l': this->merge(2, 1); this->merge(2, 0); break; // Z corner quadrant
			case 'm': this->merge(0, 1); this->merge(2, 0); this->merge(1, 2) && rotate(); break; // X-ward inner 12th, rotated for aesthetics
			case 'n': this->merge(1, 2); this->merge(0, 1); this->merge(2, 0) && rotate(); break; // Y-ward inner 12th
			case 'o': this->merge(2, 0); this->merge(1, 2); this->merge(0, 1) && rotate(); break; // Z-ward inner 12th
			// thirds
			//case 'u': this->mergeAll(0); break;
			//case 'v': this->mergeAll(1); break;
			//case 'w': this->mergeAll(2); break;
			// dimensionality collapse
			// returns false if one of the columns is zero
			case '1': return this->collapse(0, 1); break;
			case '2': return this->collapse(1, 2); break;
			case '3': return this->collapse(2, 0); break;
			// an example of an invalid transform
			//case 'i': this->mergeCenter(); break;	// center quadrant: INVALID
			default:
				return false;
		}

		return true;
	}

	// returns sextant classification of point.
	// characterizes point as left of (-1), right of (+1), or on (0)
	// each of the 3 median lines.
	// returns 4-byte value to be interpreted like "-+0"
	SexClass getSextant(const Triplet<I> &pt) const
	{
		// centroid
		Triplet<I> ctr = this->centroid();
		assert(ctr.isCoprime());

		if(numColumns() == 2)
		{
			// pt better be on the line XY
			// figure out whether pt is more toward X or Y than ctr.
			// (pt - ctr) dot (x - ctr)
			auto p = pt.project();
			auto c = ctr.project();
			auto x = m[0].project();
			vector<float> cp({ p[0] - c[0],p[1] - c[1],p[2] - c[2] });
			vector<float> cx({ x[0] - c[0],x[1] - c[1],x[2] - c[2] });
			float dotx = ::dot(cp, cx);
			//int dotx = (c*pt - p*ctr).dot(c*m[0] - x*ctr)/(c*x*c*p);
			if(dotx == 0)
				return SEX00;
			if(dotx > 0)
				return SEXX0;
			return SEXY0;
		}

		// compute the point's side relative to the three triangle medians.
		//
		// here we're computing AB x AP, BC x BP, and CA x CP.
		// a simple 2D projection is used: the point [Ax, Ay, Az] is mapped to 
		// (Ax / (Ax+Ay+Az), Ay / (Ax+Ay+Az)).
		// This projection preserves straight line properties... there's probably
		// a mathematically simpler way to do this without projection.

		//// reciprocals for 2d projections
		//I psum = pt.sum(), asum = m[0].sum(), bsum = m[1].sum(), csum = m[2].sum(), msum = ctr.sum();

		//// project to (x/(x+y+z), y/(x+y+z))
		//// division is delayed, so these values are scaled by ?sum
		//I ax = m[0].x, ay = m[0].y;
		//I bx = m[1].x, by = m[1].y;
		//I cx = m[2].x, cy = m[2].y;
		//I px = pt.x, py = pt.y;
		//I mx = ctr.x, my = ctr.y;
		//I temp;

		//// calculate cross-products to characterize P by edges MA, MB, MC
		//temp = (msum*ax - asum*mx)*(msum*py - psum*my) 
		//	 - (msum*ay - asum*my)*(msum*px - psum*mx);
		int maxmp = leftOf(ctr, m[0], pt);
		//assert(sign(temp) == maxmp);

		//temp = (msum*bx - bsum*mx)*(msum*py - psum*my) - (msum*by - bsum*my)*(msum*px - psum*mx);
		int mbxmp = leftOf(ctr, m[1], pt);
		//assert(sign(temp) == mbxmp);

		//temp = (msum*cx - csum*mx)*(msum*py - psum*my) - (msum*cy - csum*my)*(msum*px - psum*mx);
		int mcxmp = leftOf(ctr, m[2], pt);
		//assert(sign(temp) == mcxmp);

		//cerr << " {" << signchar(maxmp) << signchar(mbxmp) << signchar(mcxmp) << "}\n";

		SexClass sex(0);

		sex.asChar[0] = signchar(maxmp);
		sex.asChar[1] = signchar(mbxmp);
		sex.asChar[2] = signchar(mcxmp);
		return sex;
	}

	// matrix functions..

	TripletTriangle transpose() const 
	{
		TripletTriangle tri(*this);
		tri.m[0].y = m[1].x;
		tri.m[0].z = m[2].x;
		tri.m[1].x = m[0].y;
		tri.m[1].z = m[2].y;
		tri.m[2].x = m[0].z;
		tri.m[2].y = m[1].z;
		return tri;
	}

	void transpose(const TripletTriangle &tri) 
	{
		*this = tri;
		m[0].y = tri.m[1].x;
		m[0].z = tri.m[2].x;
		m[1].x = tri.m[0].y;
		m[1].z = tri.m[2].y;
		m[2].x = tri.m[0].z;
		m[2].y = tri.m[1].z;
	}

	// comparators

	int operator == (const TripletTriangle &tri) const 
	{
		return m[0] == tri.m[0] && m[1] == tri.m[1] && m[2] == tri.m[2];
	}

	// returns 1 if {this} is lower priority than {tri}.
	//
	// for priority queues, or map keys.
	// operator < is strict, as it is also used for equality testing.
	// that is, if x < y, it's guaranteed that !(y<x).
	// if !(x<y) and !(y<x), then x==y.
	// This ordering assigns lower values to ...
	inline int operator < (const TripletTriangle &tri) const 
	{
		int cmp = compare(tri);
		return (cmp > 0);
	}

	// returns:
	// 1 if {this} is lower priority than {tri},
	// -1 if {this} is higher priority than {tri},
	// 0 if {this} and {tri} are equal--the same matrix/triangle.
	int compare(const TripletTriangle &tri) const 
	{
		// smaller depth means higher priority
		if(depth < tri.depth) {
			return -1;
		}
		if(tri.depth < depth) {
			return 1;	// this is lower priority than tri
		}

		auto l = this->centroid();
		auto r = tri.centroid();
		if(l < r) return 1;
		if(r < l) return -1;
		for(int i = 0; i < 3; ++i) {
			if(m[i] < tri.m[i]) return 1;
			if(tri.m[i] < m[i]) return -1;
		}
		// they are equal
		return 0;
	}

	//friend ostream& operator << (ostream &out, const TripletTriangle<I> &m);
	//friend std::string to_string(const TripletTriangle<int> &tri);

};		// class TripletTriangle

// returns true if the two segments intersect, including endpoint intersections
template<class I>
bool segmentsIntersect(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &c, const Triplet<I> &d) 
{
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
bool trianglesIntersect(const TripletTriangle<I> &a, const TripletTriangle<I> &b) 
{
	// if any vertex is inside the other triangle, the triangles overlap

	if(    b.containsPoint(a[0])
		|| b.containsPoint(a[1])
		|| b.containsPoint(a[2])
		|| a.containsPoint(b[0])
		|| a.containsPoint(b[1])
		|| a.containsPoint(b[2])) {
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
bool pointInCircle(const Triplet<I> &point, const Triplet<I> &ctr, float radius) 
{
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
inline F distsq3(const F *a, const F *b) 
{
	F ab[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
	return sumsq3(ab);
}

//	returns true if any part of the line segment is inside a circle
//	a, b, and ctr should be 3D points on the plane x+y+z=1
template<class F>
bool lineSegmentIntersectsCircle(const F *a, const F *b, const F *ctr, F radius) 
{
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

//	tests whether the line segment ab (projected onto x+y+z=1)
//	intersects the circle
template<class I>
bool lineSegmentIntersectsCircle(const Triplet<I> &a, const Triplet<I> &b, const Triplet<I> &ctr, float radius) 
{
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
bool triangleIntersectsCircle(const TripletTriangle<I> &tri, const Triplet<I> &ctr, float radius) 
{
	// if any vertex is inside the circle
	//if(    pointInCircle(tri[0], ctr, radius)
	//	|| pointInCircle(tri[1], ctr, radius)
	//	|| pointInCircle(tri[2], ctr, radius)) {
	//	return true;
	//}

	if(tri.numColumns() < 3)
		cout << "col: " << tri.numColumns() << endl;

	// if circle's center is inside triangle, they intersect
	if(tri.containsPoint(ctr)) {
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
ostream& operator << (ostream &out, const TripletTriangle<I> &m) 
{
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

namespace std
{
	std::string to_string(const TripletTriangle<int> &tri)
	{
		std::ostringstream strstr;
		strstr << tri;
		return strstr.str();
	}
}



//
enum class GrowthEnum
{
	NONE = 0, TRIANGLE = 1, CIRCLE, FILL, POINT_SEARCH, STOCHASTIC
};

constexpr const char * GrowthEnumString(GrowthEnum e)
{
	switch(e)
	{
	case GrowthEnum::NONE:			return "NONE";
	case GrowthEnum::TRIANGLE:		return "TRIANGLE";
	case GrowthEnum::CIRCLE:		return "CIRCLE";
	case GrowthEnum::FILL:			return "FILL";
	case GrowthEnum::POINT_SEARCH: return "POINT_SEARCH";
	case GrowthEnum::STOCHASTIC:	return "STOCHASTIC";
	}
};

namespace std
{
	string to_string(GrowthEnum e)
	{
		return GrowthEnumString(e);
	}
}

template<class I>
class TripletSearch
{
public:
	// search params
	Triplet<I> m_target;
	float m_targetRadius;
    unsigned int m_maxDepth = 10;
	unsigned int m_maxHeight;
	unsigned int m_nMaxTriangleSum = 1000;
	unsigned int m_maxNumEnumerationResults;
	TripletTriangle<I> m_clip;
	string m_operations;

	GrowthEnum m_growth = GrowthEnum::STOCHASTIC;
	//static std::map<GrowthEnum, string> s_growth_enum_names;

	// search results
	vector<string> m_paths;
	string m_bestPath;
	int m_countMaxDepth;
	int m_countMaxHeight;
	int m_countMaxSum = 0;
	int m_countCollision = 0;
	int m_countDeadEnds;
	bool m_bVerbose;

	// enumeration results
	std::map<Triplet<I>, string> m_enumeration;

	TripletSearch() 
	{
		m_targetRadius = 0;
		m_maxDepth = 18;
		m_maxHeight = 0x00010000;
		m_maxNumEnumerationResults = 100000;
		m_operations = "xyzXYZ123";// "468";// "456789";// "xyzXYZ";
		m_bVerbose = true;
	}

	bool findAll(const Triplet<I> &target)
	{
		m_target = target;
		m_paths.clear();
		m_bestPath.clear();
		m_countMaxDepth = 0;
		m_countMaxHeight = 0;
		m_countDeadEnds = 0;

		//cout << "--- Searching for " << m_target << " Ops: " << m_operations << " Depth: " << m_maxDepth << endl;

		TripletTriangle<I> m;
		findAllR(m, 0);

		//cout << "" << m_paths.size() << " paths. Maxdepth: " << m_countMaxDepth << " Deadends: " << m_countDeadEnds << endl;
		return (m_paths.size() > 0);
	}

private:
	bool findAllR(const TripletTriangle<I>& triangle, unsigned int depth) 
	{
		Triplet<I> ctr = triangle.centroid();
		assert(ctr.isCoprime());
		if(ctr == m_target) 
		{
			string path = triangle.path + "-ctr";
			if(std::find(m_paths.begin(), m_paths.end(), path) == m_paths.end()) 
			{
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

		// is this step necessary? seems that every triple should eventually be a vertex..
		if(p01 == m_target) { m_paths.push_back(triangle.path + "-p01"); return true; }
		if(p12 == m_target) { m_paths.push_back(triangle.path + "-p12"); return true; }
		if(p02 == m_target) { m_paths.push_back(triangle.path + "-p02"); return true; }

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
			//TripletTriangle<I> m2(*ops, triangle);
			findAllR(triangle + *ops, depth + 1);
		}

		return false;
	}

public:
	const int ERROR_INTERNAL = -9;

	// sextant tree search. each recursion drills down into 1/6 of the parent search area.
	int search(const Triplet<I> &target)
	{
		m_target = target;
		m_paths.clear();
		m_bestPath.clear();
		m_countMaxDepth = 0;
		m_countMaxHeight = 0;
		m_countDeadEnds = 0;

		if(m_bVerbose) 
		{
			cout << "--- Sextant search --- Target: " << m_target 
				<< " Growth: " << GrowthEnumString(m_growth) 
				<< " Depth: " << m_maxDepth << " ---\n";
		}

		TripletTriangle<I> m;
		bool found = (searchR(m, 0) > 0);

		//cout << endl;
		std::sort(m_paths.begin(), m_paths.end());
		return (m_paths.size() > 0);
	}

private:
	int searchR(const TripletTriangle<I> &triangle, unsigned int depth) 
	{
		if(triangle.isZero())
		{
			return 0;
		}

		Triplet<I> ctr = triangle.centroid();
		assert(ctr.isCoprime());
		//if(!(ctr.isCoprime())) {
		//	cerr << "\n -- TEST FAILED: \n" << triangle << endl;
		//	return ERROR_INTERNAL;
		//}

		if(!triangle.containsPoint(m_target)) 
		{
			cerr << ".. Rejecting[" << depth << "]..\n";
			cerr << triangle;
			return 0;
		}

		if( !triangle[0].precedes(m_target) &&
			!triangle[1].precedes(m_target) &&
			!triangle[2].precedes(m_target)) 
		{
			cerr << " ** wrong turn somewhere. no vertex precedes " << m_target << endl;
			cerr << triangle;
			return 0;
		}

		if(	   ctr.x > m_target.x
			|| ctr.y > m_target.y
			|| ctr.z > m_target.z)
		{
			cerr << " ** missed the exit somewhere. ctr=" << ctr << " > target=" << m_target << endl;
			cerr << triangle;
			return 0;
		}

		//if(m_bVerbose)
		//{
		//	cout << ".. Searching[" << depth << "].. " << path << "\n";
		//	cout << triangle;
		//}

		//bool found = false;		

		/* // method 1		
		// check vertices?
		if(triangle[0] == m_target) { found = true; m_paths.push_back(path + "-vX"); return 1; }
		if(triangle[1] == m_target) { found = true; m_paths.push_back(path + "-vY"); return 1; }
		if(triangle[2] == m_target) { found = true; m_paths.push_back(path + "-vZ"); return 1; }

		//// compute the triangle edge midpoints
		//Triplet<I> p01 = triangle[0] + triangle[1];
		//Triplet<I> p12 = triangle[1] + triangle[2];
		//Triplet<I> p02 = triangle[0] + triangle[2];

		//if(p01 == m_target) { found = true; m_paths.push_back(path + "-p01"); }
		//if(p12 == m_target) { found = true; m_paths.push_back(path + "-p12"); }
		//if(p02 == m_target) { found = true; m_paths.push_back(path + "-p02"); }
*/

		SexClass sextant = triangle.getSextant(m_target);

		if(m_bVerbose)
		{
			cout << "Sextant: " << sextant.asChar << endl;
		}

		// method 2
		if(triangle.numColumns() == 1)
		{
			if(ctr == m_target)
			{
				//found = true;
				if(m_bVerbose)
					cout << "FOUND: " << triangle.path << "." << endl;

				m_paths.push_back(triangle.path + ".");
				return 1;
			}

			cerr << " ** unknown error. single column miss" << endl;
			cerr << triangle;
			return 0;
		}

		int foundCount = 0;

/*
		// method 1
		if(sextant.asInt == SEX00) {
			found = true;
			m_paths.push_back(path + "-ctr");
			return 1;
		}//*/


		//if(!found)
		{
			if(depth >= m_maxDepth) 
			{
				++m_countMaxDepth;
				m_bestPath = (triangle.path + "...");
				return 0;
			}

			/*
			// build 6 subregions, characterize point: deprecate
			TripletTriangle<I> mx('x', triangle); int inmx = mx.containsPoint(m_target);
			TripletTriangle<I> my('y', triangle); int inmy = my.containsPoint(m_target);
			TripletTriangle<I> mz('z', triangle); int inmz = mz.containsPoint(m_target);
			TripletTriangle<I> mX('X', triangle); int inmX = mX.containsPoint(m_target);
			TripletTriangle<I> mY('Y', triangle); int inmY = mY.containsPoint(m_target);
			TripletTriangle<I> mZ('Z', triangle); int inmZ = mZ.containsPoint(m_target);
			int regionCount = inmx + inmy + inmz + inmX + inmY + inmZ;
			*/

			/*
			//	METHOD ONE
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

			if(sextant == SEXX0 || sextant == SEXXN)
			{
				foundCount += searchR(TripletTriangle<I>('x', triangle), path + 'x', depth + 1);
			}
			else if(sextant == SEXXY || sextant == SEXYP)
			{
				foundCount += searchR(TripletTriangle<I>('Y', triangle), path + 'Y', depth + 1);
			}
			else if(sextant == SEXY0 || sextant == SEXYN)
			{
				foundCount += searchR(TripletTriangle<I>('y', triangle), path + 'y', depth + 1);
			}
			else if(sextant == SEXYZ || sextant == SEXZP)
			{
				foundCount += searchR(TripletTriangle<I>('Z', triangle), path + 'Z', depth + 1);
			}
			else if(sextant == SEXZ0 || sextant == SEXZN)
			{
				foundCount += searchR(TripletTriangle<I>('z', triangle), path + 'z', depth + 1);
			}
			else if(sextant == SEXZX || sextant == SEXXP)
			{
				foundCount += searchR(TripletTriangle<I>('X', triangle), path + 'X', depth + 1);
			}
			else
			{
				cerr << "!!??\n";
			}
			//*/

			//	METHOD TWO

			if(sextant == SEX00)
			{
				//found = true;
				if(m_bVerbose)
					cerr << "Path found: " << triangle.path << "." << endl;

				m_paths.push_back(triangle.path + ".");
				return 1;
			}
			else if(triangle.numColumns() == 2)
			{
				// 1D binary search
				if(sextant == SEXX0) 		// Left (toward X)
					foundCount += searchR(triangle + 'a', depth + 1);
				else if(sextant == SEXY0)	// Right (toward Y)
					foundCount += searchR(triangle + 'A', depth + 1);
				else
				{
					cerr << "!!??\n";
				}
			}
			else if(sextant == SEXXN)
				foundCount += searchR( triangle + 'x', 		depth + 1);
			else if(sextant == SEXYP)
				foundCount += searchR( triangle + 'Y', 		depth + 1);
			else if(sextant == SEXYN)
				foundCount += searchR( triangle + 'y', 		depth + 1);
			else if(sextant == SEXZP)
				foundCount += searchR( triangle + 'Z', 		depth + 1);
			else if(sextant == SEXZN)
				foundCount += searchR( triangle + 'z', 		depth + 1);
			else if(sextant == SEXXP)
				foundCount += searchR( triangle + 'X', 		depth + 1);
			else if(sextant == SEXX0) 	// on line between X and ctr: merge YZ->X, then toward Y
				foundCount += searchR( triangle + '2' + 'A', depth + 1);
			else if(sextant == SEXYZ) 	// on line between ctr and YZ's midpoint: merge YZ->X, then toward X
				foundCount += searchR( triangle + '2' + 'a', depth + 1);
			else if(sextant == SEXY0)	// on line between Y and ctr: merge XZ->X, then toward Y
				foundCount += searchR( triangle + '3' + 'A', depth + 1);
			else if(sextant == SEXZX)	// on line between ctr and XZ's midpoint: merge XZ->X, then toward X
				foundCount += searchR( triangle + '3' + 'a', depth + 1);
			else if(sextant == SEXZ0)	// on line between Z and ctr: merge XY->X, then toward Y
				foundCount += searchR( triangle + '1' + 'A', depth + 1);
			else if(sextant == SEXXY)	// on line between ctr and XY's midpoint: merge XY->X, then toward X
				foundCount += searchR( triangle + '1' + 'a', depth + 1);
			else
			{
				cerr << "!!??\n";
			}
		}

		//if(foundCount && m_bVerbose) {
		//	cout << "** found in " << path << " **\n" << triangle;
		//}

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

		assert(m_maxDepth >= 0 && m_maxDepth < 1000);
		assert(m_maxHeight > 1);
		//assert(m_clip.isCoprime());
		assert(m_clip.isTriangle());

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

		if(!m_clip.containsPoint(p)) {
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

	std::map<unsigned int, unsigned int> m_depthCounts;

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
		assert(ctr.isCoprime());

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

	static int runTests() 
	{
		// GCD algs
		{
			int nn[12] = { 16, 16, 16, 27, 27, 9, 0, 0, 0, 8, 8, 8 };
			assert(::gcd(nn, 12) == 1);
			int mm[12] = { 48, 15, 12, 27, 27, 9, 0, 0, 0, 90, 24, 21 };
			assert(::gcd(mm, 12) == 3);

			assert(::gcd(8, 1) == 1);
			assert(::gcd(0, 1) == 1);
			assert(::gcd(0, 8) == 8);
			assert(::gcd(40, 32) == 8);
			assert(::gcd(0, 0) == 0);

			int n[3] = { 8, 8, 8 };
			assert(::gcd(n, 3) == 8);
			n[2] = 5;
			assert(::gcd(n, 3) == 1);
			n[2] = 0;
			assert(::gcd(n, 3) == 8);
			n[1] = 0;
			assert(::gcd(n, 3) == 8);
			n[0] = 0;
			assert(::gcd(n, 3) == 0);
		}

		// triangle algs

		TripletTriangle<int> tri;

		for(int i = 0; i < 1000; ++i) {
			tri = TripletTriangle<int>();
			for(int j = 0; j < 20; ++j) {
				int c = rand() % 3;
				int d = (c + 1 + rand() % 2) % 3;
				tri.merge(c, d);
				assert(tri.isCoprime());
				tri = tri.transpose();
				c = rand() % 3;
				d = (c + 1 + rand() % 2) % 3;
				tri.merge(c, d);
				assert(tri.isCoprime());
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
					assert(tri.isCoprime());
					assert(tri.transpose().isCoprime());
					++count;
				}
			}
		}

		TripletTriangle<I> mBase;
		assert(mBase.isCoprime());
		assert(mBase.isTriangle());
		assert(leftOf(mBase[0], mBase[1], mBase[2]));

		TripletTriangle<I> mTest1(vec3(15, 10, 8), vec3(6, 4, 3), vec3(25, 17, 14));
		TripletTriangle<I> mTest2(vec3(15, 10, 8), vec3(21, 14, 11), vec3(46, 31, 25));
		bool bb;

		cout << "Test 1\n" << mTest1;
		assert(mTest1.isCoprime());
		assert(mTest1.isTriangle());
		bb = mTest1.containsPoint(vec3(36, 24, 19));
		assert(bb);

		cout << "Test 2\n" << mTest2;
		assert(mTest2.isCoprime());
		assert(mTest2.isTriangle());
		bb = mTest2.containsPoint(vec3(36, 24, 19));
		assert(bb);

		int signabc;
		vec3 a(1, 1, 1), b(3, 1, 2), c(3, 2, 1), temp;
		signabc = leftOf(a, b, c);
		assert(signabc == 1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		temp = a; a = b; b = temp;
		signabc = leftOf(a, b, c);
		assert(signabc == -1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		a.set(1, 0, 0); b.set(0, 1, 0); c.set(0, 0, 1);
		signabc = leftOf(a, b, c);
		assert(signabc == 1);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;
		// collinear test
		a.set(3, 3, 4); b.set(3, 4, 6); c.set(3, 5, 8);
		signabc = leftOf(a, b, c);
		assert(signabc == 0);
		cout << "leftof(" << a << " , " << b << " , " << c << " ): " << signabc << endl;

		TripletSearch<int> s;
		s.m_growth = GrowthEnum::POINT_SEARCH;
		s.m_bVerbose = true;
		TripletTriangle<I> t;
		for(int i = 3; i <= 5; ++i) 
		{
			for(int j = 3; j <= i; ++j) 
			{
				for(int k = 1; k <= j; ++k) 
				{
					vec3 pt(i, j, k);
					if(pt.isCoprime()) 
					{
						SexClass sex = t.getSextant(pt);
						cout << pt << " Sex: " << sex.asChar;

						s.search(pt);
						cout << " search: " << s.m_paths.size() << " paths";

						assert(s.m_paths.size() == 1);

						s.findAll(pt);
						cout << " findAll: " << s.m_paths.size() << " paths found.\n";
					}
				}
			}
		}
		assert(t.getSextant(vec3(1, 1, 1)).asInt == *((int*)("000")));
		assert(t.getSextant(vec3(1, 1, 2)).asInt == *((int*)("-+0")));
		assert(t.getSextant(vec3(1, 1, 3)).asInt == *((int*)("-+0")));
		assert(t.getSextant(vec3(1, 2, 1)).asInt == *((int*)("+0-")));
		assert(t.getSextant(vec3(1, 2, 2)).asInt == *((int*)("0+-")));
		assert(t.getSextant(vec3(1, 2, 3)).asInt == *((int*)("-+-")));
		assert(t.getSextant(vec3(1, 3, 1)).asInt == *((int*)("+0-")));
		assert(t.getSextant(vec3(1, 3, 2)).asInt == *((int*)("++-")));
		assert(t.getSextant(vec3(1, 3, 3)).asInt == *((int*)("0+-")));
		assert(t.getSextant(vec3(2, 1, 1)).asInt == *((int*)("0-+")));
		assert(t.getSextant(vec3(2, 1, 2)).asInt == *((int*)("-0+")));
		assert(t.getSextant(vec3(2, 1, 3)).asInt == *((int*)("-++")));
		assert(t.getSextant(vec3(2, 2, 1)).asInt == *((int*)("+-0")));
		assert(t.getSextant(vec3(2, 2, 3)).asInt == *((int*)("-+0")));
		assert(t.getSextant(vec3(2, 3, 1)).asInt == *((int*)("+--")));
		assert(t.getSextant(vec3(2, 3, 2)).asInt == *((int*)("+0-")));
		assert(t.getSextant(vec3(2, 3, 3)).asInt == *((int*)("0+-")));
		assert(t.getSextant(vec3(3, 1, 1)).asInt == *((int*)("0-+")));
		assert(t.getSextant(vec3(3, 1, 2)).asInt == *((int*)("--+")));
		assert(t.getSextant(vec3(3, 1, 3)).asInt == *((int*)("-0+")));
		assert(t.getSextant(vec3(3, 2, 1)).asInt == *((int*)("+-+")));
		assert(t.getSextant(vec3(3, 2, 2)).asInt == *((int*)("0-+")));
		assert(t.getSextant(vec3(3, 2, 3)).asInt == *((int*)("-0+")));
		assert(t.getSextant(vec3(3, 3, 1)).asInt == *((int*)("+-0")));
		assert(t.getSextant(vec3(3, 3, 2)).asInt == *((int*)("+-0")));

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
				auto ctr = x.centroid();
				int g = gcd(&ctr.x, 3);
				ASSERT(ctr.isCoprime(), string("not coprime: ") +std::to_string(g)+" "+ std::to_string(x));
			}
		}

		cout << "--- Completed tests ---\n>";
		char dummy;
		cin >> dummy;

		return 1;
	}
};		// class TripletSearch

