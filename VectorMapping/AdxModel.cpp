#include "stdafx.h"
#include "AdxModel.h"
#include "GeneralFunction.h"
#include <iterator>
#include <regex>

int IncludeFilePath(CString& parent_path, CString& include_file, CString& include_path);


AdxModel::AdxModel()
{
}


AdxModel::~AdxModel()
{
}
int AdxModel::ElementSetCount() const
{
	return static_cast<int>(element_sets_.size());
}

AdxElementSet& AdxModel::ElementSetAt(int index)
{
	return *(element_sets_[index]);
}

const AdxElementSet& AdxModel::ElementSetAt(int index) const
{
	return *(element_sets_[index]);
}

void AdxModel::FinishNodeSet(std::unique_ptr<AdxNodeSet>& node_set)
{
	if (node_set.get() != NULL) {
		node_sets_.push_back(std::move(node_set));
	}
}

void AdxModel::FinishElementSet(std::unique_ptr<AdxElementSet>& element_set)
{
	if (element_set.get() != NULL) {
		element_set_name_to_index_[element_set->adx_name_] = static_cast<int>(element_sets_.size());
		element_sets_.push_back(std::move(element_set));
	}
}

int AdxModel::HandleReadDirective(CString& fpath, CString& line, CStringArray& words, ReadState& state)
{
	if (line.Left(1) != "$") {
		return 0;
	}

	CString delimiters = _T(" =:");
	SplitString(line, delimiters, words);

	if (words[0] == "$Include") {
		CString include_path;
		IncludeFilePath(fpath, words[1], include_path);
		Read(include_path);
	}
	else if (words[0] == "$Node") {
		state = ReadState::Node;
	}
	else if (words[0] == "$Element" && words[1] == "3DQuadraticTetrahedron") {
		state = ReadState::Element;
	}

	return 0;
}

int AdxModel::ReadNodeSectionLine(CString& line, CStringArray& words, std::unique_ptr<AdxNodeSet>& node_set, ReadState& state, bool& re_parse)
{
	CString delimiters = _T(" =:");
	SplitString(line, delimiters, words);

	if (line.Left(1) == "$") {
		FinishNodeSet(node_set);
		node_set.reset(NULL);
		state = ReadState::None;
		re_parse = true;
		return 0;
	}

	if (words[0] == "node_set") {
		node_set.reset(new AdxNodeSet);
		node_set->adx_name_ = words[1];
		return 0;
	}

	if (words[0] == "dimension" || words[0] == "index_format" || words[0] == "format") {
		return 0;
	}

	std::unique_ptr<AdxNode> node(new AdxNode);
	node->Read(words);
	nodes_.push_back(std::move(node));

	if (node_set != NULL) {
		node_set->node_indices_.push_back(static_cast<int>(nodes_.size()) - 1);
	}

	return 0;
}

int AdxModel::ReadElementSectionLine(CString& line, CStringArray& words, std::unique_ptr<AdxElementSet>& element_set, ReadState& state, bool& re_parse)
{
	CString delimiters = _T(" =:");
	SplitString(line, delimiters, words);

	if (line.Left(1) == "$") {
		FinishElementSet(element_set);
		element_set.reset(NULL);
		state = ReadState::None;
		re_parse = true;
		return 0;
	}

	if (words[0] == "element_set") {
		element_set.reset(new AdxElementSet);
		CString tmp_name;
		CString tmp_comment;
		int tmp_id;
		ParseElementSetInfo(line, tmp_name, tmp_id, tmp_comment);
		element_set->adx_name_ = tmp_name;
		element_set->user_name_ = tmp_comment;
		element_set->id_ = tmp_id;
		return 0;
	}

	if (words[0] == "num_nodes_per_element" || words[0] == "index_format" || words[0] == "format") {
		return 0;
	}

	std::unique_ptr<AdxElement> element(new AdxElement);
	element->Read(words);
	elements_.push_back(std::move(element));

	if (element_set != NULL) {
		element_set->element_indices_.push_back(static_cast<int>(elements_.size()) - 1);
	}

	return 0;
}
int IncludeFilePath( CString& parent_path, CString& include_file, CString& include_path )
{
	// parent folder
	int ipos = parent_path.ReverseFind('\\');
	CString parent_folder = parent_path.Left(ipos);

	// include_path
	include_path = parent_folder + "\\" + include_file;

	return 0;
}

