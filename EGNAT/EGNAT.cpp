#include "EGNAT.h"


void EGNAT::build(char *dataFileName, char*indexFileName) {


	ifstream dataFile(dataFileName, ios::in);
	dataFile >> dim >> objNum >> func;


	leafPageObjCnt = (pageSize - sizeof(int)) / (sizeof(double)+dim*sizeof(float));
	internalPageObjCnt = 0;
	while (sizeof(int)+internalPageObjCnt*dim*sizeof(float) + internalPageObjCnt*sizeof(int) + internalPageObjCnt * internalPageObjCnt * 2 * sizeof(double) < pageSize) internalPageObjCnt++;
	internalPageObjCnt -= 1;
	cout << "dim=" << dim << ",objNum=" << objNum << endl;
	cout << "leafPageObjCnt=" << leafPageObjCnt << ",internalPageObjCnt=" << internalPageObjCnt << endl;
	writeIndexHeader(indexFileName);
	indexFile.create(indexFileName);

	float *obj = (float*)malloc(dim*sizeof(float));
	for (int i = 0; i < objNum; i++) {
		for (int j = 0; j < dim; j++) dataFile >> obj[j];
		insert(0, obj,-1);
	}
	dataFile.close();
	free(obj);
	obj = NULL;
	indexFile.close();
}


