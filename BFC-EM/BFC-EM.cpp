#include<bits/stdc++.h>
#include<bits/stdc++.h>
#include"BFC-EM.h"
#include "../BFC-VP++/timer.h"
#include "stdio.h"
#include "wolfsort/src/wolfsort.h"
using namespace std;

timer calcTime;
template<class T> void frr(FILE* fp, T& dest, long long size = 1){
    fread(&dest, sizeof(T), size, fp);
}

template<class T> void frr(FILE* fp, T* dest, long long size = 1){
    fread(dest, sizeof(T), size, fp);
}

template <class T>
void wrr(FILE *fp, T* val, long long size = 1){
    fwrite(val, sizeof(T), size, fp);
}
template <class T>
void wrr(FILE *fp, T val, long long size = 1){
    fwrite(&val, sizeof(T), size, fp);
}
inline int cmp_long(const void * a, const void * b)
{
	const long long fa = *(const long long *) a;
	const long long fb = *(const long long *) b;
	return (fa > fb) - (fa < fb);
}

class buffer{
public:
    buffer(string _path, long long _size){
        size = _size;
        bufferArray = new long long [size];
        num = 0;
        path = _path;
    }
    bool append(long long val){
        if (num >= size) return 0;
        else bufferArray[num++] = val;
        return 1;
    }
    bool write(){
        //ioTime.start();
        FILE* fp = fopen(path.c_str(), "w");
        if (fp == NULL){
            printf("write error");
            return 0;
        }      
        wrr(fp, num, 1);
        calcTime.start();
        //sort(bufferArray, bufferArray + num);
        wolfsort(bufferArray, num, 8, cmp_long);
        calcTime.fin();
   
        wrr(fp, bufferArray, num);
        fclose(fp);
        //ioTime.fin();
    }
    void reset(string _path){
        num = 0;
        path = _path;
    }
    long long size, num;
    string path;
    long long* bufferArray;
};
string getPath(int num){    
    stringstream ss;
    ss << num;
    string ans;
    ss >> ans;
    return "diskData/" + ans + ".in";
}
class bufferPool{
public:
    bufferPool(string _dirPath, int _p, long long _maxBufferSize){
        dirPath = _dirPath;
        bufferNum = 0;
        p = _p;
        b = new buffer(getPath(bufferNum), _maxBufferSize);
    }
    void append(long long val){
        if (!b->append(val)){
            b->write();
            bufferNum++;
            b->reset(getPath(bufferNum));
            b->append(val);      
        }
    }
    void finish(){
        b->write();
        bufferNum++;
    }
    string dirPath;
    int bufferNum, p;
    buffer* b;
};

class bufferReader{
public:
    bufferReader(int num, long long _bufferSize){
        filePath = getPath(num);
        //ioTime.start();
        fp = fopen(filePath.c_str(), "r");
        assert(fp != NULL);
        fread(&n, 8, 1, fp);
        //ioTime.fin();
        bufferSize = _bufferSize;
        buffer = new long long[bufferSize];
        s = 0;
        head = bufferSize;
        //fclose(fp);
    }
    void load(){
        //ioTime.start();
        //fp = fopen(filePath.c_str(), "r");
        //assert(fp != NULL);
        //fseek(fp, 1LL* (s + 1) *8, 0);
        head = 0;
        bufferSize = min(bufferSize, n - s);
        fread(buffer, 8, bufferSize, fp);
        //ioTime.fin();
        //fclose(fp);
    }
    long long get(){
        if (head == bufferSize && !isEmpty()){
            load();
        }
        if (head < bufferSize){
            s++;
            return buffer[head++];
        }
    }
    bool isEmpty(){
        if (s >= n) return 1;
        else return 0;
    }
    long long n, s, bufferSize, head;
    long long* buffer;
    string filePath;
    FILE* fp;
};

long long hashing(long long a, long long b, int vertexCount){
    return  a * vertexCount + b;
}
struct node{
    long long val;
    int pos;
    node(){}
    node(long long _val, int _pos){
        val = _val;
        pos = _pos;
    }
};
bool operator <(node a, node b){
    return a.val > b.val;
}

int bfcEm(string graphName, long long storageSize){
    timer totalTime;
    totalTime.start();
    long long maxBufferSize = storageSize / 8 * 1024 * 1024;
    graph1 g;
    g.loadgraph("/home/shbing/datasetsNew/datasets/bipartite/"+ graphName + "/sorted", -1);
    int num = 0;
    bufferPool b("diskData/", 1, maxBufferSize);
    for(int i = 0; i < g.vertexCount; i++){
        int len = g.beginPos1[i + 1] - g.beginPos1[i];
        g.loadsubGraph(g.beginPos1[i], g.beginPos1[i + 1]);
        for(int j = 0; j < len; j++){
            if (g.edgeList[j] >= i) break;
            for(int k = j + 1; k < len; k++){
                b.append(hashing(g.edgeList[k], g.edgeList[j], g.vertexCount));
            }
        }
    }
    b.finish();
    delete b.b->bufferArray;
    bufferReader** brList = new bufferReader*[b.bufferNum];
    priority_queue<node> q;
    long long nn = 0;
    for(int i = 0; i < b.bufferNum; i++){
        brList[i] = new bufferReader(i, maxBufferSize / b.bufferNum);
        nn++;
        q.push(node(brList[i]->get(), i));
    }
    long long tmp = -1, s = 0, ans = 0;
    //vector<long long> c;
    while(!q.empty()){
        node tmpNode = q.top();
        q.pop();
        //c.push_back(tmpNode.val);
        if (tmpNode.val != tmp){
            tmp = tmpNode.val;
            ans += s * (s - 1) / 2;
            s = 1;
        }else s++;
        if (!brList[tmpNode.pos]->isEmpty()){
            nn++;
            q.push(node(brList[tmpNode.pos]->get(), tmpNode.pos));
        }
    }
    ans += s * (s - 1) / 2;
    totalTime.fin();
    FILE* ansfile = fopen("testOut.csv", "a+");
    if (ansfile == NULL){
        printf("ans open wrong!");
    }
    fprintf(ansfile, "%s,%lld,%lld,%f,%f\n", graphName.c_str(), storageSize, ans,  totalTime.getTime(), calcTime.getTime());
    fclose(ansfile);
    //cout << n << "ans << endl;
}