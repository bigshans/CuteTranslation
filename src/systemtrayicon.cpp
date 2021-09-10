#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include "systemtrayicon.h"
#include "shortcut.h"
#include "xdotool.h"
#include "configtool.h"
#include "messagebox.h"
#include "errno.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent):QSystemTrayIcon(parent),
    modeSubMenu("取词模式", &menu), helpSubMenu("帮助", &menu), icon(":/pic/icon.png"), icon_grey(":/pic/icon-grey.png")
{
    quit_action.setText("退出");
    config_action.setText("配置");
    help_action.setText("用户手册");
    homepage_action.setText("项目主页");
    donate_action.setText("捐赠");
    autostart_action.setText("开机启动");
    change_mode_all_action.setText("全局");
    change_mode_custom_action.setText("自定义");
    change_mode_none_action.setText("禁用");
    search_action.setText("文字翻译 " + configTool->SearchBarShortCutString);
    ocr_translate_action.setText("截图翻译 " + configTool->OCRTranslateShortCutString);
    ocr_text_action.setText("文字识别 " + configTool->OCRTextShortCutString);
    setIcon(icon);
    if (configTool->GetMode() == Mode_ALL)
        change_mode_all_action.setText("✓ 全局");
    else if (configTool->GetMode() == Mode_CUSTOM)
        change_mode_custom_action.setText("✓ 自定义");
    else if (configTool->GetMode() == Mode_NONE)
    {
        change_mode_none_action.setText("✓ 禁用");
        setIcon(icon_grey);
    }



    modeSubMenu.addAction(&change_mode_all_action);
    modeSubMenu.addAction(&change_mode_custom_action);
    modeSubMenu.addAction(&change_mode_none_action);
    menu.addMenu(&modeSubMenu);


    menu.addAction(&search_action);
    menu.addAction(&ocr_translate_action);
    menu.addAction(&ocr_text_action);
    menu.addAction(&config_action);
    menu.addAction(&autostart_action);

    helpSubMenu.addAction(&help_action);
    helpSubMenu.addAction(&homepage_action);
    helpSubMenu.addAction(&donate_action);
    menu.addMenu(&helpSubMenu);

    menu.addAction(&quit_action);

    setContextMenu(&menu);

    if (QFile::exists(QDir::homePath() + "/.config/autostart/CuteTranslation.desktop"))
    {
        autostart_action.setText("✓ 开机启动");
    }



    connect(&donate_action, &QAction::triggered, this, []{
        QDesktopServices::openUrl(QUrl("https://github.com/jiangzc/CuteTranslation#%E6%8D%90%E8%B5%A0"));
        MessageBox::information("提示", "已在浏览器中打开网页");
    });

    connect(&homepage_action, &QAction::triggered, this, []{
        QDesktopServices::openUrl(QUrl("https://github.com/jiangzc/CuteTranslation"));
        MessageBox::information("提示", "已在浏览器中打开网页");
    });

    connect(&help_action, &QAction::triggered, this, []{
        QProcess::startDetached("xdg-open", QStringList(appDir.filePath("guide.txt")));
    });

    connect(&autostart_action, &QAction::triggered, this, [this]{
        if (QFile::exists(QDir::homePath() + "/.config/autostart/CuteTranslation.desktop") == false)
        {
            int res = QFile::copy(appDir.filePath("CuteTranslation.desktop"),
                        QDir::homePath() + "/.config/autostart/CuteTranslation.desktop");
            if (res)
            {
                MessageBox::information("提示", "添加成功");
                this->autostart_action.setText("✓ 开机启动");
            }
            else
            {
                MessageBox::information("提示", "无法添加 " + QString::number(res));
            }
        }
        else
        {
            int res = QFile::remove(QDir::homePath() + "/.config/autostart/CuteTranslation.desktop");
            if (res)
            {
                MessageBox::information("提示", "取消成功");
                this->autostart_action.setText("开机启动");
            }
            else
            {
                MessageBox::information("提示", "无法取消 " + QString::number(res));
            }
        }
    });

    connect(&change_mode_all_action, &QAction::triggered, this, []{
        configTool->SetMode(Mode_ALL);
    });

    connect(&change_mode_custom_action, &QAction::triggered, this, []{
        configTool->SetMode(Mode_CUSTOM);
    });

    connect(&change_mode_none_action, &QAction::triggered, this, []{
        configTool->SetMode(Mode_NONE);
    });

    connect(&quit_action, &QAction::triggered, this, []{
        xdotool->eventMonitor.terminate();
        xdotool->eventMonitor.wait();
        qInfo() << "---------- Exit --------------";
        qApp->quit();
    });

    this->show();
}

void SystemTrayIcon::OnModeChanged(ModeSet mode)
{
    change_mode_all_action.setText("全局");
    change_mode_custom_action.setText("自定义");
    change_mode_none_action.setText("禁用");
    setIcon(icon);
    if (mode == Mode_ALL)
        change_mode_all_action.setText("✓ 全局");
    else if (mode == Mode_CUSTOM)
        change_mode_custom_action.setText("✓ 自定义");
    else if (mode == Mode_NONE)
    {
        change_mode_none_action.setText("✓ 禁用");
        setIcon(icon_grey);
    }
}

