#include "stdafx.h"
#include "SurfaceNodeMap.h"
#include <algorithm>


SurfaceNodeMap::SurfaceNodeMap()
{
	mapped_force_.Set(0.0, 0.0, 0.0);
	loss_force_.Set(0.0, 0.0, 0.0);
}

SurfaceNodeMap::~SurfaceNodeMap()
{
	 spatial_grid_.Clear();
}
void SurfaceNodeMap::SetLogFile(CStdioFile* log_file)
{
	log_file_ = log_file;
}

void SurfaceNodeMap::AddNode(std::unique_ptr<AdxNode> node)
{
	nodes_.push_back(std::move(node));
}

const CMzPoint& SurfaceNodeMap::MappedForce() const
{
	return mapped_force_;
}

const CMzPoint& SurfaceNodeMap::LossForce() const
{
	return loss_force_;
}

int SurfaceNodeMap::BuildSpatialGrid()
{
	if (nodes_.empty()) {
		return 1;
	}

	spatial_grid_.Build(nodes_, 50.0);

	return 0;
}

int SurfaceNodeMap::MapForces(NastranModel& nastran, double upper_limit, CMzPoint ratio)
{

	const auto& nastran_nodes = nastran.Nodes();
	for (int i = 0; i < nastran_nodes.size(); i++) {
		NastranNode& n = *(nastran_nodes[i]);

		/* Check Code
		{
			AdxNode *p_node_m;
			double   distance_m;
			int      rc_m;
			rc_m = NearestNode(n.coord_, upper_limit, &p_node_m, distance_m);

			AdxNode *p_node_s;
			double   distance_s;
			int      rc_s;
			rc_s = NearestNode(n.coord_, upper_limit, &p_node_s, distance_s);

			if (rc_m == rc_s && distance_m == distance_s && p_node_m == p_node_s) {
				msg = _T("NearestNode Same Result\n");
				WriteLog(msg);
			}
			else {
				msg = _T("NearestNode Not Same Result\n");
				WriteLog(msg);
				msg.Format(_T("rc_m(%d) no(%16x) distance_m(%10.3f)"), rc_m, p_node_m, distance_m);
				WriteLog(msg);
				msg.Format(_T("rc_s(%d) no(%16x) distance_s(%10.3f)"), rc_s, p_node_s, distance_s);
				WriteLog(msg);
			}
		}
		*/
		
		CMzPoint map_force;

		map_force.x = n.force_vector_.x * ratio.x;
		map_force.y = n.force_vector_.y * ratio.y;
		map_force.z = n.force_vector_.z * ratio.z;

		std::vector<std::pair<AdxNode*, double>> nearest_nodes;
		if (FindNearestNodes(n.coord_, upper_limit, 4, nearest_nodes) == 0) {
			DistributeForce(map_force, nearest_nodes);
			mapped_force_ += map_force;
		}
		else {
			/*
			if ( p_node == NULL ) {
				msg.Format(_T("PAIR_NOT_EXIST Nastran_Node(%d) ADX_NODE(00000000) DST(%10.3f)\n"), n.id_, distance);
			}
			else {
				msg.Format(_T("PAIR_NOT_EXIST Nastran_Node(%d) ADX_NODE(%d) DST(%10.3f)\n"), n.id_, p_node->id_,distance);
			}
			WriteLog(msg);
			*/
			loss_force_ += map_force;
		}
	}

	return 0;
}

int SurfaceNodeMap::FindNearestNodeByBruteForce(CMzPoint& point, double upper_limit, AdxNode** node, double& distance)
{
	*node = NULL;
	distance = 1E100;

	double upper_limit2 = upper_limit*upper_limit;
	bool exist_node = false;
	double min_dst2 = 1E100;
	AdxNode *p_nearest_node = NULL;

	for (int i = 0; i < nodes_.size(); i++) {
		AdxNode& n = *(nodes_[i]);
		double tmp_dst2 = n.coord_.DistanceSquared(point);
		if (tmp_dst2 < min_dst2) {
			min_dst2 = tmp_dst2;
			p_nearest_node = nodes_[i].get();
			exist_node = true;
		}
	}

	if (exist_node) {
		if (min_dst2 < upper_limit2) {
			*node = p_nearest_node;
			distance = sqrt(min_dst2);
			return 0;
		}
	}
	
	return 1;
}


