#include "RealTime3DBuildGUI.h"
#include "ui_RealTime3DBuildGUI.h"
#include <QDebug>

RealTime3DBuildGUI::RealTime3DBuildGUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RealTime3DBuildGUI)
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
    QTreeWidgetItem *tree_ImgItem = new QTreeWidgetItem({"影像"});
    QTreeWidgetItem *tree_ModelItem = new QTreeWidgetItem({"模型"});
    tree_ImgItem->setIcon(0, QIcon(":/Images/pictures"));
    tree_ModelItem->setIcon(0, QIcon(":/Images/models"));
    ui->treeWidget_file->addTopLevelItem(tree_ImgItem);
    ui->treeWidget_file->addTopLevelItem(tree_ModelItem);
    m_CT = new CommeTools();
    FileIOFilter::InitInternalFilters(); //load all known I/O filters (plugins will come later!)
    ccNormalVectors::GetUniqueInstance(); //force pre-computed normals array initialization
    ccColorScalesManager::GetUniqueInstance(); //force pre-computed color tables initialization
    ccGui::ParamStruct temp;
    temp.backgroundCol = ccColor::FromQColor(QColor(0,0,0));
    ui->openGLWidget->setDisplayParameters(temp);
    ui->openGLWidget->setShaderPath(qApp->applicationDirPath() + "/shaders");
    m_camerasetDlg = nullptr;

    m_batfilename = "NULL";
    m_edlplugin = nullptr;
    m_ssaoplugin = nullptr;
    loadPlugins();

    m_procresswin = new QDialog(this);
    m_procresswin->resize(320, 320);
    QVBoxLayout *verticalLayout = new QVBoxLayout(m_procresswin);
    QLabel *label = new QLabel(m_procresswin);
    verticalLayout->addWidget(label);
    label->setScaledContents(true);
    m_procresswin->setWindowFlags(Qt::FramelessWindowHint);//需要去掉标题栏
    m_procresswin->setAttribute(Qt::WA_TranslucentBackground);
    m_procresswin->setStyleSheet("#centralWidget{background:rgba(0,0,0,0);}"
       "#label{background:rgba(0,0,0,0);}");
    QMovie *movie = new QMovie(":/Images/wait.gif");
    label->setMovie(movie);
    movie->start();

}

RealTime3DBuildGUI::~RealTime3DBuildGUI()
{
    delete ui;
    if (m_CT != NULL)
    {
        delete [] m_CT;
        m_CT = NULL;
    }
}

void RealTime3DBuildGUI::ShowMsg(QString msg)
{
    ui->textBrowser_log->moveCursor(QTextCursor::End);
    ui->textBrowser_log->append(">>" + msg);
}


ccHObject *RealTime3DBuildGUI::LoadModeFromFile(QString filename)
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


void RealTime3DBuildGUI::Add2Tree(int id, std::vector<QString> data)
{
    //先删除原来的节点
    int num = ui->treeWidget_file->topLevelItem(id)->childCount();
    for (int i(0); i<num; ++i)
    {
        ui->treeWidget_file->topLevelItem(id)->removeChild(ui->treeWidget_file->topLevelItem(id)->child(0));
    }
    //判断图标
    QIcon icon(":/Images/picture.png");
    if (id == 1)
    {
        icon = QIcon(":/Images/model.png");
    }
    //正式开始添加
    for (int i(0); i < data.size(); ++i)
    {
        QTreeWidgetItem *mitem = new QTreeWidgetItem({data.at(i)});
        mitem->setIcon(0, icon);
        ui->treeWidget_file->topLevelItem(id)->addChild(mitem);
    }
}


void RealTime3DBuildGUI::loadPlugins()
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

void RealTime3DBuildGUI::updateDisplay()
{
    ui->openGLWidget->redraw();
}


void RealTime3DBuildGUI::on_toolButton_ImageFile_clicked()
{
    QString filepath = QFileDialog::getExistingDirectory(this,
                                                         "打开影像文件夹",
                                                         ".");
    //获取文件名
    std::vector<QString> names;
    m_CT->GetFilenames(filepath, names);
    for (int i(0); i<names.size(); ++i)
    {
        m_imgnames.push_back(names[i]);
    }
    //设置目录树
    Add2Tree(0, m_imgnames);
    //显示最后一张影像
    QImage img(m_imgnames[m_imgnames.size()-1]);
    ui->graphicsView->SetImage(img);
    ui->tabWidget_show->setCurrentIndex(0);
    //显示日志
    ShowMsg("影像打开成功:" + filepath);
    //显示弹窗
    m_CT->information("好的", "提示", "影像文件夹打开成功");
}


