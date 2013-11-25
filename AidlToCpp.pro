# iFeelSmart
#
# Copyright © 2012-2013, iFeelSmart.
#
#
# GNU Lesser General Public License Usage
# This file may be used under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation and
# appearing in the file LICENSE.LGPL included in the packaging of this
# file. Please review the following information to ensure the GNU Lesser
# General Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
#
# File: AidlToCpp.pro
#

QT       += core
QT       -= gui

TARGET    = AidlToCpp

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CFLAGS += -std=c99

SHARED_LIBS += -lfl

SOURCES += search_path.cpp
SOURCES += options.cpp
SOURCES += generate_java_rpc.cpp
SOURCES += generate_java_binder.cpp
SOURCES += generate_java.cpp
SOURCES += aidl_language.cpp
SOURCES += aidl.cpp
SOURCES += Type.cpp
SOURCES += AST.cpp

HEADERS += search_path.h
HEADERS += options.h
HEADERS += generate_java.h
HEADERS += aidl_language.h
HEADERS += Type.h
HEADERS += AST.h

FLEXSOURCES += aidl_language_l.l

BISONSOURCES += aidl_language_y.y

OTHER_FILES += $$FLEXSOURCES
OTHER_FILES += $$BISONSOURCES

flex.commands = flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = lex.yy.c
flex.variable_out = SOURCES
flex.depends = y.tab.h
flex.name = flex
QMAKE_EXTRA_COMPILERS += flex

bison.commands = bison -d -t -y -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = y.tab.c
bison.variable_out = SOURCES
bison.name = bison
QMAKE_EXTRA_COMPILERS += bison

bisonheader.commands = @true
bisonheader.input = BISONSOURCES
bisonheader.output = y.tab.h
bisonheader.variable_out = HEADERS
bisonheader.name = bison header
bisonheader.depends = y.tab.c
QMAKE_EXTRA_COMPILERS += bisonheader
