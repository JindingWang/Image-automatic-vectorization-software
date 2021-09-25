#ifndef  _SHP_WRITER_H
#define  _SHP_WRITER_H

#include <vector>
using namespace std;

extern "C"
{
	_declspec(dllexport) void writeSHPpolygon(char* FileName, vector<vector<pair<float, float>>>& outLine, vector<vector<pair<float, float>>>& inLine,
		vector<vector<unsigned int>>& holeIndex, vector<int>& label);
}

#endif