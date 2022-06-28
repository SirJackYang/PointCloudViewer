#include "PointCloudViewer.h"
#include "ui_PointCloudViewer.h"
#include <QDebug>

PointCloudViewer::PointCloudViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PointCloudViewer)
{
    setWindowIcon(QIcon("gui-icon.ico"));
    ui->setupUi(this);
    m_menuhideflag = false;
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 5);
    ui->splitter_2->setStretchFactor(0, 4);
    ui->splitter_2->setStretchFactor(1, 1);
    this->showMaximized();
    this->setStyle(QStyleFactory::create("Fusion"));
    ui->treeWidget_file->setColumnCount(1);
    ui->treeWidget_file->setHeaderLabels(QStringList()<<"文件");
    ui->treeWidget_file->setItemsExpandable(true);
    QTreeWidgetItem *tree_ModelItem = new QTreeWidgetItem({"模型"});
    tree_ModelItem->setIcon(0, QIcon(":/Images/models"));
    ui->treeWidget_file->addTopLevelItem(tree_ModelItem);
    ui->treeWidget_file->setContextMenuPolicy(Qt::CustomContextMenu);
    FileIOFilter::InitInternalFilters(); //load all known I/O filters (plugins will come later!)
    ccNormalVectors::GetUniqueInstance(); //force pre-computed normals array initialization
    ccColorScalesManager::GetUniqueInstance(); //force pre-computed color tables initialization
    ccGui::ParamStruct temp;
    temp.backgroundCol = ccColor::FromQColor(QColor(0,0,0));
    ui->openGLWidget->setDisplayParameters(temp);
    ui->openGLWidget->setShaderPath(qApp->applicationDirPath() + "/shaders");
    m_camerasetDlg = nullptr;

    m_edlplugin = nullptr;
    m_ssaoplugin = nullptr;
    loadPlugins();

//    ui->openGLWidget->

//    ui->openGLWidget->showCursorCoordinates(true);
    connect(ui->openGLWidget, &ccGLWindow::itemPicked, this, &PointCloudViewer::processPickedItem, Qt::UniqueConnection);
//    connect(ui->openGLWidget, &QObject::destroyed, this, &PointCloudViewer::onActiveWindowDeleted);
    m_label = new cc2DLabel();
    m_label->setSelected(true);
//    m_manylabels = new cc2DLabel();
//    m_manylabels->setSelected(true);
    m_ppdlg = new PointPickerDlg(this);
    connect(m_ppdlg, SIGNAL(DeleteLastPoint()), this, SLOT(DeleteLastPoint()));
    connect(m_ppdlg, SIGNAL(CloseMe()), this, SLOT(StopPointPicker()));
    m_whypick = 0;

    this->setAcceptDrops(true);
    connect(ui->openGLWidget, SIGNAL(loadFile(QString)), this, SLOT(loadFile(QString)));
}

PointCloudViewer::~PointCloudViewer()
{
    delete ui;
}


void PointCloudViewer::dragEnterEvent(QDragEnterEvent *event)  //将文件拖进来的事件
{
       event->acceptProposedAction();
       QWidget::dragEnterEvent(event);
}

void PointCloudViewer::dropEvent(QDropEvent *event)    //放下事件
{
    const QMimeData *qm=event->mimeData();//获取MIMEData
//    qDebug()<<"drop file Name:"<<qm->urls()[0].toLocalFile();
    QString filename = qm->urls()[0].toLocalFile();	//获取拖入的文件名
    loadFile(filename);
    QWidget::dropEvent(event);
}

void PointCloudViewer::loadFile(QString filename)
{
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    if (entity)
    {
        QStringList tstrs = filename.split("/");
        QString modelname = tstrs[tstrs.size()-1];
        m_modelnames.push_back(modelname);
        //设置目录树
        Add2Tree(modelname);
        //读取模型
        m_models.push_back(entity);
        //显示模型
        ui->openGLWidget->ShowModel(entity);
        ui->openGLWidget->SetModelName(modelname);
        m_recentmodel = entity;
        //显示日志
        ShowMsg("模型打开成功:" + filename);
        //显示弹窗
        CommeTools::information("好的", "提示", "模型文件打开成功");
    }
    else
    {
        //显示日志
        ShowMsg("模型打开失败:" + filename);
        //显示弹窗
        CommeTools::information("好的", "提示", "不支持的格式");
    }
}

