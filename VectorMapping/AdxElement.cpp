#include "StdAfx.h"
#include "AdxElement.h"
#include "GeneralFunction.h"

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