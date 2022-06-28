#include "MyGLWidget.h"
//#include "qopenglfunctions_2_1.h"

MyGLWidget::MyGLWidget(QWidget* parent)
{

}

MyGLWidget::~MyGLWidget()
{

}

void MyGLWidget::ShowModel(ccHObject *entity)
{
    entity->setDisplay_recursive(this);

    ccHObject* currentRoot = this->getSceneDB();
    if (currentRoot)
    {
        //already a pure 'root'
//        if (currentRoot->isA(CC_TYPES::HIERARCHY_OBJECT))
//        {
//            currentRoot->addChild(entity);
//        }
//        else
//        {
        this->setSceneDB(nullptr);
//        ccHObject* root = new ccHObject("root");
//        root->addChild(currentRoot);
//        root->addChild(entity);
        this->setSceneDB(entity);
//        }
    }
    else
    {
        this->setSceneDB(entity);
    }
}


void MyGLWidget::dragEnterEvent(QDragEnterEvent *event)  //将文件拖进来的事件
{
    event->acceptProposedAction();
    ccGLWindow::dragEnterEvent(event);
}

void MyGLWidget::dropEvent(QDropEvent *event)    //放下事件
{
    const QMimeData *qm=event->mimeData();//获取MIMEData
//    qDebug()<<"drop file Name:"<<qm->urls()[0].toLocalFile();
    QString filename = qm->urls()[0].toLocalFile();	//获取拖入的文件名
    emit loadFile(filename);
    ccGLWindow::dropEvent(event);
}