void PointCloudViewer::processPickedItem(ccHObject* entity, unsigned itemIndex, int x, int y, const CCVector3& P3D, const CCVector3d& uvw)
{
    if (!entity)
        return;
    //获取PickedItem
    PickedItem item;
    {
        item.clickPoint = QPoint(x, y);
        item.entity = entity;
        item.itemIndex = itemIndex;
        item.P3D = P3D;
        item.uvw = uvw;
        if (entity && entity->isA(CC_TYPES::SPHERE))
        {
            //whether the center of sphere entities should be used when a point is picked on the surface
            static QMessageBox::StandardButton s_pickSphereCenter = QMessageBox::Yes;

            if (s_pickSphereCenter != QMessageBox::YesToAll && s_pickSphereCenter != QMessageBox::NoToAll)
            {
                s_pickSphereCenter = QMessageBox::question(ui->openGLWidget->asWidget(), tr("Sphere picking"), tr("From now on, do you want to pick sphere centers instead of a point on their surface?"), QMessageBox::YesToAll | QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll, QMessageBox::YesToAll);
            }
            if (s_pickSphereCenter == QMessageBox::Yes || s_pickSphereCenter == QMessageBox::YesToAll)
            {
                //replace the input point by the sphere center
                item.P3D = static_cast<ccSphere*>(entity)->getOwnBB().getCenter();
                item.entityCenter = true;
            }
        }
    }
    if (m_whypick == 1)
    {
        //判断是在干嘛
        switch (m_pickingMode)
        {
        case POINT_INFO:
            m_label->clear();
            break;
        case POINT_POINT_DISTANCE:
            if (m_label->size() >= 2)
                m_label->clear();
            break;
        case POINTS_ANGLE:
            if (m_label->size() >= 3)
                m_label->clear();
            break;
        }


        if (item.entity->isKindOf(CC_TYPES::POINT_CLOUD))
        {
            m_label->addPickedPoint(static_cast<ccGenericPointCloud*>(item.entity), item.itemIndex, item.entityCenter);
        }
        else if (item.entity->isKindOf(CC_TYPES::MESH))
        {
            m_label->addPickedPoint(static_cast<ccGenericMesh*>(item.entity), item.itemIndex, CCVector2d(item.uvw.x, item.uvw.y), item.entityCenter);
        }

        m_label->setVisible(true);
        m_label->displayPointLegend(m_label->size() == 3); //we need to display 'A', 'B' and 'C' for 3-points labels
        if (m_label->size() == 1)
        {
            m_label->setPosition(static_cast<float>(item.clickPoint.x() + 20) / ui->openGLWidget->glWidth(), static_cast<float>(item.clickPoint.y() + 20) / ui->openGLWidget->glHeight());
        }
        ui->openGLWidget->addToOwnDB(m_label);
        ui->openGLWidget->redraw();
    }
    else if(m_whypick == 2)
    {
        cc2DLabel* newLabel = new cc2DLabel();
        if (item.entity->isKindOf(CC_TYPES::POINT_CLOUD))
        {
            newLabel->addPickedPoint(static_cast<ccGenericPointCloud*>(item.entity), item.itemIndex, item.entityCenter);
        }
        else if (item.entity->isKindOf(CC_TYPES::MESH))
        {
            newLabel->addPickedPoint(static_cast<ccGenericMesh*>(item.entity), item.itemIndex, CCVector2d(item.uvw.x, item.uvw.y), item.entityCenter);
        }
        else
        {
//            delete newLabel;
            assert(false);
            return;
        }
        newLabel->setVisible(true);
        newLabel->setDisplayedIn2D(false);
        newLabel->displayPointLegend(true);
        newLabel->setCollapsed(true);
//        newLabel->setDisplay(ui->openGLWidget);
        QSize size = ui->openGLWidget->getScreenSize();
        newLabel->setPosition(	static_cast<float>(item.clickPoint.x() + 20) / size.width(),
                                static_cast<float>(item.clickPoint.y() + 20) / size.height() );        
        ui->openGLWidget->addToOwnDB(newLabel);
        ui->openGLWidget->redraw();
        //获取坐标
        const cc2DLabel::PickedPoint& PP = newLabel->getPickedPoint(0);
        CCVector3d P = PP.cloudOrVertices()->toGlobal3d(PP.getPointPosition());
        //界面刷新
        int num = ui->openGLWidget->getOwnDB()->getChildrenNumber();
        QString pname = "P" + QString::number(num);
        newLabel->setName(pname);
        m_ppdlg->Add2Table(pname, P.x, P.y, P.z);
        m_ppdlg->update();
    }
}


