#include "stdafx.h"
#include "SurfaceFaceMap.h"
#include "GeneralFunction.h"

SurfaceFaceMap::SurfaceFaceMap()
{
	mapped_force_.Set(0.0, 0.0, 0.0);
	loss_force_.Set(0.0, 0.0, 0.0);
	log_file_ = NULL;
}

SurfaceFaceMap::~SurfaceFaceMap()
{
}

void SurfaceFaceMap::SetLogFile(CStdioFile* log_file)
{
	log_file_ = log_file;
}

int SurfaceFaceMap::AddNode(std::unique_ptr<AdxNode> node)
{
	nodes_.push_back(std::move(node));
	return static_cast<int>(nodes_.size()) - 1;
}

int SurfaceFaceMap::AddFace(int node0, int node1, int node2)
{
	SurfaceFace face;
	face.node_indices_[0] = node0;
	face.node_indices_[1] = node1;
	face.node_indices_[2] = node2;
	face.centroid_.Set(0.0, 0.0, 0.0);
	face.area_ = 0.0;
	faces_.push_back(face);
	return 0;
}

const CMzPoint& SurfaceFaceMap::MappedForce() const
{
	return mapped_force_;
}

const CMzPoint& SurfaceFaceMap::LossForce() const
{
	return loss_force_;
}

int SurfaceFaceMap::BuildGeometry()
{
	for (int i = 0; i < static_cast<int>(faces_.size()); i++) {
		SurfaceFace& face = faces_[i];
		AdxNode& n0 = *(nodes_[face.node_indices_[0]]);
		AdxNode& n1 = *(nodes_[face.node_indices_[1]]);
		AdxNode& n2 = *(nodes_[face.node_indices_[2]]);

		face.centroid_.x = (n0.coord_.x + n1.coord_.x + n2.coord_.x) / 3.0;
		face.centroid_.y = (n0.coord_.y + n1.coord_.y + n2.coord_.y) / 3.0;
		face.centroid_.z = (n0.coord_.z + n1.coord_.z + n2.coord_.z) / 3.0;
		face.area_ = AreaTriangle(n0.coord_, n1.coord_, n2.coord_);
	}

	return 0;
}

int SurfaceFaceMap::MapForces(NastranModel& nastran, double upper_limit, CMzPoint ratio)
{
	mapped_force_.Set(0.0, 0.0, 0.0);
	loss_force_.Set(0.0, 0.0, 0.0);

	if (BuildGeometry() != 0) {
		return 1;
	}

	for (int i = 0; i < static_cast<int>(faces_.size()); i++) {
		SurfaceFace& face = faces_[i];
		const double sample_points[3][3] = {
			{ 2.0 / 3.0, 1.0 / 6.0, 1.0 / 6.0 },
			{ 1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0 },
			{ 1.0 / 6.0, 1.0 / 6.0, 2.0 / 3.0 }
		};
		const double sample_area = face.area_ / 3.0;

		for (int sample_index = 0; sample_index < 3; sample_index++) {
			const double* barycentric_weights = sample_points[sample_index];
			CMzPoint sample_point = CalculateFaceSamplePoint(face, barycentric_weights);
			const NastranElement* source_element = NULL;
			double distance = 0.0;

			if (FindNearestNastranElement(nastran, sample_point, upper_limit, &source_element, distance) != 0) {
				continue;
			}

			CMzPoint pressure = CalculateElementPressure(nastran, *source_element);
			CMzPoint sample_force;
			sample_force.x = pressure.x * sample_area * 1000.0 * ratio.x;
			sample_force.y = pressure.y * sample_area * 1000.0 * ratio.y;
			sample_force.z = pressure.z * sample_area * 1000.0 * ratio.z;

			AddWeightedSampleForce(face, sample_force, barycentric_weights);
			mapped_force_ += sample_force;
		}
	}

	return 0;
}

int SurfaceFaceMap::FindNearestNastranElement(NastranModel& nastran, CMzPoint& point, double upper_limit, const NastranElement** element, double& distance)
{
	*element = NULL;
	distance = 1E100;

	double upper_limit2 = upper_limit * upper_limit;
	double min_distance2 = 1E100;
	const auto& elements = nastran.Elements();

	for (int i = 0; i < static_cast<int>(elements.size()); i++) {
		const NastranElement& candidate = *(elements[i]);
		if (candidate.type_ == NastranElementType::Beam) {
			continue;
		}

		CMzPoint centroid = CalculateElementCentroid(nastran, candidate);
		double distance2 = point.DistanceSquared(centroid);
		if (distance2 < upper_limit2 && distance2 < min_distance2) {
			min_distance2 = distance2;
			*element = &candidate;
		}
	}

	if (*element == NULL) {
		return 1;
	}

	distance = sqrt(min_distance2);
	return 0;
}

