#include "stdafx.h"
#include "NastranNode.h"

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