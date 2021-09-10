#ifndef WORDPAGE_H
#define WORDPAGE_H

#include <QWidget>
#include <QVector>
#include <QLabel>

class FlowLayout;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QMediaPlayer;
class QBuffer;

class WordPage : public QWidget
{
    Q_OBJECT
public:
    WordPage(QWidget *parent = nullptr);
    void initUI();
    QLabel *titleLabel;
    QLabel *leftAudioLabel;
    QLabel *rightAudioLabel;
    QVBoxLayout *mainlayout;
    QMediaPlayer *player;
    void updateDescription(const QJsonObject &obj);
    QString getText();
private:
    QVector<QHBoxLayout*> descriptions;
    QVector<QLabel *> tags;
    FlowLayout *tagslayout;
    QBuffer *voiceBuffer;
    void paintEvent(QPaintEvent *event);

private slots:
    void onVoiceButtonClicked();

};

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR);
    ~ClickableLabel();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);

};

#endif // WORDPAGE_H
