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
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//  * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific prior written
//    permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
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
