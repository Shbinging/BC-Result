import os

testGraphList = ["condmat", "marvel", "github", "amazon", "twitter", "flicker", "reuters", "livejournal"]

for i in range(0, len(testGraphList)):
    os.system(f"sh testBFC-EM.sh {testGraphList[i]}")