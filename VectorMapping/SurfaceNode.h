#pragma once

#include <memory>
#include <vector>
#include"AdxNode.h"
#include"AreaMap.h"
#include"MzPoint.h"
#include"CNastran.h"

class CSurfaceNode
{
public:
	CSurfaceNode();
	~CSurfaceNode();

	std::vector<std::unique_ptr<CAdxNode>>	nodes_;
	SpatialGrid				spatial_grid_;
	CMzPoint				m_MappedForce;
	CMzPoint				m_LossForce;

	CStdioFile*				m_LogFile;

	int MapForces(CNastran& nastran, double upper_limit, CMzPoint ratio);
	int FindNearestNode(CMzPoint& point, double upper_limit, CAdxNode** node, double& distance);
	int FindNearestNodeBruteForce(CMzPoint& point, double upper_limit, CAdxNode** node, double& distance);
	int BuildSpatialGrid();
	int ExportAdxForce(CString& fpath, CString& process);
	void LogWrite(CString &msg);
	int DumpNode(CString& fpath);
	int Dump(CString& fpath);
	void Clear();
};

