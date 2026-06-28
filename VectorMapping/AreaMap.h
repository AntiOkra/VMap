#pragma once

#include <memory>
#include <vector>
#include "MzPoint.h"
#include "AdxNode.h"

class SpatialGridCell
{
public:
	std::vector<AdxNode*> nodes_;

	int FindNearestNode(CMzPoint& point, AdxNode** node, double& distance);
	bool IsEmpty() const;
};

class SpatialGrid
{
public:
	std::vector<SpatialGridCell> cells_;

	int min_index_[3];
	int max_index_[3];
	int x_count_;
	int y_count_;
	int z_count_;

	CMzPoint origin_;
	double cell_size_;

	int Build(std::vector<std::unique_ptr<AdxNode>>& nodes, double pitch);
	int Initialize(CMzPoint& min_point, CMzPoint& max_point, double pitch);
	int GetCellCoordinates(CMzPoint& point, int& x, int& y, int& z);
	int AddNode(AdxNode* node);
	int Clear();

	int GetCellIndex(int& x, int& y, int& z, int& cell_index);
};
