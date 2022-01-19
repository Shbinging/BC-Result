#include"BFC-EM.h"
#include"gpu_helper.h"
#include"timer.h"
#include <cub/cub.cuh>
#include <bits/stdc++.h>
#include <thread>
#define fileLL(a) (1LL * a / 8)
#define fileInBytes(a) (1LL * a * 8)
using namespace cub;
typedef long long LL;
typedef unsigned long long ULL;


__device__ __managed__ long long vertexCount;

__device__ unsigned long long hashing(long long v, long long w){
    return v * vertexCount + w;
}


__global__ void bfcWrite(int beginU, int size, long long beginIndex, long long * beginPos, int* edgeList, unsigned long long* countSize, unsigned long long * hashTable){
    int idx = blockIdx.x;
    int u = beginU + blockIdx.x;
    int vLen = beginPos[blockIdx.x + 1] - beginPos[blockIdx.x];
    for(int j = threadIdx.x; j < vLen; j += blockDim.x){
        int v = edgeList[beginPos[idx] + j - beginIndex];
        if (v >= u) break;
        unsigned long long hashBeginPos = atomicAdd(countSize, vLen - j - 1);
        for(int k = 1; k <= vLen - j - 1; k++){
            int w = edgeList[beginPos[idx] + j + k - beginIndex];
            hashTable[hashBeginPos + k - 1] = hashing(v, w);
        }
    }
}

struct node{
    unsigned long long* file;
    long long n;
    node(){}
    node(unsigned long long * _file, long long _n){
        file = _file;
        n = _n;
    }
};
const long long oneG = 1024*1024*512;

void sortPhase(int sta, int threadNum, vector<node>& hashFile){
    timer tt1;
    tt1.start();
    double ts = 0;
    long long* d_keys_in;
    long long* d_keys_out;
    GEC(cudaMallocManaged(&d_keys_in, oneG));
    GEC(cudaMallocManaged(&d_keys_out, oneG));
    for(long long i = sta; i < hashFile.size(); i += threadNum){
        long long num_items = hashFile[i].n;
        timer tt2;
        tt2.start();
        memcpy(d_keys_in, hashFile[i].file, fileInBytes(num_items));
        // for(int j = 0; j < 10; j++){
        //     printf("%lld %lld\n", d_keys_in[j], hashFile[i].file[j]);
        // }
        tt2.fin();
        ts += tt2.getTime();
        void     *d_temp_storage = NULL;
        size_t   temp_storage_bytes = 0;
        cub::DeviceRadixSort::SortKeys(d_temp_storage, temp_storage_bytes, d_keys_in, d_keys_out, num_items);
        cudaMalloc(&d_temp_storage, temp_storage_bytes);
        cub::DeviceRadixSort::SortKeys(d_temp_storage, temp_storage_bytes, d_keys_in, d_keys_out, num_items);
        cudaDeviceSynchronize();
        tt2.start();
        memcpy(hashFile[i].file, d_keys_out, fileInBytes(num_items));
        tt2.fin();
        ts += tt2.getTime();
        cudaFree(d_temp_storage);
        //printf("sort file %d done!\n", i);
        fflush(stdout);
    }
    cudaFree(d_keys_in);
    cudaFree(d_keys_out);
    tt1.fin();
    //printf("sub sort time is %f\n", tt1.getTime());
    //printf("sub transfer time is %f\n", ts);
    fflush(stdout);
}

void sampleCount(node fileNode, vector<long long>& sumL, vector<long long>& sampleList){
    sumL.resize(sampleList.size() + 1);
    for(int i = 0; i < sumL.size(); i++) sumL[i] = 0;
    for(long long i = 0; i < fileNode.n; i++){
        sumL[lower_bound(sampleList.begin(), sampleList.end(), fileNode.file[i]) - sampleList.begin()]++;
    }
}

void alltoallGather(node fileNode, vector<long long>& prefixIndex, vector<node>& sortFileList, vector<long long>& sampleList){
    for(long long i = 0; i < fileNode.n; i++){
        int fileIndex = lower_bound(sampleList.begin(), sampleList.end(), fileNode.file[i]) - sampleList.begin();
        sortFileList[fileIndex].file[prefixIndex[fileIndex]++] = fileNode.file[i];
    }
}

void reduceCount(node fileNode, long long& ans){
    long long s = 0;
    unsigned long long pre = 0;
    ans = 0;
    for(long long i = 0; i < fileNode.n; i++){
        unsigned long long key = fileNode.file[i];
        if (key != pre){
            pre = key;
            ans += s * (s - 1) / 2;
            s = 1;
        }else s++;
    }
    ans += s * (s - 1) / 2;
}