CMzPoint SurfaceFaceMap::CalculateElementPressure(NastranModel& nastran, const NastranElement& element)
{
	CMzPoint pressure;
	pressure.Set(0.0, 0.0, 0.0);

	int node_count = (element.type_ == NastranElementType::Triangle) ? 3 : 4;
	for (int i = 0; i < node_count; i++) {
		const NastranNode& node = nastran.NodeAt(element.node_indices_[i]);
		pressure += node.pressure_vector_;
	}

	pressure.x /= node_count;
	pressure.y /= node_count;
	pressure.z /= node_count;
	return pressure;
}

CMzPoint SurfaceFaceMap::CalculateElementCentroid(NastranModel& nastran, const NastranElement& element)
{
	CMzPoint centroid;
	centroid.Set(0.0, 0.0, 0.0);

	int node_count = (element.type_ == NastranElementType::Triangle) ? 3 : 4;
	for (int i = 0; i < node_count; i++) {
		const NastranNode& node = nastran.NodeAt(element.node_indices_[i]);
		centroid += node.coord_;
	}

	centroid.x /= node_count;
	centroid.y /= node_count;
	centroid.z /= node_count;
	return centroid;
}

CMzPoint SurfaceFaceMap::CalculateFaceSamplePoint(SurfaceFace& face, const double barycentric_weights[3])
{
	CMzPoint sample_point;
	sample_point.Set(0.0, 0.0, 0.0);

	for (int i = 0; i < 3; i++) {
		AdxNode& node = *(nodes_[face.node_indices_[i]]);
		sample_point.x += node.coord_.x * barycentric_weights[i];
		sample_point.y += node.coord_.y * barycentric_weights[i];
		sample_point.z += node.coord_.z * barycentric_weights[i];
	}

	return sample_point;
}

void SurfaceFaceMap::AddWeightedSampleForce(SurfaceFace& face, CMzPoint& force, const double barycentric_weights[3])
{
	for (int i = 0; i < 3; i++) {
		CMzPoint node_force;
		node_force.x = force.x * barycentric_weights[i];
		node_force.y = force.y * barycentric_weights[i];
		node_force.z = force.z * barycentric_weights[i];
		nodes_[face.node_indices_[i]]->force_vector_ += node_force;
	}
}

int SurfaceFaceMap::ExportAdxForces(CString& opath, CString& process)
{
	BOOL ret;
	CStdioFile oFile;
	CString buf;

	ret = oFile.Open(opath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	buf.Format(_T("########################################################################################\n"));
	oFile.WriteString(buf);
	buf.Format(_T("$Load\n"));
	oFile.WriteString(buf);
	buf.Format(_T("########################################################################################\n"));
	oFile.WriteString(buf);
	buf.Format(_T("process_id = %s\n"), process);
	oFile.WriteString(buf);

	for (int i = 0; i < static_cast<int>(nodes_.size()); i++) {
		AdxNode& node = *(nodes_[i]);
		if (node.force_vector_.x != 0.0 || node.force_vector_.y != 0.0 || node.force_vector_.z != 0.0) {
			buf.Format(_T("%8d 0 %12.6f\n"), node.id_, node.force_vector_.x);
			oFile.WriteString(buf);
			buf.Format(_T("%8d 1 %12.6f\n"), node.id_, node.force_vector_.y);
			oFile.WriteString(buf);
			buf.Format(_T("%8d 2 %12.6f\n"), node.id_, node.force_vector_.z);
			oFile.WriteString(buf);
		}
	}

	oFile.Close();
	return 0;
}

int SurfaceFaceMap::DumpNodes(CString& fpath)
{
	BOOL ret;
	CStdioFile oFile;
	CString buf;

	ret = oFile.Open(fpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	buf.Format(_T("NODE_ID,x,y,z,Force_x,Force_y,Force_z\n"));
	oFile.WriteString(buf);

	for (int i = 0; i < static_cast<int>(nodes_.size()); i++) {
		AdxNode& node = *(nodes_[i]);
		buf.Format(_T("%8d,%12.6f,%12.6f,%12.6f,%12.6f,%12.6f,%12.6f\n"), node.id_, node.coord_.x, node.coord_.y, node.coord_.z, node.force_vector_.x, node.force_vector_.y, node.force_vector_.z);
		oFile.WriteString(buf);
	}

	oFile.Close();
	return 0;
}

void SurfaceFaceMap::Clear()
{
	nodes_.clear();
	faces_.clear();
	mapped_force_.Set(0.0, 0.0, 0.0);
	loss_force_.Set(0.0, 0.0, 0.0);
}
