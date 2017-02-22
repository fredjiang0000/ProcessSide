#-------------------------------------------------
#
# Project created by QtCreator 2015-04-21T17:37:10
#
#-------------------------------------------------

QT       += core gui
QT += network
CONFIG   += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProcessSide
TEMPLATE = app


SOURCES +=\
    widget.cpp \
    enlarge.cpp \
    my_qlabel.cpp \
    ImgProcess.cpp \
    Main.cpp \
    CaptureThread.cpp \
    ProcessThread.cpp \
    mainwindow.cpp \
    connection.cpp \
    TruckRecognize.cpp \
    bgsubmog.cpp \
    celldetecter.cpp \
    cellphone_detection.cpp \
    Config.cpp \
    detection.cpp \
    display.cpp \
    imageprocess.cpp \
    MultipleKernelTracking.cpp \
    myclass.cpp \
    objdata.cpp \
    objtrack.cpp \
    skeleton.cpp \
    skindetecter.cpp \
    stdafx.cpp \
    type.cpp \
    VlcThread.cpp \
    smtp.cpp \
    check_thread.cpp \
    outputLog.cpp

HEADERS  += \
    widget.h \
    enlarge.h \
    my_qlabel.h \
    ConcurrentQueue.h \
    ImgProcess.h \
    CaptureThread.h \
    ProcessThread.h \
    mainwindow.h \
    connection.h \
    TruckRecognize.h \
    bgsubmog.h \
    cellphone_detection.h \
    Config.h \
    detection.h \
    display.h \
    header.h \
    MultipleKernelTracking.h \
    Myclass.h \
    objdata.h \
    objtrack.h \
    skeleton.h \
    stdafx.h \
    targetver.h \
    check_thread.h \
    warningmessage.h \
    VlcThread.h \
    smtp.h \
    outputLog.h

FORMS    += \
    mainwindow.ui \
    enlarge.ui \
    connection.ui

INCLUDEPATH += /usr/include/c++/{gcc_version}/
LIBS += -lWs2_32 #For Win Socket

################ Include OpenCV headers and Library ###################
INCLUDEPATH += C:\opencv2.4.9\install\include
LIBS += -LC:\opencv2.4.9\lib
LIBS +=   -lopencv_core249  -lopencv_calib3d249 -lopencv_contrib249  \
          -lopencv_highgui249 -lopencv_features2d249 -lopencv_gpu249 \
          -lopencv_imgproc249 -lopencv_objdetect249 -lopencv_ocl249 \
          -lopencv_video249
LIBS +=   C:\opencv2.4.9\openCV\bin\opencv_ffmpeg249.dll
#######################################################################

################ Include VLC headers and Library ######################
INCLUDEPATH += C:\vlc-2.2.1\sdk\include
LIBS += -LC:\vlc-2.2.1\sdk\lib
LIBS += -lvlccore -llibvlc
#######################################################################

