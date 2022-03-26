#include<bits/stdc++.h>
#include<bits/stdc++.h>
#include"BFC-EM.h"
#include "../BFC-VP++/timer.h"
#include "stdio.h"
#include "wolfsort/src/wolfsort.h"
using namespace std;

static uint32_t totalNodeGloable;
template<class T> void frr(FILE* fp, T& dest, u_int64_t size = 1){
    fread(&dest, sizeof(T), size, fp);
}

template<class T> void frr(FILE* fp, T* dest, u_int64_t size = 1){
    fread(dest, sizeof(T), size, fp);
}

template <class T>
void wrr(FILE *fp, T* val, u_int64_t size = 1){
    fwrite(val, sizeof(T), size, fp);
}
template <class T>
void wrr(FILE *fp, T val, u_int64_t size = 1){
    fwrite(&val, sizeof(T), size, fp);
}
inline int cmp_long(const void * a, const void * b)
{
	const u_int64_t fa = *(const u_int64_t *) a;
	const u_int64_t fb = *(const u_int64_t *) b;
	return (fa > fb) - (fa < fb);
}
inline int cmp_int(const void * a, const void * b)
{
	const u_int32_t fa = *(const u_int32_t *) a;
	const u_int32_t fb = *(const u_int32_t *) b;
	return (fa > fb) - (fa < fb);
}
inline uint32_t getLowerKey(uint64_t u){
    return u & 0xffffffffL;
}
inline uint32_t getHigherKey(uint64_t u){
    return (u >> 32);
}

void radixOneRound(u_int64_t* bufferArray, u_int64_t* tmpBuffer, u_int64_t num, u_int32_t (*getKey)(uint64_t), u_int32_t totalNode, uint32_t* keyOrderList){
    memset(keyOrderList, 0, sizeof(keyOrderList));
    for(uint32_t i = 0; i < num; i++){
        uint32_t key = getKey(bufferArray[i]);
        keyOrderList[key]++;
    }
    int n = 0;
    for(uint32_t i = 0; i < totalNode + 1; i++){
        if (keyOrderList[i]){
            keyOrderList[n++] = i;
        }
    }
    vector<uint32_t> keyEndPos, keyBeginPos;
    keyBeginPos.resize(n + 10, 0);
    keyEndPos.resize(n + 10, 0);
    wolfsort(keyOrderList, n, 4, cmp_int);
    uint32_t* keyOrderMap = new uint32_t[keyOrderList[n - 1] + 1];
    //sort(keyOrderList.begin(), keyOrderList.end());
    for(int i = 0; i < n; i++){
        keyOrderMap[keyOrderList[i]] = i;
    }
    for(int i = 0; i < num; i++){
        uint32_t key = getKey(bufferArray[i]);
        //keyBeginPos[0]++;
        keyBeginPos[keyOrderMap[key] + 1]++;
    }
    for(int i = 1; i <= n; i++){
        keyBeginPos[i] += keyBeginPos[i - 1];
    }
    for(int i = 1; i <= n; i++){
        keyEndPos[i] = keyBeginPos[i];
    }
    int kk = 0;
    for(int i = 0; i < num; i++){
        uint32_t key = getKey(bufferArray[i]);
        //tmpBuffer[kk++] = bufferArray[i]; 
        tmpBuffer[keyEndPos[keyOrderMap[key]]++] = bufferArray[i];
    }
    delete keyOrderMap;
    //printf("%lld\n", n);
}

void calcBuffer(u_int64_t* &bufferArray, u_int64_t num){
    uint32_t* keyOrderList = new uint32_t[totalNodeGloable + 1];
    uint64_t* tmpBuffer = new uint64_t[num];
    radixOneRound(bufferArray, tmpBuffer, num, getLowerKey, totalNodeGloable, keyOrderList);
    radixOneRound(tmpBuffer, bufferArray, num, getHigherKey, totalNodeGloable, keyOrderList);
    delete keyOrderList;
    delete tmpBuffer;
}