void EGNAT::insert(int pageNo, float* obj,double dist) {
	LeafPage *leafPage = new LeafPage();
	InternalPage *internalPage = new InternalPage();
	int header;
	header = indexFile.readPage(pageNo, leafPage, internalPage);

	if(header>=0){

		delete internalPage;
		if (leafPage->num < leafPageObjCnt) {
			for (int i = 0; i < dim; i++) {
				leafPage->leaves[leafPage->num][i] = obj[i];
			}
			leafPage->dist_to_parent[leafPage->num] = dist;
			leafPage->num++;
			indexFile.writePage(pageNo, leafPage);
		}
		else {
			//select pivots
			InternalPage *newpage = new InternalPage();
			bool *selected = (bool*)malloc((leafPageObjCnt + 1)*sizeof(bool));
			memset(selected, false, (leafPageObjCnt + 1)*sizeof(bool));
			srand((unsigned)time(NULL));
			int count = 0;
			while (count < internalPageObjCnt) {
				int index = rand() % (leafPageObjCnt + 1);
				if (selected[index]) continue;
				if (index == leafPageObjCnt) {
					for (int j = 0; j < dim; j++) newpage->pivots[count][j] = obj[j];
				}
				else {
					for (int j = 0; j < dim; j++) newpage->pivots[count][j] = leafPage->leaves[index][j];
				}
				newpage->max[count][count] = 0;
				newpage->min[count][count] = 0;
				selected[index] = true;
				count++;
			}
			//calculate distances between pivots
			for (int i = 0; i < internalPageObjCnt; i++) {
				for (int j = i + 1; j < internalPageObjCnt; j++) {
					double tmp = distance(newpage->pivots[i], newpage->pivots[j]);
					newpage->max[i][j] = tmp;
					newpage->min[i][j] = tmp;
					newpage->max[j][i] = tmp;
					newpage->min[j][i] = tmp;
				}
			}
			//distribute objects
			double *distArray = (double*)malloc(internalPageObjCnt*sizeof(double));
			LeafPage **leafPages=(LeafPage**)malloc(internalPageObjCnt*sizeof(LeafPage*));
			for (int i = 0; i < internalPageObjCnt; i++) leafPages[i] =  new LeafPage();//(LeafPage*)malloc(pageSize);
			for (int i = 0; i < leafPageObjCnt+1; i++) {
				if (!selected[i]) {
					float *tmpData;
					if(i==leafPageObjCnt) tmpData = obj;
					else tmpData = leafPage->leaves[i];

					distArray[0] = distance(tmpData, newpage->pivots[0]);
					double minDist = distArray[0];
					int minId = 0;
					for (int j = 1; j < internalPageObjCnt; j++) {
						distArray[j] = distance(tmpData,newpage->pivots[j]);
						if (distArray[j] < minDist) {
							minDist = distArray[j];
							minId = j;
						}
					}
					for (int j = 0; j < internalPageObjCnt; j++) {
						newpage->max[j][minId] = newpage->max[j][minId]>distArray[j] ? newpage->max[j][minId]: distArray[j];
						newpage->min[j][minId] = newpage->min[j][minId]<distArray[j] ? newpage->min[j][minId]: distArray[j];
					}
					int index = leafPages[minId]->num;
					for (int j = 0; j < dim;j++)leafPages[minId]->leaves[index][j]=tmpData[j];
					leafPages[minId]->dist_to_parent[index] = minDist;
					leafPages[minId]->num++;
				}
			}

			for (int i = 0; i < internalPageObjCnt; i++) {
				if (leafPages[i]->num > 0) {
					newpage->childPageNo[i] = indexFile.pageNum;
					indexFile.writePage(indexFile.pageNum, leafPages[i]);
					indexFile.pageNum++;
				}
			}
			indexFile.writePage(pageNo, newpage);
			delete newpage;
			newpage = NULL;

			free(distArray);
			distArray = NULL;
			for (int i = 0; i < internalPageObjCnt; i++) {
				delete leafPages[i];
				leafPages[i] = NULL;
			}
			free(selected);
			selected = NULL;
			free(leafPages);
			leafPages = NULL;
		}
		delete leafPage;
		leafPage = NULL;
	}

	else {

		delete leafPage;
		double *distArray = (double*)malloc(internalPageObjCnt*sizeof(double));
		distArray[0] = distance(obj, internalPage->pivots[0]);
		double minDist = distArray[0];
		int minId = 0;
		for (int i = 1; i < internalPageObjCnt; i++) {
			distArray[i] = distance(obj,internalPage->pivots[i]);
			if (distArray[i] < minDist) {
				minDist = distArray[i];
				minId = i;
			}
		}
		for (int i = 0; i < internalPageObjCnt; i++) {
			internalPage->max[i][minId] = internalPage->max[i][minId]>distArray[i] ? internalPage->max[i][minId]: distArray[i];
			internalPage->min[i][minId] = internalPage->min[i][minId]<distArray[i] ? internalPage->min[i][minId]: distArray[i];
		}

		int nextPageNo = internalPage->childPageNo[minId];
		if (nextPageNo >= 0) {
			indexFile.writePage(pageNo, internalPage);
			delete internalPage;
			internalPage = NULL;
			insert(nextPageNo, obj, distArray[minId]);
		}
		else {
			LeafPage *newLeafPage = new LeafPage();
			for (int i = 0; i < dim; i++) {
				newLeafPage->leaves[0][i] = obj[i];
			}
			newLeafPage->dist_to_parent[0] = distArray[minId];
			newLeafPage->num++;
			internalPage->childPageNo[minId] = indexFile.pageNum;
			indexFile.writePage(indexFile.pageNum, newLeafPage);
			indexFile.pageNum++;
			indexFile.writePage(pageNo, internalPage);
			delete internalPage;
			internalPage = NULL;
			delete newLeafPage;
			newLeafPage = NULL;
		}

		free(distArray);
		distArray = NULL;

	}
}


void EGNAT::bulkLoad(char *dataFileName, char*indexFileName) {
	ifstream dataFile(dataFileName, ios::in);
	dataFile >> dim >> objNum >> func;

	leafPageObjCnt = (pageSize - sizeof(int)) / (sizeof(double) + dim*sizeof(float));
	internalPageObjCnt = 0;
	while (sizeof(int) + internalPageObjCnt*dim*sizeof(float) + internalPageObjCnt*sizeof(int) + internalPageObjCnt * internalPageObjCnt * 2 * sizeof(double) < pageSize) internalPageObjCnt++;
	internalPageObjCnt -= 1;

	path = dataFileName;
	string tmpFileName = string(path) + ".0";
	prepare(dataFile, tmpFileName);
	writeIndexHeader(indexFileName);

	indexFile.create(indexFileName);
	_bulkLoad(0, objNum);
	indexFile.close();
}

