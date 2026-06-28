#pragma once

#include <memory>
#include <vector>
#include <utility>
#include"AdxNode.h"
#include"SpatialGrid.h"
#include"MzPoint.h"
#include"NastranModel.h"

class SurfaceNodeMap
{
public:
	SurfaceNodeMap();
	~SurfaceNodeMap();

	void SetLogFile(CStdioFile* log_file);
	void AddNode(std::unique_ptr<AdxNode> node);
	const CMzPoint& MappedForce() const;
	const CMzPoint& LossForce() const;

	int MapForces(NastranModel& nastran, double upper_limit, CMzPoint ratio);
	int FindNearestNode(CMzPoint& point, double upper_limit, AdxNode** node, double& distance);
	int FindNearestNodeByBruteForce(CMzPoint& point, double upper_limit, AdxNode** node, double& distance);
	int BuildSpatialGrid();
	int ExportAdxForces(CString& fpath, CString& process);
	void WriteLog(CString &msg);
	int DumpNodes(CString& fpath);
	int Dump(CString& fpath);
	void Clear();

private:
	int FindNearestNodes(CMzPoint& point, double upper_limit, int max_count, std::vector<std::pair<AdxNode*, double>>& nearest_nodes);
	void DistributeForce(const CMzPoint& force, const std::vector<std::pair<AdxNode*, double>>& nearest_nodes);
	std::vector<std::unique_ptr<AdxNode>>	nodes_;
	SpatialGrid				spatial_grid_;
	CMzPoint				mapped_force_;
	CMzPoint				loss_force_;

	CStdioFile*				log_file_;
};