//void PointCloudViewer::onActiveWindowDeleted(QObject* obj)
//{
//    if (obj == ui->openGLWidget)
//    {
//        ShowMsg("wuhu");
//    }
//}

void PointCloudViewer::deleteItem(QString itemname)
{

}
void PointCloudViewer::renameWell(QString itemname)
{

}

bool PointCloudViewer::SetColor(ccHObject *data, bool colorize)
{
    QColor colour = QColorDialog::getColor(Qt::white, this, QString(), QColorDialog::ShowAlphaChannel);

    if (!colour.isValid())
        return false;
    if (data->isA(CC_TYPES::POINT_CLOUD) || data->isA(CC_TYPES::MESH))
    {
        ccPointCloud* cloud = nullptr;
        if (data->isA(CC_TYPES::POINT_CLOUD))
        {
            cloud = static_cast<ccPointCloud*>(data);
        }
        else
        {
            ccMesh* mesh = static_cast<ccMesh*>(data);
            ccGenericPointCloud* vertices = mesh->getAssociatedCloud();
            if (	!vertices
                    ||	!vertices->isA(CC_TYPES::POINT_CLOUD)
                    ||	(vertices->isLocked() && !mesh->isAncestorOf(vertices)) )
            {
                ccLog::Warning(QObject::tr("[SetColor] Can't set color for mesh '%1' (vertices are not accessible)").arg(data->getName()));
            }

            cloud = static_cast<ccPointCloud*>(vertices);
        }

        if (colorize)
        {
            cloud->colorize(static_cast<float>(colour.redF()),
                            static_cast<float>(colour.greenF()),
                            static_cast<float>(colour.blueF()),
                            static_cast<float>(colour.alphaF()));
        }
        else
        {
            cloud->setColor(ccColor::FromQColora(colour));
        }
        cloud->showColors(true);
        cloud->showSF(false); //just in case
//        cloud->prepareDisplayForRefresh();
        ui->openGLWidget->toBeRefreshed();

        if (data != cloud)
        {
            data->showColors(true);
        }
        else if (cloud->getParent() && cloud->getParent()->isKindOf(CC_TYPES::MESH))
        {
            cloud->getParent()->showColors(true);
            cloud->getParent()->showSF(false); //just in case
        }
    }
    else if (data->isKindOf(CC_TYPES::PRIMITIVE))
    {
        ccGenericPrimitive* prim = ccHObjectCaster::ToPrimitive(data);
        ccColor::Rgb col(	static_cast<ColorCompType>(colour.red()),
                            static_cast<ColorCompType>(colour.green()),
                            static_cast<ColorCompType>(colour.blue()) );
        prim->setColor(col);
        data->showColors(true);
        data->showSF(false); //just in case
//        data->prepareDisplayForRefresh();
        ui->openGLWidget->toBeRefreshed();
    }
    else if (data->isA(CC_TYPES::POLY_LINE))
    {
        ccPolyline* poly = ccHObjectCaster::ToPolyline(data);
        poly->setColor(ccColor::FromQColor(colour));
        data->showColors(true);
        data->showSF(false); //just in case
//        data->prepareDisplayForRefresh();
        ui->openGLWidget->toBeRefreshed();
    }
    else if (data->isA(CC_TYPES::FACET))
    {
        ccFacet* facet = ccHObjectCaster::ToFacet(data);
        facet->setColor(ccColor::FromQColor(colour));
        data->showColors(true);
        data->showSF(false); //just in case
//        data->prepareDisplayForRefresh();
        ui->openGLWidget->toBeRefreshed();
    }
    else
    {
        ccLog::Warning(QObject::tr("[SetColor] Can't change color of entity '%1'").arg(data->getName()));
    }

    return true;
}


