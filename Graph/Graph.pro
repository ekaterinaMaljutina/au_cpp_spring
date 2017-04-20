TEMPLATE = app
CONFIG += console c++14 -O0
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    filtered_graph.h \
    iterator.h \
    graph.h \
    path_finding.h