void RealTime3DBuildGUI::on_toolButton_Image_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("请选择影像"),
                                                    ".",
                                                    tr("影像文件(*jpg *tif *png *bmp);;"));
    m_imgnames.push_back(filename);
    //设置目录树
    Add2Tree(0, m_imgnames);
    //显示影像
    QImage img(filename);
    ui->graphicsView->SetImage(img);
    ui->tabWidget_show->setCurrentIndex(0);
    //显示日志
    ShowMsg("影像打开成功:" + filename);
    //显示弹窗
    m_CT->information("好的", "提示", "影像文件打开成功");
}


void RealTime3DBuildGUI::on_toolButton_Pt_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("请选择模型文件"),
                                                    ".",
                                                    tr("模型文件(*ply *obj *las *pts *txt);;"));
    m_modelnames.push_back(filename);
    //设置目录树
    Add2Tree(1, m_modelnames);
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    m_models.push_back(entity);
    //显示模型
    ui->openGLWidget->ShowModel(entity);
    ui->tabWidget_show->setCurrentIndex(1);
    //显示日志
    ShowMsg("模型打开成功:" + filename);
    //显示弹窗
    m_CT->information("好的", "提示", "模型文件打开成功");
}

bool RealTime3DBuildGUI::runbat()
{
    if (m_batfilename != "NULL")
    {
        QString strInfo;
        QProcess p(nullptr);
        p.setWorkingDirectory(qApp->applicationDirPath()); //设置工作目录
        p.start(m_batfilename);  //运行脚本文件
        p.waitForFinished(-1);
    //    qDebug() << p.errorString();
        return true;
    }
    else
    {
        return 0;
    }
}

void RealTime3DBuildGUI::on_toolButton_PointMatch_clicked()
{
    QString scmd;
    scmd = m_spath;

    scmd += "echo 1. Intrinsics analysis\n";
    scmd += "MVG_main_SfMInit_ImageListing.exe -i %Path_Images% -d %Path_Camera_Database% -o %matches_dir%";
    scmd += " >>%reconstruction_dir%\\SfMInit_ImageListing.txt";
    scmd += "\n";

    scmd += "echo 2. Compute features\n";
    scmd += "MVG_main_ComputeFeatures -i %matches_dir%\\sfm_data.json -o %matches_dir%";
    scmd += " >>%reconstruction_dir%\\ComputeFeatures.txt";
    scmd += "\n";

    scmd += "echo 3. Compute matches\n";
    scmd += "MVG_main_ComputeMatches -i %matches_dir%\\sfm_data.json -o %matches_dir%";
    scmd += " >>%reconstruction_dir%\\Compute_Matches.txt";
    scmd += "\n";

    QFile file(qApp->applicationDirPath() + "/mvgmvs.bat");
    file.open(QIODevice::WriteOnly);
    file.write(scmd.toLocal8Bit());
    file.close();

    //弹窗提示等待
    m_procresswin->show();

    QString batpath = qApp->applicationDirPath() + "/mvgmvs.bat";
    m_batfilename = batpath;
    QFuture<bool> future = QtConcurrent::run(this,&RealTime3DBuildGUI::runbat);
    while(!future.isFinished())
    {

        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    future.cancel();
    m_procresswin->close();
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "同名点匹配成功");
    //显示弹窗
    m_CT->information("好的", "提示", "同名点匹配成功");
}


void RealTime3DBuildGUI::on_toolButton_PosCal_clicked()
{
    QString scmd;
    scmd = m_spath;

    scmd += "echo 4. Do Incremental reconstruction\n";
    scmd += "MVG_main_IncrementalSfM -i %matches_dir%\\sfm_data.json -m %matches_dir% -o %reconstruction_dir%";
    scmd += " >>%reconstruction_dir%\\IncrementalSfM.txt";
    scmd += "\n";

    scmd += "echo 5. ComputeSfM DataColor\n";
    scmd += "MVG_main_ComputeSfM_DataColor -i %reconstruction_dir%\\sfm_data.bin -o %reconstruction_dir%\\colorized.ply";
    scmd += " >>%reconstruction_dir%\\ComputeSfM_DataColor.txt";
    scmd += "\n";

    QFile file(qApp->applicationDirPath() + "/mvgmvs.bat");
    file.open(QIODevice::WriteOnly);
    file.write(scmd.toLocal8Bit());
    file.close();

    //弹窗提示等待
    m_procresswin->show();

    QString batpath = qApp->applicationDirPath() + "/mvgmvs.bat";
    m_batfilename = batpath;
    QFuture<bool> future = QtConcurrent::run(this,&RealTime3DBuildGUI::runbat);
    while(!future.isFinished())
    {
       QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    future.cancel();
    m_procresswin->close();

    //展示新建的稀疏点云----------------------
    QString filename = m_newbuildmodelpath + "/colorized.ply";
    m_modelnames.push_back(filename);
    //设置目录树
    Add2Tree(1, m_modelnames);
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    //显示模型
    ui->openGLWidget->ShowModel(entity);
    ui->tabWidget_show->setCurrentIndex(1);
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "稀疏重建成功");
    //显示弹窗
    m_CT->information("好的", "提示", "稀疏重建成功");
}


