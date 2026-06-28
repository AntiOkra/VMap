#pragma once

#include<vector>
#include<map>
#include<unordered_map>
#include<memory>

#include "NastranNode.h"
#include "NastranElement.h"

class NastranModel
{
public:
	NastranModel();
	~NastranModel();

	void SetLogFile(CStdioFile* log_file);
	const std::vector<std::unique_ptr<NastranNode>>& Nodes() const;

	void Clear();
	int  ReadModelFile(const CString& fpath);
	int ReadNormalPressureFile(const CString& fpath);		// Nastran deha nai kedo
	int ReadTangentPressureFile(const CString& fpath);	// Nastran deha nai kedo
	bool IsEmpty();
	int  CalculateTotalForce(CMzPoint& force);

	void WriteLog(CString &msg);
	int  DumpNodes(CString& fpath);
	int  DumpElements(CString& fpath);

	int  Indexing();
	int  CalculateForces();

private:
	std::vector<std::unique_ptr<NastranNode>>			nodes_;
	std::vector<std::unique_ptr<NastranElement>>		elements_;

	std::unordered_map<int, int>		node_id_to_index_;
	std::unordered_map<int, int>		element_id_to_index_;

	CStdioFile*				log_file_;

	int ReadPressureFile(const CString& fpath, bool normal_pressure);
	int ReadGridLine(CStdioFile& file, CString& line, int& line_count);
	int ReadElementLine(const CString& line, int line_count);
	int AccumulateElementAreas();
	void CalculateNodeForces();
};
