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
 * File: search_path.h
 *
 */

#ifndef DEVICE_TOOLS_AIDL_SEARCH_PATH_H
#define DEVICE_TOOLS_AIDL_SEARCH_PATH_H

#include <stdio.h>

#if __cplusplus
#include <vector>
#include <string>
using namespace std;
extern "C" {
#endif

// returns a FILE* and the char* for the file that it found
// given is the class name we're looking for
char* find_import_file(const char* given);

#if __cplusplus
}; // extern "C"
void set_import_paths(const vector<string>& importPaths);
#endif

#endif // DEVICE_TOOLS_AIDL_SEARCH_PATH_H