void RealTime3DBuildGUI::on_toolButton_DenseMatch_clicked()
{
    QString scmd;
    scmd = m_spath;

    scmd += "echo 6. Structure from Known Poses (robust triangulation)\n";
    scmd += "MVG_main_ComputeStructureFromKnownPoses -i %reconstruction_dir%\\sfm_data.bin -o %reconstruction_dir%\\robust.bin -m %matches_dir% -f %matches_dir%/matches.f.bin";
    scmd += " >>%reconstruction_dir%\\ComputeStructureFromKnownPoses.txt\n";
    scmd += "MVG_main_ComputeSfM_DataColor -i %reconstruction_dir%\\robust.bin -o %reconstruction_dir%\\robust_colorized.ply";
    scmd += " >>%reconstruction_dir%\\ComputeSfM_DataColor.txt";
    scmd += "\n";

    scmd += "echo 7.  MVG to MVS\n";
    scmd += "MVG_main_MVG2MVS -i %reconstruction_dir%\\sfm_data.bin -o %reconstruction_dir%\\scene.mvs -d %undistorted_images_path%";
    scmd += "\n";

    scmd += "echo 8. Densify PointCloud\n";
//    if (num != 0)
//    {
//        scmd += "DensifyPointCloud --resolution-level %RL% -i %reconstruction_dir%\\scene.mvs -o scene_dense.mvs -w %reconstruction_dir%";
//    }
//    else
//    {
    scmd += "DensifyPointCloud -i %reconstruction_dir%\\scene.mvs -o scene_dense.mvs -w %reconstruction_dir%";
//    }
    scmd += "\n";

    QFile file(qApp->applicationDirPath() + "/mvgmvs.bat");
    file.open(QIODevice::WriteOnly);
    file.write(scmd.toLocal8Bit());
    file.close();

    //弹窗提示等待
    m_procresswin->show();

    QString batpath = qApp->applicationDirPath() + "/mvgmvs.bat";
    m_batfilename = batpath;
    QFuture<bool> future = QtConcurrent::run(this,&RealTime3DBuildGUI::runbat);
    while(!future.isFinished())
    {
       QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    future.cancel();
    m_procresswin->close();

    //展示新建的稠密点云----------------------
    QString filename = m_newbuildmodelpath + "/scene_dense.ply";
    m_modelnames.push_back(filename);
    //设置目录树
    Add2Tree(1, m_modelnames);
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    //显示模型
    ui->openGLWidget->ShowModel(entity);
    ui->tabWidget_show->setCurrentIndex(1);
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "稠密重建成功");
    //显示弹窗
    m_CT->information("好的", "提示", "稠密重建成功");
}


void RealTime3DBuildGUI::on_toolButton_ModelBuild_clicked()
{
    QString scmd;
    scmd = m_spath;

    scmd += "echo 9. Reconstruct Mesh \n";
    scmd += "ReconstructMesh -i %reconstruction_dir%\\scene_dense.mvs -o scene_dense_mesh.mvs -w %reconstruction_dir%";

    QFile file(qApp->applicationDirPath() + "/mvgmvs.bat");
    file.open(QIODevice::WriteOnly);
    file.write(scmd.toLocal8Bit());
    file.close();

    //弹窗提示等待
    m_procresswin->show();

    QString batpath = qApp->applicationDirPath() + "/mvgmvs.bat";
    m_batfilename = batpath;
    QFuture<bool> future = QtConcurrent::run(this,&RealTime3DBuildGUI::runbat);
    while(!future.isFinished())
    {
       QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    future.cancel();
    m_procresswin->close();

    //展示新建的mesh----------------------
    QString filename = m_newbuildmodelpath + "/scene_dense_mesh.ply";
    m_modelnames.push_back(filename);
    //设置目录树
    Add2Tree(1, m_modelnames);
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    //显示模型
    ui->openGLWidget->ShowModel(entity);
    ui->tabWidget_show->setCurrentIndex(1);
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "模型构网成功");
    //显示弹窗
    m_CT->information("好的", "提示", "模型构网成功");
}

