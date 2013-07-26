#
# Copyright (c) 2011-2013 Research In Motion Limited.
# Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

QT       += core gui declarative

TARGET = ColoredItems

TEMPLATE = app


SOURCES += main.cpp \
    mydataobject.cpp \
    mydatamodel.cpp

OTHER_FILES += \
    main.qml \
    bar-descriptor.xml

RESOURCES += \
    colored_items.qrc

HEADERS += \
    mydataobject.h \
    mydatamodel.h
