#ifndef RESULTSET
#define RESULTSET
#include <malloc.h>
#include "EGNATfile.h"
extern int dim;
class ResultSet {
public:
	int num;
	int k;
	float **result;
	double *dist;
	ResultSet(int k) {
		num = 0;
		this->k = k;
		result = (float**)malloc((k+1)*sizeof(float*));
		for (int i = 0; i < k + 1; i++) {
			result[i] = (float*)malloc(dim*sizeof(float));
			memset(result[i], 0, dim*sizeof(float));
		}
		dist = (double*)malloc((k + 1)*sizeof(double));
		memset(dist, INFINITY, (k + 1)*sizeof(double));
	}

	void addElement(float *obj, double dist_to_query) {
		bool insert = false;
		for (int i = num-1; i >= 0; i--) {
			if (dist[i] > dist_to_query) {
				for (int j = 0; j < dim; j++) {
					result[i + 1][j] = result[i][j];
					dist[i + 1] = dist[i];
				}
			}
			else {
				for (int j = 0; j < dim; j++) {
					result[i + 1][j] = obj[j];
					dist[i + 1] = dist_to_query;
				}
				insert = true;
				break;
			}
		}
		if (!insert) {
			for (int j = 0; j < dim; j++) {
				result[0][j] = obj[j];
				dist[0] = dist_to_query;
			}
		}
		num++;
		if (num > k) num = k;
	}


	~ResultSet() {
		if (result != NULL) {
			for (int i = 0; i < k + 1; i++) {
				if (result[i] != NULL) {
					free(result[i]);
					result[i] = NULL;
				}
			}
			free(result);
			result = NULL;
		}

		if (dist != NULL) {
			free(dist);
			dist = NULL;
		}
		
	}
};
#endif
