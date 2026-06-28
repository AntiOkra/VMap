#pragma once

enum class NastranElementType { Triangle, Quad, Beam };

class NastranElement
{
public:
	NastranElement();
	~NastranElement();

	int id_;

	NastranElementType type_;
	int node_ids_[4];
	int node_indices_[4];
	double area_;

	int Read(const CString& line);
};