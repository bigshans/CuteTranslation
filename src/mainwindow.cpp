#include <QColor>
#include <QGuiApplication>
#include <QDebug>
#include <QPainter>
#include <QtMath>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QEventLoop>
#include <QShowEvent>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QLabel>
#include <QScreen>
#include <QClipboard>
#include <QPainterPath>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xdotool.h"
#include "picker.h"
#include "configtool.h"
#include "baidutranslate.h"
#include "wordpage.h"

// Why does “extern const int n;” not work as expected?
// https://stackoverflow.com/questions/14894698/why-does-extern-const-int-n-not-work-as-expected

extern const int Direction_Up;
extern const int Direction_Down;
const int Direction_Up = 0;
const int Direction_Down = 1;

enum PICKTYPE : int { Translate, HanDict };

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
    TriangleHeight(configTool->TriangleHeight),
    TriangleWidth(configTool->TriangleWidth)

{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(configTool->GetMainWindowWidth(), configTool->MainWindowHeight); // MainWindowHeight 过时，无效参数


    // 关闭按钮
    QPushButton *closeButton = new QPushButton(this->centralWidget());
    closeButton->setGeometry(this->width()- 40, 12, 25, 25);
    closeButton->setFlat(true);
    //closeButton->setStyleSheet("QPushButton{border:none;}");
    closeButton->setIconSize(QSize(25, 25));
    closeButton->setIcon(QIcon(":/pic/icons-x.png"));
    closeButton->setFocusPolicy(Qt::NoFocus);
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::hide);

    // 固定按钮
    QPushButton *fixButton = new QPushButton(this->centralWidget());
    fixButton->setGeometry(this->width()- 70, 12, 25, 25);
    fixButton->setFlat(true);
    fixButton->setCheckable(true);
    fixButton->setChecked(configTool->GetMainWindowPin());
    fixButton->setStyleSheet("QPushButton{border:none;} QPushButton:checked{background-color:rgb(222, 222, 222);}");
    fixButton->setIconSize(QSize(25, 25));
    fixButton->setIcon(QIcon(":/pic/icons-pin-grey.png"));
    fixButton->setFocusPolicy(Qt::NoFocus);
    connect(fixButton, &QPushButton::clicked, this, [](bool checked){
        configTool->SetMainWindowPin(checked);
    });

    // 刷新按钮
    refreshButton = new QPushButton(this->centralWidget());
    refreshButton->setGeometry(this->width()- 100, 12, 25, 25);
    refreshButton->setFlat(true);
    refreshButton->setIconSize(QSize(25, 25));
    refreshButton->setIcon(QIcon(":/pic/icons-refresh.png"));
    refreshButton->setFocusPolicy(Qt::NoFocus);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshButtonPressed);

    // 星星按钮
    QPushButton *starButton = new QPushButton(this->centralWidget());
    starButton->setGeometry(this->width()- 130, 12, 25, 25);
    starButton->setFlat(true);
    starButton->setIconSize(QSize(25, 25));
    starButton->setFocusPolicy(Qt::NoFocus);
    starButton->setIcon(QIcon(":/pic/icons-star.png"));

    // 声音按钮
//    QPushButton *voiceButton = new QPushButton(this->centralWidget());
//    voiceButton->setGeometry(this->width()- 160, 13, 25, 25);
//    voiceButton->setFlat(true);
//    voiceButton->setIconSize(QSize(25, 25));
//    voiceButton->setIcon(QIcon(":/pic/icons-voice.png"));

    // 换行N按钮
    QPushButton *newLineButton = new QPushButton(this->centralWidget());
    newLineButton->setGeometry(this->width()- 160, 12, 25, 25);
    newLineButton->setFlat(true);
    newLineButton->setCheckable(true);
    newLineButton->setChecked(false);
    newLineButton->setStyleSheet("QPushButton{border:none;} QPushButton:checked{background-color:rgb(222, 222, 222);}");
    newLineButton->setIconSize(QSize(25, 25));
    newLineButton->setIcon(QIcon(":/pic/icons-n.png"));
    newLineButton->setFocusPolicy(Qt::NoFocus);
    connect(newLineButton, &QPushButton::clicked, this, [=](bool checked){
        picker->ignoreCRLF = checked;
    });

    // 复制按钮
    QPushButton *copyButton = new QPushButton(this->centralWidget());
    copyButton->setGeometry(this->width()- 190, 13, 25, 25);
    copyButton->setFlat(true);
    copyButton->setIconSize(QSize(25, 25));
    copyButton->setIcon(QIcon(":/pic/copy.png"));
    copyButton->setFocusPolicy(Qt::NoFocus);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyButtonPressed);

    // 翻译内容  NOTICE: 不要把上面按钮挡住了
    // stackWidget 里面有两个控件，0. 长文本翻译 1. wordPage
    stackWidget = new QStackedWidget(this->centralWidget());
    stackWidget->setGeometry(20, 40, this->width() - 40, this->height() - 20 );

    // 长文本翻译 控件
    textLabel = new QLabel(this);
    textLabel->setObjectName("textLabel");
    stackWidget->addWidget(textLabel);

    textLabel->setMargin(20);
    textLabel->setFrameShape(QFrame::NoFrame);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setWordWrap(true);
    textLabel->setFixedWidth(stackWidget->width());
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // 单词翻译界面 控件
    wordPage = new WordPage(this);
    wordPage->setContentsMargins(0, -10, 0, 0);
    stackWidget->addWidget(wordPage);

    stackWidget->setCurrentIndex(0);

   // this->ui->centralWidget->setStyleSheet(" .QWidget{ background-color: yellow; } ");
   // this->ui->centralWidget

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
    if (Direction == Direction_Up)
    {
        // 三角形占用了上面的区域
        this->setFixedHeight(this->height() + TriangleHeight);
    }
    e->ignore();
}

