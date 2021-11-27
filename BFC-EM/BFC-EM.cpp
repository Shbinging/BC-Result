#include<bits/stdc++.h>
#include<bits/stdc++.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include"BFC-EM.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/tbb_allocator.h>
#include "tbb/tick_count.h"
#include <tbb/mutex.h>
using namespace std;

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
        FILE* fp = fopen(path.c_str(), "w");
        if (fp == NULL){
            printf("write error");
            return 0;
        }      
        wrr(fp, num, 1);
        sort(bufferArray, bufferArray + num);
        wrr(fp, bufferArray, num);
        fclose(fp);
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
    bufferPool(string _dirPath, int _p, int _maxBufferSize){
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
    bufferReader(int num, int _bufferSize){
        fp = fopen(getPath(num).c_str(), "r");
        fread(&n, 8, 1, fp);
        bufferSize = _bufferSize;
        buffer = new long long[bufferSize];
        s = 0;
        head = bufferSize;
    }
    void load(){
        head = 0;
        bufferSize = min(bufferSize, (int)n - s);
        fread(buffer, 8, bufferSize, fp);
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
    long long n;
    int s;
    int bufferSize;
    int head;
    long long* buffer;
    FILE* fp;
};

long long hashing(long long a, long long b, int vertexCount){
    return  a * vertexCount + b;
}
int bfcEm(graph1& g, int maxBufferSize){
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
    vector<long long>a;
    for(int i = 0; i < b.bufferNum; i++){
        bufferReader br(i, maxBufferSize);
        for(int j = 0; j < br.n; j++){
            a.push_back(br.get());
        }
    }
    int n = a.size();
    sort(a.begin(), a.end());
    long long ans = 0, s = 0, tmp = -1;
    for(int i = 0; i < n; i++){
        if (a[i] != tmp){
            tmp = a[i];
            ans += s * (s - 1) / 2;
            s = 1;
        }else s++;
    }
    ans += s * (s - 1) / 2;
    cout << n << " " << ans << endl;
}