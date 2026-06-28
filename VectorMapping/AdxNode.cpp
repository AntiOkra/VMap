#include "StdAfx.h"
#include "AdxNode.h"
#include "GeneralFunction.h"


AdxNode::AdxNode(void)
{
	id_ = -1;
	coord_.Set(0.0, 0.0, 0.0);
	force_vector_.Set(0.0, 0.0, 0.0);
	type_ = UndefinedNode;
}


AdxNode::~AdxNode(void)
{
}

//
// 3                2.100002e+002    2.000002e+002    0.000000e+000   
//
int AdxNode::Read(CStringArray& words)
{
	ASSERT(words.GetCount()>=4);

	id_ = _ttoi(words[0]);

	coord_.x = _tstof(words[1]);
	coord_.y = _tstof(words[2]);
	coord_.z = _tstof(words[3]);

	return 0;
}

int AdxNode::Copy(AdxNode& n)
{
	id_ = n.id_;
	coord_ = n.coord_;
	force_vector_ = n.force_vector_;
	type_ = n.type_;

	return 0;
}

void AdxNode::SetType(AdxNodeType node_type)
{
	if (type_ == UndefinedNode) {
		type_ = node_type;
	}
	else {
		if (type_ != node_type) {
			CString msg;
			msg.Format(_T("NodeType(Žå,•›)‚ª’è‚Ü‚è‚Ü‚¹‚ñ.Node_ID(%d)"), id_);
			AfxMessageBox(msg);
		}
	}
}