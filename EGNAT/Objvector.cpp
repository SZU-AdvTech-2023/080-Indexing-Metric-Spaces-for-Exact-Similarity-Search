#include "Objvector.h"

Objvector::Objvector() {
	data = NULL;
}

Objvector::Objvector(vector<float> vec) {
	int len = vec.size();
	if (len != 0) {
		data= new float[len];
		for (int i = 0; i < len; i++) {
			data[i] = vec[i];
		}
	}
}

Objvector::Objvector(const Objvector &obj) {	
	data = new float[dim];
	for (int i = 0; i < dim; i++) {
		data[i] = obj.data[i];
	}	
}

Objvector & Objvector::operator = (const Objvector & obj) {
	if (data == NULL) data = new float[dim];
	for (int i = 0; i < dim; i++) {
		data[i] = obj.data[i];
	}
	return *this;
}
double Objvector::distance(Objvector obj) {
	double tot = 0, dif;
	if (func == 1) {
		for (int i = 0; i<dim; i++) {
			dif = (data[i] - obj.data[i]);
			if (dif < 0) dif = -dif;
			tot += dif;
		}
	}
	else if (func == 2) {
		for (int i = 0; i<dim; i++) {
			tot += pow(data[i] - obj.data[i], 2);
		}
		tot = sqrt(tot);
	}
	else {
		double max = 0;
		for (int i = 0; i<dim; i++)
		{
			dif = (data[i] - obj.data[i]);
			if (dif < 0) dif = -dif;
			if (dif > max) max = dif;
		}
		tot = max;
	}
	return tot;
}

Objvector::~Objvector() {
	if (data != NULL) {
		delete[] data;
		data = NULL;
	}	
}