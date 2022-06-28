#ifndef COMMETOOLS_H
#define COMMETOOLS_H
#include <QString>
#include <vector>
#include <QDir>
#include <QMessageBox>
#include "time.h"

namespace CommeTools
{
    bool GetFilenames(QString filepath, std::vector<QString> &filenames);//获取指定文件夹下所有文件名
    int GetFileLabel(QString name);//获取文件后缀名
    int findPosVector(std::vector<QString> input, QString aim);//寻找QString vector中指定元素位置
    void warning(QString buttonYesText, QString title, QString content);//中文警告窗口
    void information(QString buttonYesText, QString title, QString content);
    bool question(QString buttonYesText, QString buttonNoText, QString title, QString content);//中文提问窗口
    QString GetNowTime();//获取当前系统时间
};

#endif // COMMETOOLS_H
