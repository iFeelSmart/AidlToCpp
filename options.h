/* iFeelSmart
 *
 * Copyright © 2012-2013, iFeelSmart.
 *
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation and
 * appearing in the file LICENSE.LGPL included in the packaging of this
 * file. Please review the following information to ensure the GNU Lesser
 * General Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * File: options.h
 *
 */

#ifndef DEVICE_TOOLS_AIDL_H
#define DEVICE_TOOLS_AIDL_H

#include <string.h>
#include <string>
#include <vector>

using namespace std;

enum {
    COMPILE_AIDL,
    PREPROCESS_AIDL
};

// This struct is the parsed version of the command line options
struct Options
{
    int task;
    bool failOnParcelable;
    vector<string> importPaths;
    vector<string> preprocessedFiles;
    string inputFileName;
    string outputFileName;
    string outputBaseFolder;
    string depFileName;
    bool autoDepFile;

    vector<string> filesToPreprocess;
};

// takes the inputs from the command line and fills in the Options struct
// Returns 0 on success, and nonzero on failure.
// It also prints the usage statement on failure.
int parse_options(int argc, const char* const* argv, Options *options);

#endif // DEVICE_TOOLS_AIDL_H