int SurfaceNodeMap::FindNearestNode(CMzPoint& point, double upper_limit, AdxNode** node, double& distance)
{
	std::vector<std::pair<AdxNode*, double>> nearest_nodes;
	if (FindNearestNodes(point, upper_limit, 1, nearest_nodes) != 0) {
		*node = NULL;
		distance = 1E100;
		return 1;
	}

	*node = nearest_nodes[0].first;
	distance = nearest_nodes[0].second;
	return 0;
}

int SurfaceNodeMap::FindNearestNodes(CMzPoint& point, double upper_limit, int max_count, std::vector<std::pair<AdxNode*, double>>& nearest_nodes)
{
	nearest_nodes.clear();

	if (max_count <= 0 || upper_limit <= 0.0 || spatial_grid_.CellSize() <= 0.0) {
		return 1;
	}

	int ax, ay, az;
	spatial_grid_.GetCellCoordinates(point, ax, ay, az);

	int cell_radius = static_cast<int>(ceil(upper_limit / spatial_grid_.CellSize()));
	if (cell_radius < 1) {
		cell_radius = 1;
	}

	double upper_limit2 = upper_limit * upper_limit;
	int area_index;

	for (int ix = ax - cell_radius; ix <= ax + cell_radius; ix++) {
		for (int iy = ay - cell_radius; iy <= ay + cell_radius; iy++) {
			for (int iz = az - cell_radius; iz <= az + cell_radius; iz++) {
				if (spatial_grid_.GetCellIndex(ix, iy, iz, area_index) != 0) {
					continue;
				}

				SpatialGridCell& cell = spatial_grid_.cells_[area_index];
				for (int i = 0; i < static_cast<int>(cell.nodes_.size()); i++) {
					AdxNode* candidate = cell.nodes_[i];
					double distance2 = point.DistanceSquared(candidate->coord_);
					if (distance2 < upper_limit2) {
						nearest_nodes.push_back(std::make_pair(candidate, sqrt(distance2)));
					}
				}
			}
		}
	}

	if (nearest_nodes.empty()) {
		return 1;
	}

	std::sort(nearest_nodes.begin(), nearest_nodes.end(), [](const std::pair<AdxNode*, double>& lhs, const std::pair<AdxNode*, double>& rhs) {
		return lhs.second < rhs.second;
	});

	if (static_cast<int>(nearest_nodes.size()) > max_count) {
		nearest_nodes.resize(max_count);
	}

	return 0;
}