void EGNAT::_bulkLoad(int pageNo, int objCnt) {
	EGNATfile curTmpFile;
	curTmpFile.open((string(path) + "." + to_string(pageNo)).c_str());
	if (objCnt <= leafPageObjCnt) {
		LeafPage *leafPage = new LeafPage();
		curTmpFile.readPage(0, leafPage, NULL);
		indexFile.writePage(pageNo, (Page*)leafPage);
		delete leafPage;
		curTmpFile.close();
		remove((string(path) + "." + to_string(pageNo)).c_str());
	}
	else {
		InternalPage *internalPage = new InternalPage();
		LeafPage *tmpPage = new LeafPage();
		bool *selected = (bool*)malloc(objCnt*sizeof(bool));
		memset(selected, false, objCnt*sizeof(bool));
		//select pivots
		srand((unsigned)time(NULL));
		int count = 0;
		while (count < internalPageObjCnt) {
			//int index = rand() % objCnt;
			int index = (rand() * RAND_MAX + rand()) % objCnt;
			if (selected[index])continue;
			selected[index] = true;
			int tmpPageNo;
			int tmpPos;
			getPivotPos(index,tmpPageNo, tmpPos);
			tmpPage->init();
			curTmpFile.readPage(tmpPageNo, tmpPage, NULL);
			for (int i = 0; i < dim; i++) internalPage->pivots[count][i] = tmpPage->leaves[tmpPos][i];
			internalPage->max[count][count] = 0;
			internalPage->min[count][count] = 0;
			count++;
		}


		for (int i = 0; i < internalPageObjCnt; i++) {
			for (int j = i + 1; j < internalPageObjCnt; j++) {
				double dist = distance(internalPage->pivots[i], internalPage->pivots[j]);
				internalPage->max[i][j] = dist;
				internalPage->min[i][j] = dist;
				internalPage->max[j][i] = dist;
				internalPage->min[j][i] = dist;
			}
		}
		vector<EGNATfile> childFiles(internalPageObjCnt);
		vector<LeafPage*> childPages(internalPageObjCnt);
		vector<int> childCnt(internalPageObjCnt, 0);
		vector<int> childPageNo(internalPageObjCnt, -1);
		for (int i = 0; i < internalPageObjCnt; i++) {
			childFiles[i].create((string(path) + "." + to_string(indexFile.pageNum + i)).c_str());
			childPages[i] = new LeafPage();
			internalPage->childPageNo[i] = indexFile.pageNum  + i;
			childPageNo[i] = internalPage->childPageNo[i];
		}
		double *distArray = (double*)malloc(internalPageObjCnt*sizeof(double));
		memset(distArray, 0, internalPageObjCnt*sizeof(double));
		int pos = 0;
		for (int i = 0; i < objCnt; i++) {
			if (i%leafPageObjCnt == 0) {
				tmpPage->init();
				curTmpFile.readPage(i / leafPageObjCnt, tmpPage, NULL);
				pos = 0;
			}
			if (selected[i]) {
				pos++;
				continue;
			}
			double minDist = INFINITY;
			int minId = -1;
			for (int j = 0; j < internalPageObjCnt; j++) {
				distArray[j] = distance(tmpPage->leaves[pos], internalPage->pivots[j]);
				if (distArray[j] < minDist) {
					minDist = distArray[j];
					minId = j;
				}
			}

			for (int j = 0; j < internalPageObjCnt; j++) {
				internalPage->max[j][minId] = internalPage->max[j][minId]>distArray[j] ? internalPage->max[j][minId] : distArray[j];
				internalPage->min[j][minId] = internalPage->min[j][minId]<distArray[j] ? internalPage->min[j][minId] : distArray[j];
			}
			childCnt[minId]++;
			int index = childPages[minId]->num;
			for (int j = 0; j < dim; j++) childPages[minId]->leaves[index][j] = tmpPage->leaves[pos][j];
			childPages[minId]->dist_to_parent[index] = minDist;
			childPages[minId]->num++;
			if (childPages[minId]->num%leafPageObjCnt == 0) {
				childFiles[minId].writePage((childCnt[minId]/ leafPageObjCnt)-1, (Page*)childPages[minId]);
				childPages[minId]->init();
			}
			pos++;
		}
		if (selected != NULL) {
			free(selected); selected = NULL;
		}
		if (distArray != NULL) {
			free(distArray); distArray = NULL;
		}
		curTmpFile.close();
		remove((string(path) + "." + to_string(pageNo)).c_str());
		for (int i = 0; i < internalPageObjCnt; i++) {
			childFiles[i].writePage(childCnt[i] / leafPageObjCnt, childPages[i]);
			childFiles[i].close();
			delete childPages[i];
			indexFile.pageNum++;
		}
		vector<EGNATfile>().swap(childFiles);
		vector<LeafPage*>().swap(childPages);
		indexFile.writePage(pageNo, (Page*)internalPage);
		delete tmpPage;
		delete internalPage;
		for (int i = 0; i < internalPageObjCnt; i++) {
			_bulkLoad(childPageNo[i],childCnt[i]);
		}
		vector<int>().swap(childPageNo);
		vector<int>().swap(childCnt);

	}
}


