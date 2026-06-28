#include "StdAfx.h"
#include "SpatialGrid.h"

bool SpatialGridCell::IsEmpty() const
{
	return nodes_.empty();
}

int SpatialGrid::Build(std::vector<std::unique_ptr<AdxNode>>& nodes, double pitch)
{
	CMzPoint min_point;
	CMzPoint max_point;

	min_point.Set(1E100, 1E100, 1E100);
	max_point.Set(-1E100, -1E100, -1E100);

	for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
		AdxNode& node = *(nodes[i]);
		if (node.coord_.x < min_point.x) min_point.x = node.coord_.x;
		if (node.coord_.y < min_point.y) min_point.y = node.coord_.y;
		if (node.coord_.z < min_point.z) min_point.z = node.coord_.z;

		if (node.coord_.x > max_point.x) max_point.x = node.coord_.x;
		if (node.coord_.y > max_point.y) max_point.y = node.coord_.y;
		if (node.coord_.z > max_point.z) max_point.z = node.coord_.z;
	}

	Initialize(min_point, max_point, pitch);

	for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
		AddNode(nodes[i].get());
	}

	return 0;
}

int SpatialGrid::Initialize(CMzPoint& min_point, CMzPoint& max_point, double pitch)
{
	Clear();

	cell_size_ = pitch;

	GetCellCoordinates(min_point, min_index_[0], min_index_[1], min_index_[2]);
	GetCellCoordinates(max_point, max_index_[0], max_index_[1], max_index_[2]);

	x_count_ = max_index_[0] - min_index_[0] + 1;
	y_count_ = max_index_[1] - min_index_[1] + 1;
	z_count_ = max_index_[2] - min_index_[2] + 1;

	cells_.resize(x_count_ * y_count_ * z_count_);

	return 0;
}

int SpatialGrid::GetCellCoordinates(CMzPoint& point, int& x, int& y, int& z)
{
	x = int(floor(point.x / cell_size_));
	y = int(floor(point.y / cell_size_));
	z = int(floor(point.z / cell_size_));

	return 0;
}

int SpatialGrid::GetCellIndex(int& x, int& y, int& z, int& cell_index)
{
	cell_index = -1;

	if (x < min_index_[0] || x > max_index_[0]) {
		return 1;
	}

	if (y < min_index_[1] || y > max_index_[1]) {
		return 1;
	}

	if (z < min_index_[2] || z > max_index_[2]) {
		return 1;
	}

	cell_index = (z - min_index_[2]) * x_count_ * y_count_ + (y - min_index_[1]) * x_count_ + (x - min_index_[0]);

	return 0;
}

double SpatialGrid::CellSize() const
{
	return cell_size_;
}

int SpatialGrid::Clear()
{
	cells_.clear();

	x_count_ = 0;
	y_count_ = 0;
	z_count_ = 0;
	cell_size_ = 0.0;
	origin_.Set(0.0, 0.0, 0.0);

	for (int i = 0; i < 3; i++) {
		min_index_[i] = 0;
		max_index_[i] = 0;
	}

	return 0;
}

int SpatialGrid::AddNode(AdxNode* node)
{
	int x, y, z;

	if (GetCellCoordinates(node->coord_, x, y, z) != 0) {
		return 1;
	}

	int cell_index;
	if (GetCellIndex(x, y, z, cell_index) != 0) {
		return 1;
	}

	if (cell_index < 0 || cell_index >= static_cast<int>(cells_.size())) {
		return 1;
	}

	SpatialGridCell& cell = cells_[cell_index];
	cell.nodes_.push_back(node);

	return 0;
}

int SpatialGridCell::FindNearestNode(CMzPoint& point, AdxNode** node, double& distance)
{
	*node = NULL;
	distance = 1E100;

	AdxNode* nearest_node = NULL;
	double min_distance2 = 1E100;

	for (int i = 0; i < static_cast<int>(nodes_.size()); i++) {
		AdxNode& candidate = *(nodes_[i]);
		double distance2 = point.DistanceSquared(candidate.coord_);
		if (distance2 < min_distance2) {
			nearest_node = nodes_[i];
			min_distance2 = distance2;
		}
	}

	if (nearest_node != NULL) {
		*node = nearest_node;
		distance = sqrt(min_distance2);
		return 0;
	}

	distance = 1E10;
	return 1;
}