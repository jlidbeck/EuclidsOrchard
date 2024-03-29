// EuclidsOrchard.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Euclid.h"

typedef TripletTriangle<int> mat3;
typedef Triplet<int> vec3;


int main(int argc, const char ** argv)
{
	if(argc > 1)
	{
		// EuclidsOrchard /e 8
		// enumerates all triplets to depth
		if(string(argv[1]) == "/e") {
			TripletSearch<int> s;
			if(argc >= 2) {
				int depth = atoi(argv[2]);
				s.m_maxDepth = depth;
				if(argc >= 3) {
					s.m_maxHeight = atoi(argv[3]);
				}
			}

			// set a tiny clip region to test efficiency of sextant method--
			// should search only 1/6 for several levels
			vec3 pt(rand(), rand(), rand());
			s.m_clip.set(pt, pt + vec3(1, 0, 0), pt + vec3(0, 1, 0));
			s.enumerate();
			s.dumpEnumerationResults();
			return 0;
		}

		//	EuclidsOrchard /s 16,42,5
		//	searches for given triplet
		if(string(argv[1]) == "/s") 
		{
			vec3 triplet(3, 2, 1);
			std::stringstream ss(argv[2]);
			ss >> triplet;
			if(!triplet.isCoprime()) {
				cerr << "Not coprime: " << triplet << endl;
				return 0;
			}

			TripletSearch<int> s;
			s.m_maxDepth = 1050;
			s.search(triplet);
			s.dumpSearchResults();
			return 0;
		}
	}

	TripletSearch<int>::runTests();



	vec3 target(
		121, 77, 27
		//107, 92, 43
	);

	char cmd = 0;
	mat3 m;
	int repeat = 1;
	while(1) 
	{
		cout << m;

		if(--repeat <= 0) {
			char c;
			cout << ">";
			cin >> c;
			if(c == '.') {
				repeat = 10;
			}
			else {
				cmd = c;
			}
		}

		if(cmd == 'q') {
			break;
		}
		int fromCol, toCol;
		if(m.operate(cmd)) 
		{
			// char was cmd--done
			cout << "op: " << cmd << endl;
		}
		else 
		{
			switch(cmd) 
			{

			case '.': repeat = 10; break;
			case 'r':
				fromCol = rand() % 3;
				toCol = (fromCol + rand() % 2 + 1) % 3;
				m.merge(fromCol, toCol);
				break;
			case '0': m = mat3(); continue;
			case 'e':
				{
					TripletSearch<int> s;
					s.m_maxDepth = 8;
					s.enumerate();
					s.dumpEnumerationResults();
				}
				break;

			default: cout << "wut.\n"; continue;
			}
		}
	}

    return 0;
}