void EGNAT::openIndex(char* indexFileName) {
	readIndexHeader(indexFileName);
	indexFile.open(indexFileName);
}

void EGNAT::writeIndexHeader(char *indexFileName) {
	ofstream indexHeader((string(indexFileName) + ".header").c_str(), ios::out);
	indexHeader << dim << endl;
	indexHeader << objNum << endl;
	indexHeader << func << endl;
	indexHeader << leafPageObjCnt << endl;
	indexHeader << internalPageObjCnt << endl;
	indexHeader.close();
}

void EGNAT::readIndexHeader(char* indexFileName) {
	ifstream indexHeader((string(indexFileName) + ".header").c_str(), ios::in);
	indexHeader >> dim;
	indexHeader >> objNum;
	indexHeader >> func;
	indexHeader >> leafPageObjCnt;
	indexHeader >> internalPageObjCnt;
	indexHeader.close();
}

void EGNAT::prepare(ifstream& dataFile, string tmpFileName) {
	EGNATfile tmpFile;
	tmpFile.create(tmpFileName.c_str());

	int pageCnt = objNum / leafPageObjCnt;

	LeafPage *leafPage = new LeafPage();
	for (int i = 0; i < pageCnt; i++) {
		for (int j = 0; j < leafPageObjCnt; j++) {
			for (int k = 0; k < dim; k++) {
				dataFile >> leafPage->leaves[j][k];

			}
			leafPage->dist_to_parent[j] = -1;

		}
		leafPage->num = leafPageObjCnt;
		tmpFile.writePage(i, (Page*)leafPage);
	}


	int start = leafPageObjCnt*pageCnt;
	int num = objNum - start;
	leafPage->num = num;
	for (int i = 0; i < num; i++) {
		for (int k = 0; k < dim; k++) dataFile >> leafPage->leaves[i][k];
	}
	tmpFile.writePage(pageCnt, (Page*)leafPage);
	tmpFile.close();
	delete leafPage;
}

void EGNAT::getPivotPos(int index, int& tmpPageNo, int& tmpPos) {
	tmpPageNo = index / leafPageObjCnt;
	tmpPos = index % leafPageObjCnt;
}

double EGNAT::distance(float *data1, float *data2) {
	compdists++;
	double tot = 0, dif;
	if (func == 1) {
		for (int i = 0; i<dim; i++) {
			dif = (data1[i] - data2[i]);
			if (dif < 0) dif = -dif;
			tot += dif;
		}
	}
	else if (func == 2) {
		for (int i = 0; i<dim; i++) {
			tot += pow(data1[i] - data2[i], 2);
		}
		tot = sqrt(tot);
	}
	else {
		double max = 0;
		for (int i = 0; i<dim; i++)
		{
			dif = (data1[i] - data2[i]);
			if (dif < 0) dif = -dif;
			if (dif > max) max = dif;
		}
		tot = max;
	}
	return tot;
}

