import os
import sys
import getopt
import string

dirsToIgnore = ['.svn', 'debug', 'release', 'Debug', 'Release', 'ext', 'CL']

legal_notice_begin = """// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
"""
legal_notice = """// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// This library is free software; you can redistribute it and/or modify it under the terms of the 
// GNU Lesser General Public License version 3 as published by the Free Software Foundation.
// 
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License in the file 
// "LICENSE.txt" along with this library. If not, see <http://www.gnu.org/licenses/>.
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
            if (file.endswith('.cpp') or file.endswith('.h')):
                updateLegalHeaderInFile(root + '\\' + file)

if (len(sys.argv) > 1):
    for file in sys.argv[1:]:
        if (os.path.exists(file)):
            walkDirectory(file)
else:
    walkDirectory(os.getcwd())
