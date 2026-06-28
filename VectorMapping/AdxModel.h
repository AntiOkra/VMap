#pragma once

#include<vector>
//#include<map>
#include<unordered_map>
#include<memory>

#include "AdxNode.h"
#include "AdxNodeSet.h"
#include "AdxElement.h"
#include "AdxElementSet.h"

#include "SpatialGrid.h"
#include "SurfaceNodeMap.h"

class AdxModel
{
public:
	AdxModel();
	~AdxModel();

	std::vector<std::unique_ptr<AdxNode>>			m_vNode;
	std::vector<std::unique_ptr<AdxNodeSet>>		m_vNodeSet;

	std::vector<std::unique_ptr<AdxElement>>		m_vElement;
	std::vector<std::unique_ptr<AdxElementFace>>	m_vElementFace;
	std::vector<std::unique_ptr<AdxElementSet>>	m_vElementSet;

	std::unordered_map<int, int>	m_mNodeIDtoIndex;
	std::unordered_map<int, int>	m_mElementIDtoIndex;
	std::map<CString, int>			m_mElementsetNametoIndex;

	void RemoveAll();
	int Read(CString& fpath);
	int Activate();

	int ExportObj(CString& fpath, CStringArray& es_names);
	int ExportSurfaceNode(CString& fpath, CStringArray& es_names);

	bool IsEmpty();

	bool FaceExist(int& s, int& c, int& e, int& face_index);
	int  SurfaceExtract(CString& es_name, std::vector<int>& vnode, std::vector<int>& vef);
	int  ExtractSurfaceNode(CStringArray& es_names, SurfaceNodeMap& surface_node);
	int  ElementSetInf(CString& nodeset_line, CString& name, int& id, CString& comment);
	int  SortElementSet();
};

bool CompareAdxElementSet(AdxElementSet* e1, AdxElementSet* e2);

