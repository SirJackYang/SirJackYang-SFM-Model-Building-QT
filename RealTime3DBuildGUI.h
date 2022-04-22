#ifndef REALTIME3DBUILDGUI_H
#define REALTIME3DBUILDGUI_H
#pragma execution_character_set("UTF-8")
#define NOMINMAX
#include <QWidget>
#include "MyQGraphicsView.h"
#include <QStyleFactory>
#include "CommeTools.h"
#include "string"
//#include "windows.h"
#include <QIcon>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <Windows.h>
//#include <osgDB/ReadFile>
//#include <osg/ref_ptr>
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QProcess>
#include <QStringList>
//#include <QAction>
#include <QFuture>
#include <QtConcurrent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>

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


QT_BEGIN_NAMESPACE
namespace Ui { class RealTime3DBuildGUI; }
QT_END_NAMESPACE


class RealTime3DBuildGUI : public QWidget
{
    Q_OBJECT

public:
    RealTime3DBuildGUI(QWidget *parent = nullptr);
    ~RealTime3DBuildGUI();
    void initNewModel();
    void doActionSetView();

private slots:
    void on_toolButton_ImageFile_clicked();

    void on_toolButton_Image_clicked();

    void on_toolButton_Pt_clicked();

//    void on_toolButton_PtFile_clicked();

    void on_toolButton_PosCal_clicked();

    void on_toolButton_DenseMatch_clicked();

    void on_toolButton_ModelBuild_clicked();

    void on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_toolButton_PointMatch_clicked();

    void on_tabWidget_fun_tabBarDoubleClicked(int index);

    void on_toolButton_Model2Img_clicked();

    void on_toolButton_GetOutPath_clicked();

    void on_toolButton_GetTexture_clicked();

    void on_toolButtonSetViewTop_clicked();

    void on_toolButtonSetViewLeft_clicked();

    void on_toolButtonSetViewFront_clicked();

    void on_toolButtonSetViewBottom_clicked();

    void on_toolButtonSetViewRight_clicked();

    void on_toolButtonSetViewBack_clicked();

    void on_toolButton_SetShader_clicked();

    void on_toolButton_SetShader2_clicked();

    void on_toolButton_Set_clicked();

    void on_toolButton_CameraSet_clicked();

    void on_toolButton_CloseShader_clicked();

private:
    Ui::RealTime3DBuildGUI *ui;
    ccHObject *LoadModeFromFile(QString filenames);
    void Add2Tree(int id, std::vector<QString> data);
    void ShowMsg(QString msg);
    bool runbat();
    void loadPlugins();
    void updateDisplay();


private:
    bool m_menuhideflag; //用于标记菜单栏是否被隐藏
    ccCameraParamEditDlg *m_camerasetDlg;//相机设置窗口
    ccGLPluginInterface *m_edlplugin;//edlshader插件
    ccGLPluginInterface *m_ssaoplugin;//ssaoshader插件
    QDialog *m_procresswin;//进度条窗口
    CommeTools *m_CT;//用于一些常用工具
    std::vector<QString> m_imgnames;//影像名称
    std::vector<QString> m_modelnames;//模型名称
//    std::vector<QImage> m_imgs;//影像
    std::vector<ccHObject*> m_models;//模型
    QString m_projpath;//工程文件夹
    QString m_spath;//bat文件路径事宜
    QString m_newbuildmodelpath;//新建的模型的路径
    QString m_batfilename;//bat文件名称

};
#endif // REALTIME3DBUILDGUI_H
