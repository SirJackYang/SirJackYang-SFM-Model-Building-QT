#include "MyGLWidget.h"

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
