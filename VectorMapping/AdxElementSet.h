#pragma once

#include <vector>
#include <map>
#include "AdxElement.h"


//##TSV BODY  2 "Body_2"
//########################################################################################
//$Element  3DQuadraticTetrahedron
//########################################################################################
//element_set = Body_2_PID1_e8  #Body_2
//971               528               529               530               531               554               556               557               555               559               558               
//972               528               530               532               533               556               561               562               560               564               563               

class AdxElementSet
{
public:
	AdxElementSet(void);
	~AdxElementSet(void);

	int     id_;		// ID(TSV_PRE)
	CString adx_name_;	// Adxデータ上の名称
	CString user_name_;	// ユーザー設定名称

	std::vector<int>	element_indices_;

	//bool Compare(const AdxElementSet* a, const AdxElementSet* b) { if (a->id_ < b->id_) { return true; } else { return false; } }
};