void SurfaceNodeMap::DistributeForce(const CMzPoint& force, const std::vector<std::pair<AdxNode*, double>>& nearest_nodes)
{
	if (nearest_nodes.empty()) {
		return;
	}

	const double epsilon = 1.0E-12;
	if (nearest_nodes[0].second <= epsilon) {
		nearest_nodes[0].first->force_vector_ += force;
		return;
	}

	double total_weight = 0.0;
	for (int i = 0; i < static_cast<int>(nearest_nodes.size()); i++) {
		total_weight += 1.0 / nearest_nodes[i].second;
	}

	if (total_weight <= 0.0) {
		nearest_nodes[0].first->force_vector_ += force;
		return;
	}

	for (int i = 0; i < static_cast<int>(nearest_nodes.size()); i++) {
		double weight = (1.0 / nearest_nodes[i].second) / total_weight;
		CMzPoint distributed_force;
		distributed_force.x = force.x * weight;
		distributed_force.y = force.y * weight;
		distributed_force.z = force.z * weight;
		nearest_nodes[i].first->force_vector_ += distributed_force;
	}
}
int SurfaceNodeMap::ExportAdxForces(CString& opath, CString& process)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	// 出力ファイル
	ret = oFile.Open(opath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	// 先頭部
	buf.Format(_T("########################################################################################\n"));
	oFile.WriteString(buf);
	buf.Format(_T("$Load\n"));
	oFile.WriteString(buf);
	buf.Format(_T("########################################################################################\n"));
	oFile.WriteString(buf);
	buf.Format(_T("process_id = %s\n"),process);
	oFile.WriteString(buf);

	// データ部
	for (int i = 0; i < nodes_.size(); i++) {
		AdxNode& n = *(nodes_[i]);

		if (n.force_vector_.x != 0.0 || n.force_vector_.y != 0.0 || n.force_vector_.z != 0.0) {
			buf.Format(_T("%8d 0 %12.6f\n"), n.id_, n.force_vector_.x);
			oFile.WriteString(buf);
			buf.Format(_T("%8d 1 %12.6f\n"), n.id_, n.force_vector_.y);
			oFile.WriteString(buf);
			buf.Format(_T("%8d 2 %12.6f\n"), n.id_, n.force_vector_.z);
			oFile.WriteString(buf);
		}
	}

	oFile.Close();

	return 0;
}

void SurfaceNodeMap::WriteLog(CString& msg)
{
	if (log_file_ != NULL) {
		log_file_->WriteString(msg);
	}
}


int SurfaceNodeMap::DumpNodes(CString& fpath)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	ret = oFile.Open(fpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	buf.Format(_T("NODE_ID,x,y,z,Force_x,Force_y,Force_z\n"));
	oFile.WriteString(buf);
	
	for (int i = 0; i < nodes_.size(); i++) {
		AdxNode& n = *(nodes_[i]);
		buf.Format(_T("%8d,%12.6f,%12.6f,%12.6f,%12.6f,%12.6f,%12.6f\n"), n.id_, n.coord_.x, n.coord_.y, n.coord_.z, n.force_vector_.x, n.force_vector_.y, n.force_vector_.z);
		oFile.WriteString(buf);
	}

	oFile.Close();

	return 0;
}


int SurfaceNodeMap::Dump(CString& fpath)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	ret = oFile.Open(fpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	int array_index=0;

	for ( int i=spatial_grid_.min_index_[0]; i<= spatial_grid_.max_index_[0]; i++ ) {
		for (int j = spatial_grid_.min_index_[1]; j <= spatial_grid_.max_index_[1]; j++) {
			for (int k = spatial_grid_.min_index_[2]; k <= spatial_grid_.max_index_[2]; k++) {

				buf.Format(_T("------ AREA X:%d,Y:%d,Z:%d ----- "), i, j, k);
				oFile.WriteString(buf);

				if (spatial_grid_.GetCellIndex(i, j, k, array_index) == 0) {

					buf.Format(_T("GetArrayIndex=%d\n"),array_index);
					oFile.WriteString(buf);

					SpatialGridCell& a = spatial_grid_.cells_[array_index];
					for (int l = 0; l < a.nodes_.size(); l++) {
						AdxNode&n = *(a.nodes_[l]);
						buf.Format(_T("ID(%d) X:%10.3f, Y:%10.3f, Z:%10.3f"), n.id_, n.coord_.x, n.coord_.y, n.coord_.z);
						oFile.WriteString(buf);
					}

				} else {
					buf.Format(_T("GetArrayIndex ERROR"));
					oFile.WriteString(buf);
				}

			}
		}
	}

	oFile.Close();

	return 0;
}

void SurfaceNodeMap::Clear()
{
	nodes_.clear();

	spatial_grid_.Clear();

	mapped_force_.Set(0.0, 0.0, 0.0);
	loss_force_.Set(0.0, 0.0, 0.0);

}