#ifndef EGNATINCLUDED
#define EGNATINCLUDED

#include <stdlib.h>
#include <fstream>
#include "EGNATfile.h"
#include "ResultSet.h"
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
extern int dim;
extern int func;
extern int objNum;
extern int pageSize;
extern int leafPageObjCnt;
extern int internalPageObjCnt;
extern int tmpPageObjCnt;
extern double compdists;
using namespace std;


class EGNAT {
public:
	EGNATfile indexFile;
	char *path;
	void build(char *dataFileName, char*indexFileName);
	void insert(int pageNo, float* obj, double dist);
	void bulkLoad(char *dataFileName, char*indexFileName);
	void _bulkLoad(int pageNo,int objCnt);


	void openIndex(char* indexFileName);
	void writeIndexHeader(char *indexFileName);
	void readIndexHeader(char* indexFileName);

	void prepare(ifstream& dataFile, string tmpFileName);
	void getPivotPos(int index, int& tmpPageNo, int& tmpPos);
	double distance(float *data1, float *data2);

	int rangeSearch(float *query,double radius);
	int _rangeSearch(int pageNo,float *query, double radius, double p_q_dist);
	double knnSearch(float *query,int k);
	void _knnSearch(int pageNo, float *query, int k, ResultSet *resultSet, double p_q_dist);
};
#endif