void MainWindow::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen;
    pen.setWidth(3);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    // 要根据皮肤设置颜色
    if (configTool->Theme == "light")
    {
        pen.setColor(QColor(192, 192, 192));
        brush.setColor(Qt::white);
    }
    else
    {
        pen.setColor(QColor(100, 100, 100)); // dark
        brush.setColor(QColor("#323232"));
    }

    painter.setPen(pen);
    painter.setBrush(brush);

    QPolygon polygon;
    QPainterPath path;
    if (showTriangle == false)
    {
        centralWidget()->move(0, 0);
        path.addRoundedRect(5, 5, this->width() -5 -5, this->height()-5-5 , 15, 15);
        //showTriangle = true;
    }
    else if (Direction == Direction_Down)
    {
        centralWidget()->move(0, 0);
        path.moveTo(0, 0);
        path.arcMoveTo(5,5,30,30,90);
        path.arcTo(5,5,30,30,90, 90);
        path.lineTo(5, this->height() - TriangleHeight - 15);
        path.arcTo(5,this->height() - TriangleHeight - 30,30,30,180, 90);
        path.lineTo(this->width() / 2 - TriangleWidth + TriangleOffset, this->height() - TriangleHeight);
        path.lineTo(this->width() / 2 + TriangleOffset, this->height());
        path.lineTo(this->width() / 2 + TriangleWidth + TriangleOffset, this->height() - TriangleHeight);
        path.lineTo(this->width() -5 -15, this->height() - TriangleHeight);
        path.arcTo(this->width() -5 -30 ,this->height() - TriangleHeight - 30,30,30,270, 90);
        path.lineTo(this->width() -5 , 5+15);
        path.arcTo(this->width() -5 -30 ,5,30,30,0, 90);
        path.closeSubpath();

    }
    else if (Direction == Direction_Up)
    {
        centralWidget()->move(centralWidget()->x(), TriangleHeight);
        path.moveTo(0, 0);
        path.arcMoveTo(5,TriangleHeight,30,30,90);
        path.arcTo(5,TriangleHeight,30,30,90, 90);
        path.lineTo(5, this->height() - 5 -15 );
        path.arcTo(5,this->height() - 5 - 30,30,30,180, 90);
        path.lineTo(this->width() -5 -15, this->height() - 5);
        path.arcTo(this->width() -5 -30 ,this->height() - 5 - 30,30,30,270, 90);
        path.lineTo(this->width() -5 , TriangleHeight+15);
        path.arcTo(this->width() -5 -30 ,TriangleHeight,30,30,0, 90);
        path.lineTo(this->width() / 2 + TriangleWidth + TriangleOffset, TriangleHeight);
        path.lineTo(this->width() / 2 + TriangleOffset, 0);
        path.lineTo(this->width() / 2 - TriangleWidth + TriangleOffset, TriangleHeight);
        path.closeSubpath();
    }
    painter.drawPath(path);
    event->accept();
}

void MainWindow::onMouseButtonPressed(int x, int y)
{
    if (!this->isHidden() && (x < this->x() || x > this->x() + width() || y < this->y() || y > this->y() + height()))
        if (!configTool->GetMainWindowPin())
            hide();

}

