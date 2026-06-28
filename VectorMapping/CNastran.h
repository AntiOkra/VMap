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

	int m_ID;
	CMzPoint m_Coord;					// ノード座標値

	CMzPoint m_NormalPressureVector;	// 法線方向圧力ベクトル
	CMzPoint m_TangentPressureVector;	// 接線方向圧力ベクトル
	CMzPoint m_PressureVector;			// PressureVector = NormalPressureVector + TangentPressureVector
	CMzPoint m_ForceVector;				// ForceVector = PressureVector * Area
	double	 m_Area;					// ノード面積(仮想的なもの)

	//int Read(CStringArray& words);
	int Read(CString& line1, CString& line2);

};

class NastranElement
{
public:
	NastranElement();
	~NastranElement();

	int m_ID;

	NastranElementType m_type;
	int m_NodeID[4];
	int m_NodeIndex[4];
	double m_Area;

	int Read(const CString& line);
};

class NastranModel
{
public:
	NastranModel();
	~NastranModel();

	std::vector<std::unique_ptr<NastranNode>>			m_vNode;
	std::vector<std::unique_ptr<NastranElement>>		m_vElement;

	std::unordered_map<int, int>		m_mNodeIDtoIndex;
	std::unordered_map<int, int>		m_mElementIDtoIndex;

	CStdioFile*				m_LogFile;
	
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