bool PointCloudViewer::setColorGradient(ccHObject* data)
{
    ccColorGradientDlg dlg(this);
    if (!dlg.exec())
        return false;

    unsigned char dim = dlg.getDimension();
    ccColorGradientDlg::GradientType ramp = dlg.getType();

    ccColorScale::Shared colorScale(nullptr);
    if (ramp == ccColorGradientDlg::Default)
    {
        colorScale = ccColorScalesManager::GetDefaultScale();
    }
    else if (ramp == ccColorGradientDlg::TwoColors)
    {
        colorScale = ccColorScale::Create("Temp scale");
        QColor first;
        QColor second;
        dlg.getColors(first,second);
        colorScale->insert(ccColorScaleElement(0.0, first), false);
        colorScale->insert(ccColorScaleElement(1.0, second), true);
    }

    Q_ASSERT(colorScale || ramp == ccColorGradientDlg::Banding);

    const double frequency = dlg.getBandingFrequency();

//    for (ccHObject* ent : selectedEntities)
//    {
        bool lockedVertices = false;
        ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(data, &lockedVertices);
//        if (lockedVertices)
//        {
//            ccUtils::DisplayLockedVerticesWarning(data->getName(), selectedEntities.size() == 1);
//            continue;
//        }

        if (cloud && cloud->isA(CC_TYPES::POINT_CLOUD)) // TODO
        {
            ccPointCloud* pc = static_cast<ccPointCloud*>(cloud);

            bool success = false;
            if (ramp == ccColorGradientDlg::Banding)
                success = pc->setRGBColorByBanding(dim, frequency);
            else
                success = pc->setRGBColorByHeight(dim, colorScale);

            if (success)
            {
                data->showColors(true);
                data->showSF(false); //just in case
//                data->prepareDisplayForRefresh();
                ui->openGLWidget->toBeRefreshed();
            }
        }
//    }

    return true;
}


void PointCloudViewer::ShowMsg(QString msg)
{
    ui->textBrowser_log->moveCursor(QTextCursor::End);
    ui->textBrowser_log->append(">>" + msg);
}


ccHObject *PointCloudViewer::LoadModeFromFile(QString filename)
{
    FileIOFilter::LoadParameters parameters;
    parameters.alwaysDisplayLoadDialog = true;
    parameters.shiftHandlingMode = ccGlobalShiftManager::NO_DIALOG_AUTO_SHIFT;
    parameters.parentWidget = this;
    parameters.autoComputeNormals = true;

    const ccOptions& options = ccOptions::Instance();
    FileIOFilter::ResetSesionCounter();

    CC_FILE_ERROR result = CC_FERR_NO_ERROR;
    ccHObject* newGroup = FileIOFilter::LoadFromFile(filename, parameters, result);

    if (newGroup)
    {
        if (!options.normalsDisplayedByDefault)
        {
            //disable the normals on all loaded clouds!
            ccHObject::Container clouds;
            newGroup->filterChildren(clouds, true, CC_TYPES::POINT_CLOUD);
            for (ccHObject* cloud : clouds)
            {
                if (cloud)
                {
                    static_cast<ccGenericPointCloud*>(cloud)->showNormals(false);
                }
            }
        }
    }
    return newGroup;
}


void PointCloudViewer::Add2Tree(QString data)
{
    //图标
    QIcon icon(":/Images/model.png");
    //正式开始添加
    QTreeWidgetItem *mitem = new QTreeWidgetItem({data});
    mitem->setIcon(0, icon);
    ui->treeWidget_file->topLevelItem(0)->addChild(mitem);
}


void PointCloudViewer::loadPlugins()
{
    QStringList tstrs;
    tstrs.append(qApp->applicationDirPath() + "/plugins");
    ccPluginManager::Get().setPaths(tstrs);
    ccPluginManager::Get().loadPlugins();

    for (ccPluginInterface* plugin : ccPluginManager::Get().pluginList())
    {
        if (plugin == nullptr)
        {
            Q_ASSERT(false);
            continue;
        }

        // is this a GL plugin?
        if (plugin->getType() == CC_GL_FILTER_PLUGIN)
        {
            ccGLPluginInterface* glPlugin = static_cast<ccGLPluginInterface*>(plugin);

            const QString pluginName = glPlugin->getName();

            Q_ASSERT(!pluginName.isEmpty());

            // 是EDLshader的话
            if (pluginName == "EDL Shader")
            {
                m_edlplugin = glPlugin;
            }
            else if(pluginName == "SSAO Shader")
            {
                m_ssaoplugin = glPlugin;
            }
            else
            {
                continue;
            }
        }
    }
}

void PointCloudViewer::updateDisplay()
{
    ui->openGLWidget->redraw();
}


void PointCloudViewer::on_toolButton_Pt_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("请选择模型文件"),
                                                    ".",
                                                    tr("模型文件(*ply *obj *las *pts *txt);;"));
    loadFile(filename);
}


