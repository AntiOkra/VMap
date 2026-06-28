#pragma once

// 4 面体2次要素の面番号と対応する節点番号
const int cElementFaceIndex[4][6] = { {1,2,3,7,8,9},{0,3,2,6,8,5},{0,1,3,4,9,6},{0,2,1,5,7,4} };

class AdxElementFace
{
public:
	AdxElementFace(void);
	AdxElementFace(int node_index[6], int& face_index);
	~AdxElementFace(void);

	int node_indices_[6];

	int front_element_index_;	// この面が表側になる要素
	int back_element_index_;		// この面が裏側になる要素
	int sorted_vertex_node_indices_[3];	// 昇順にソートされた頂点ノードインデックス
};


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

