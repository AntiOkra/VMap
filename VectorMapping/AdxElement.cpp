#include "StdAfx.h"
#include "AdxElement.h"
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


AdxElement::AdxElement(void)
{
	id_ = -1;
}


AdxElement::~AdxElement(void)
{
}

//
// 971               528               529               530               531               554               556               557               555               559               558               

//
int AdxElement::Read(CStringArray& words)
{
	ASSERT(words.GetCount() >= 11);

	id_ = _ttoi(words[0]);

	for (int i = 0; i < 10; i++) {
		node_ids_[i] = _ttoi(words[i + 1]);
	}

	return 0;
}