void PointCloudViewer::on_toolButtonSetViewTop_clicked()
{
    ui->openGLWidget->setView(CC_TOP_VIEW);
}


void PointCloudViewer::on_toolButtonSetViewLeft_clicked()
{
    ui->openGLWidget->setView(CC_LEFT_VIEW);
}


void PointCloudViewer::on_toolButtonSetViewFront_clicked()
{
    ui->openGLWidget->setView(CC_FRONT_VIEW);
}


void PointCloudViewer::on_toolButtonSetViewBottom_clicked()
{
    ui->openGLWidget->setView(CC_BOTTOM_VIEW);
}


void PointCloudViewer::on_toolButtonSetViewRight_clicked()
{
    ui->openGLWidget->setView(CC_RIGHT_VIEW);
}


void PointCloudViewer::on_toolButtonSetViewBack_clicked()
{
    ui->openGLWidget->setView(CC_BACK_VIEW);
}


void PointCloudViewer::on_toolButton_SetShader_clicked()
{
    ccGlFilter* filter = m_edlplugin->getFilter();
    if (ui->openGLWidget->areGLFiltersEnabled())
    {
       ui->openGLWidget->setGlFilter(filter);
    }
}


void PointCloudViewer::on_toolButton_SetShader2_clicked()
{
    ccGlFilter* filter = m_ssaoplugin->getFilter();
    if (ui->openGLWidget->areGLFiltersEnabled())
    {
       ui->openGLWidget->setGlFilter(filter);
    }
}


void PointCloudViewer::on_toolButton_Set_clicked()
{
    ccDisplayOptionsDlg clmDlg(this);
    connect(&clmDlg, &ccDisplayOptionsDlg::aspectHasChanged, this, &PointCloudViewer::updateDisplay);
    clmDlg.exec();
    disconnect(&clmDlg, nullptr, nullptr, nullptr);
}


void PointCloudViewer::on_toolButton_CameraSet_clicked()
{
    if (!m_camerasetDlg)
    {
        m_camerasetDlg = new ccCameraParamEditDlg(this, nullptr);
        m_camerasetDlg->linkWith(ui->openGLWidget);
    }
    m_camerasetDlg->show();
}


void PointCloudViewer::on_toolButton_CloseShader_clicked()
{
    ui->openGLWidget->setGlFilter(nullptr);
    ui->openGLWidget->redraw();
}


void PointCloudViewer::on_toolButton_Model2Img_clicked()
{
    ccRenderToFileDlg rtfDlg(ui->openGLWidget->glWidth(), ui->openGLWidget->glHeight(), this);

    if (rtfDlg.exec())
    {
        QApplication::processEvents();
        ui->openGLWidget->renderToFile(rtfDlg.getFilename(), rtfDlg.getZoom(), rtfDlg.dontScalePoints(), rtfDlg.renderOverlayItems());
        //显示日志
        ShowMsg(CommeTools::GetNowTime() + "场景保存成功");
        ShowMsg("保存路径为:"+rtfDlg.getFilename());
        //显示弹窗
        CommeTools::information("好的", "提示", "场景保存成功");
    }
}

void PointCloudViewer::on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString str = item->text(column);
    if (str == "模型") return;
    ShowMsg(str);
    int mpos = CommeTools::findPosVector(m_modelnames, str);
    //显示
    ui->openGLWidget->ShowModel(m_models[mpos]);
    ui->openGLWidget->SetModelName(str);
    m_recentmodel = m_models[mpos];
}


void PointCloudViewer::on_treeWidget_file_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* curItem=ui->treeWidget_file->currentItem();  //获取当前被点击的节点
    if(curItem==NULL)return;           //这种情况是右键的位置不在treeItem的范围内，即在空白位置右击
    QString wellName = curItem->text(0);
    if(wellName != "模型")
    {
        QAction deleteWell("删除该模型",this);//删除井
        QAction reNameWell("重命名该模型",this);//重命名井
        //在界面上删除该item
        connect(&deleteWell, SIGNAL(triggered()), this, SLOT(deleteItem(wellName)));
        connect(&reNameWell,SIGNAL(triggered()),this, SLOT(renameWell(wellName)));

//        QPoint pos;
        QMenu menu(ui->treeWidget_file);
        menu.addAction(&deleteWell);
        menu.addAction(&reNameWell);
        menu.exec(QCursor::pos());  //在当前鼠标位置显示

//        qDebug()<<wellName;
    }
}


