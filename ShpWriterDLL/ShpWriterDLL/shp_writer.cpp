#include <stdlib.h>
#include <vector>
#include <string>
#include <ostream>
#include "shp_writer.h"
#include "shapefil.h"
using namespace std;

void writeSHPpolygon(char* FileName, vector<vector<pair<float, float>>>& outLine, vector<vector<pair<float, float>>>& inLine,
	vector<vector<unsigned int>>& holeIndex, vector<int>& label)
{
	int polygonNum = outLine.size();
	int ShapeType = SHPT_POLYGON;
	string strFileName = FileName;
	string pszShapeFile = strFileName + ".shp"; //shp name
	string dpfShapeFile = strFileName + ".dbf";//dbf name

	SHPHandle hSHP = SHPCreate(pszShapeFile.c_str(), ShapeType);
	DBFHandle hDbf = DBFCreate(dpfShapeFile.c_str());

	if (hSHP == NULL || hDbf == NULL)
	{
		printf("Unable to create:%s\n", pszShapeFile);
		exit(1);
	}

	int ptsize = 0;
	for (int i = 0; i < polygonNum; i++)
	{
		int pointNum = outLine[i].size();
		vector<int> startPos;
		startPos.push_back(0);
		for (int j = 1; j < int(holeIndex[i].size()); j++)
		{
			startPos.push_back(pointNum);
			pointNum += inLine[holeIndex[i][j]].size();
		}

		double* pdx = new double[pointNum];
		double* pdy = new double[pointNum];
		//memset(pdx, 0, pointNum);
		//memset(pdy, 0, pointNum);

		int writeNum = 0;
		for (int j = 0; j < int(outLine[i].size()); j++)
		{
			pdx[j] = outLine[i][j].second;
			pdy[j] = outLine[i][j].first;
		}
		writeNum += outLine[i].size();
		for (int j = 1; j < int(holeIndex[i].size()); j++)
		{
			for (int k = 0; k < int(inLine[holeIndex[i][j]].size()); k++)
			{
				pdx[writeNum] = inLine[holeIndex[i][j]][k].second;
				pdy[writeNum] = inLine[holeIndex[i][j]][k].first;
				writeNum++;
			}
		}
		ptsize = ptsize + pointNum;

		int * partStart = new int[startPos.size()];
		int * partType = new int[startPos.size()];
		for (int j = 0; j < int(startPos.size()); j++)
		{
			partStart[j] = startPos[j];
			partType[j] = SHPP_RING;
		}

		SHPObject* ishpline = SHPCreateObject(SHPT_POLYGON, -1, startPos.size(), partStart, partType, pointNum, pdx, pdy, NULL, NULL);
		//SHPT_ARC是矢量的形式：点、线、多边形等
		//-1,表示新的要素
		//1，要数有几个部分组成
		//0，各部件的起始点位置
		//NULL，各个部分的类型
		//nNumTotalPoint，点的数量
		//pdx,pdy，点的平面坐标
		//NULL，点的高程坐标
		//NULL，点的M值

		//shp,重新计算包围盒
		SHPComputeExtents(ishpline);

		//shp,写入记录
		SHPWriteObject(hSHP, -1, ishpline);
		//-1表示写入新的形状

		if (pdx) delete[] pdx;
		if (pdy) delete[] pdy;
		if (partStart) delete[] partStart;
		if (partType) delete[] partType;
	}

	//dbf,添加字段
	int ifield = DBFAddNativeFieldType(hDbf, "ID", 'N', 6, 2);
	int classField = DBFAddNativeFieldType(hDbf, "class", 'N', 6, 2);

	//dbf，写入字段值
	int nrecordCnt = 0;
	for (int i = 0; i < polygonNum; i++)
	{
		DBFWriteIntegerAttribute(hDbf, i, ifield, i);
		DBFWriteIntegerAttribute(hDbf, i, classField, label[i]);
		//nrecordCnt = DBFGetRecordCount(hDbf);
	}

	SHPClose(hSHP);
	DBFClose(hDbf);
	return;
}