void RealTime3DBuildGUI::on_toolButton_GetTexture_clicked()
{
    QString scmd;
    scmd = m_spath;

    scmd += "echo 10. Texture Mesh\n";
    scmd += "TextureMesh -i %reconstruction_dir%\\scene_dense_mesh.mvs -o scene_dense_mesh_texture.mvs -w %reconstruction_dir% --color-vertices=true ";

    QFile file(qApp->applicationDirPath() + "/mvgmvs.bat");
    file.open(QIODevice::WriteOnly);
    file.write(scmd.toLocal8Bit());
    file.close();

    //弹窗提示等待
    m_procresswin->show();

    QString batpath = qApp->applicationDirPath() + "/mvgmvs.bat";
    m_batfilename = batpath;
    QFuture<bool> future = QtConcurrent::run(this,&RealTime3DBuildGUI::runbat);
    while(!future.isFinished())
    {
       QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    future.cancel();
    m_procresswin->close();

    //展示新建的mesh----------------------
    QString filename = m_newbuildmodelpath + "/scene_dense_mesh_texture.ply";
    m_modelnames.push_back(filename);
    //设置目录树
    Add2Tree(1, m_modelnames);
    //读取模型
    ccHObject *entity = LoadModeFromFile(filename);
    //显示模型
    ui->openGLWidget->ShowModel(entity);
    ui->tabWidget_show->setCurrentIndex(1);
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "纹理映射成功");
    //显示弹窗
    m_CT->information("好的", "提示", "纹理映射成功");
}

void RealTime3DBuildGUI::on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString str = item->text(column);
    int miffx = m_CT->GetFileLabel(str);
    if (miffx)
    {
        ShowMsg(str);
        if (miffx == 1)
        {
            int mpos = m_CT->findPosVector(m_imgnames, str);
            //显示
            QImage img(m_imgnames[mpos]);
            ui->graphicsView->SetImage(img);
            ui->tabWidget_show->setCurrentIndex(0);
        }
        else if (miffx == 2)
        {
            int mpos = m_CT->findPosVector(m_modelnames, str);
            //显示
            ui->openGLWidget->ShowModel(m_models[mpos]);
            ui->tabWidget_show->setCurrentIndex(1);
        }
    }
}



void RealTime3DBuildGUI::on_tabWidget_fun_tabBarDoubleClicked(int index)
{
    if (m_menuhideflag)
    {
        ui->tab_deal->setFixedHeight(138);
        ui->tab_file->setFixedHeight(138);
        ui->verticalLayout->setStretch(0, 0);
        ui->verticalLayout->setStretch(1, 1);
        m_menuhideflag = false;
    }
    else
    {
        ui->tab_deal->setFixedHeight(0);
        ui->tab_file->setFixedHeight(0);
        ui->verticalLayout->setStretch(0, 1);
        ui->verticalLayout->setStretch(1, 100000);
        m_menuhideflag = true;
    }
}


void RealTime3DBuildGUI::on_toolButton_Model2Img_clicked()
{
    ccRenderToFileDlg rtfDlg(ui->openGLWidget->glWidth(), ui->openGLWidget->glHeight(), this);

    if (rtfDlg.exec())
    {
        QApplication::processEvents();
        ui->openGLWidget->renderToFile(rtfDlg.getFilename(), rtfDlg.getZoom(), rtfDlg.dontScalePoints(), rtfDlg.renderOverlayItems());
        //显示日志
        ShowMsg(m_CT->GetNowTime() + "场景保存成功");
        ShowMsg("保存路径为:"+rtfDlg.getFilename());
        //显示弹窗
        m_CT->information("好的", "提示", "场景保存成功");
    }
}


