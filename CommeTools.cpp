#include "CommeTools.h"


//获取文件夹下所有文件名
bool CommeTools::GetFilenames(QString filepath, std::vector<QString> &filenames)
{
    // 判断路径是否存在
    QDir dir(filepath);
    if(!dir.exists())
    {
        return 0;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();
    //判断是否有文件
    if(list.count() <= 0)
    {
        return 0;
    }
    for(int i=0; i<list.count(); i++)
    {
//        qDebug() << list.at(i);
        filenames.push_back(list.at(i).filePath());
    }
    return 1;
}

//获取文件类型
//1：影像文件；2:模型文件
int CommeTools::GetFileLabel(QString name)
{
    QStringList strs =  name.split('.');
    QString miffx = strs.at(strs.size()-1);
    int label(0);
    if (miffx == "jpg" || miffx == "png" || miffx == "tif" ||
        miffx == "bmp" || miffx == "PNG" || miffx == "JPG")
    {
        label = 1;
    }
    else if(miffx == "txt" || miffx == "ply" || miffx == "las" || miffx == "obj" || miffx == "pts")
    {
        label = 2;
    }
    else
    {
        label = 0;
    }
    return label;
}

//寻找vector中指定元素位置
int CommeTools::findPosVector(std::vector<QString> input, QString aim)
{
    std::vector<QString>::iterator iter=std::find(input.begin(),input.end(),aim);//返回的是一个迭代器指针
    if(iter == input.end())
    {
        return -1;
    }
    else
    {
        return std::distance(input.begin(),iter);
    }
}


//警告，显示是
void CommeTools::warning(QString buttonYesText, QString title, QString content)
{
     QMessageBox msg(QMessageBox::Warning,title, content, QMessageBox::Ok);
     msg.setButtonText(QMessageBox::Ok, buttonYesText);
     msg.exec();
}

//信息，显示是
void CommeTools::information(QString buttonYesText, QString title, QString content)
{
     QMessageBox msg(QMessageBox::Information,title, content, QMessageBox::Ok);
     msg.setButtonText(QMessageBox::Ok, buttonYesText);
     msg.exec();
}

//提问，显示是否
bool CommeTools::question(QString buttonYesText, QString buttonNoText, QString title, QString content)
{
    QMessageBox msg(QMessageBox::Question,title,content,QMessageBox::Yes | QMessageBox::No);
    msg.setButtonText(QMessageBox::Yes,buttonYesText);
    msg.setButtonText(QMessageBox::No,buttonNoText);
    msg.exec();
    if (msg.clickedButton() == msg.button(QMessageBox::Yes)) {
        return true;
    }
    else
        return false;
}


//获取当前系统时间
QString CommeTools::GetNowTime()
{
    //获取系统时间
    time_t now_time=time(NULL);
    //获取本地时间
    tm*  t_tm = localtime(&now_time);
    QString nowtime = asctime(t_tm);
    return nowtime.split(QRegExp("[\r\n]"),QString::SkipEmptyParts)[0];
}
