#include <QCheckBox>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QScreen>
#include <QFileInfo>
#include <QDir>


#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{


    /****** 样式设计 ******/

    // 关闭标题栏、任务栏，对话框背景透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 复选框样式
    changeStyle();

    // 最小化到托盘图标
    createSystemTray();
    

    // TODO:读取配置文件，实现初始化窗体位置
    initSettings();

    /*********************/

    //每到新一天，程序重新启动一次（作刷新用）
    NewDay();

    //读取配置文件,初始化按钮
    initList();
    refreshList();

    ui->setupUi(this);

}

Dialog::~Dialog()
{
    delete ui;
}

Dialog::addItem(int y, QString text) {
    QCheckBox *cb = new QCheckBox(this);
    cb->move(10, y*60 + 80);
    cb->setText(text);
    this->boxNum++;
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(on_checkBox_clicked(int)));
}

void Dialog::on_checkBox_clicked(int state) {
    if(state == Qt::Checked){
        this->checkedCount++;
    }
    else{
        this->checkedCount--;
    }
    if(this->boxNum != 0 && this->checkedCount == this->boxNum){
        this->setVisible(false);
    }
}

void Dialog::NewDay() {
    data_time = new QTimer(this);
    QDateTime storedTime = QDateTime::currentDateTime();
    storedTimeStr = storedTime.toString("dd");
    connect(data_time, &QTimer::timeout, this, &Dialog::refresh);
    data_time->start(1000);
}


void Dialog::refresh() {
    QDateTime current_time = QDateTime::currentDateTime();
    QString curStr = current_time.toString(("dd"));
    if(curStr != storedTimeStr) {
        qApp->exit(RETCODE_RESTART);
    }
    storedTimeStr = curStr;
}

void Dialog::loadJsonFile() {
    QFile file(".\\list.json");
    file.open(QIODevice::ReadOnly);
    QByteArray data(file.readAll());
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qDebug()<<"Error config!";
        return;
    }

    QJsonObject obj = doc.object();
    if(obj.contains("Title")) {
        this->title = obj.value("Title").toString();
    }
    if(obj.contains("List")) {
        QJsonValue arrayTmp = obj.value("List");
        QJsonArray array = arrayTmp.toArray();
        for(int i = 0; i < array.size(); i++) {
            QJsonValue sub = array.at(i);
            QJsonObject subObj = sub.toObject();
            QJsonValue contextTmp = subObj.value("context");
            QString context = contextTmp.toString();
            QJsonValue idTmp = subObj.value("id");
            int id = idTmp.toInt();
            this->list[i].id = id;
            this->list[i].context = context;
        }
    }
}

void Dialog::initList(){
    for(int i = 0; i < LIST_COUNT; i++) {
        this->list[i].id = -1;
    }
}

void Dialog::refreshList() {
    loadJsonFile();
    int i = 0;

    // Display label
    this->titleLabel = new QLabel(this);
    this->titleLabel->setText(this->title);
    this->titleLabel->move(0, 0);

    for(; this->list[i].id != -1; i++)
        addItem(i+1, this->list[i].context);

}

void Dialog::changeStyle() {
    QFile file(".\\style.qss");
    if(file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        this->setStyleSheet(qss);
        qDebug()<<qss;
        file.close();
    }

}

void Dialog::createSystemTray(){
    // 初始化按钮
    QAction *autoRunAction = new QAction(QString("开机启动"));
    autoRunAction->setCheckable(true);
    QAction *quitAction = new QAction(QString("退出"));

    connect(autoRunAction, SIGNAL(triggered(bool)), this, SLOT(isAutoRunChecked(bool)));
    connect(quitAction, &QAction::triggered, this, [=](){
        QApplication::exit(0);
    });

    // 初始化菜单并添加项
    QMenu *trayMenu = new QMenu(this); //菜单.
    trayMenu->addAction(autoRunAction);
    trayMenu->addAction(quitAction);

    // 创建系统托盘
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(".\\moquan.png"));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void Dialog::initSettings() {
    QScreen *screen = qApp->primaryScreen();

    // 读取配置文件
    this->settings = new QSettings(".\\config.ini", QSettings::IniFormat);
    settings->beginGroup("POSITION");
    float xDivision = settings->value("xDivision").toFloat();
    float yDivition = settings->value("yDivision").toFloat();
    settings->endGroup();

    xPos = screen->size().width() * xDivision;
    yPos = screen->size().height() * yDivition;

    this->move(xPos, yPos);
}

void Dialog::setProcessAutoRunSelf(const QString &appPath, bool isChecked) {
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::Registry64Format);

    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    // toNativeSeparators: 将 "/" 替换为 "\"
    QString newPath = QDir::toNativeSeparators(appPath);
    if(isChecked) {
        if(path != newPath) {
            settings.setValue(name, newPath);
        }
    }
    else{
        settings.remove(name);
    }
}

void Dialog::isAutoRunChecked(bool isChecked){
    if(isChecked)
        !isChecked;
    this->setProcessAutoRunSelf(qApp->applicationFilePath(), isChecked);
}

// 窗口嵌入桌面
//PDWORD_PTR result = nullptr;
//HWND hWorkerW = nullptr;
//HWND hDefView = nullptr;

//SendMessageTimeout(FindWindow(L"Progman",NULL), 0x52c, 0, 0, SMTO_NORMAL, 1000, result);

//hWorkerW = FindWindowEx(NULL, NULL, L"WorkerW", NULL);

//    while ((!hDefView) && hWorkerW)
//    {
//        hDefView = FindWindowEx(hWorkerW, NULL, L"SHELLDLL_DefView", NULL);
//        hWorkerW = FindWindowEx(NULL, hWorkerW, L"WorkerW", NULL);
//    }

//    ShowWindow(hWorkerW,0);

//    HWND hwndDesktop = FindWindow(L"Progman",NULL);

//    SetParent((HWND)this->winId(),hwndDesktop);
