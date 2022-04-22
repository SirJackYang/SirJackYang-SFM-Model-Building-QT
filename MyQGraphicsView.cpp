#include "MyQGraphicsView.h"
#include <QDebug>

MyQGraphicsView::MyQGraphicsView(QWidget *parent):
    QGraphicsView(parent)
{
    //设置鼠标样式
//    setCursor(Qt::CrossCursor);
    //使用抗锯齿渲染
//    setRenderHint(QPainter::Antialiasing);
    //设置缓冲背景，加速渲染
//    setCacheMode(QGraphicsView::CacheBackground);
//    setStyleSheet("border: 1px solid black");
//    setAlignment(Qt::AlignVCenter | Qt::AlignTop);
//    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scalnum = 1.;
    m_flag = 0;
    setMouseTracking(true);
}

void MyQGraphicsView::SetLinkPara(double para[3])
{
    double scalenum = para[0];
    double posx = para[1];
    double posy = para[2];
    //设置缩放
    if (scalenum > m_scalnum)
    {
        this->scale(1.3, 1.3);
        m_scalnum *= 1.3;
    }
    else if (scalenum < m_scalnum)
    {
        this->scale(1/1.3, 1/1.3);
        m_scalnum /= 1.3;
    }
    //设置移动
    if (posx != m_posx && posy != m_posy)
    {
        this->horizontalScrollBar()->setValue(posx);
        this->verticalScrollBar()->setValue(posy);
        m_posx = posx;
        m_posy = posy;
    }
}

void MyQGraphicsView::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
    {
        this->scale(1.3, 1.3);
        m_scalnum *= 1.3;
    }
    else
    {
        this->scale(1/1.3, 1/1.3);
        m_scalnum /= 1.3;
    }
    if (m_linkflag == 1)
    {
        m_linkgraphicsview->SetLinkPara(this->GetLinkPara());
    }
    QGraphicsView::wheelEvent(event);
}


void MyQGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        this->setCursor(Qt::PointingHandCursor);
        m_startpos = mapToScene(event->pos());
    }
    //鼠标左键选点并画线
    else if (event->button() == Qt::LeftButton && m_flag == 1)
    {
        QPointF p = this->mapToScene(event->pos());
//        qDebug()<<p;
        m_points.push_back(p);
        //绘制点
        QPen *pen = new QPen();
        pen->setColor(QColor(255, 0, 0));
        pen->setWidth(2);
        this->scene()->addEllipse(p.x()-1, p.y()-1, 2, 2, *pen);
        int num = m_points.size();
        //绘制线
        if (num >= 2)
        {
            pen->setColor(QColor(241, 89, 42));
            pen->setWidth(1);
            this->scene()->addLine(m_points[num-2].x(),
                                   m_points[num-2].y(),
                                   m_points[num-1].x(),
                                   m_points[num-1].y(),
                                   *pen);
        }
    }
    //鼠标左键只选点
    else if (event->button() == Qt::LeftButton && m_flag == 2)
    {
        QPointF p = this->mapToScene(event->pos());
        m_points.push_back(p);
        //画点
        QPen *pen = new QPen();
        pen->setColor(QColor(255, 0, 0));
        pen->setWidth(2);
        this->scene()->addEllipse(p.x()-1, p.y()-1, 4, 4, *pen);
        //添加文本
        QFont *font = new QFont("Roman times", 20, QFont::Bold);
        QGraphicsTextItem *text = new QGraphicsTextItem(QString::number(m_points.size()));
        text->setPos(p.x()-10, p.y()-10);
        text->setDefaultTextColor(QColor(248, 201, 0));
        text->setFont(*font);
        this->scene()->addItem(text);
    }
    //鼠标右键完成当前区域选点
    else if (event->button() == Qt::RightButton && m_flag == 1)
    {
        m_allpoints.push_back(m_points);
        //连接终点与起点
        QPen *pen = new QPen();
        pen->setColor(QColor(241, 89, 42));
        pen->setWidth(1);
        int num = m_points.size() - 1;
//        qDebug()<<m_points[num];
        this->scene()->addLine(m_points[num].x(),
                               m_points[num].y(),
                               m_points[0].x(),
                               m_points[0].y(),
                               *pen);
        //添加文本
        QFont *font = new QFont("Roman times", 20, QFont::Bold);
        QGraphicsTextItem *text = new QGraphicsTextItem(QString::number(m_allpoints.size()));
        QPointF mpos = GetMeanPos(m_points);
        text->setPos(mpos.x()-20, mpos.y()-20);
        text->setDefaultTextColor(QColor(248, 201, 0));
        text->setFont(*font);
        this->scene()->addItem(text);
        //清除已经选好的点集
        m_points.clear();
    }
    QGraphicsView::mousePressEvent(event);
}


void MyQGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    //鼠标中建弹起进行视图移动
    if (event->button() == Qt::MiddleButton)
    {
        if (m_flag != 0)
            this->setCursor(Qt::CrossCursor);
        else
            this->setCursor(Qt::ArrowCursor);
        m_endpos = mapToScene(event->pos());
        //获取滚动条当前位置
        double oposx = this->horizontalScrollBar()->value();
        double oposy = this->verticalScrollBar()->value();
        //计算鼠标移动的距离
        QPointF offset = m_endpos - m_startpos;
        //计算新的滚轮位置
        double nposx = oposx-offset.x()*m_scalnum;
        double nposy = oposy-offset.y()*m_scalnum;
        //设置新的滚轮位置
        this->horizontalScrollBar()->setValue(nposx);
        this->verticalScrollBar()->setValue(nposy);
//        QPointF offset = -m_endpos + m_startpos;
//        this->scene()->setSceneRect(this->scene()->sceneRect().x()+offset.x(),
//                                    this->scene()->sceneRect().y()+offset.y(),
//                                    this->scene()->sceneRect().width(),
//                                    this->scene()->sceneRect().height());
        this->scene()->update();
        //记录备用
        m_posx = nposx;
        m_posy = nposy;
        //进行联动
        if (m_linkflag == 1)
        {
            m_linkgraphicsview->SetLinkPara(this->GetLinkPara());
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}


void MyQGraphicsView::SetImage(QImage img)
{
    //把影像添加到画布
    QPixmap Images = QPixmap::fromImage(img);
    QGraphicsPixmapItem *map = new QGraphicsPixmapItem(Images);
//    map = new QGraphicsPixmapItem(Images);
    map->setFlag(QGraphicsPixmapItem::ItemIsSelectable, true);
    map->setFlag(QGraphicsPixmapItem::ItemIsMovable, false);
    map->setFlag(QGraphicsPixmapItem::ItemSendsGeometryChanges,true);
    QGraphicsScene *scene = new QGraphicsScene(); //场景 = new QGraphicsScene();
    //画布添加至场景
    scene->addItem(map);
    //场景绑定到控件
    this->setScene(scene);
    this->show();
}

QPointF MyQGraphicsView::GetMeanPos(std::vector<QPointF> data)
{
    double sumx(0);
    double sumy(0);
    for (int i(0); i<data.size(); ++i)
    {
        sumx += data[i].x();
        sumy += data[i].y();
    }
    return QPointF(sumx/data.size(), sumy/data.size());
}
