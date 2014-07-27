import os
import sys
import getopt
import string

dirsToIgnore = ['.svn', 'debug', 'release', 'Debug', 'Release', 'ext', '.git', '.gitignore']

legal_notice_begin = """// ================================================================================================
// 
"""
legal_notice = """// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
"""
legal_notice_end = """// 
// ================================================================================================"""

def updateLegalHeaderInFile(file):
    # read file
    f = open(file)
    data = f.read()
    f.close()

    # look for existing legal notice
    start = data.find(legal_notice_begin)
    end = data.find(legal_notice_end)
    newdata = ''
    
    if (start == -1 or end == -1):
        # no legal notice found, add one
        newdata = legal_notice_begin + legal_notice + legal_notice_end + '\n\n' + data

    else:
        # legal notice found, check whether a update is necessary:
        start2 = start + len(legal_notice_begin)
        oldNotice = data[start2:end]
        if (oldNotice != legal_notice):
            # update necessary
            newdata = data[:start2] + legal_notice + data[end:]
        else:
            # no update necessary
            return

    # save changes
    f = open(file, "w")
    f.write(newdata)
    f.close()
    print('Updated "' + file + '"')
    
def walkDirectory(directory):
    for root, dirs, files in os.walk(directory):
        for dir in dirsToIgnore:
            try:
                dirs.remove(dir)
            except ValueError:
                pass
        for file in files:
            if (file.endswith('.cpp') or file.endswith('.h') or file.endswith('.vert') or file.endswith('.frag')):
                updateLegalHeaderInFile(root + '\\' + file)

if (len(sys.argv) > 1):
    for file in sys.argv[1:]:
        if (os.path.exists(file)):
            walkDirectory(file)
else:
    walkDirectory(os.getcwd())