// 返回MainWindow弹出的位置
QPoint MainWindow::getAdjustedPosition(QPoint mousePressPosition, QPoint mouseReleasedPosition)
{
    // 获取默认方向向 重置三角形偏移量
    int direction = configTool->Direction;
    TriangleOffset = 0;

    QPoint mid(0, 0);
    mid.rx() = (mousePressPosition.x() + mouseReleasedPosition.x() - width()) / 2;

    if (direction == Direction_Up)
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    else
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;


    // 判断应该出现在哪个屏幕上
    QRect targetScreenRect;
    for (auto screen : QGuiApplication::screens())
    {
        if (screen->availableGeometry().contains((mousePressPosition + mouseReleasedPosition) / 2))
        {
            targetScreenRect = screen->availableGeometry();

            break;
        }
    }

    // 判断是否超出屏幕上边界
    if (direction == Direction_Down && mid.y() < targetScreenRect.y())
    {
        direction = Direction_Up;
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    }
    // 判断是否超出屏幕下边界
    if (direction == Direction_Up && mid.y() + this->height() > targetScreenRect.y() + targetScreenRect.height())
    {
        direction = Direction_Down;
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;
    }
    Direction = direction;
    // 判断是否超出屏幕左边界
    if (mid.x() < configTool->Edge + targetScreenRect.x())
    {
        TriangleOffset = configTool->Edge - (mid.x() - targetScreenRect.x());
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = configTool->Edge + targetScreenRect.x();
        TriangleOffset = -TriangleOffset;
    }
    // 判断是否超出屏幕右边界
    if (mid.x() + this->width() > targetScreenRect.x() + targetScreenRect.width() - configTool->Edge)
    {
        TriangleOffset = mid.x() - (targetScreenRect.x() + targetScreenRect.width() - configTool->Edge - this->width());
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = targetScreenRect.x() + targetScreenRect.width() - configTool->Edge - this->width();
    }

    return mid;
}

void MainWindow::onFloatButtonPressed(QPoint mousePressPosition, QPoint mouseReleasedPosition, PICKTYPE type)
{
    if (type == PICKTYPE::HanDict)
    {
        qDebug() << "han";
        QString res = BaiduTranslate::instance().HanDict(picker->getSelectedText());
        resultParser(CuteAction::PICK_HANDICT, res);
    }
    else
    {
        // 获取翻译
        QString res = BaiduTranslate::instance().TranslateText(picker->getSelectedText(), configTool->TextTimeout);
        QString res_short = res;
        res_short.truncate(30);
        qInfo() << res_short << "...";
        resultParser(CuteAction::PICK, res);
        // 等待页面加载完成
    }

    move(getAdjustedPosition(mousePressPosition, mouseReleasedPosition));

    showTriangle = true;
    previousAction.Action = CuteAction::PICK;
    previousAction.point1 = mousePressPosition;
    previousAction.point2 = mouseReleasedPosition;
    previousAction.type = type;
    this->show();


}

void MainWindow::onOCRTranslateShortCutPressed()
{
    QString res = BaiduTranslate::instance().OCRTranslate(configTool->OCRTimeout, screenshot);
    QPoint mousePressPosition = xdotool->eventMonitor.mousePressPosition;
    QPoint mouseReleasedPosition = xdotool->eventMonitor.mouseReleasedPosition;

    QString res_short = res;
    res_short.truncate(30);
    qInfo() << res_short << "...";
    resultParser(CuteAction::OCRTranslate, res);
    // 等待页面加载完成

    move(getAdjustedPosition(mousePressPosition, mouseReleasedPosition));
    showTriangle = true;
    this->show();

    previousAction.Action = CuteAction::OCRTranslate;
    previousAction.point1 = mousePressPosition;
    previousAction.point2 = mouseReleasedPosition;
    this->activateWindow();
}

void MainWindow::onOCRTextShortCutPressed()
{
    QString res = BaiduTranslate::instance().OCRText(configTool->OCRTimeout, screenshot);
    QPoint mousePressPosition = xdotool->eventMonitor.mousePressPosition;
    QPoint mouseReleasedPosition = xdotool->eventMonitor.mouseReleasedPosition;

    QString res_short = res;
    res_short.truncate(30);
    qInfo() << res_short << "...";
    resultParser(CuteAction::OCRText, res);
    // 等待页面加载完成

    showTriangle = true;
    move(getAdjustedPosition(mousePressPosition, mouseReleasedPosition));
    this->show();

    previousAction.Action = CuteAction::OCRText;
    previousAction.point1 = mousePressPosition;
    previousAction.point2 = mouseReleasedPosition;
    this->activateWindow();
}