void test(graph& g) {
    timer tall;
    tall.start();
    vertexCount = g.vertexCount;
    vector<node> hashFile;
    long long uStart = 0;
    long long check = 0;
    unsigned long long* tmpHashTable, *GTmpHashTable;
    timer t;
    t.start();
    float tCalc = 0;
    float tPre = 0;
    printf("========generate process by CPU & GPU========\n");
    while(uStart < g.vertexCount){
        long long uEnd;
        long long fileSize = 0;
        int u;
        long long uSize = 0;
        timer tt1;
        tt1.start();
        for(u = uStart; u < g.vertexCount; u++){
            uSize = 0;
            for(long long i = g.beginPos[u]; i < g.beginPos[u + 1]; i++){
                int v = g.edgeList[i];
                if (v >= u) break;
                fileSize += g.beginPos[u + 1] - i - 1;
                uSize += g.beginPos[u + 1] - i - 1;
            }
            if (fileSize >= fileLL(oneG)) break;
        }
        if (fileSize <= oneG / 8){
            uEnd = u;
        }else{
            uEnd = u - 1;
            fileSize -= uSize;
        }
        if (uEnd == g.vertexCount) uEnd -= 1;
        tt1.fin();
        tPre += tt1.getTime();
        int size = uEnd - uStart + 1;
        unsigned long long* countSize;
        cudaMallocManaged(&countSize, 8);
        long long* GBeginPos;
        int* GEdgeList;
        GEC(cudaMallocManaged(&GTmpHashTable, fileSize * 8));
        GEC(cudaMallocManaged(&GBeginPos, (uEnd - uStart + 2) * 8));
        //printf("%d\n", (g.beginPos1[uEnd + 1] - g.beginPos1[uStart]) / 1024 / 1024 * 4);
        GEC(cudaMallocManaged(&GEdgeList, (g.beginPos1[uEnd + 1] - g.beginPos1[uStart]) * 4));
        memcpy(GBeginPos, g.beginPos1 + uStart, (uEnd - uStart + 2) * 8);
        memcpy(GEdgeList, g.edgeList + g.beginPos1[uStart], (g.beginPos1[uEnd + 1] - g.beginPos1[uStart]) * 4);
        *countSize = 0;
        timer tt;
        tt.start();
        bfcWrite <<< size, 512 >>> (uStart, size, g.beginPos1[uStart], GBeginPos, GEdgeList, countSize, GTmpHashTable);
        cudaDeviceSynchronize();
        tt.fin();
        check += fileSize;
        tmpHashTable = new unsigned long long[fileSize];
        cudaMemcpy(tmpHashTable, GTmpHashTable, fileInBytes(fileSize), cudaMemcpyDeviceToHost);
        hashFile.push_back(node(tmpHashTable, fileSize));
        tCalc += tt.getTime();
        //printf("%d %d\n", fileSize, *countSize);
        cudaFree(countSize);
        cudaFree(GBeginPos);
        cudaFree(GEdgeList);
        cudaFree(GTmpHashTable);
        uStart = uEnd + 1;
    }
    t.fin();
    printf("total time is %f\npre time is %f\ncaculate time is %f\ntransfer time is %f\n", t.getTime(), tPre, tCalc, t.getTime() - tCalc - tPre);
    printf("total count pair without deduping is %lld\n", check);
    
    fflush(stdout);
    printf("==============simple sample process by CPU===========\n");
    timer tt3;
    tt3.start();
    vector<long long> a;
    long long ttPairCheck = 0;
    for(int i = 0; i < hashFile.size(); i++){
        ttPairCheck += hashFile[i].n;
        long long step = hashFile[i].n / 8192;
        for(int j = 0; j < hashFile[i].n; j += step){
            a.push_back(hashFile[i].file[j]);
        }
    }
    sort(a.begin(), a.end());
    int sampleFileSize = hashFile.size() * 1.1;//attention *>1 to avoid greater than oneG
    int step = a.size() / sampleFileSize;
    int k = sampleFileSize - 1, i = step;
    vector<long long> sampleList;
    while(k){
        sampleList.push_back(a[i]);
        i += step;
        k--;
    }
    vector<vector<long long>> sumList;
    sumList.resize(hashFile.size());
    thread threadsS[hashFile.size()];
    for(int i = 0; i < hashFile.size(); i++){
        threadsS[i] = thread(sampleCount, hashFile[i], ref(sumList[i]), ref(sampleList));
    }
    for(int i = 0; i < hashFile.size(); i++){
        threadsS[i].join();
    }
    vector<long long> sampleFileSizeList;
    sampleFileSizeList.resize(sampleFileSize);
    for(int i = 0; i < sumList.size(); i++){
        for(int j = 0; j < sampleFileSize; j++){
            sampleFileSizeList[j] += sumList[i][j];
        }
    }
    long long ttPair = 0;
    long long mx = 0, mi = 0xfffffff;
    for(int i = 0; i < sampleFileSizeList.size(); i++){
        ttPair += sampleFileSizeList[i];
        mx = max(mx, sampleFileSizeList[i]);
        mi = min(mi, sampleFileSizeList[i]);
    }
    tt3.fin();            
    printf("sample result: %lld %lld %d\n", mx, mi, sampleFileSize);
    printf("total pair is %lld\n", ttPair);                                                                                                                       
    printf("cost time is %f\n", tt3.getTime());
    printf("===========all-to-all gather process by CPU==========\n");
    timer tt4;
    tt4.start();
    vector<vector<long long>> prefixSizeList;
    prefixSizeList.assign(hashFile.size(), vector<long long>(sampleFileSize, 0));
    for(int i = 1; i < hashFile.size(); i++){
        for(int j = 0; j < sampleFileSize; j++){
            prefixSizeList[i][j] += prefixSizeList[i - 1][j] + sumList[i - 1][j];
        }
    }
    vector<node> sortFileList;
    for(int i = 0; i < sampleFileSizeList.size(); i++){
        unsigned long long * tmpFile = new unsigned long long[sampleFileSizeList[i]];
        sortFileList.push_back(node(tmpFile, sampleFileSizeList[i]));
    } 
    thread threadsS1[hashFile.size()];
    for(int i = 0; i < hashFile.size(); i++){
        threadsS1[i] = thread(alltoallGather, hashFile[i], ref(prefixSizeList[i]), ref(sortFileList), ref(sampleList));
    }
    for(int i = 0; i < hashFile.size(); i++){
        threadsS1[i].join();
    }
    vector<vector<long long>> checkPrefixSizeList;
    checkPrefixSizeList.assign(hashFile.size(), vector<long long>(sampleFileSize, 0));
    for(int i = 0; i < hashFile.size(); i++){
        for(int j = 0; j < sampleFileSize; j++){
            if (i == 0) checkPrefixSizeList[i][j] = sumList[i][j];
            else checkPrefixSizeList[i][j] += checkPrefixSizeList[i - 1][j] + sumList[i][j];
        }
    }
    for(int i = 0; i < hashFile.size(); i++){
        for(int j = 0; j < sampleFileSize; j++){
            assert(prefixSizeList[i][j] == checkPrefixSizeList[i][j]);
        }
    }
    //     for(int i = 0; i < 10; i++){
    //     printf("%lld\n", sortFileList[0].file[i]);
    // }
    tt4.fin();
    printf("cost time is %f\n", tt4.getTime());
    fflush(stdout);
    printf("===========part sort process by GPU===========\n");
    timer t2;
    t2.start();
    int n = 2;
    thread threads2[n];
    for(int i = 0; i < n; i++){
        threads2[i] = thread(sortPhase, i, n, ref(sortFileList));
    }
    for(int i = 0; i < n; i++){
        threads2[i].join();
    }
    // for(int i = 0; i < 10; i++){
    //     printf("%lld\n", sortFileList[0].file[i]);
    // }
    t2.fin();
    printf("cost time is %f\n", t2.getTime());
    printf("==========reduce process by CPU=======\n");
    timer t3;
    t3.start();
    vector<long long> ansList(sortFileList.size());
    thread threads3[sortFileList.size()];
    for(int i = 0; i < sortFileList.size(); i++){
        threads3[i] = thread(reduceCount, sortFileList[i], ref(ansList[i]));
    }
    for(int i = 0; i < sortFileList.size(); i++){
        threads3[i].join();
    } 
    long long ans = 0;
    for(int i = 0; i < sortFileList.size(); i++){
        //printf("%lld\n", ansList[i]);
        ans += ansList[i];
    }
    t3.fin();
    printf("cost time is %f\n", t3.getTime());
    printf("=============Done=============\n");
    tall.fin();
    printf("total cost time is %f\n", tall.getTime());
    printf("ans is %lld\n", ans);
}