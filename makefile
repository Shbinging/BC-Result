exe=butterfly.bin
N=1
cc= "$(shell which g++)"
#commflags=-lcudart -L"$(shell dirname $(cucc))"/../lib64  -O3 -W -Wall -Wno-unused-function -Wno-unused-parameter

.SILENT: cc
.SILENT: %.o


objs	= $(patsubst %.cpp,%.o,$(wildcard *.cpp) $(wildcard ./BFC-VP++/*.cpp)) 
			


deps	= 	$(wildcard ./*.hpp) \
			$(wildcard ./*.h) 

# foldobjs = 	$(patsubst %.cu,%.o,$(wildcard countingAlgorithm/*.cu)) 


$(exe):$(objs)
	$(cc) -fcilkplus $(objs) -o $(exe)

%.o:%.cpp 
	$(cc) -c -fcilkplus $< -o $@ 


# rm -rf *.o 
# ./butterfly.bin ../dataset/bipartite/wiki-it/ 0

clean:
	rm -rf *.o countingAlgorithm/*.o $(exe)

test:
	./butterfly.bin

check:
	./butterfly.bin  ~/datasetsNew/datasets/bipartite/condmat/sorted check 0 -1