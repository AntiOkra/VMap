#pragma once

#include <memory>
#include <vector>
#include "AdxNode.h"
#include "MzPoint.h"
#include "NastranModel.h"

struct SurfaceFace
{
	int node_indices_[3];
	CMzPoint centroid_;
	double area_;
};

class SurfaceFaceMap
{
public:
	SurfaceFaceMap();
	~SurfaceFaceMap();

	void SetLogFile(CStdioFile* log_file);
	int AddNode(std::unique_ptr<AdxNode> node);
	int AddFace(int node0, int node1, int node2);

	const CMzPoint& MappedForce() const;
	const CMzPoint& LossForce() const;

	int BuildGeometry();
	int MapForces(NastranModel& nastran, double upper_limit, CMzPoint ratio);
	int ExportAdxForces(CString& fpath, CString& process);
	int DumpNodes(CString& fpath);
	void Clear();

private:
	std::vector<std::unique_ptr<AdxNode>> nodes_;
	std::vector<SurfaceFace> faces_;
	CMzPoint mapped_force_;
	CMzPoint loss_force_;
	CStdioFile* log_file_;

	int FindNearestNastranElement(NastranModel& nastran, CMzPoint& point, double upper_limit, const NastranElement** element, double& distance);
	int DetermineSubdivisionCount(NastranModel& nastran, SurfaceFace& face, double upper_limit);
	CMzPoint CalculateElementPressure(NastranModel& nastran, const NastranElement& element);
	CMzPoint CalculateElementCentroid(NastranModel& nastran, const NastranElement& element);
	CMzPoint CalculateFaceSamplePoint(SurfaceFace& face, const double barycentric_weights[3]);
	void AddWeightedSampleForce(SurfaceFace& face, CMzPoint& force, const double barycentric_weights[3]);
	void MapSubtriangleSample(NastranModel& nastran, SurfaceFace& face, double upper_limit, CMzPoint ratio, const double barycentric_weights[3], double sample_area);
};
