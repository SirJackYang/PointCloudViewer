#include "PointPickerDlg.h"
#include "ui_PointPickerDlg.h"

PointPickerDlg::PointPickerDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PointPickerDlg)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint); // 设置窗口无边框，设置后窗口无法移动
    //设置表头
    QTableWidgetItem *headerItem;
    QStringList headerText;
    headerText<<"点名称"<<"X"<<"Y"<<"Z";  //表头标题用QStringList来表示
    ui->tableWidget->setHorizontalHeaderLabels(headerText);
    ui->tableWidget->setColumnCount(headerText.count());//列数设置为与 headerText的行数相等
    for (int i=0;i<ui->tableWidget->columnCount();i++)//列编号从0开始
    {
        //cellItem=ui->tableInfo->horizontalHeaderItem(i);
        headerItem=new QTableWidgetItem(headerText.at(i)); //新建一个QTableWidgetItem， headerText.at(i)获取headerText的i行字符串
        QFont font=headerItem->font();//获取原有字体设置
        font.setBold(true);//设置为粗体
        font.setPointSize(12);//字体大小
        headerItem->setTextColor(Qt::black);//字体颜色
        headerItem->setFont(font);//设置字体
        ui->tableWidget->setHorizontalHeaderItem(i,headerItem); //设置表头单元格的Item
    }
    m_saved = 0;
}

PointPickerDlg::~PointPickerDlg()
{
    delete ui;
}


void PointPickerDlg::Add2Table(QString name, double x, double y, double z)
{
    //同时要把坐标信息弄到string里边，以帮助后边保存
    QString xx = QString::number(x, 'f', 6);
    QString yy = QString::number(y, 'f', 6);
    QString zz = QString::number(z, 'f', 6);
    QString linestr = name+" "+xx+" "+yy+" "+zz;
    m_points.push_back(linestr);
    int rowid = ui->tableWidget->rowCount();
    //插入一行
    ui->tableWidget->insertRow(rowid);
    //为一行的单元格创建 Items
    QTableWidgetItem *item1 = new  QTableWidgetItem(name);
    item1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(rowid, 0, item1);

    QTableWidgetItem *item2 = new  QTableWidgetItem(xx);
    item2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(rowid, 1, item2);

    QTableWidgetItem *item3 = new  QTableWidgetItem(yy);
    item3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(rowid, 2, item3);

    QTableWidgetItem *item4 = new  QTableWidgetItem(zz);
    item4->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(rowid, 3, item4);

    ui->tableWidget->scrollToBottom();
    ui->lineEdit->setText(QString::number(ui->tableWidget->rowCount()));

}


void PointPickerDlg::on_toolButton_Save_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
            tr("保存点"),
            "PickedPoints.txt",
            tr("*.txt"));
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QString allstr;
    for(int i(0); i<m_points.size(); ++i)
    {
        allstr += m_points[i];
        if (i != m_points.size()-1)
            allstr += "\n";
    }
    file.write(allstr.toLocal8Bit());
    file.close();
    m_saved = 1;
}


void PointPickerDlg::on_toolButton_F5_clicked()
{
    m_points.pop_back();
    int row = ui->tableWidget->rowCount() - 1;
    ui->tableWidget->removeRow(row);
    ui->lineEdit->setText(QString::number(ui->tableWidget->rowCount()));
    emit DeleteLastPoint();
}

void PointPickerDlg::closeEvent(QCloseEvent *event)
{
    if (!m_saved)
    {
        bool temp =  CommeTools::question("好的", "就不", "提示", "选的点还没保存");
        if (temp)
        {
            event->ignore();
        }
        else
        {
            event->accept();
            for(int row = ui->tableWidget->rowCount() - 1;row >= 0; row--)
            {
                ui->tableWidget->removeRow(row);
            }
            ui->lineEdit->setText(" ");
            m_points.clear();
            emit CloseMe();
        }
    }
    else
    {
        event->accept();
        for(int row = ui->tableWidget->rowCount() - 1;row >= 0; row--)
        {
            ui->tableWidget->removeRow(row);
        }
        ui->lineEdit->setText(" ");
        m_points.clear();
        emit CloseMe();
    }
}


