#include <QApplication>
#include <QDebug>
#include "shortcut.h"
#include "configtool.h"
#include "xdotool.h"
#include <algorithm>


ShortCut::ShortCut()
{
    // keyCodes是双向队列，记录最近按下的3个键
    keyCodes.push_back(0);
    keyCodes.push_back(0);
    keyCodes.push_back(0);
    // 获取 keyName -> keyCode 映射
    auto keyMap = xdotool->getKeyMap();
    // 加载 FloatBarShortCut
    auto shortcut = configTool->SearchBarShortCutString.split("+", QString::SkipEmptyParts);
    for (auto &it : shortcut)
    {
        if (keyMap.find(it) != keyMap.end())
            SearchBarShortCut.push_back(keyMap.at(it));
        else
        {
            qInfo() << "Unknow keyName: " + it;
            qApp->quit();
        }
    }
    // 加载 OCRTranslateShortCut
    shortcut = configTool->OCRTranslateShortCutString.split("+", QString::SkipEmptyParts);
    for (auto &it : shortcut)
    {
        if (keyMap.find(it) != keyMap.end())
            OCRTranslateShortCut.push_back(keyMap.at(it));
        else
        {
            qInfo() << "Unknow keyName: " + it;
            qApp->quit();
        }
    }
    // 加载 OCRTextShortCut
    shortcut = configTool->OCRTextShortCutString.split("+", QString::SkipEmptyParts);
    for (auto &it : shortcut)
    {
        if (keyMap.find(it) != keyMap.end())
            OCRTextShortCut.push_back(keyMap.at(it));
        else
        {
            qInfo() << "Unknow keyName: " + it;
            qApp->quit();
        }
    }
}

void ShortCut::onKeyPressed(int keyCode)
{
    keyCodes.pop_front();
    keyCodes.push_back(keyCode);
    if (std::equal(SearchBarShortCut.cbegin(), SearchBarShortCut.cend(),
                   keyCodes.cbegin() + 3 - int(SearchBarShortCut.size())))
    {
        qInfo() << "SearchBarShortCut";
        emit SearchBarShortCutPressed();
    }

    if (std::equal(OCRTranslateShortCut.cbegin(), OCRTranslateShortCut.cend(),
                   keyCodes.cbegin() + 3 - int(OCRTranslateShortCut.size())))
    {
        qInfo() << "OCRTranslateShortCut";
        emit OCRTranslateShortCutPressed();
    }
    if (std::equal(OCRTextShortCut.cbegin(), OCRTextShortCut.cend(),
                   keyCodes.cbegin() + 3 - int(OCRTextShortCut.size())))
    {
        qInfo() << "OCRTextShortCut";
        emit OCRTextShortCutPressed();
    }
}

void ShortCut::onKeyReleased(int keyCode)
{
    auto releasedCode = std::find(keyCodes.begin(), keyCodes.end(), keyCode);
    if (releasedCode != keyCodes.end())
    {
        *releasedCode = 0;
    }

}
