TARGET = PointCloudViewer

QT       += core concurrent widgets gui opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CommeTools.cpp \
    MyGLWidget.cpp \
    PointPickerDlg.cpp \
    ccColorGradientDlg.cpp \
    ccSORFilterDlg.cpp \
    main.cpp \
    PointCloudViewer.cpp

HEADERS += \
    CommeTools.h \
    MyGLWidget.h \
    PointCloudViewer.h \
    PointPickerDlg.h \
    ccColorGradientDlg.h \
    ccSORFilterDlg.h

FORMS += \
    PointCloudViewer.ui \
    PointPickerDlg.ui \
    colorGradientDlg.ui \
    sorFilterDlg.ui


INCLUDEPATH += ccViewer/include\CC_FBO_LIB
INCLUDEPATH += ccViewer/include\CCAppCommon
INCLUDEPATH += ccViewer/include\CCCoreLib
INCLUDEPATH += ccViewer/include\CCPluginAPI
INCLUDEPATH += ccViewer/include\QCC_DB_LIB
INCLUDEPATH += ccViewer/include\QCC_GL_LIB
INCLUDEPATH += ccViewer/include\QCC_IO_LIB
INCLUDEPATH += ccViewer/include\CCPluginStub
INCLUDEPATH += ccViewer/include\qEDL
INCLUDEPATH += ccViewer/include\qSSAO

LIBS += ccViewer\lib\CC_FBO_LIB.lib
LIBS += ccViewer\lib\CCAppCommon.lib
LIBS += ccViewer\lib\CCCoreLib.lib
LIBS += ccViewer\lib\libgmp-10.lib
LIBS += ccViewer\lib\CCPluginAPI.lib
LIBS += ccViewer\lib\QCC_DB_LIB.lib
LIBS += ccViewer\lib\QCC_GL_LIB.lib
LIBS += ccViewer\lib\QCC_IO_LIB.lib
LIBS += ccViewer\lib\CCPluginStub.lib
LIBS += ccViewer\lib\QEDL_GL_PLUGIN.lib
LIBS += ccViewer\lib\QSSAO_GL_PLUGIN.lib

RESOURCES += IconsSet.qrc
RC_ICONS = gui-icon.ico


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