void PointCloudViewer::on_toolButton_SetColor_clicked()
{
    ccHObject *object = ui->openGLWidget->getSceneDB();
    if (object->getChildrenNumber() == 0)
        SetColor(object, 1);
    else
        SetColor(object->getFirstChild(), 1);
    updateDisplay();
}


void PointCloudViewer::on_toolButton_SetColorGradient_clicked()
{
    ccHObject *object = ui->openGLWidget->getSceneDB();
    if (object->getChildrenNumber() == 0)
        setColorGradient(object);
    else
        setColorGradient(object->getFirstChild());
    updateDisplay();
}


void PointCloudViewer::on_toolButton_measure_clicked()
{
    m_whypick = 1;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::POINT_PICKING);
    m_pickingMode = POINT_INFO;
}


void PointCloudViewer::on_toolButton_measure2_clicked()
{
    m_whypick = 1;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::POINT_PICKING);
    m_pickingMode = POINT_POINT_DISTANCE;
}


void PointCloudViewer::on_toolButton_measure3_clicked()
{
    m_whypick = 1;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::POINT_PICKING);
    m_pickingMode = POINTS_ANGLE;
}


void PointCloudViewer::on_toolButton_stopmeasure_clicked()
{
    m_whypick = 0;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::NO_PICKING);
    m_label->clear();
//    m_label->setVisible(false);
    ccHObject *temp = ui->openGLWidget->getOwnDB();
    int pnum = temp->getChildrenNumber();
    for(int i(pnum-1); i>=0; --i)
    {
        ccHObject *tt = temp->getChild(i);
        if (tt->isA(CC_TYPES::LABEL_2D))
        {
            ui->openGLWidget->removeFromOwnDB(tt);
        }
    }
    ui->openGLWidget->redraw();
}


void PointCloudViewer::on_toolButton_SORfitter_clicked()
{
    ccSORFilterDlg sorDlg(this);

    //set semi-persistent/dynamic parameters
    static int s_sorFilterKnn = 6;
    static double s_sorFilterNSigma = 1.0;
    sorDlg.setKNN(s_sorFilterKnn);
    sorDlg.setNSigma(s_sorFilterNSigma);
    if (!sorDlg.exec())
        return;

    //update semi-persistent/dynamic parameters
    s_sorFilterKnn = sorDlg.KNN();
    s_sorFilterNSigma = sorDlg.nSigma();

    ccProgressDialog pDlg(true, this);
    pDlg.setAutoClose(false);

    ccHObject *entity = ui->openGLWidget->getSceneDB()->getFirstChild();
    QString entityname = ui->openGLWidget->GetModelName();
    QStringList tstrs = entityname.split(".");
    QString newentityname = tstrs[0] + "_clean." + tstrs[1];
    //specific test for locked vertices
    bool lockedVertices;
    ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity, &lockedVertices);

    //computation
    CCCoreLib::ReferenceCloud* selection = CCCoreLib::CloudSamplingTools::sorFilter(cloud,
                                                                            s_sorFilterKnn,
                                                                            s_sorFilterNSigma,
                                                                            nullptr,
                                                                            &pDlg);
    ShowMsg("原大小" + QString::number(cloud->size()));
    ShowMsg("滤波后大小" + QString::number(selection->size()));
    if (selection && cloud)
    {
        if (selection->size() == cloud->size())
        {
            ShowMsg(tr("[DoActionSORFilter] No points were removed from cloud '%1'").arg(cloud->getName()));
        }
        else
        {
            ccPointCloud* cleanCloud = cloud->partialClone(selection);
            ccHObject *nentity = new ccHObject();
            nentity->addChild(cleanCloud);
            ui->openGLWidget->ShowModel(nentity);
            m_recentmodel = nentity;
            ui->openGLWidget->SetModelName(newentityname);
            Add2Tree(newentityname);
            m_modelnames.push_back(newentityname);
            m_models.push_back(nentity);
        }
    }
    else
    {
        ShowMsg("不支持的格式");
    }
    updateDisplay();
}


