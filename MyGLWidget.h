#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H
//#program once
#include "ccGLWindow.h"
//qCC_db
#include <ccGenericMesh.h>
#include <ccHObjectCaster.h>
#include <ccPointCloud.h>

class ccGLWindow;

class MyGLWidget: public ccGLWindow
{
public:
    MyGLWidget(QWidget* parent = 0);
    ~MyGLWidget();
    void ShowModel(ccHObject *entity);
};

#endif // MYGLWIDGET_H
