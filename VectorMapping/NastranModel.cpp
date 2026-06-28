#include "stdafx.h"
#include "NastranModel.h"
#include "GeneralFunction.h"

NastranModel::NastranModel()
{
	log_file_ = NULL;
}

NastranModel::~NastranModel()
{
}
void NastranModel::SetLogFile(CStdioFile* log_file)
{
	log_file_ = log_file;
}

const std::vector<std::unique_ptr<NastranNode>>& NastranModel::Nodes() const
{
	return nodes_;
}


const std::vector<std::unique_ptr<NastranElement>>& NastranModel::Elements() const
{
	return elements_;
}

const NastranNode& NastranModel::NodeAt(int index) const
{
	return *(nodes_[index]);
}
void NastranModel::Clear()
{
	nodes_.clear();
	elements_.clear();
	node_id_to_index_.clear();
	element_id_to_index_.clear();
	log_file_ = NULL;
}

void NastranModel::WriteLog( CString& msg )
{
	if ( log_file_ != NULL ) {
		log_file_->WriteString( msg );
	}
}
int NastranModel::ReadGridLine(CStdioFile& file, CString& line, int& line_count)
{
	CString line2;
	CString msg;

	if (!file.ReadString(line2)) {
		msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
		WriteLog(msg);
		return 1;
	}
	line_count++;

	std::unique_ptr<NastranNode> node(new NastranNode);
	if (node->Read(line, line2) != 0) {
		msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
		WriteLog(msg);
		return 1;
	}

	nodes_.push_back(std::move(node));
	return 0;
}

int NastranModel::ReadElementLine(const CString& line, int line_count)
{
	CString msg;
	std::unique_ptr<NastranElement> element(new NastranElement);

	if (element->Read(line) != 0) {
		msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
		WriteLog(msg);
		return 1;
	}

	elements_.push_back(std::move(element));
	return 0;
}

//
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
// GRID*           16282904                -0.761911865E+03 0.278623383E+03F      1
// * 1 0.636766167E+02
// CTRIA3  17245527       2163397611632467916324683
// CQUAD4  17591331       316290559162905581629106316291075
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
int NastranModel::ReadModelFile(const CString& fpath)
{
	CStdioFile cFile;
	CString line;
	CString msg;

	Clear();

	if (cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		msg.Format(_T("ファイルがOpenできません(%s)\n"), fpath);
		WriteLog(msg);
		return 1;
	}

	int line_count = 0;

	while (cFile.ReadString(line)) {
		line_count++;

		if (line.Left(5) == "GRID*") {
			if (ReadGridLine(cFile, line, line_count) != 0) {
				return 1;
			}
		}
		else if (line.Left(6) == "CTRIA3" || line.Left(6) == "CQUAD4" || line.Left(5) == "CBEAM") {
			if (ReadElementLine(line, line_count) != 0) {
				return 1;
			}
		}
	}

	cFile.Close();

	if (Indexing() != 0) {
		msg.Format(_T(" #ERROR インデックスを作成できません\n"), line_count);
		WriteLog(msg);
		return 1;
	}

	return 0;
}
int NastranModel::Indexing()
{
	node_id_to_index_.clear();
	for ( int i=0; i<nodes_.size(); i++ ) {
		NastranNode& n = *(nodes_[i]);
		node_id_to_index_[n.id_] = i;
	}

	element_id_to_index_.clear();
	for ( int i=0; i<elements_.size(); i++ ) {
		NastranElement& e = *(elements_[i]);
		element_id_to_index_[e.id_] = i;
	}

	for (int i = 0; i<elements_.size(); i++) {
		NastranElement& e = *(elements_[i]);
		int node_cnt = 0;
		if (e.type_ == NastranElementType::Triangle) {
			node_cnt = 3;
		}
		else {
			node_cnt = 4;
		}
		for (int j = 0; j < node_cnt; j++) {
			e.node_indices_[j] = node_id_to_index_[e.node_ids_[j]];
		}
	}

	return 0;
}

int NastranModel::ReadPressureFile(const CString& fpath, bool normal_pressure)
{
	CStdioFile cFile;
	CString delimiters;
	CString line;
	CString msg;
	CStringArray words;

	if (cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		msg.Format(_T("ファイルがOpenできません(%s)\n"), fpath);
		WriteLog(msg);
		return 1;
	}

	int line_count = 0;

	for (int i = 0; i < 9; i++) {
		line_count++;
		if (!cFile.ReadString(line)) {
			msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
			WriteLog(msg);
			return 1;
		}
	}

	delimiters = " ";

	while (cFile.ReadString(line)) {
		line_count++;
		SplitString(line, delimiters, words);

		if (words.GetCount() == 4) {
			int current_id = _ttoi(words[0]);
			auto it = node_id_to_index_.find(current_id);
			if (it == node_id_to_index_.end()) {
				msg.Format(_T(" #ERROR ノードIDが見つかりません(%d)行目. ID=(%d)\n"), line_count, current_id);
				WriteLog(msg);
				return 1;
			}

			NastranNode& node = *(nodes_[it->second]);
			CMzPoint& vector = normal_pressure ? node.normal_pressure_vector_ : node.tangent_pressure_vector_;
			vector.x = _ttof(words[1]);
			vector.y = _ttof(words[2]);
			vector.z = _ttof(words[3]);
		}
	}

	cFile.Close();
	return 0;
}

int NastranModel::ReadNormalPressureFile(const CString& fpath)
{
	return ReadPressureFile(fpath, true);
}

