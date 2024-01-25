
#include<iostream>
#include<fstream>
#include<time.h>
#include"EGNAT.h"
using namespace std;
int dim=2;
int func=2;
int objNum;
double compdists;
double IOread;
double IOwrite;
int pageSize;
int leafPageObjCnt;
int internalPageObjCnt;
int tmpPageObjCnt;
int main(int argc, char**argv) {
	clock_t begin = 0, end = 0;
	char *dataFileName;
	char *indexFileName;
	char *costFileName;
	char *queryFileName;


	dataFileName = argv[1];
	indexFileName = argv[2];
	costFileName = argv[3];
	queryFileName = argv[4];
	pageSize = atoi(argv[5]);

	ofstream costFile(costFileName,ios::out);
	cout << "pageSize=" << pageSize << endl;
	cout << "sizeof(InternalPage)=" << sizeof(InternalPage) << endl;
	cout << "sizeof(LeafPage)=" << sizeof(LeafPage) << endl;

    cout<<"start building EGNAT......"<<endl;
	IOread = 0;
	IOwrite = 0;
	compdists = 0;
	begin = clock();
	EGNAT egnat;
	//egnat.build(dataFileName,indexFileName);
	egnat.bulkLoad(dataFileName, indexFileName); //build EGNAT
	end = clock() - begin;
	costFile << "building..." << endl;
	costFile << "finished... " << (double)end / CLOCKS_PER_SEC << " build time" << endl;
	costFile << "finished... " << compdists << " distances computed" << endl;
	costFile << "finished... " << IOread << " IOreads" << endl;
	costFile << "finished... " << IOwrite << " IOwrites" << endl;

	int qcount = 100;
	double radius[7];
	int kvalues[] = { 1, 5, 10, 20, 50, 100 };

	if (string(dataFileName).find("LA") != -1) {
		double r[] = { 473, 692, 989, 1409, 1875, 2314, 3096 };
		memcpy(radius, r, sizeof(r));
		dim = 2;
	}
	else if (string(dataFileName).find("integer") != -1) {
		double r[] = { 2321, 2733, 3229, 3843, 4614, 5613, 7090 };
		memcpy(radius, r, sizeof(r));
		dim = 20;

	}
	else if (string(dataFileName).find("mpeg_1M") != -1) {
		double r[] = { 3838, 4092, 4399, 4773, 5241, 5904, 7104 };
		memcpy(radius, r, sizeof(r));
		dim = 282;

	}
	else if (string(dataFileName).find("sf") != -1) {
		double r[] = { 100, 200, 300, 400, 500, 600, 700 };
		memcpy(radius, r, sizeof(r));
	}

	float *query = (float*)malloc(dim*sizeof(float));
	memset(query, 0, dim*sizeof(float));

	egnat.openIndex(indexFileName);
	cout << "start knnSearching......" << endl;
	double resultRadius = 0;
	for (int i = 0; i < 6; i++) {
		IOread = 0;
		IOwrite = 0;
		compdists = 0;
		resultRadius = 0;
		begin = clock();
		ifstream queryFile(queryFileName, ios::in);
		for (int j = 0; j < qcount; j++) {
			for (int k = 0; k < dim; k++) {
				queryFile >> query[k];
			}
			resultRadius += egnat.knnSearch(query, kvalues[i]); //knnSearch
		}
		end = clock() - begin;
		costFile << "k: " << kvalues[i] << endl;
		costFile << "finished... " << (double)end / CLOCKS_PER_SEC / (double)qcount << " query time" << endl;
		costFile << "finished... " << compdists / (double)qcount << " distances computed" << endl;
		costFile << "finished... " << IOread / (double)qcount << " IO times" << endl;
		costFile << "finished... " << resultRadius / (double)qcount << " radius" << endl;
		queryFile.close();
	}

	cout << "start rangeSearching......" << endl;
	double resultNum = 0;
	for (int i = 0; i < 7; i++) {
		IOread = 0;
		IOwrite = 0;
		compdists = 0;
		begin = clock();
		resultNum = 0;
		ifstream queryFile(queryFileName, ios::in);
		for (int j = 0; j < qcount; j++) {
			for (int k = 0; k < dim; k++) {
				queryFile >> query[k];
			}
			resultNum += egnat.rangeSearch(query, radius[i]); //rangeSearch
		}
		end = clock() - begin;
		costFile << "radius: " << radius[i] << endl;
		costFile << "finished... " << (double)end / CLOCKS_PER_SEC / (double)qcount << " query time" << endl;
		costFile << "finished... " << compdists / (double)qcount << " distances computed" << endl;
		costFile << "finished... " << IOread / (double)qcount << " IO times" << endl;
		costFile << "finished... " << resultNum / (double)qcount << " objs" << endl;
		queryFile.close();
	}
	free(query);
	query = NULL;
	costFile.close();
	egnat.indexFile.close();
	return 0;
}
