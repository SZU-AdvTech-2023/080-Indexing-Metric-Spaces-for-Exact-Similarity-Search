#include "GHT.h"
#include <algorithm>
#include <fstream>
using namespace std;
#include <iostream>
#include <chrono>
using namespace chrono;

GHT_t::GHT_t(db_t* db) :
	index_t(db)
{
}

void GHT_t::build()
{
	vector<int> objects;
	for (int i = 0; i < (int)db->size(); ++i) {
		objects.push_back(i);
	}
	random_shuffle(objects.begin(), objects.end());
	cout << "database size: " << objects.size() << endl;
	_build(&root, objects, 2);
}

void GHT_t::select(size_t& pivot_cnt, vector<int>& objects, GHT_node_t* root)
{
	auto& pivot = root->pivot;

	for (int i = 0; i < pivot_cnt; i++) {
		int ind = rand() % objects.size();
		pivot.push_back(objects[ind]);
		objects.erase(objects.begin() + ind);
		if (objects.empty()) {
			pivot_cnt = pivot.size();
			return;
		}
	}
}

void GHT_t::_build(GHT_node_t * root, vector<int> objects, size_t pivot_cnt)
{
	if (objects.empty()) {
		return;
	}
	auto& pivot = root->pivot;
	auto& children = root->children;
	auto& rdist = root->rdist;
	rdist.resize(pivot_cnt);
	for (auto &r : rdist) {
		r = 0;
	}
	select(pivot_cnt, objects, root);
	static size_t completed = 0;
	completed += pivot_cnt;	
	if (objects.empty()) {
		return;
	}
	vector<vector<int>> objs_children(pivot_cnt);
	for (int obj : objects) {
		vector<double> dist_pivot(pivot_cnt);
		for (size_t i = 0; i < pivot_cnt; ++i) {
			dist_pivot[i] = dist(obj, pivot[i]);
		}
		size_t closest_pivot = min_element(dist_pivot.begin(), dist_pivot.end()) - dist_pivot.begin();
		objs_children[closest_pivot].push_back(obj);
		rdist[closest_pivot] = max(rdist[closest_pivot], dist_pivot[closest_pivot]);
	}
	children.resize(pivot_cnt);
	for (size_t i = 0; i < pivot_cnt; ++i) {
		_build(&children[i], objs_children[i], pivot_cnt);
	}
}

void GHT_t::_rangeSearch(const GHT_node_t * root, int query, double range, int& res_size)
{
	auto& pivot = root->pivot;
	auto& children = root->children;
	auto& rdist = root->rdist;

	vector<double> dists(pivot.size(), 0);
	int f;
	double mind = -1;
	for (int i = 0; i < pivot.size(); i++)
	{
		dists[i] = dist(query, pivot[i]);
		if (dists[i] <= range)
		{
			res_size++;
		}
		if ((mind == -1) || (dists[i] < mind)) { mind = dists[i]; f = i; }
	}
	if (children.empty())
		return;

	/* Now we know the closest one. get into all that can be */
	for (int i = 0; i < pivot.size(); i++)
	{
		if ((dists[i] - range <= mind + range) && (dists[i] - range <= rdist[i]))
			_rangeSearch(&children[i], query, range, res_size);
	}
}

void GHT_t::rangeSearch(vector<int> &queries, double range, int& res_size)
{
	for (auto query : queries)
		_rangeSearch(&root, query, range, res_size);
}

static void addResult(int k, double d, priority_queue<double>& result, double& r) {
	if (result.size() < k || d < result.top()) {
		result.push(d);		
	}
	if (result.size() > k) {		
		result.pop();		
	}
	r = result.top();
}

void GHT_t::_knnSearch(const GHT_node_t * root, int query, int k, priority_queue<double>& result, double& r)
{
	auto& pivot = root->pivot;
	auto& children = root->children;
	auto& rdist = root->rdist;
	vector<pair<double, int>> od(pivot.size());
	for (int i = 0; i < pivot.size(); i++)
	{
		od[i].first = dist(query, pivot[i]);
		od[i].second = i;
		addResult(k, od[i].first, result, r);
	}
	if (children.empty())
		return;
	sort(od.begin(), od.end());
	for (int i = 0; i < pivot.size(); i++)
	{
		if (result.size() == k && ((od[i].first - od[0].first) / 2) > result.top())
			break;
		if (result.empty() || (od[i].first - result.top() <= rdist[od[i].second]))
			_knnSearch(&children[od[i].second], query, k, result, r);
	}
	
}

void GHT_t::knnSearch(vector<int> &queries, int k, double& ave_r)
{
	double r = 0;
	for (int q : queries) {
		priority_queue<double> result;
		//_knnSearch(&root, q, k, result, ave_r);
		_knnSearch(&root, q, k, result, r);
		ave_r += r;
	}
}
