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

import os
import os.path
import xml.etree.ElementTree as et
import numpy as np
import shutil as fio

from skimage import io, color
from xml.etree import ElementTree
from xml.dom import minidom
from skimage.measure import structural_similarity as ssim

# Return a pretty-printed XML string for the Element
def prettify(elem):
    rough_string = ElementTree.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")

refDir = 'reference/';
testDir = 'testruns/';
resultDir = 'results/';
failedDir = 'failed/';
if (not os.path.exists(resultDir)) :
        os.mkdir(resultDir)

casesDir = os.listdir(refDir);

# Find or create results.xml file created by test-campvis
xmlFile = "result.xml";
if (os.path.isfile(xmlFile)) :
    tree = et.parse(xmlFile);
    root = tree.getroot();
else :
    # XML
    root = et.Element("testsuites", {"tests":"0", "failures":"0", "disabled":"0", 
    "errors":"0", "timestamp":"2014-08-24T01:35:42", "time":"0", "name":"AllTests"}); 
    tree = et.ElementTree(root);
    
curTestDir = testDir;
resultSaveDir = resultDir;
for case in casesDir :
    refCaseDir = refDir + case + "/";
    testCaseDir = curTestDir + case + "/";
    resCaseDir = resultSaveDir + case + "/";
    # if no corresponding test directory - continue
    if (not os.path.exists(testCaseDir)) :
        continue;
    if (not os.path.exists(resCaseDir)) :
        os.mkdir(resCaseDir)

    files = os.listdir(refCaseDir)
    if (len(files) != 0) :
        # XML
        suite = et.SubElement(root, "testsuite", {"name":refCaseDir, "tests":"0", 
        "failures":"0", "disabled":"0", "errors":"0", "time":"0"});

    i = 0;
    for file in files :
        refFilePath = refCaseDir + file;
        testFilePath = testCaseDir + file;
        resFilePath = resCaseDir + file;
        alphaFilePath = resFilePath[:-4]+"_aplha"+resFilePath[-4:];
        # Check existence of test file       
        if (not os.path.isfile(testFilePath)) :
            continue;

        if (refFilePath[-4:] != ".jpg" and refFilePath[-4:] != ".png" 
        and refFilePath[-4:] != ".tif") :
            continue;

        ref = io.imread(refFilePath);
        testim = io.imread(testFilePath);
        # Check dimension of the file before finding difference
        if (ref.shape == testim.shape) :
            test = ref-testim;
        else :
            print "i'm in else! HELP me!"
            test = ref;
        # Store fully opaque image    
        rgb = test[:, :, : 3];
        alpha = test[:, :, 3:];
        opaque = [[[255]*alpha.shape[2]] * alpha.shape[1]] * alpha.shape[0]
        #io.imsave(resFilePath, rgb);
        io.imsave(resFilePath, np.concatenate((rgb, opaque), axis=2));
        io.imsave(alphaFilePath, alpha);
        # Calculate MSE and SSIM
        mse = np.linalg.norm(test);
        reff = color.rgb2gray(ref);
        testf = color.rgb2gray(test);
        ssimval = ssim(reff, testf);

        # XML
        case = et.SubElement(suite, "testcase", {"name":file, "status":"run", 
        "time":"0", "classname":refCaseDir});
        suite.set("tests", str(int(suite.get("tests"))+1));
        root.set("tests", str(int(root.get("tests"))+1));

        if (np.sum(test) != 0) :
            # Prepare and write messages to show in stacktrace
            failure = et.SubElement(case, "failure", {"message":"", "type":""});
            failure.set("message", "Image difference is not 0");
            alphamsg = "differ in both RGB and aplha channels";
            if (sum(1 for x in rgb if x.any() > 0) == 0):
                alphamsg = "differ in transparency level only";
            elif (sum(1 for x in alpha if x.any() > 0) == 0) :
                alphamsg = "differ in RGB channels only";

            failure.text = "Reference and test images differ in " \
            + str(sum(1 for x in test if x.any() > 0)) + " pixel/s\n" \
            + "and images " + alphamsg \
            + "\nMSE: " + str(mse) + " SSIM: " + str(ssimval);

            suite.set("failures", str(int(suite.get("failures")) + 1));
            root.set("failures", str(int(root.get("failures")) + 1));
            
            # Copy artifacts to failed directory
            if (not os.path.exists(failedDir + refCaseDir)) :
                os.makedirs(failedDir + refCaseDir);
            fio.copy(refFilePath, failedDir + refFilePath);
            
            if (not os.path.exists(failedDir + testCaseDir)) :
                os.makedirs(failedDir + testCaseDir);
            fio.copy(testFilePath, failedDir + testFilePath);
            
            if (not os.path.exists(failedDir + resCaseDir)) :
                os.makedirs(failedDir + resCaseDir);
            fio.copy(resFilePath, failedDir + resFilePath);
            fio.copy(alphaFilePath, failedDir + alphaFilePath);
        #break;
tree.write(xmlFile);