void RealTime3DBuildGUI::on_toolButton_GetOutPath_clicked()
{
    QString filepath = QFileDialog::getExistingDirectory(this,
                                                         "打开工程保存文件夹",
                                                         ".");
    m_projpath = filepath;
    //创建文件夹
    QDir dir;
    dir.cd(m_projpath);
    if(!dir.exists("images"))
    {
        dir.mkdir("images");
    }
    if(!dir.exists("matches"))
    {
        dir.mkdir("matches");
    }
    if(!dir.exists("reconstruction_sequential"))
    {
        dir.mkdir("reconstruction_sequential");
    }
    if(!dir.exists("reconstruction_sequential/undistorted_images"))
    {
        dir.mkdir("reconstruction_sequential/undistorted_images");
    }

    m_newbuildmodelpath = m_projpath+"/reconstruction_sequential";

    //把影像复制过去
    for (int i(0); i<m_imgnames.size(); ++i)
    {
        QString oldpath_name = m_imgnames[i];
        QStringList tstrs = oldpath_name.split('.');
        QString newpath_name = m_projpath + "/images/" + QString::number(i) + "." + tstrs[tstrs.size()-1];
        QFile::copy(m_imgnames[i], newpath_name);
    }
    //设置bat文件的路径事宜
    m_spath = "@echo on\n";
    m_spath += "\n";

    m_spath += "set Path_Proj=";
    m_spath += m_projpath;
    m_spath += "\n";

    m_spath += "set Path_Camera_Database=";
    m_spath += qApp->applicationDirPath();
    m_spath += "/sensor_width_camera_database.txt\n";

    m_spath += "set Path_Images=";
    m_spath += m_projpath;
    m_spath += "/images\n";

    m_spath += "set Script_path=%~dp0\n";
    m_spath += "set Path_Bin=%Script_path%/MVG_MVS\n";

    m_spath += "set matches_dir=%Path_Proj%/matches\n";
    m_spath += "set reconstruction_dir=%Path_Proj%/reconstruction_sequential\n";
    m_spath += "set undistorted_images_path=%Path_Proj%/reconstruction_sequential/undistorted_images\n";
    m_spath += "cd %Path_Bin%\n";
    //显示日志
    ShowMsg(m_CT->GetNowTime() + "输出文件夹设置成功");
    ShowMsg("输出文件夹为:" + m_projpath);
    //显示弹窗
    m_CT->information("好的", "提示", "输出文件夹设置成功");
}





void RealTime3DBuildGUI::on_toolButtonSetViewTop_clicked()
{
    ui->openGLWidget->setView(CC_TOP_VIEW);
}


void RealTime3DBuildGUI::on_toolButtonSetViewLeft_clicked()
{
    ui->openGLWidget->setView(CC_LEFT_VIEW);
}


void RealTime3DBuildGUI::on_toolButtonSetViewFront_clicked()
{
    ui->openGLWidget->setView(CC_FRONT_VIEW);
}


void RealTime3DBuildGUI::on_toolButtonSetViewBottom_clicked()
{
    ui->openGLWidget->setView(CC_BOTTOM_VIEW);
}


void RealTime3DBuildGUI::on_toolButtonSetViewRight_clicked()
{
    ui->openGLWidget->setView(CC_RIGHT_VIEW);
}


void RealTime3DBuildGUI::on_toolButtonSetViewBack_clicked()
{
    ui->openGLWidget->setView(CC_BACK_VIEW);
}


void RealTime3DBuildGUI::on_toolButton_SetShader_clicked()
{
    ccGlFilter* filter = m_edlplugin->getFilter();
    if (ui->openGLWidget->areGLFiltersEnabled())
    {
       ui->openGLWidget->setGlFilter(filter);
    }
}


void RealTime3DBuildGUI::on_toolButton_SetShader2_clicked()
{
    ccGlFilter* filter = m_ssaoplugin->getFilter();
    if (ui->openGLWidget->areGLFiltersEnabled())
    {
       ui->openGLWidget->setGlFilter(filter);
    }
}


void RealTime3DBuildGUI::on_toolButton_Set_clicked()
{
    ccDisplayOptionsDlg clmDlg(this);
    connect(&clmDlg, &ccDisplayOptionsDlg::aspectHasChanged, this, &RealTime3DBuildGUI::updateDisplay);
    clmDlg.exec();
    disconnect(&clmDlg, nullptr, nullptr, nullptr);
}


void RealTime3DBuildGUI::on_toolButton_CameraSet_clicked()
{
    if (!m_camerasetDlg)
    {
        m_camerasetDlg = new ccCameraParamEditDlg(this, nullptr);
        m_camerasetDlg->linkWith(ui->openGLWidget);
    }
    m_camerasetDlg->show();
}


void RealTime3DBuildGUI::on_toolButton_CloseShader_clicked()
{
    ui->openGLWidget->setGlFilter(nullptr);
    ui->openGLWidget->redraw();
}

