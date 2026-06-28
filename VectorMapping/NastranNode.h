#pragma once

#include "MzPoint.h"

class NastranNode
{
public:
	NastranNode();
	~NastranNode();

	int id_;
	CMzPoint coord_;					// ノード座標値

	CMzPoint normal_pressure_vector_;	// 法線方向圧力ベクトル
	CMzPoint tangent_pressure_vector_;	// 接線方向圧力ベクトル
	CMzPoint pressure_vector_;			// PressureVector = NormalPressureVector + TangentPressureVector
	CMzPoint force_vector_;				// ForceVector = PressureVector * Area
	double	 area_;					// ノード面積(仮想的なもの)

	int Read(CString& line1, CString& line2);
};