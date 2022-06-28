#ifndef POINTCLOUDVIEWER_H
#define POINTCLOUDVIEWER_H

#pragma execution_character_set("UTF-8")

#include <QWidget>
#include <QStyleFactory>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QAction>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
//#include <QClipboard>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "CommeTools.h"
#include "string"

//qCC_db
#include <ccGenericMesh.h>
#include <ccHObjectCaster.h>
#include <ccPointCloud.h>

//common dialogs
#include <ccCameraParamEditDlg.h>
#include <ccDisplayOptionsDlg.h>
#include <ccColorScalesManager.h>

//plugins
#include "ccIOPluginInterface.h"
#include "ccRenderToFileDlg.h"
#include "ccPluginManager.h"
#include "ccGLPluginInterface.h"
//#include "qEDL.h"

#include "ccMesh.h"
#include "ccGenericPrimitive.h"
#include "ccPolyline.h"
#include "ccFacet.h"

#include "ccColorGradientDlg.h"
#include "ccSORFilterDlg.h"
#include "PointPickerDlg.h"
//#include "ccRasterizeTool.h"
//#include "ccPointPropertiesDlg.h"

#include "ccMainAppInterface.h"
#include "ccPickingHub.h"
#include "ccSphere.h"
#include "cc2DLabel.h"
#include "ccProgressDialog.h"
#include "CloudSamplingTools.h"
#include "ccScalarField.h"
//#include "ccEntityAction.h"
//#include "SaveParameters.h"
//#include "ccEntityAction.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PointCloudViewer; }
QT_END_NAMESPACE


//! Picked item
struct PickedItem
{
    PickedItem()
        : entity(nullptr)
        , itemIndex(0)
        , entityCenter(false)
    {}

    QPoint clickPoint; //position of the user click
    ccHObject* entity; //picked entity (if any)
    unsigned itemIndex; //e.g. point or triangle index
    CCVector3 P3D; //picked point in 3D (if any)
    CCVector3d uvw; //picked point barycentric coordinates (if picked on a triangle)
    bool entityCenter; //the point doesn't correspond to a real 'item' but to the entity center
};


class PointCloudViewer : public QWidget
{
    Q_OBJECT

public:
    PointCloudViewer(QWidget *parent = nullptr);
    ~PointCloudViewer();
    void SetAModelFile(QString filename)
    {
        loadFile(filename);
    }

private slots:
    void on_toolButton_Pt_clicked();

    void on_toolButtonSetViewTop_clicked();

    void on_toolButtonSetViewLeft_clicked();

    void on_toolButtonSetViewFront_clicked();

    void on_toolButtonSetViewBottom_clicked();

    void on_toolButtonSetViewRight_clicked();

    void on_toolButtonSetViewBack_clicked();

    void on_toolButton_SetShader_clicked();

    void on_toolButton_SetShader2_clicked();

    void on_toolButton_CloseShader_clicked();

    void on_toolButton_Set_clicked();

    void on_toolButton_CameraSet_clicked();

    void on_toolButton_Model2Img_clicked();

    void on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_file_customContextMenuRequested(const QPoint &pos);

    void on_toolButton_SetColor_clicked();

    void on_toolButton_SetColorGradient_clicked();

    void on_toolButton_measure_clicked();

private:
    Ui::PointCloudViewer *ui;
    ccHObject *LoadModeFromFile(QString filename);
    void Add2Tree(QString data);
    void ShowMsg(QString msg);
    void loadPlugins();
    void updateDisplay();
    bool SetColor(ccHObject *data, bool colorize);
    bool setColorGradient(ccHObject* data);
    void processPickedItem(ccHObject* entity, unsigned itemIndex, int x, int y, const CCVector3& P3D, const CCVector3d& uvw);
    void ComputeMesh(CCCoreLib::TRIANGULATION_TYPES type);


protected:
    void dragEnterEvent(QDragEnterEvent*event) Q_DECL_OVERRIDE;//拖动进入事件
    void dropEvent(QDropEvent*event) Q_DECL_OVERRIDE;


private slots:
    void deleteItem(QString itemname);//删除一条目录树
    void renameWell(QString itemname);//重命名一条目录树

    void on_toolButton_measure2_clicked();

    void on_toolButton_measure3_clicked();

    void on_toolButton_stopmeasure_clicked();

    void on_toolButton_SORfitter_clicked();

    void on_toolButton_point2mesh_clicked();

    void on_toolButton_pointer_clicked();

    void StopPointPicker();

    void DeleteLastPoint();

    void loadFile(QString filename);

private:
    bool m_menuhideflag; //用于标记菜单栏是否被隐藏
    ccCameraParamEditDlg *m_camerasetDlg;//相机设置窗口
    ccGLPluginInterface *m_edlplugin;//edlshader插件
    ccGLPluginInterface *m_ssaoplugin;//ssaoshader插件
    std::vector<QString> m_modelpaths;//模型路径
    std::vector<QString> m_modelnames;//模型名称
    std::vector<ccHObject*> m_models;//模型
    ccHObject *m_recentmodel;//当前显示的模型

    PointPickerDlg *m_ppdlg;

private:
    //! Associated 3D label
    cc2DLabel* m_label;

    //! Picking mode
    enum Mode
    {
        POINT_INFO,
        POINT_POINT_DISTANCE,
        POINTS_ANGLE,
    };

    //! Current picking mode
    Mode m_pickingMode;
    int m_whypick;//是选很多点收集起来？还是普通模式
    std::vector<cc2DLabel*> m_manylabels;
};
#endif // POINTCLOUDVIEWER_H
