#include "stdafx.h"
#include "NastranModel.h"
#include "GeneralFunction.h"

NastranNode::NastranNode()
{
	area_ = 0.0;
}

NastranNode::~NastranNode()
{
}

//           1         2         3         4         5         6         7         8
// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
// GRID*           16282904                -0.761911865E+03 0.278623383E+03F      1
// *      1 0.951755142E+01
int NastranNode::Read(CString& line1, CString& line2)
{
	CString sid, scoord_x, scoord_y, scoord_z;

	id_ = -1;

	sid = line1.Mid(16, 8);
	scoord_x = line1.Mid(40, 16);
	scoord_y = line1.Mid(56, 16);
	scoord_z = line2.Mid(8, 16);

	id_ = _ttoi(sid);
	coord_.x = _ttof(scoord_x);
	coord_.y = _ttof(scoord_y);
	coord_.z = _ttof(scoord_z);

	return 0;
}

NastranElement::NastranElement()
{
	area_ = 0.0;
}

NastranElement::~NastranElement()
{
}

//           1         2         3         4         5         6         7         8
// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
// CTRIA3  17245527       2163397611632467916324683
// CQUAD4  17591331       316290559162905581629106316291075
// CBEAM        706       1   88558   88559
int NastranElement::Read(const CString& line)
{
	CString stype, sid;
	CString snode_id[4];

	id_ = -1;

	stype = line.Left(6);
	stype.Trim();
	sid = line.Mid(8, 8);

	if ( stype=="CTRIA3" ) {
		type_ = CTRIA3;
		snode_id[0] = line.Mid(24,8);
		snode_id[1] = line.Mid(32,8);
		snode_id[2] = line.Mid(40,8);
		snode_id[3] = "-1";
	} else if ( stype=="CQUAD4" ) {
		type_ = CQUAD4;
		snode_id[0] = line.Mid(24,8);
		snode_id[1] = line.Mid(32,8);
		snode_id[2] = line.Mid(40,8);
		snode_id[3] = line.Mid(48,8);
	} else if ( stype=="CBEAM" ) {
		type_ = CBEAM;
		snode_id[0] = line.Mid(24,8);
		snode_id[1] = line.Mid(32,8);
		snode_id[2] = "-1";
		snode_id[3] = "-1";
	} else {
		return 1;
	}

	id_ = _ttoi(sid);
	node_ids_[0] = _ttoi(snode_id[0]);
	node_ids_[1] = _ttoi(snode_id[1]);
	node_ids_[2] = _ttoi(snode_id[2]);
	node_ids_[3] = _ttoi(snode_id[3]);

	return 0;
}

NastranModel::NastranModel()
{
	log_file_ = NULL;
}

NastranModel::~NastranModel()
{
}

void NastranModel::RemoveAll()
{
	nodes_.clear();
	elements_.clear();
	node_id_to_index_.clear();
	element_id_to_index_.clear();
	log_file_ = NULL;
}

void NastranModel::LogWrite( CString& msg )
{
	if ( log_file_ != NULL ) {
		log_file_->WriteString( msg );
	}
}

