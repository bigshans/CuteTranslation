#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEngineSettings>

#include "configtool.h"
#include "picker.h"


namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow *ui;
    void paintEvent(QPaintEvent *event);
    const int TriangleHeight = configTool.TriangleHeight;
    const int TriangleWidth = configTool.TriangleWidth;
    int TriangleOffset = 0;
    int Direction;
    QWebEngineView *view;
    QString html;
    bool showTriangle = true;
    virtual void showEvent(QShowEvent *e);

  signals:
    void gotHeight();


  public slots:
    void onMouseButtonPressed(int x, int y);
    void onFloatButtonPressed(QPoint mousePressPosition, QPoint mouseReleasedPosition);
    void onOCRCompleted(QString res);
    void onSearchBarReturned(QPoint pos, QPoint size, QString res);
};

#endif // MAINWINDOW_H