//
// element_set = Body_2_PID1_e8  #Body_2
//
int AdxModel::ParseElementSetInfo(CString& nodeset_line, CString& name, int& id, CString& comment)
{
	id = 0;
	comment = "";

	CStringArray words;
	CString delimiters = _T(" =");
	SplitString(nodeset_line, delimiters, words);

	// Name
	name = words[1];

	// Comment
	if (words.GetSize() >= 3) {
		comment = words[2];
		if (!comment.IsEmpty()) {
			if (comment.Left(1) == "#") {
				comment = comment.Right(comment.GetLength() - 1);
			}
		}
	}

	// 最初の数字　無ければ 0
	std::wstring sname(name.GetBuffer());

	std::wregex   pattern(L"[0-9]+");
	std::wsmatch  match_results;

	if (std::regex_search(sname, match_results, pattern)) 
	{
			id = stoi(match_results.str());
	}

	/*
	int ipos_body = nodeset_line.Find(_T("Body_"), 0);
	if (ipos_body>=0) {
		int ipos = nodeset_line.Find(_T("_"), ipos_body+5);
		CString id_str = nodeset_line.Mid(ipos_body+5, ipos - (ipos_body+5));
		if (IsInteger(id_str)) {
			id = _ttoi(id_str);
		}
	}
	*/
	
	return 0;
}

