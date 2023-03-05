#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>
#include <QSettings>
#include <QLabel>

#include <windows.h>

static const int RETCODE_RESTART = 773;
#define LIST_COUNT 20

typedef struct item{
    int id;
    QString context;
}Item;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    int boxNum = 0;       // 选项数量
    int checkedCount = 0; // 计数值
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    addItem(int y, QString text);

    void initList();
    void refreshList();

private:
    QSettings *settings;

    int xPos;
    int yPos;

    QString storedTimeStr;
    QTimer *data_time;

    QString title;
    QLabel *titleLabel;

    Item list[LIST_COUNT];

    // 右键菜单栏选项
    QAction *m_pActionQuit;

    void changeStyle();
    void loadJsonFile();

    Ui::Dialog *ui;
    void NewDay();
    void refresh();

    void createSystemTray();

    void initSettings();

    void setProcessAutoRunSelf(const QString &appPath, bool isChecked);
    void isAutoRunChecked(bool isChecked);

private slots:
    void on_checkBox_clicked(int);
};

#endif // DIALOG_H
