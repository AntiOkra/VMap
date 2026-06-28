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

	void Clear();
	int Read(CString& fpath);
	int Activate();

	int ExportObjFile(CString& fpath, CStringArray& es_names);
	int ExportSurfaceNodes(CString& fpath, CStringArray& es_names);

	bool IsEmpty();
	int ElementSetCount() const;
	AdxElementSet& ElementSetAt(int index);
	const AdxElementSet& ElementSetAt(int index) const;

	bool FindExistingFace(int& s, int& c, int& e, int& face_index);
	int  ExtractSurfaceFaces(CString& es_name, std::vector<int>& vnode, std::vector<int>& vef);
	int  ExtractSurfaceNodes(CStringArray& es_names, SurfaceNodeMap& surface_node);
	int  ParseElementSetInfo(CString& nodeset_line, CString& name, int& id, CString& comment);
	int  SortElementSets();

private:
	std::vector<std::unique_ptr<AdxNode>>			nodes_;
	std::vector<std::unique_ptr<AdxNodeSet>>		node_sets_;

	std::vector<std::unique_ptr<AdxElement>>		elements_;
	std::vector<std::unique_ptr<AdxElementFace>>	element_faces_;
	std::vector<std::unique_ptr<AdxElementSet>>	element_sets_;

	std::unordered_map<int, int>	node_id_to_index_;
	std::unordered_map<int, int>	element_id_to_index_;
	std::map<CString, int>			element_set_name_to_index_;

	void FinishNodeSet(std::unique_ptr<AdxNodeSet>& node_set);
	void FinishElementSet(std::unique_ptr<AdxElementSet>& element_set);
};

bool CompareAdxElementSet(AdxElementSet* e1, AdxElementSet* e2);
