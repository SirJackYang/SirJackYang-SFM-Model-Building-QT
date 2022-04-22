#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H
#pragma execution_character_set("UTF-8")
#include <QKeyEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPen>
#include <vector>

class MyQGraphicsView: public QGraphicsView
{
private:
    QPointF GetMeanPos(std::vector<QPointF> data);//获取一组点中间位置

protected:
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    double m_scalnum; //缩放系数
    QPointF m_startpos; //鼠标中建开始点
    QPointF m_endpos; //鼠标中建结束点
    double m_posx; //视图移动参数x
    double m_posy; //试图移动参数y
    int m_flag; //是否进行选点
    int m_linkflag; //是否进行联动
    MyQGraphicsView *m_linkgraphicsview; //进行联动的窗口
    std::vector<QPointF> m_points; //选择的点
    std::vector<std::vector<QPointF>> m_allpoints; //选择的所有点

public:
    explicit MyQGraphicsView(QWidget *parent = 0);
    void SetImage(QImage img);
    // 设置是否进行联动
    void SetLinkFlag(int flag)
    {
        m_linkflag = flag;
    }
    //设置联动窗口
    void SetLinkWidget(MyQGraphicsView *graphicsview)
    {
        m_linkgraphicsview = graphicsview;
    }
    // 设置联动参数
    void SetLinkPara(double para[3]);
    // 获取联动参数
    double* GetLinkPara()
    {
        double *para = new double[3];
        para[0] = m_scalnum;
        para[1] = m_posx;
        para[2] = m_posy;
        return para;
    }
    // view初始化
    void InitializeView()
    {
        m_points.clear();
        m_allpoints.clear();
        m_flag = 0;
        m_scalnum = 1;
        this->setCursor(Qt::ArrowCursor);
    }
    //设置是否选点
    void SetChoosePoint(int flag)
    {
        m_flag = flag;
        if (flag == 0)
        {
            this->setCursor(Qt::ArrowCursor);
        }
        else if (flag == 1 || flag == 2)
        {
            this->setCursor(Qt::CrossCursor);
        }
    }
    //获取当前选点状态
    int GetFlag()
    {
        return m_flag;
    }
    //获取单个区域点集
    std::vector<QPointF> GetPoints()
    {
        return m_points;
    }
    //删除单个点集
    void ClearPoints()
    {
        m_points.clear();
    }
    //获取所有点集
    std::vector<std::vector<QPointF>> GetAllPoints()
    {
        return m_allpoints;
    }
    //删除所有点集
    void CalerAllPoints()
    {
        m_allpoints.clear();
    }
};

#endif // MYQGRAPHICSVIEW_H
