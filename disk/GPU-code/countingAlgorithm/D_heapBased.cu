#include <iostream>
#include <stdio.h>
#include <cub/cub.cuh>
#include <cub/util_type.cuh>
using namespace std;

struct heap
{
    int element;
    int* now;
    int* end;
};

__device__ int* binarySearch_2(int* a, int* b, int x)
{
    while (a<b)
    {
        int* mid=a+((b-a)/2);
        if (*mid<=x) a=mid+1; else b=mid;
    }
    return a;
}
//merge nb_list to 2-hop nb_list
__device__ 
void heapBased_Sorted2Hop(int vertex, long long *beginPos, int *edgeList, int num_frT , int num_srT, int *Sorted_NB, int start_address)
{
    double startTime,storeTime;
    struct heap H[11];
    H[0].element=-1;
    int k=1;
    int frT_no=threadIdx.x%(num_frT+num_srT);
    long long num_NB=beginPos[vertex+1]-beginPos[vertex];
    // first creat the heap 
    for (int i=0; i<10&&beginPos[vertex]+frT_no*10+i<beginPos[vertex+1]; i++)
    {
        int oneHopNeighborID=beginPos[vertex+1]-frT_no*10-i-1;
        int oneHopNeighbor=edgeList[oneHopNeighborID];
        if (oneHopNeighbor<=vertex) break;
        // get the first neighbor in each oneHopNeighbor's neighbor list 
        H[k].end=edgeList+beginPos[oneHopNeighbor+1];
        H[k].now=binarySearch_2(edgeList+beginPos[oneHopNeighbor], H[k].end, vertex);
        if (H[k].now>=H[k].end)  continue;
        
      //  printf("vertex %d : there is %d 2-hop neighbor of neighbor %d need to be pushed\n",vertex,H[k].end-H[k].now,oneHopNeighbor);
        H[k].element=*H[k].now;

        // update the heap
        int p=k;
        while (H[p].element<H[p/2].element)
        {
            struct heap t;
            t=H[p];
            H[p]=H[p/2];
            p/=2;
            H[p]=t;
        }
        k++;
    }
    // second pop the top element in heap and add new element from its corresponding neighbor list
    long long part_mem_address=start_address+frT_no;
    for(k--;k>=1;)
    {
        // update the count of butterflies
        int nowElement=H[1].element;
        //store nowElement into memory
        // startTime=wtime();
        Sorted_NB[part_mem_address]=nowElement;
        // storeTime=wtime()-startTime;
        part_mem_address+=num_frT;
        // add the next element into heap
        H[1].now++;
        if (H[1].now>=H[1].end) 
        {
            struct heap t;
            t=H[1];
            H[1]=H[k];
            H[k]=t;
            k--;
        }
        else H[1].element=*(H[1].now);

        // update the heap
        int p=1;
        for(;;)
        {
            if (p*2>k) break; 
            p*=2;
            if (p+1<=k&&H[p].element>H[p+1].element) p++;
            if (H[p].element<H[p/2].element)
            {
                struct heap t;
                t=H[p];
                H[p]=H[p/2];
                H[p/2]=t;
            }
            else
            break;
        }
        

    }

}
//get heapBased VertexCounting
__device__ 
void heapBased_Counting( int vertex, long long *beginPos,int *edgelist,int *Sorted_NB,int num_frT, int num_srT, int start_address, unsigned long long *count)
{
    struct heap H[11];
    H[0].element=-1;
    int k=1;
    double startTime,loadTime;
    for (int oneHopNeighborID=0; oneHopNeighborID<num_frT&&oneHopNeighborID<beginPos[vertex+1]-beginPos[vertex]; oneHopNeighborID+=10)
    {
        int list_length=0;
        for(int i=0;i<10&&oneHopNeighborID+i<beginPos[vertex+1]-beginPos[vertex];i++)
        {
          //Neighbor  is the no. in the neighbor list (reverse)
          //NB_pos is the address of this neighbor in the edgelist
          //NB is the ID of this neighbor
          //beginPos[NB+1]-beginPos[NB] : the number of neighbors of this neighbor == the number of two hop neighbor
          int Neighbor=oneHopNeighborID+i;
          int NB_pos=beginPos[vertex+1]-Neighbor;
          int NB=edgelist[NB_pos];
          if(NB>vertex)
          {
              int* tmp=(binarySearch_2(edgelist+beginPos[NB], edgelist+beginPos[NB+1], vertex));
              list_length+=edgelist+beginPos[NB+1]-tmp;
          }
        }
        // startTime=wtime();
        int* NeighborBegin=Sorted_NB+start_address+k-1;
        H[k].end=NeighborBegin+(list_length)*num_frT;
        H[k].now=NeighborBegin;
        if (H[k].now>=H[k].end)  continue;
        H[k].element=*H[k].now;
        // loadTime=startTime-wtime();
        int p=k;
        while (H[p].element<H[p/2].element)
        {
            struct heap t;
            t=H[p];
            H[p]=H[p/2];
            p/=2;
            H[p]=t;
        }
        k++;
    }
    int previousElement=-1,cc=1;
    // second pop the top element in heap and add new element from its corresponding neighbor list
    for(k--;k>=1;)
    {
        // update the count of butterflies
        int nowElement=H[1].element;
        if (nowElement==previousElement) {cc++;}
        else 
        { 
          *count+=cc*(cc-1)/2; 
          cc=1; previousElement=nowElement;
        }
        // add the next element into heap
        H[1].now+=num_frT;
        // startTime=wtime();
        if (H[1].now>=H[1].end) 
        {
            struct heap t;
            t=H[1];
            H[1]=H[k];
            H[k]=t;
            k--;
        }
        else H[1].element=*(H[1].now);
        // loadTime+=startTime-wtime();
        // update the heap
        int p=1;
        for(;;)
        {
            if (p*2>k) break;
            p*=2;
            if (p+1<=k&&H[p].element>H[p+1].element) p++;
            if (H[p].element<H[p/2].element)
            {
                struct heap t;
                t=H[p];
                H[p]=H[p/2];
                H[p/2]=t;
            }
            else
            break;
        }
    }
    //*load_time+=loadTime;
    *count+=cc*(cc-1)/2; // the last series of element need to be added
}
__global__ 
void D_heapBasedButterflyCounting(long long *beginPos, int *edgeList, int *Sorted_NB, int total_size,int num_frT, int num_srT, int uCount, int vCount, unsigned long long* globalCount, int* perVertexCount, int* hashTable, int startVertex, int endVertex)
{
    __shared__ unsigned long long sharedCount;
    __shared__ int a[100];
    int * available=a;
    //__shared__ 
    if (threadIdx.x==0) sharedCount=0;
    unsigned long long count=0;
    //define the number of threads executing each round of merging: num_frT and num_srT
    int num_VT=num_srT+num_frT;
    int part_num=0;
    //the num of groups of thread in a grid, accessing same part of global memory
    int vec_block=blockDim.x/num_VT; //a block can be divided into vec_block groups
    int index_global=blockIdx.x*vec_block+threadIdx.x/num_VT;//data of this group should be loaded into (index_global) part of global memory
    int group_size=total_size/(gridDim.x*vec_block);//every group can access (group_size) of memory
    int part_mem_size=group_size/num_srT;
    int start_address_group=group_size*index_global;
    //the index of this group in the block
    int index=threadIdx.x/num_VT;
    if(threadIdx.x<100)
    {
        available[threadIdx.x]=0;
    }
    __syncthreads();
       // printf("thread No.%d   ready----vector %d\n",threadIdx.x,vertex);
        //Sorted_NB: memory stored intermediate result of nb_list
        //part_mem_size: the size of a partition of memory assigned to one vertex for its nb_list
        //num of memory partition is num_srT，the num of thread for second round merging
        //available: if the part of memory is available with data ready
        //part_num: the part of memory assigned for current vertex
        // perVertexCount[vertex]=count;
        //first round of merging 
        if(threadIdx.x%num_VT<num_frT)
        {
            for (int vertex=startVertex+blockIdx.x*blockDim.x/num_VT+threadIdx.x/num_VT; vertex<endVertex; vertex+=gridDim.x*blockDim.x/num_VT)
            {
                part_num=part_num%(num_srT);
                while(1)
                {
                    if(available[part_num+index*num_srT]==0)
                    {
                        int start_address=start_address_group+part_num*part_mem_size;
                        //set memory available=0: in processing
                        //heapBased_Sorted2Hop(vertex, beginPos, edgeList, num_frT,num_srT, Sorted_NB, part_mem_size,part_num++,&store_time);
                        heapBased_Sorted2Hop(vertex, beginPos, edgeList, num_frT,num_srT, Sorted_NB, start_address);
                        atomicAdd(available+index*num_srT+part_num,1);
                        break;
                        //set memory available=num_frT: all new data ready
                    }
                    
                }
            }
            
            
        }
        //m thread of second round of heap_based ,Second round  merge 2-hop NB-list and count Butterfly
        if(threadIdx.x%(num_frT+num_srT)>=num_frT)
        { 
            for (int vertex=startVertex+blockIdx.x*blockDim.x/num_VT+threadIdx.x/num_VT+gridDim.x*blockDim.x*(threadIdx.x%(num_frT+num_srT)-num_frT)/num_VT; vertex<endVertex; vertex+=gridDim.x*blockDim.x*num_srT/num_VT)
            {
              while(1)
              {
                  if(available[index*num_srT+threadIdx.x%(num_frT+num_srT)-num_frT]==num_frT)
                  {
                      int start_address=start_address_group+(threadIdx.x%(num_frT+num_srT)-num_frT)*part_mem_size;
                      heapBased_Counting(vertex,beginPos,edgeList,Sorted_NB,num_frT, num_srT,start_address,&count);
                     atomicMin(available+index*num_srT+threadIdx.x%(num_frT+num_srT)-num_frT,0);
                      
                      break;
                  }
              }
            }
        }    
    
    atomicAdd(&sharedCount,count);
    __syncthreads();
    if (threadIdx.x==0) atomicAdd(globalCount,sharedCount);
  //  atomicAdd(global_store_time,store_time);
  //  atomicAdd(global_load_time,load_time);
}
