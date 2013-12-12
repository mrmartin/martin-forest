TEMPLATE = app
# CONFIG += console
CONFIG -= qt

#CONFIG += link_pkgconfig
# PKGCONFIG += opencv

SOURCES += train_multiclass_forest.cpp

QMAKE_CXXFLAGS += -std=gnu++11

#HEADERS += \
#    main.h 
