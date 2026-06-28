#pragma once

#include<vector>
#include<map>
#include<unordered_map>
#include<memory>

#include"MzPoint.h"

typedef enum { CTRIA3, CQUAD4, CBEAM } NastranElementType;

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

	//int Read(CStringArray& words);
	int Read(CString& line1, CString& line2);

};

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

class NastranModel
{
public:
	NastranModel();
	~NastranModel();

	std::vector<std::unique_ptr<NastranNode>>			nodes_;
	std::vector<std::unique_ptr<NastranElement>>		elements_;

	std::unordered_map<int, int>		node_id_to_index_;
	std::unordered_map<int, int>		element_id_to_index_;

	CStdioFile*				log_file_;
	
	void RemoveAll();
	int  ReadNastranFile(const CString& fpath);
	int ReadNormalVectorFile(const CString& fpath);		// Nastran deha nai kedo
	int ReadTangentVectorFile(const CString& fpath);	// Nastran deha nai kedo
	bool IsEmpty();
	int  GetTotalForce(CMzPoint& force);

	void LogWrite(CString &msg);
	int  Dump_N(CString& fpath);
	int  Dump_E(CString& fpath);

	int  Indexing();
	int  ForceCalc();


};