//
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
// GRID*           16282904                -0.761911865E+03 0.278623383E+03F      1
// * 1 0.636766167E+02
// CTRIA3  17245527       2163397611632467916324683
// CQUAD4  17591331       316290559162905581629106316291075
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
int NastranModel::ReadNastranFile(const CString& fpath)
{

	CStdioFile		cFile;
	BOOL			ret = FALSE;
	CString			delimiters;
	CString			line,line2;
	CString			msg;
	CStringArray	words;
	int				rc = 0;

	std::unique_ptr<NastranNode>	crnt_node;
	std::unique_ptr<NastranElement> crnt_element;

	RemoveAll();

	ret = cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone);
	if (ret == FALSE) {
		msg.Format(_T("ファイルがOpenできません(%s)\n"),fpath );
		LogWrite( msg );
		return 1;
	}

	delimiters = " =:";

	int line_count = 0;

	while (cFile.ReadString(line)) {

		line_count++;

		if (line.Left(5) == "GRID*") {

			if (!cFile.ReadString(line2)) {
				msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
				LogWrite(msg);
				return 1;
			}
			line_count++;

			crnt_node.reset(new NastranNode);

			if (crnt_node->Read(line, line2) != 0) {
				msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
				LogWrite(msg);
				return 1;
			}

			nodes_.push_back(std::move(crnt_node));

		}
		else if (line.Left(6) == "CTRIA3") {

			crnt_element.reset(new NastranElement);

			if (crnt_element->Read(line) != 0) {
				msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
				LogWrite(msg);
				return 1;
			}

			elements_.push_back(std::move(crnt_element));

		}
		else if (line.Left(6) == "CQUAD4") {

			crnt_element.reset(new NastranElement);

			if (crnt_element->Read(line) != 0) {
				msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
				LogWrite(msg);
				return 1;
			}

			elements_.push_back(std::move(crnt_element));
		}
		else if (line.Left(5) == "CBEAM") {

			crnt_element.reset(new NastranElement);

			if (crnt_element->Read(line) != 0) {
				msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
				LogWrite(msg);
				return 1;
			}

			elements_.push_back(std::move(crnt_element));
		}
	}

	cFile.Close();

	if (Indexing() != 0) {
		msg.Format(_T(" #ERROR インデックスを作成できません\n"), line_count);
		LogWrite(msg);
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
		if (e.type_ == CTRIA3) {
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

int NastranModel::ReadNormalVectorFile(const CString& fpath)
{

	CStdioFile		cFile;
	BOOL			ret = FALSE;
	CString			delimiters;
	CString			line;
	CString			msg;
	CStringArray	words;

	//RemoveAll();

	ret = cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone);
	if (ret == FALSE) {
		msg.Format(_T("ファイルがOpenできません(%s)\n"),fpath );
		LogWrite( msg );
		return 1;
	}

	int line_count = 0;

	// 9行スキップ
	for ( int i=0; i<9; i++ ) {
		line_count++;
		if ( ! cFile.ReadString(line) ) {
			msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
			LogWrite(msg);
			return 1;
		}
	}

	delimiters = " ";

	// データ
	while (cFile.ReadString(line)) {

		line_count++;

		SplitString( line, delimiters, words );

		if ( words.GetCount() == 4 ) {

			int crnt_id = _ttoi(words[0]);

			auto it =  node_id_to_index_.find(crnt_id);
			if (it == node_id_to_index_.end() ) {
				msg.Format(_T(" #ERROR ノードIDが見つかりません(%d)行目. ID=(%d)\n"), line_count,crnt_id);
				LogWrite(msg);
				return 1;
			}

			int index = (*it).second;
			NastranNode& n = *(nodes_[index]);

			n.normal_pressure_vector_.x = _ttof(words[1]);
			n.normal_pressure_vector_.y = _ttof(words[2]);
			n.normal_pressure_vector_.z = _ttof(words[3]);
		}
	}

	cFile.Close();

	return 0;
}

int NastranModel::ReadTangentVectorFile(const CString& fpath)
{

	CStdioFile		cFile;
	BOOL			ret = FALSE;
	CString			delimiters;
	CString			line;
	CString			msg;
	CStringArray	words;

	//RemoveAll();

	ret = cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone);
	if (ret == FALSE) {
		msg.Format(_T("ファイルがOpenできません(%s)\n"),fpath );
		LogWrite( msg );
		return 1;
	}

	int line_count = 0;

	// 9行スキップ
	for ( int i=0; i<9; i++ ) {
		line_count++;
		if ( ! cFile.ReadString(line) ) {
			msg.Format(_T(" #ERROR フォーマット異常あり(%d)行目\n"), line_count);
			LogWrite(msg);
			return 1;
		}
	}

	delimiters = " ";

	// データ
	while (cFile.ReadString(line)) {

		line_count++;

		SplitString( line, delimiters, words );

		if ( words.GetCount() ==4 ) {

			int crnt_id = _ttoi(words[0]);

			auto it =  node_id_to_index_.find(crnt_id);
			if (it == node_id_to_index_.end() ) {
				msg.Format(_T(" #ERROR ノードIDが見つかりません(%d)行目. ID=(%d)\n"), line_count,crnt_id);
				LogWrite(msg);
				return 1;
			}

			int index = (*it).second;
			NastranNode& n = *(nodes_[index]);

			n.tangent_pressure_vector_.x = _ttof(words[1]);
			n.tangent_pressure_vector_.y = _ttof(words[2]);
			n.tangent_pressure_vector_.z = _ttof(words[3]);
		}
	}

	cFile.Close();

	return 0;
}

// ノード毎の荷重ベクトルを求める
int NastranModel::ForceCalc()
{

	// ノードの担当面積を求める
	for (int i=0; i<elements_.size(); i++ ) {

		NastranElement& e = *(elements_[i]);

		// 要素面積を求める

		if ( e.type_ == CTRIA3 ) {

			NastranNode& n0 = *(nodes_[e.node_indices_[0]]);
			NastranNode& n1 = *(nodes_[e.node_indices_[1]]);
			NastranNode& n2 = *(nodes_[e.node_indices_[2]]);

			e.area_ = AreaTriangle(n0.coord_, n1.coord_, n2.coord_);

			double node_area = e.area_ / 3.0;
			n0.area_ += node_area;
			n1.area_ += node_area;
			n2.area_ += node_area;

		} else if ( e.type_ == CQUAD4 ) {

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

		} else if ( e.type_ == CBEAM ) {

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

	// 荷重を計算
	for ( int i=0; i<nodes_.size(); i++ ) {
		NastranNode& n = *(nodes_[i]);
		n.pressure_vector_ = n.normal_pressure_vector_ + n.tangent_pressure_vector_;
		n.force_vector_ = n.pressure_vector_ * n.area_ * 1000.0;	// 単位:(N)
	}

	return 0;
}

int NastranModel::Dump_N(CString& fpath)
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

int NastranModel::Dump_E(CString& fpath)
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

int  NastranModel::GetTotalForce(CMzPoint& force)
{
	CMzPoint total_force(0.0, 0.0, 0.0);

	for (int i = 0; i < nodes_.size(); i++) {
		NastranNode& n = *(nodes_[i]);
		total_force += n.force_vector_;
	}

	force = total_force;

	return 0;
}