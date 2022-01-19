#ifndef GRAPH_H
#define GRAPH_H
#include <string>

using namespace std;
class graph
{
    public:
    long long* beginPos1;
    int* beginPos;
    int* edgeList;
    int* deg;
    long long* edgeListDeg;
    int uCount,vCount,breakVertex32,breakVertex10,vertexCount;
    long long edgeCount;
    void loadgraph(string folderName,int bound);
    int* beginPos16;
    int s16;
    ~graph();
};

#endif