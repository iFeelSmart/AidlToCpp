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
 * File: aidl_language.cpp
 *
 */

#include "aidl_language.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_MS_C_RUNTIME
int isatty(int  fd)
{
    return (fd == 0);
}
#endif

#if 0
ParserCallbacks k_parserCallbacks = {
    NULL
};
#endif

ParserCallbacks* g_callbacks = NULL; // &k_parserCallbacks;