int AdxModel::Read(CString& fpath)
{
	CStdioFile cFile;
	CString line;
	CStringArray words;
	bool is_eof = false;
	bool re_parse = false;
	std::unique_ptr<AdxNodeSet> crnt_node_set;
	std::unique_ptr<AdxElementSet> crnt_element_set;
	ReadState crnt_state = ReadState::None;

	if (cFile.Open(fpath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		return 1;
	}

	while (!is_eof) {
		if (re_parse) {
			re_parse = false;
		}
		else if (cFile.ReadString(line) == FALSE) {
			is_eof = true;
			line = "$EOF";
		}

		if (line.Left(1) == "#" || line.Trim() == "") {
			continue;
		}

		switch (crnt_state) {
		case ReadState::None:
			HandleReadDirective(fpath, line, words, crnt_state);
			break;

		case ReadState::Node:
			ReadNodeSectionLine(line, words, crnt_node_set, crnt_state, re_parse);
			break;

		case ReadState::Element:
			ReadElementSectionLine(line, words, crnt_element_set, crnt_state, re_parse);
			break;
		}
	}

	cFile.Close();
	return 0;
}
void AdxModel::Clear()
{
	node_sets_.clear();
	nodes_.clear();

	element_sets_.clear();
	elements_.clear();
	element_faces_.clear();

	element_id_to_index_.clear();
	node_id_to_index_.clear();
	element_set_name_to_index_.clear();
}

//
int AdxModel::Activate()
{
	node_id_to_index_.clear();
	element_id_to_index_.clear();
	element_faces_.clear();

	// NodeMap作成
	for (int i = 0; i < static_cast<int>(nodes_.size()); i++) {
		AdxNode& n = *(nodes_[i]);
		n.element_face_indices_.clear();
		n.type_ = UndefinedNode;
		node_id_to_index_[n.id_] = i;
	}

	// ElementMap作成
	for (int i = 0; i < static_cast<int>(elements_.size()); i++) {
		AdxElement& e = *(elements_[i]);
		element_id_to_index_[e.id_] = i;
	}

	// Element 内 Node_Index set
	for (int i = 0; i < static_cast<int>(elements_.size()); i++) {
		AdxElement& e = *(elements_[i]);

		for (int j = 0; j < 10; j++) {
			auto node_it = node_id_to_index_.find(e.node_ids_[j]);
			if (node_it == node_id_to_index_.end()) {
				return 1;
			}
			e.node_indices_[j] = node_it->second;
		}

		for (int j = 0; j < 4; j++) {
			int index_node = e.node_indices_[j];
			nodes_[index_node]->SetType(CornerNode);
		}
		for (int j = 4; j < 10; j++) {
			int index_node = e.node_indices_[j];
			nodes_[index_node]->SetType(MidSideNode);
		}
	}

	// ElementFace生成
	int n_index[6];
	int face_index = -1;
	for (int i = 0; i < static_cast<int>(elements_.size()); i++) {

		AdxElement& e = *(elements_[i]);

		for (int j = 0; j < 4; j++) {

			for (int k = 0; k < 6; k++) {
				n_index[k] = e.node_indices_[cElementFaceIndex[j][k]];
			}

			if (FindExistingFace(n_index[0], n_index[1], n_index[2], face_index)) {
				AdxElementFace& ef = *(element_faces_[face_index]);
				ef.back_element_index_ = i;
				e.face_indices_[j] = face_index;
			}
			else {

				// Face生成
				std::unique_ptr<AdxElementFace> new_face;
				new_face.reset(new AdxElementFace(n_index, i));

				// Adxに登録
				element_faces_.push_back(std::move(new_face));
				int new_face_index = static_cast<int>(element_faces_.size()) - 1;

				// Elementに登録
				e.face_indices_[j] = new_face_index;

				// Nodeに登録
				int v0, v1, v2;
				num3sort(n_index[0], n_index[1], n_index[2], v0, v1, v2);
				AdxNode& n = *(nodes_[v0]);
				n.element_face_indices_.push_back(new_face_index);
			}
		}
	}

	return 0;
}
// 同一の３ノードで構成される面が既にあるか確認する
bool AdxModel::FindExistingFace(int& s, int& c, int& e, int& face_index)
{
	face_index = -1;

	int v0 = -1;
	int v1 = -1;
	int v2 = -1;

	num3sort(s, c, e, v0, v1, v2);

	// 最小インデクスのノードを頂点に持つ面とマッチング
	AdxNode& dst_node = *(nodes_[v0]);

	bool face_exist = false;

	for (int i = 0;i < dst_node.element_face_indices_.size(); i++) {
		AdxElementFace& ef = *(element_faces_[dst_node.element_face_indices_[i]]);
		if (ef.sorted_vertex_node_indices_[0] == v0 && ef.sorted_vertex_node_indices_[1] == v1 && ef.sorted_vertex_node_indices_[2] == v2) {
			face_exist = true;
			face_index = dst_node.element_face_indices_[i];
			break;
		}
	}

	return face_exist;
}

int AdxModel::ExtractSurfaceFaces(CString& es_name, std::vector<int>& vnode, std::vector<int>& vef)
{
	vnode.clear();
	vef.clear();

	// Search
	int index = -1;
	for (int i = 0; i < element_sets_.size(); i++) {
		AdxElementSet& es = *(element_sets_[i]);
		if (es.adx_name_ == es_name) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return 1;
	}

	// 表面Faceのみピックアップ
	AdxElementSet& es = *(element_sets_[index]);
	for (int i = 0; i < es.element_indices_.size(); i++) {
		int element_index = es.element_indices_[i];
		AdxElement& e = *(elements_[element_index]);
		for (int k = 0; k < 4; k++) {
			int face_index = e.face_indices_[k];
			AdxElementFace& ef = *(element_faces_[face_index]);
			if (ef.back_element_index_ == -1) {
				vef.push_back(face_index);
			}
		}
	}

	// 表面Faceのノードリスト作成
	std::map<int, int> map_node;
	for (int i = 0; i < vef.size(); i++) {
		AdxElementFace& ef = *(element_faces_[vef[i]]);
		for (int j = 0; j < 6; j++) {
			map_node[ef.node_indices_[j]] = vef[i];
		}
	}
	for (auto itr = map_node.begin(); itr != map_node.end(); ++itr) {
		vnode.push_back(itr->first);
	}

	return 0;
}

int AdxModel::ExportObjFile(CString& opath, CStringArray& es_names)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	// 表面face,node抽出
	std::vector<int> vnode;
	std::vector<int> vef;

	for (int i = 0; i < es_names.GetSize(); i++) {

		std::vector<int> tmp_vnode;
		std::vector<int> tmp_vef;

		CString name_element_set = es_names[i];
		ExtractSurfaceFaces(name_element_set, tmp_vnode, tmp_vef);

		std::copy(tmp_vnode.begin(), tmp_vnode.end(), std::back_inserter(vnode));
		std::copy(tmp_vef.begin(), tmp_vef.end(), std::back_inserter(vef));
	}

	// ノード Index　変換テーブル
	std::map<int, int> map_index;
	for (int i = 0; i < vnode.size(); i++) {
			map_index[vnode[i]] = i + 1;
	}

	// 出力
	ret = oFile.Open(opath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	// ノード出力
	for (int i = 0; i < vnode.size(); i++) {
		AdxNode& n = *(nodes_[vnode[i]]);
		buf.Format(_T("v %10.3f %10.3f %10.3f\n"), n.coord_.x, n.coord_.y, n.coord_.z);
		oFile.WriteString(buf);
	}

	// 面要素出力
	for (int i = 0; i < vef.size(); i++) {
		AdxElementFace& ef = *(element_faces_[vef[i]]);
		buf.Format(_T("f %8d %8d %8d\n"), map_index[ef.node_indices_[0]], map_index[ef.node_indices_[1]], map_index[ef.node_indices_[2]]);
		oFile.WriteString(buf);
	}

	oFile.Close();

	return 0;
}


int AdxModel::ExportSurfaceNodes(CString& opath, CStringArray& es_names)
{
	BOOL			ret;
	CStdioFile		oFile;
	CString			buf;

	// 出力ファイル
	ret = oFile.Open(opath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	if (ret == FALSE) {
		return 1;
	}

	for (int i = 0; i < es_names.GetSize(); i++) {

		CString name_element_set = es_names[i];

		// 表面face,node抽出
		std::vector<int> vnode;
		std::vector<int> vef;

		ExtractSurfaceFaces(name_element_set, vnode, vef);

		// ノード Index　変換テーブル
		std::map<int, int> map_index;
		for (int i = 0; i < vnode.size(); i++) {
			map_index[vnode[i]] = i;
		}
		vnode.clear();
		for (auto x : map_index) {
			vnode.push_back(x.first);
		}

		// ノード出力
		for (int i = 0; i < vnode.size(); i++) {
			AdxNode& n = *(nodes_[vnode[i]]);
			buf.Format(_T("%8d,%10.3f,%10.3f,%10.3f\n"),n.id_, n.coord_.x, n.coord_.y, n.coord_.z);
			oFile.WriteString(buf);
		}

	}

	oFile.Close();

	return 0;
}

bool AdxModel::IsEmpty()
{
	if (nodes_.size() > 0) {
		return false;
	}
	else {
		return true;
	}
}

int AdxModel::ExtractSurfaceNodes(CStringArray& es_names, SurfaceNodeMap& surface_node)
{
	std::map<int, int> map_node_index;

	for (int i = 0; i < es_names.GetSize(); i++) {

		CString name_element_set = es_names[i];

		// 表面face,node抽出
		std::vector<int> vnode;
		std::vector<int> vef;

		ExtractSurfaceFaces(name_element_set, vnode, vef);

		// ノード追加
		for (int i = 0; i < vnode.size(); i++) {
			map_node_index[vnode[i]] = i;
		}	
	}

	// 実体ノードClone
	for (auto x : map_node_index) {
		int adx_node_index = x.first;

		// 2次ノードのみ抽出
		//if (nodes_[adx_node_index]->type_ == MidSideNode) {
			std::unique_ptr<AdxNode> new_node(new AdxNode);
			new_node->Copy(*(nodes_[adx_node_index]));
			new_node->force_vector_.Set(0.0, 0.0, 0.0);
			surface_node.AddNode(std::move(new_node));
		//}
	}

	return 0;
}

int AdxModel::ExtractSurfaceFaces(CStringArray& es_names, SurfaceFaceMap& surface_face)
{
	surface_face.Clear();

	std::map<int, int> local_node_indices;
	std::vector<int> surface_face_indices;

	for (int i = 0; i < es_names.GetSize(); i++) {
		CString name_element_set = es_names[i];
		std::vector<int> vnode;
		std::vector<int> vef;

		if (ExtractSurfaceFaces(name_element_set, vnode, vef) != 0) {
			return 1;
		}

		std::copy(vef.begin(), vef.end(), std::back_inserter(surface_face_indices));
	}

	for (int i = 0; i < static_cast<int>(surface_face_indices.size()); i++) {
		AdxElementFace& face = *(element_faces_[surface_face_indices[i]]);
		for (int j = 0; j < 3; j++) {
			int adx_node_index = face.node_indices_[j];
			if (local_node_indices.find(adx_node_index) == local_node_indices.end()) {
				std::unique_ptr<AdxNode> new_node(new AdxNode);
				new_node->Copy(*(nodes_[adx_node_index]));
				new_node->force_vector_.Set(0.0, 0.0, 0.0);
				local_node_indices[adx_node_index] = surface_face.AddNode(std::move(new_node));
			}
		}

		surface_face.AddFace(
			local_node_indices[face.node_indices_[0]],
			local_node_indices[face.node_indices_[1]],
			local_node_indices[face.node_indices_[2]]);
	}

	return surface_face.BuildGeometry();
}
int AdxModel::SortElementSets()
{
	return 0;
}

bool CompareAdxElementSet(AdxElementSet* e1, AdxElementSet* e2)
{
	if (e1->id_ < e2->id_) {
		return true;
	}
	else {
		return false;
	}
}