int EGNAT::_rangeSearch(int pageNo, float *query, double radius,double p_q_dist) {
	int resultNum = 0;
	LeafPage *leafPage = new LeafPage();
	InternalPage *internalPage = new InternalPage();
	int header = indexFile.readPage(pageNo,leafPage,internalPage);

	if (header >= 0) {
		delete internalPage;
		double dist=0;
		for (int i = 0; i < leafPage->num; i++) {
			if (fabs(leafPage->dist_to_parent[i] - p_q_dist) <= radius) {
				dist = distance(query, leafPage->leaves[i]);
				if (dist <= radius) resultNum++;
			}
		}
		delete leafPage;
	}
	else {

		delete leafPage;
		double *dist = (double*)malloc(internalPageObjCnt*sizeof(double));
		dist[0] = distance(query, internalPage->pivots[0]);
		double minDist = dist[0];
		int minId = 0;
		for (int i = 1; i < internalPageObjCnt; i++) {
			dist[i] = distance(query,internalPage->pivots[i]);
			if (dist[i] < minDist) {
				minDist = dist[i];
				minId = i;
			}
		}
		int min = dist[minId] - radius;
		int max = dist[minId] + radius;
		int sum = 2 * radius;
		for (int i = 0; i < internalPageObjCnt; i++) {
			int sum2 = internalPage->max[minId][i] - internalPage->min[minId][i];
			int upper = max > internalPage->max[minId][i] ? max : internalPage->max[minId][i];
			int lower = min < internalPage->min[minId][i] ? min : internalPage->min[minId][i];
			if((upper-lower)<=(sum2 + sum)){
				if (dist[i] <= radius) resultNum++;
				if (internalPage->childPageNo[i] > 0) {
					resultNum += _rangeSearch(internalPage->childPageNo[i], query, radius, dist[i]);
				}
			}

		}
		delete internalPage;
		free(dist);
		dist = NULL;
	}
	return resultNum;
}

int EGNAT::rangeSearch(float *query, double radius) {
	return _rangeSearch(0, query, radius, 0);
}

void EGNAT::_knnSearch(int pageNo, float *query, int k,ResultSet *resultSet, double p_q_dist) {
	LeafPage *leafPage = new LeafPage();
	InternalPage *internalPage = new InternalPage();
	int header = indexFile.readPage(pageNo, leafPage, internalPage);

	if (header >= 0) {

		delete internalPage;
		double dist = 0;
		for (int i = 0; i < leafPage->num; i++) {
			if (resultSet->num < k || fabs(leafPage->dist_to_parent[i] - p_q_dist) <= resultSet->dist[k-1]) {
				dist = distance(query, leafPage->leaves[i]);
				if (resultSet->num < k || dist < resultSet->dist[k - 1]) {
					resultSet->addElement(leafPage->leaves[i],dist);
				}
			}
		}
		delete leafPage;
	}
	else {

		delete leafPage;
		double *dist = (double*)malloc(internalPageObjCnt*sizeof(double));
		dist[0] = distance(query, internalPage->pivots[0]);
		double minDist = dist[0];
		int minId = 0;
		for (int i = 1; i < internalPageObjCnt; i++) {
			dist[i] = distance(query, internalPage->pivots[i]);
			if (dist[i] < minDist) {
				minDist = dist[i];
				minId = i;
			}
		}

		for (int i = 0; i < internalPageObjCnt; i++) {
			int min = dist[minId] - resultSet->dist[k - 1];
			int max = dist[minId] + resultSet->dist[k - 1];
			int sum = 2 * resultSet->dist[k - 1];
			int sum2 = internalPage->max[minId][i] - internalPage->min[minId][i];
			int upper = max > internalPage->max[minId][i] ? max : internalPage->max[minId][i];
			int lower = min < internalPage->min[minId][i] ? min : internalPage->min[minId][i];
			if (resultSet->num < k || (upper - lower) <= (sum2 + sum)) {
				if (resultSet->num < k || dist[i] < resultSet->dist[k - 1]) {
					resultSet->addElement(internalPage->pivots[i],dist[i]);
				}
				if (internalPage->childPageNo[i] > 0) {
					_knnSearch(internalPage->childPageNo[i], query, k, resultSet, dist[i]);
				}
			}

		}
		delete internalPage;
		free(dist);
		dist = NULL;
	}
}

double EGNAT::knnSearch(float *query, int k) {
	ResultSet *resultSet = new ResultSet(k);
	_knnSearch(0, query, k, resultSet, 0);
	return resultSet->dist[k-1];
}
