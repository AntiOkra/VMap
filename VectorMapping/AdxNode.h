#pragma once

#include "MzPoint.h"
#include "AdxNode.h"
#include <vector>

typedef enum { UndefinedNode, CornerNode, MidSideNode } AdxNodeType;		// 	未定義、主節点、中間節点

class AdxNode
{
public:
	AdxNode(void);
	~AdxNode(void);

	int			id_;
	CMzPoint	coord_;
	CMzPoint	force_vector_;
	AdxNodeType	type_;

	std::vector<int>	element_face_indices_;	// このノードを頂点(最小インデクス)として持つ面インデクス

	int Read(CStringArray& words);
	int Copy(AdxNode& n);
	void SetType(AdxNodeType node_type);

};

