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
 * File: generate_java.cpp
 *
 */
 
#include "generate_java.h"
#include "Type.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =================================================
VariableFactory::VariableFactory(const string& base)
    :m_base(base),
     m_index(0)
{
}

Variable*
VariableFactory::Get(Type* type)
{
    char name[100];
    sprintf(name, "%s%d", m_base.c_str(), m_index);
    m_index++;
    Variable* v = new Variable(type, name);
    m_vars.push_back(v);
    return v;
}

Variable*
VariableFactory::Get(int index)
{
    return m_vars[index];
}

// =================================================
string
gather_comments(extra_text_type* extra)
{
    string s;
    while (extra) {
        if (extra->which == SHORT_COMMENT) {
            s += extra->data;
        }
        else if (extra->which == LONG_COMMENT) {
            s += "/*";
            s += extra->data;
            s += "*/";
        }
        extra = extra->next;
    }
    return s;
}

string
append(const char* a, const char* b)
{
    string s = a;
    s += b;
    return s;
}

// =================================================
int
generate_java(const string& filename, const string& originalSrc,
                interface_type* iface)
{
    Class** cl;

    if (iface->document_item.item_type == INTERFACE_TYPE_BINDER) {
        cl = generate_binder_interface_class(iface);
    }
    /*else if (iface->document_item.item_type == INTERFACE_TYPE_RPC) {
        cl = generate_rpc_interface_class(iface);
    }*/

    Document* document = new Document;
        document->comment = "";
        if (iface->package) document->package = iface->package;
        document->originalSrc = originalSrc;
    int i = 0;
    while (cl[i] != 0) {
        document->classes.push_back(cl[i]);
        i++;
    }

//    printf("outputting... filename=%s\n", filename.c_str());
    FILE* toC;
    if (filename == "-") {
        toC = stdout;
    } else {
       /* open file in binary mode to ensure that the tool produces the
        * same output on all platforms !!
        */
        toC = fopen((filename + ".cpp").c_str(), "wb");
        if (toC == NULL) {
            fprintf(stderr, "unable to open %s for write\n", filename.c_str());
            return 1;
        }
    }

    FILE* toH;
    if (filename == "-") {
        toH = stdout;
    } else {
       /* open file in binary mode to ensure that the tool produces the
        * same output on all platforms !!
        */
        toH = fopen((filename + ".h").c_str(), "wb");
        if (toH == NULL) {
            fprintf(stderr, "unable to open %s for write\n", filename.c_str());
            return 1;
        }
    }

    document->Write(toC, toH);

    fclose(toH);
    fclose(toC);
    return 0;
}

