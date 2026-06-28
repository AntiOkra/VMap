#pragma once

#include"AdxNode.h"

#include<vector>
#include<map>

class AdxNodeSet
{
public:
	AdxNodeSet(void);
	~AdxNodeSet(void);

	CString adx_name_;	// Adxデータ上の名称
	CString user_name_;	// ユーザー設定名称

	std::vector<int>	node_indices_;
};

