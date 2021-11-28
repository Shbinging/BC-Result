# -*- coding: utf-8 -*-

import os

def file_name(file_dir):
    lst = []
    for root, dirs, files in os.walk(file_dir):
        if ("sorted" in root):
            lst.append(root)
    return lst

pathList = file_name("/home/shbing/dataUse/datasets/bipartite/")

# for path in pathList:
#     oriPath = path
#     newPath = path.replace("dataUse", "datasetsNew")
#     #print(oriPath, newPath)
#     name = newPath.split("/")[-2]
#     out = os.system(f"./butterfly.bin {newPath} run -1 {name} 32")
# for path in pathList:
#     oriPath = path
#     newPath = path.replace("dataUse", "datasetsNew")
#     #print(oriPath, newPath)
#     print(newPath)
#     #a = os.system(f"./try {newPath}")
#     #print(a)
    