class buffer{
public:
    buffer(string _path, u_int64_t _size){
        size = _size;
        bufferArray = new u_int64_t [size];
        num = 0;
        path = _path;
    }
    bool append(u_int64_t val){
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
        //calcTime.start();
        //sort(bufferArray, bufferArray + num);
        //wolfsort(bufferArray, num, 8, cmp_long);
        //calcBuffer(bufferArray, num);
       
        //calcTime.fin();
        wrr(fp, bufferArray, num);
        fclose(fp);
        //ioTime.fin();
    }
    void reset(string _path){
        num = 0;
        path = _path;
    }
    void clear(){
        delete bufferArray;
    }
    u_int64_t size, num;
    string path;
    u_int64_t* bufferArray;
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
    bufferPool(string _dirPath, int _p, u_int64_t _maxBufferSize){
        dirPath = _dirPath;
        bufferNum = 0;
        p = _p;
        b = new buffer(getPath(bufferNum), _maxBufferSize);
    }
    void append(u_int64_t val){
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
    bufferReader(int num, u_int64_t _bufferSize){
        filePath = getPath(num);
        fp = fopen(filePath.c_str(), "r");
        assert(fp != NULL);
        fread(&n, 8, 1, fp);
        bufferSize = _bufferSize;
        buffer = new u_int64_t[bufferSize];
        s = 0;
        head = bufferSize;
    }
    void load(){
        head = 0;
        bufferSize = min(bufferSize, n - s);
        fread(buffer, 8, bufferSize, fp);
    }
    u_int64_t get(){
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
    ~bufferReader(){
        delete buffer;
    }
    u_int64_t n, s, bufferSize, head;
    u_int64_t* buffer;
    string filePath;
    FILE* fp;
};

u_int64_t hashing(u_int64_t a, u_int64_t b, int vertexCount){
    return  (a << 32) + b;
}
struct node{
    uint64_t val;
    uint64_t pos;
    node(){}
    node(u_int64_t _val, uint64_t _pos){
        val = _val;
        pos = _pos;
    }
};
bool operator <(node a, node b){
    return a.val > b.val;
}

int bfcEm(string graphName, u_int64_t storageSize){
    timer totalTime;
    timer genTime;
    timer sortTime;
    timer readTime;
    timer writeTime;
    timer mergeTime;
    totalTime.start();
    u_int64_t maxBufferSize = storageSize / 8 / 2 * 1024 * 1024;
    graph1 g;
    g.loadgraph("/home/shbing/datasetsNew/datasets/bipartite/"+ graphName + "/sorted", -1);
    totalNodeGloable = g.vertexCount;
    int num = 0;
    genTime.start();
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
    genTime.fin();
    for(int i = 0; i < b.bufferNum; i++){
        char path[100];
        sprintf(path, "diskData/%d.in", i);
        FILE* fp = fopen(path, "r+");
        assert(fp != NULL);
        uint64_t num;
        readTime.start();
        frr(fp, &num, 1);
        uint64_t* bufferArray = new uint64_t[num + 1];
        frr(fp, bufferArray, num);
        readTime.fin();
        sortTime.start();
        calcBuffer(bufferArray, num);
        sortTime.fin();
        fseek(fp, 0, SEEK_SET);
        writeTime.start();
        wrr(fp, num, 1);
        wrr(fp, bufferArray, num);
        fclose(fp);
        writeTime.fin();
    }
    mergeTime.start();
    bufferReader** brList = new bufferReader*[b.bufferNum];
    priority_queue<node> q;
    u_int64_t nn = 0;
    for(int i = 0; i < b.bufferNum; i++){
        brList[i] = new bufferReader(i, maxBufferSize / b.bufferNum);
        nn++;
        q.push(node(brList[i]->get(), i));
    }
    u_int64_t tmp = 0, s = 0, ans = 0;
    vector<u_int64_t> c;
    while(!q.empty()){
        node tmpNode = q.top();
        q.pop();
        if (tmpNode.val == tmp){
            s++;
        }else{
            tmp = tmpNode.val;
            ans += s * (s - 1) / 2;
            s = 1;
        }
        if (!brList[tmpNode.pos]->isEmpty()){
            nn++;
            q.push(node(brList[tmpNode.pos]->get(), tmpNode.pos));
        }
    }
    ans += s * (s - 1) / 2;
    mergeTime.fin();
    totalTime.fin();
    FILE* ansfile = fopen("testOut.csv", "a+");
    if (ansfile == NULL){
        printf("ans open wrong!");
    }
    fprintf(ansfile, "%s,%lld,%lld,%f,%f,%f,%f,%f,%f\n", graphName.c_str(), storageSize, ans,  totalTime.getTime(), genTime.getTime(), readTime.getTime(), writeTime.getTime(), sortTime.getTime(), mergeTime.getTime());
    fclose(ansfile);
}