#pragma once
#include <unordered_map>
#include <queue>
#include "index.h"

struct GHT_node_t
{
	vector<double> rdist;
	vector<int> pivot;
	vector<GHT_node_t> children;
};

class GHT_t : public index_t
{
	GHT_node_t root;
	void select(size_t& pivot_cnt, vector<int>& objects, GHT_node_t* root);
	void _build(GHT_node_t* root, vector<int> objects, size_t pivot_size);
	void _rangeSearch(const GHT_node_t* root, int query, double range, int& res_size);
	void _knnSearch(const GHT_node_t * root, int query, int k, priority_queue<double>& result, double& r);
public:

	GHT_t(db_t* db);
	void build();
	void rangeSearch(vector<int> &queries, double range, int& res_size) override;
	void knnSearch(vector<int> &queries, int k, double& ave_r) override;
};