#pragma once

#include "AdxElementFace.h"

class AdxElement
{
public:
	AdxElement(void);
	~AdxElement(void);

	int id_;
	int node_ids_[10];
	int node_indices_[10];
	int face_indices_[4];

	int Read(CStringArray& words);
};

