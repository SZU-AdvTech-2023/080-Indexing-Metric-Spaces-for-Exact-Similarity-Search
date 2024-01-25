#ifndef EGNATFILE
#define EGNATFILE
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
extern int dim;
extern int internalPageObjCnt;
extern int leafPageObjCnt;
#define INFINITY 100000
using namespace std;

class Page {
public:
	int num;
};

class InternalPage :public Page {
public:
	float **pivots;
	double **max;
	double **min;
	int *childPageNo;
	InternalPage() {
		pivots = (float**)malloc(internalPageObjCnt*sizeof(float*));
		max = (double**)malloc(internalPageObjCnt*sizeof(double*));
		min = (double**)malloc(internalPageObjCnt*sizeof(double*));
		for (int i = 0; i < internalPageObjCnt; i++) {
			pivots[i] = (float*)malloc(dim*sizeof(float));
			memset(pivots[i], 0, dim*sizeof(float));
			max[i] = (double*)malloc(internalPageObjCnt*sizeof(double));
			min[i] = (double*)malloc(internalPageObjCnt*sizeof(double));
			memset(max[i], -1, internalPageObjCnt*sizeof(double));
			memset(min[i], INFINITY, internalPageObjCnt*sizeof(double));
		}

		childPageNo = (int*)malloc(internalPageObjCnt*sizeof(int));
		memset(childPageNo, -1, internalPageObjCnt*sizeof(int));
		num = -1;
	}

	~InternalPage() {
		if (pivots != NULL) {
			for (int i = 0; i < internalPageObjCnt; i++) {
				if (pivots[i] != NULL) {
					free(pivots[i]);
					pivots[i] = NULL;
				}
			}
			free(pivots);
			pivots = NULL;
		}
		if (max != NULL) {
			for (int i = 0; i < internalPageObjCnt; i++) {
				if (max[i] != NULL) {
					free(max[i]);
					max[i] = NULL;
				}
			}
			free(max);
			max = NULL;
		}
		if (min != NULL) {
			for (int i = 0; i < internalPageObjCnt; i++) {
				if (min[i] != NULL) {
					free(min[i]);
					min[i] = NULL;
				}
			}
			free(min);
			min = NULL;
		}
		if (childPageNo != NULL) {
			free(childPageNo);
			childPageNo = NULL;
		}
	}

	void freePage() {
		if (pivots != NULL) {
			for (int i = 0; i < internalPageObjCnt; i++) {
				if (pivots[i] != NULL) {
					free(pivots[i]);
					pivots[i] = NULL;
				}
			}
			free(pivots);
			pivots = NULL;
		}
		if (max != NULL) {
			free(max);
			max = NULL;
		}
		if (min != NULL) {
			free(min);
			min = NULL;
		}
		if (childPageNo != NULL) {
			free(childPageNo);
			childPageNo = NULL;
		}
	}
};

class LeafPage :public Page {
public:
	float **leaves;
	double *dist_to_parent;
	LeafPage() {
		leaves = (float**)malloc(leafPageObjCnt*sizeof(float*));
		for (int i = 0; i < leafPageObjCnt; i++) {
			leaves[i] = (float*)malloc(dim*sizeof(float));
			memset(leaves[i], 0, dim*sizeof(float));
		}
		dist_to_parent = (double*)malloc(leafPageObjCnt*sizeof(double));
		memset(dist_to_parent, -1, leafPageObjCnt*sizeof(double));
		num = 0;
	}
	void init() {
		for (int i = 0; i < leafPageObjCnt; i++) {
			memset(leaves[i], 0, dim*sizeof(float));
		}
		memset(dist_to_parent, -1, leafPageObjCnt*sizeof(double));
		num = 0;
	}
	~LeafPage() {
		if (leaves != NULL) {
			for (int i = 0; i < leafPageObjCnt; i++) {
				if (leaves[i] != NULL) {
					free(leaves[i]);
					leaves[i] = NULL;
				}
			}
			free(leaves);
			leaves = NULL;
		}
		if (dist_to_parent != NULL) {
			free(dist_to_parent);
			dist_to_parent = NULL;
		}
	}
	void freePage() {
		if (leaves != NULL) {
			for (int i = 0; i < leafPageObjCnt; i++) {
				if (leaves[i] != NULL) {
					free(leaves[i]);
					leaves[i] = NULL;
				}
			}
			free(leaves);
			leaves = NULL;
		}
		if (dist_to_parent != NULL) {
			free(dist_to_parent);
			dist_to_parent = NULL;
		}
	}
};


class EGNATfile {
public:
	EGNATfile() { isOpen = false; }
	void create(const char *finame);
	void open(const char *filename);
	void close();


	int readPage(double pageNo, LeafPage *leafPage, InternalPage *internalPage);
	void writePage(double pageNo, const Page *page);


	bool isOpened();
	void setOpen(bool state);

	int pageNum;
private:
	int fileHandle;
	bool isOpen;
};
#endif
