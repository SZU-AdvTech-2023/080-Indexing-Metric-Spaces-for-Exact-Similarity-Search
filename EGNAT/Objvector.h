#ifndef OBJVECTOR
#define OBJVECTOR

#include<cstdlib>
#include<vector>
using namespace std;
extern int dim;
extern int func;
extern int objNum;
class Objvector {
public:
	float *data;

	Objvector();
	Objvector(vector<float> vec);
	Objvector(const Objvector &obj);
	Objvector & operator = (const Objvector & obj);
	double distance(Objvector obj);
	~Objvector();
};
#endif
