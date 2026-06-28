#include "stdafx.h"
#include "NastranElement.h"

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
		type_ = NastranElementType::Triangle;
		snode_id[0] = line.Mid(24,8);
		snode_id[1] = line.Mid(32,8);
		snode_id[2] = line.Mid(40,8);
		snode_id[3] = "-1";
	} else if ( stype=="CQUAD4" ) {
		type_ = NastranElementType::Quad;
		snode_id[0] = line.Mid(24,8);
		snode_id[1] = line.Mid(32,8);
		snode_id[2] = line.Mid(40,8);
		snode_id[3] = line.Mid(48,8);
	} else if ( stype=="CBEAM" ) {
		type_ = NastranElementType::Beam;
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