void MainWindow::onSearchBarReturned(QPoint pos, QPoint size, QString text)
{

    QString res = BaiduTranslate::instance().TranslateText(text, configTool->TextTimeout);
    resultParser(CuteAction::Search, res);
    // 等待页面加载完成

    this->showTriangle = false;
    QPoint mid;
    mid.ry() = pos.y() + size.y();
    mid.rx() = pos.x() + size.x() / 2 - this->width() / 2;
    // SearchBar pos 落在哪一个屏幕内
    for (auto screen : QGuiApplication::screens())
    {
        if (screen->availableGeometry().contains(pos))
        {
            // 判断是否超出屏幕下边界
            if (mid.y() + this->height() > screen->availableGeometry().y() + screen->availableGeometry().height())
            {
                mid.ry() = pos.y() - this->height() - 10;
            }
            break;
        }
    }

    move(mid);
    previousAction.Action = CuteAction::Search;
    previousAction.point1 = pos;
    previousAction.point2 = size;
    previousAction.text1 = text;
    this->show();

}

void MainWindow::onRefreshButtonPressed()
{
    if (previousAction.Action == CuteAction::PICK)
    {
        this->onFloatButtonPressed(previousAction.point1, previousAction.point2, previousAction.type);
    }
    else if ( previousAction.Action == CuteAction::OCRTranslate)
    {
        xdotool->eventMonitor.mousePressPosition = previousAction.point1;
        xdotool->eventMonitor.mouseReleasedPosition = previousAction.point2;
        screenshot = false;
        this->onOCRTranslateShortCutPressed();
        screenshot = true;
    }
    else if ( previousAction.Action == CuteAction::OCRText)
    {
        xdotool->eventMonitor.mousePressPosition = previousAction.point1;
        xdotool->eventMonitor.mouseReleasedPosition = previousAction.point2;
        screenshot = false;
        this->onOCRTextShortCutPressed();
        screenshot = true;
    }
    else if (previousAction.Action == CuteAction::Search)
    {
        this->onSearchBarReturned(previousAction.point1, previousAction.point2, previousAction.text1);
    }
}

void MainWindow::resultParser(CuteAction action, QString &res)
{
    if (action == CuteAction::PICK_HANDICT)
    {
        stackWidget->setCurrentIndex(0);
        if (res.isEmpty())
            res = "汉语词典：查询结果为空\n";
        textLabel->setText(res);
        textLabel->adjustSize();
        int height = textLabel->heightForWidth(stackWidget->width());

        this->stackWidget->setFixedHeight(height + 10);
        this->setFixedHeight(stackWidget->y() + height + 30);
    }
    else // 其他动作暂时使用相同的处理方法
    {
        if(res.isEmpty())
        {
            textLabel->setText("");
            return;
        }
        else if (res.startsWith("{"))
        {
            QString text = "null";
            QJsonParseError error;
            QJsonDocument jsonDocument = QJsonDocument::fromJson(res.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError)
            {
                if (!jsonDocument.isEmpty() && jsonDocument.isObject())
                {
                    stackWidget->setCurrentIndex(1);
                    wordPage->updateDescription(jsonDocument.object());
                    wordPage->adjustSize();
                    int height = wordPage->heightForWidth(stackWidget->width());
                    wordPage->setFixedSize(stackWidget->width(), height + 20);
                    this->stackWidget->setFixedHeight(height);
                    this->setFixedHeight(stackWidget->y() + height + 30);
                }
            }
            else
            {
                // 检查错误类型
            }
        }
        else
        {
            // 外语 段落翻译
            stackWidget->setCurrentIndex(0);
            textLabel->setText(res);
            textLabel->adjustSize();
            int height = textLabel->heightForWidth(stackWidget->width());
            this->stackWidget->setFixedHeight(height + 10);
            this->setFixedHeight(stackWidget->y() + height + 30);
        }
    }


}

// This function is obsolete
void MainWindow::onAdjustSize(float zoom, int width)
{
    this->show();
    this->hide();
}

void MainWindow::onCopyButtonPressed()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (stackWidget->currentIndex() == 0 && stackWidget->widget(0) == textLabel)
    {
        clipboard->setText(textLabel->text().replace("<br/>", "\n"));
    }
    else if (stackWidget->currentIndex() == 1 && stackWidget->widget(1) == wordPage)
    {
        clipboard->setText(wordPage->getText());
    }
}
