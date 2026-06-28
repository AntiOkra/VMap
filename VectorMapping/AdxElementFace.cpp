#include "StdAfx.h"
#include "AdxElementFace.h"
#include "GeneralFunction.h"

AdxElementFace::AdxElementFace(void)
{
}
 
AdxElementFace::AdxElementFace(int node_index[6], int& face_index)
{
	for (int i = 0; i < 6; i++) {
		node_indices_[i] = node_index[i];
	}

	front_element_index_ = face_index;
	back_element_index_ = -1;

	num3sort(node_index[0], node_index[1], node_index[2], sorted_vertex_node_indices_[0], sorted_vertex_node_indices_[1], sorted_vertex_node_indices_[2]);
}

AdxElementFace::~AdxElementFace(void)
{
}