void PointCloudViewer::ComputeMesh(CCCoreLib::TRIANGULATION_TYPES type)
{
    //ask the user for the max edge length
    static double s_meshMaxEdgeLength = 0.0;
    {
        bool ok = true;
        double maxEdgeLength = QInputDialog::getDouble(this, tr("Triangulate"), tr("Max edge length (0 = no limit)"), s_meshMaxEdgeLength, 0, 1.0e9, 8, &ok);
        if (!ok)
            return;
        s_meshMaxEdgeLength = maxEdgeLength;
    }

    //select candidates
    ccHObject *entity = ui->openGLWidget->getSceneDB()->getFirstChild();
    QString entityname = ui->openGLWidget->GetModelName();
    QStringList tstrs = entityname.split(".");
    QString newentityname = tstrs[0] + "_mesh." + tstrs[1];
//    bool hadNormals = false;
    if (entity->isKindOf(CC_TYPES::POINT_CLOUD))
    {
        //if the cloud(s) already had normals, ask the use if wants to update them or keep them as is (can look strange!)
        bool updateNormals = false;

        ccProgressDialog pDlg(false, this);
        pDlg.setAutoClose(false);
        pDlg.setWindowTitle(tr("Triangulation"));
        pDlg.setInfo(tr("Triangulation in progress..."));
        pDlg.setRange(0, 0);
        pDlg.show();
        QApplication::processEvents();

        bool errors = false;
    //    ccHObject* ent = cloud;
        assert(entity->isKindOf(CC_TYPES::POINT_CLOUD));

        //compute mesh
        ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(entity);
        ShowMsg(QString::number(cloud->size()));
        ccMesh* mesh = ccMesh::Triangulate(	cloud,
                                            type,
                                            updateNormals,
                                            static_cast<PointCoordinateType>(s_meshMaxEdgeLength),
                                            2 //XY plane by default
                                            );
    //    ShowMsg(QString::number(mesh->size()));
        if (mesh)
        {
            ccHObject *nentity = new ccHObject();
            nentity->addChild(mesh);
            ui->openGLWidget->ShowModel(nentity);
            m_recentmodel = nentity;
            ui->openGLWidget->SetModelName(newentityname);
            Add2Tree(newentityname);
            m_modelnames.push_back(newentityname);
            m_models.push_back(nentity);
        }
        else
        {
            errors = true;
        }

        if (errors)
        {
            ShowMsg("发生了意外");
        }
        updateDisplay();
    }
    else
    {
        ShowMsg("只有点云可以构mesh");
    }

}


void PointCloudViewer::on_toolButton_point2mesh_clicked()
{
    if (ui->openGLWidget->getSceneDB())
        ComputeMesh(CCCoreLib::DELAUNAY_2D_BEST_LS_PLANE);
    else
        ShowMsg("没有可用模型");
}



void PointCloudViewer::on_toolButton_pointer_clicked()
{
    m_whypick = 2;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::POINT_PICKING);
    QSize size = ui->openGLWidget->getScreenSize();
    QPoint p = ui->openGLWidget->mapToGlobal(QPoint(0,0));
    int x = p.x()+size.width()-m_ppdlg->width();
    int y = p.y();
    m_ppdlg->move(QPoint(x, y));
    m_ppdlg->show();

//    qDebug()<<ui->openGLWidget->getOwnDB()->getChildrenNumber();
}

void PointCloudViewer::StopPointPicker()
{
    m_whypick = 0;
    ui->openGLWidget->setPickingMode(ccGLWindow::PICKING_MODE::NO_PICKING);
    ccHObject *temp = ui->openGLWidget->getOwnDB();
    int pnum = temp->getChildrenNumber();
//    qDebug()<<pnum;
    for(int i(pnum-1); i>=0; --i)
    {
//        qDebug() << i;
        ccHObject *tt = temp->getChild(i);
        if (tt->isA(CC_TYPES::LABEL_2D))
        {
//            tt->setVisible(false);
            ui->openGLWidget->removeFromOwnDB(tt);
        }
    }
//    ui->openGLWidget->ShowModel(m_recentmodel);
//    m_manylabels.clear();
    ui->openGLWidget->redraw();
}

void PointCloudViewer::DeleteLastPoint()
{
//    m_manylabels.pop_back();
    ccHObject *temp = ui->openGLWidget->getOwnDB();
    int num = temp->getChildrenNumber()-1;
//    qDebug()<<num;
    ccHObject *tt = temp->getChild(num);
    if (tt->isA(CC_TYPES::LABEL_2D))
    {
//        tt->setVisible(false);
        ui->openGLWidget->removeFromOwnDB(tt);
    }
    ui->openGLWidget->redraw();
}

