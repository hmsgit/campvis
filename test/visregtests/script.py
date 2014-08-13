# ================================================================================================
# 
# This file is part of the CAMPVis Software Framework.
# 
# If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
#      Christian Schulte zu Berge <christian.szb@in.tum.de>
#      Chair for Computer Aided Medical Procedures
#      Technische Universitaet Muenchen
#      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
# 
# For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
# 
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
# except in compliance with the License. You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software distributed under the 
# License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
# either express or implied. See the License for the specific language governing permissions 
# and limitations under the License.
# 
# ================================================================================================

__author__="Mahmud"
__date__ ="$Jul 10, 2014 1:58:04 AM$"

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import os

from skimage import data, img_as_float, io
from skimage.measure import structural_similarity as ssim

refDir = 'reference/';
testDir = 'testruns/';
resultDir = 'results/';
if (not os.path.exists(resultDir)) :
        os.mkdir(resultDir)

casesDir = os.listdir(refDir);
prevRunDirs = os.listdir(testDir);
prevResDirs = os.listdir(resultDir);

# Create a new directory to store result
curRun = 0;
for i in range(len(prevResDirs)) :
    prevRunDirs[i] = int(prevRunDirs[i][4:])
    prevResDirs[i] = int(prevResDirs[i][4:])
    if (prevRunDirs[i] == prevResDirs[i]) :
        curRun += 1;
        continue;
    break;

newTestDirs = prevRunDirs[curRun:];

for test in newTestDirs :
    curTestDir = testDir + str(prevRunDirs[curRun]) + "/";
    resultSaveDir = resultDir + str(prevRunDirs[curRun]) + "/";
    curRun += 1;
    os.mkdir(resultSaveDir)
    
    for case in casesDir :
        refCaseDir = refDir + case + "/";
        testCaseDir = curTestDir + case + "/";
        resCaseDir = resultSaveDir + case + "/";
        # if no corresponding test directory - continue
        if (not os.path.exists(testCaseDir)) :
            continue;
        os.mkdir(resCaseDir)
        
        files = os.listdir(refCaseDir)
        for file in files :
            refFilePath = refCaseDir + file;
            testFilePath = testCaseDir + file;
            resFilePath = resCaseDir + file;
            # Check existence of test file       
            if (not os.path.isfile(testFilePath)) :
                continue;
            
            if (refFilePath[-4:] != ".jpg" and refFilePath[-4:] != ".png" and refFilePath[-4:] != ".tif") :
                continue;

            ref = io.imread(refFilePath);
            test = io.imread(testFilePath);
            # Check dimension of the file before finding difference
            if (ref.shape == test.shape) :
                test = ref-test;
            else :
                test = ref;
            io.imsave(resFilePath, test);
            
            #(x, y, z) = test.shape            
            #plt.figure(figsize=(4, 4))
            #plt.imshow(test)#, cmap='gray', interpolation='nearest')
            #plt.axis('off')
            #plt.tight_layout()
            #plt.show()
            
print ""

