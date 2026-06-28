#pragma once

#include <memory>
#include <vector>
#include"AdxNode.h"
#include"SpatialGrid.h"
#include"MzPoint.h"
#include"NastranModel.h"

class SurfaceNodeMap
{
public:
	SurfaceNodeMap();
	~SurfaceNodeMap();

	std::vector<std::unique_ptr<AdxNode>>	nodes_;
	SpatialGrid				spatial_grid_;
	CMzPoint				mapped_force_;
	CMzPoint				loss_force_;

	CStdioFile*				log_file_;

	int MapForces(NastranModel& nastran, double upper_limit, CMzPoint ratio);
	int FindNearestNode(CMzPoint& point, double upper_limit, AdxNode** node, double& distance);
	int FindNearestNodeBruteForce(CMzPoint& point, double upper_limit, AdxNode** node, double& distance);
	int BuildSpatialGrid();
	int ExportAdxForce(CString& fpath, CString& process);
	void LogWrite(CString &msg);
	int DumpNode(CString& fpath);
	int Dump(CString& fpath);
	void Clear();
};

