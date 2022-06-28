#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H
//#program once
#include "ccGLWindow.h"
//qCC_db
#include <ccGenericMesh.h>
#include <ccHObjectCaster.h>
#include <ccPointCloud.h>

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

class ccGLWindow;

class MyGLWidget: public ccGLWindow
{
    Q_OBJECT

private:
    QString m_modelname;

protected:
    void dragEnterEvent(QDragEnterEvent*event) Q_DECL_OVERRIDE;//拖动进入事件
    void dropEvent(QDropEvent*event) Q_DECL_OVERRIDE;

signals:
    void loadFile(QString filename);

public:
    MyGLWidget(QWidget* parent = 0);
    ~MyGLWidget();
    void ShowModel(ccHObject *entity);
    void SetModelName(QString str)
    {
        m_modelname = str;
    }
    QString GetModelName()
    {
        return m_modelname;
    }
};

#endif // MYGLWIDGET_H
