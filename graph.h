#ifndef GRAPH_H
#define GRAPH_H
#include <string>

using namespace std;
struct res{
    double t;
    long long ans;
    res(){
        t = 0;
        ans = 0;
    }
    res(double _t, long long _ans){
        t = _t;
        ans = _ans;
    }
};

class graph
{
    public:
    long long* beginPos1;
    int* beginPos;
    int* edgeList;
    int* deg;
    int uCount,vCount,breakVertex32,breakVertex10,vertexCount;
    long long edgeCount;
    void loadgraph(string folderName,int bound);
    int* beginPos16;
    int s16;
    ~graph();
};

#endif