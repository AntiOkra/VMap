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

	int			m_ID;
	CMzPoint	m_Coord;
	CMzPoint	m_ForceVector;
	AdxNodeType	m_Type;

	std::vector<int>	m_ElementFaceIndex;	// このノードを頂点(最小インデクス)として持つ面インデクス

	int Read(CStringArray& words);
	int Copy(AdxNode& n);
	void SetType(AdxNodeType node_type);

};