int NastranModel::ReadTangentPressureFile(const CString& fpath)
{
	return ReadPressureFile(fpath, false);
}
// ノード毎の荷重ベクトルを求める
int NastranModel::AccumulateElementAreas()
{
	for (int i=0; i<elements_.size(); i++ ) {

		NastranElement& e = *(elements_[i]);

		if ( e.type_ == NastranElementType::Triangle ) {

			NastranNode& n0 = *(nodes_[e.node_indices_[0]]);
			NastranNode& n1 = *(nodes_[e.node_indices_[1]]);
			NastranNode& n2 = *(nodes_[e.node_indices_[2]]);

			e.area_ = AreaTriangle(n0.coord_, n1.coord_, n2.coord_);

			double node_area = e.area_ / 3.0;
			n0.area_ += node_area;
			n1.area_ += node_area;
			n2.area_ += node_area;

		} else if ( e.type_ == NastranElementType::Quad ) {

			NastranNode& n0 = *(nodes_[e.node_indices_[0]]);
			NastranNode& n1 = *(nodes_[e.node_indices_[1]]);
			NastranNode& n2 = *(nodes_[e.node_indices_[2]]);
			NastranNode& n3 = *(nodes_[e.node_indices_[3]]);

			e.area_ = AreaQuad(n0.coord_, n1.coord_, n2.coord_, n3.coord_);

			double node_area = e.area_ / 4.0;
			n0.area_ += node_area;
			n1.area_ += node_area;
			n2.area_ += node_area;
			n3.area_ += node_area;

		} else if ( e.type_ == NastranElementType::Beam ) {

			NastranNode& n0 = *(nodes_[e.node_indices_[0]]);
			NastranNode& n1 = *(nodes_[e.node_indices_[1]]);

			double seg_length = 0.0;
			seg_length = n0.coord_.Distance(n1.coord_);

			e.area_ = seg_length;

			double node_area = e.area_ / 2.0;
			n0.area_ += node_area;
			n1.area_ += node_area;
		}
	}

	return 0;
}

void NastranModel::CalculateNodeForces()
{
	for ( int i=0; i<nodes_.size(); i++ ) {
		NastranNode& n = *(nodes_[i]);
		n.pressure_vector_ = n.normal_pressure_vector_ + n.tangent_pressure_vector_;
		n.force_vector_ = n.pressure_vector_ * n.area_ * 1000.0;
	}
}

int NastranModel::CalculateForces()
{
	if (AccumulateElementAreas() != 0) {
		return 1;
	}

	CalculateNodeForces();
	return 0;
}

int NastranModel::DumpNodes(CString& fpath)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	// 出力
	ret = oFile.Open(fpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	// 要素情報出力
	/*
	for (int i = 0; i < elements_.size(); i++) {
		NastranElement& e = *(elements_[i]);
		buf.Format(_T("E ID(%8d) ,%12.7f\n"), e.id_, e.area_);
		oFile.WriteString(buf);
	}
	*/

	// ノード・ヘッダ行
	buf.Format(_T("NODE_ID,X,Y,Z,NormalPressureX,NormalPressureY,NormalPressureZ,TangentPressureX,TangentPressureY,TangentPressureZ,Area,ForceX,ForceY,ForceZ\n"));
	oFile.WriteString(buf);

	// ノード情報出力
	for (int i = 0; i < nodes_.size(); i++) {
		NastranNode& n = *(nodes_[i]);
		buf.Format(_T("%8d,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f\n"),
			n.id_,
			n.coord_.x, n.coord_.y, n.coord_.z,
			n.normal_pressure_vector_.x, n.normal_pressure_vector_.y, n.normal_pressure_vector_.z,
			n.tangent_pressure_vector_.x, n.tangent_pressure_vector_.y, n.tangent_pressure_vector_.z,
			n.area_,
			n.force_vector_.x, n.force_vector_.y, n.force_vector_.z
			);
		oFile.WriteString(buf);
	}

	oFile.Close();

	return 0;
}

int NastranModel::DumpElements(CString& fpath)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	// 出力
	ret = oFile.Open(fpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	// 要素・ヘッダ行
	buf.Format(_T("ELEMENT_ID,Area\n"));
	oFile.WriteString(buf);

	// 要素情報出力
	for (int i = 0; i < elements_.size(); i++) {
		NastranElement& e = *(elements_[i]);
		buf.Format(_T("%8d,%12.7f\n"), e.id_, e.area_);
		oFile.WriteString(buf);
	}

	/*
	// ノード・ヘッダ行
	buf.Format(_T("NODE_ID,X,Y,Z,NormalPressureX,NormalPressureY,NormalPressureZ,TangentPressureX,TangentPressureY,TangentPressureZ,Area,ForceX,ForceY,ForceZ\n"));
	oFile.WriteString(buf);

	// ノード情報出力
	for (int i = 0; i < nodes_.size(); i++) {
		NastranNode& n = *(nodes_[i]);
		buf.Format(_T("NODE_ID(%8d),%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f,%12.7f\n"),
			n.id_,
			n.coord_.x, n.coord_.y, n.coord_.z,
			n.normal_pressure_vector_.x, n.normal_pressure_vector_.y, n.normal_pressure_vector_.z,
			n.tangent_pressure_vector_.x, n.tangent_pressure_vector_.y, n.tangent_pressure_vector_.z,
			n.area_,
			n.force_vector_.x, n.force_vector_.y, n.force_vector_.z
		);
		oFile.WriteString(buf);
	}
	*/

	oFile.Close();

	return 0;
}

bool NastranModel::IsEmpty()
{
	if (nodes_.size() > 0) {
		return false;
	}
	else {
		return true;
	}
}

int  NastranModel::CalculateTotalForce(CMzPoint& force)
{
	CMzPoint total_force(0.0, 0.0, 0.0);

	for (int i = 0; i < nodes_.size(); i++) {
		NastranNode& n = *(nodes_[i]);
		total_force += n.force_vector_;
	}

	force = total_force;

	return 0;
}