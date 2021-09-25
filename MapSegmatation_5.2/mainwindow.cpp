#include "mainwindow.h"
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QToolBar>
#include <QSize>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <queue>
#include <vector>
#include <QPalette>
#include <string>
#include <utility>
#include <stack>
#include <QLibrary>
#include "shp_writer.h"
#include <list>

MainWindow::MainWindow(int color_num, int num_per_row, int block_size, int min_white, QWidget *parent) : QMainWindow(parent)
{
    display_num_color = color_num;
    display_per_row = num_per_row;
    square_size = block_size;
    white_threshold = min_white;
    if (color_num%num_per_row != 0)
    {
        display_num_color = 40;
        display_per_row = 20;
        square_size = 64;
        qDebug() << "color_num%num_per_row != 0. please quit the program";
    }
    else if (color_num > max_display_num_color)
    {
        qDebug() << "display_color_num is too big, replaced by "<<max_display_num_color;
        display_num_color = max_display_num_color;
    }
    else if (color_num < 20)
    {
        qDebug() << "display_color_num is too small, replaced by 20";
        display_num_color = 20;
    }

    if (num_per_row > 30)
    {
        qDebug() << "num_per_row is too big, replaced by 30";
        display_per_row = 30;
    }
    else if (num_per_row < 10)
    {
        qDebug() << "num_per_row is too small, replaced by 10";
        display_per_row = 10;
    }

    if (block_size > 80)
    {
        qDebug() << "block_size is too big, replaced by 64";
        square_size = 80;
    }
    else if (block_size < 40)
    {
        qDebug() << "block_size is too small, replaced by 40";
        square_size = 40;
    }
    if (min_white > 250)
    {
        qDebug() << "min white color is too big, replaced by 250";
        white_threshold = 250;
    }
    else if (min_white < 144)
    {
        qDebug() << "min white color is too small, replaced by 144";
        white_threshold = 144;
    }
    qDebug()<<"display_color_num:"<<display_num_color<<"  display_color_num_per_row:"<<display_per_row<<"  display_color_square_size:"<<square_size<<"  white threshold:"<<white_threshold;
    //this->resize(QSize(800, 600));
    this->showMaximized();
    this->setWindowFlags(Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

    OpenAction = new QAction(tr("&Open"), this);
    OpenAction->setShortcut(QKeySequence::Open);
    OpenAction->setStatusTip(tr("Open　a　file."));
    SaveAction = new QAction(tr("&save"), this);
    SaveAction->setShortcut(QKeySequence::Save);
    SaveAction->setStatusTip(tr("save file"));
    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(OpenAction);
    file->addAction(SaveAction);

    QMenu *FilterMenu = new QMenu();
    FilterMenu = menuBar()->addMenu(tr("Filter"));
    QMenu *median = FilterMenu->addMenu(tr("MedianFilter"));
    median->setStatusTip(tr("use median Filter, Please select keneal size!"));
    QMenu *major = FilterMenu->addMenu(tr("MajorFilter"));
    major->setStatusTip(tr("use major Filter, Please select keneal size!"));
    median_3_3 = new QAction(tr("3*3"), this);
    median_5_5 = new QAction(tr("5*5"), this);
    median_7_7 = new QAction(tr("7*7"), this);
    median_9_9 = new QAction(tr("9*9"), this);
    median_11_11 = new QAction(tr("11*11"), this);
    median->addAction(median_3_3);
    median->addAction(median_5_5);
    median->addAction(median_7_7);
    median->addAction(median_9_9);
    median->addAction(median_11_11);
    major_3_3 = new QAction(tr("3*3"), this);
    major_5_5 = new QAction(tr("5*5"), this);
    major_7_7 = new QAction(tr("7*7"), this);
    major_9_9 = new QAction(tr("9*9"), this);
    major_11_11 = new QAction(tr("11*11"), this);
    major_13_13 = new QAction(tr("13*13"), this);
    major_15_15 = new QAction(tr("15*15"), this);
    major->addAction(major_3_3);
    major->addAction(major_5_5);
    major->addAction(major_7_7);
    major->addAction(major_9_9);
    major->addAction(major_11_11);
    major->addAction(major_13_13);
    major->addAction(major_15_15);

    QMenu *Process = menuBar()->addMenu(tr("Process"));
    FindColorAction = new QAction(tr("FindColor"), this);
    //FindColorAction->setStatusTip(tr("Find all main colors"));
    EraseColorByNearestColorAction = new QAction(tr("EraseColorByNearestColor"), this);
    EraseSpecificColorAction = new QAction(tr("EraseSpecificColor"), this);
    EraseAllBelowColorAction = new QAction(tr("EraseAllBelowColor"), this);
    EraseAllUnshowColorAction = new QAction(tr("EraseAllUnshowColor"), this);
    ExtractColorAction = new QAction(tr("ExtractColor"), this);
    //ExtractColorAction->setStatusTip(tr("Extract aeras belonging to selected colors"));
    ConvertColorAction = new QAction(tr("ConvertColor"), this);
    //ConvertColorAction->setStatusTip(tr("convert selected colors to filled color(especially for background)"));
    EraseAllSmallContourAction = new QAction(tr("EraseAllSmallContour"), this);
    AddColorFromBinaryImageAction = new QAction(tr("AddColorFromBinaryImage"), this);
    Process->addAction(FindColorAction);
    Process->addAction(EraseColorByNearestColorAction);
    Process->addAction(EraseSpecificColorAction);
    Process->addAction(EraseAllBelowColorAction);
    Process->addAction(EraseAllUnshowColorAction);
    Process->addAction(ExtractColorAction);
    Process->addAction(ConvertColorAction);
    Process->addAction(EraseAllSmallContourAction);
    Process->addAction(AddColorFromBinaryImageAction);

    QMenu *FillMenu = menuBar()->addMenu(tr("Fill"));
    see_white = new QAction(tr("see white area"), this);
    FillMenu->addAction(see_white);
    QMenu *disk = FillMenu->addMenu(tr("disk"));
    disk_5_5 = new QAction(tr("5*5"), this);
    disk_7_7 = new QAction(tr("7*7"), this);
    disk_9_9 = new QAction(tr("9*9"), this);
    disk_11_11 = new QAction(tr("11*11"), this);
    disk->addAction(disk_5_5);
    disk->addAction(disk_7_7);
    disk->addAction(disk_9_9);
    disk->addAction(disk_11_11);
    QMenu *square = FillMenu->addMenu(tr("square"));
    square_5_5 = new QAction(tr("5*5"), this);
    square_7_7 = new QAction(tr("7*7"), this);
    square_9_9 = new QAction(tr("9*9"), this);
    square_11_11 = new QAction(tr("11*11"), this);
    square->addAction(square_5_5);
    square->addAction(square_7_7);
    square->addAction(square_9_9);
    square->addAction(square_11_11);

    QMenu *MorphologyOperation = menuBar()->addMenu(tr("Morphology"));
    QMenu *color_close_open = MorphologyOperation->addMenu(tr("color_close_open"));
    QMenu *binary_open = MorphologyOperation->addMenu(tr("binary_open"));
    QMenu *binary_close = MorphologyOperation->addMenu(tr("binary_close"));
    QMenu *color_close_open_disk = color_close_open->addMenu(tr("disk"));
    QMenu *open_disk = binary_open->addMenu(tr("disk"));
    QMenu *open_square = binary_open->addMenu(tr("square"));
    QMenu *close_disk = binary_close->addMenu(tr("disk"));
    QMenu *close_square = binary_close->addMenu(tr("square"));
    close_open_disk_5_5 = new QAction(tr("5*5"), this);
    close_open_disk_7_7 = new QAction(tr("7*7"), this);
    close_open_disk_9_9 = new QAction(tr("9*9"), this);
    color_close_open_disk->addAction(close_open_disk_5_5);
    color_close_open_disk->addAction(close_open_disk_7_7);
    color_close_open_disk->addAction(close_open_disk_9_9);
    open_disk_3_3 = new QAction(tr("3*3"), this);
    open_disk_5_5 = new QAction(tr("5*5"), this);
    open_disk_7_7 = new QAction(tr("7*7"), this);
    open_disk->addAction(open_disk_3_3);
    open_disk->addAction(open_disk_5_5);
    open_disk->addAction(open_disk_7_7);
    open_square_3_3 = new QAction(tr("3*3"), this);
    open_square_5_5 = new QAction(tr("5*5"), this);
    open_square_7_7 = new QAction(tr("7*7"), this);
    open_square->addAction(open_square_3_3);
    open_square->addAction(open_square_5_5);
    open_square->addAction(open_square_7_7);
    close_disk_3_3 = new QAction(tr("3*3"), this);
    close_disk_5_5 = new QAction(tr("5*5"), this);
    close_disk_7_7 = new QAction(tr("7*7"), this);
    close_disk->addAction(close_disk_3_3);
    close_disk->addAction(close_disk_5_5);
    close_disk->addAction(close_disk_7_7);
    close_square_3_3 = new QAction(tr("3*3"), this);
    close_square_5_5 = new QAction(tr("5*5"), this);
    close_square_7_7 = new QAction(tr("7*7"), this);
    close_square->addAction(close_square_3_3);
    close_square->addAction(close_square_5_5);
    close_square->addAction(close_square_7_7);

    QMenu *Step = menuBar()->addMenu(tr("Step"));
    ReturnAction = new QAction(tr("&Return"), this);
    ReturnAction->setStatusTip(tr("return to last step"));
    Step->addAction(ReturnAction);
    NextAction = new QAction(tr("&Next"), this);
    NextAction->setStatusTip(tr("return to next step"));
    Step->addAction(NextAction);

    BinaryToSVG = new QAction(tr("BinaryToSVG"), this);
    AllToSVG = new QAction(tr("AllToSVG"), this);
    AllToSHP = new QAction(tr("AllToSHP"), this);
    SaveAllBinaryImage = new QAction(tr("SaveAllBinaryImage"), this);
    //SaveWithDilate3x3 = new QAction(tr("SaveWithDilate3x3"), this);
    //SaveWithDilate5x5 = new QAction(tr("SaveWithDilate5x5"), this);
    QMenu *save_all_binary = menuBar()->addMenu(tr("SaveFormat"));
    save_all_binary->addAction(BinaryToSVG);
    save_all_binary->addAction(AllToSVG);
    save_all_binary->addAction(AllToSHP);
    save_all_binary->addAction(SaveAllBinaryImage);
    //save_all_binary->addAction(SaveWithDilate3x3);
    //save_all_binary->addAction(SaveWithDilate5x5);
    connect(BinaryToSVG, &QAction::triggered, this, &MainWindow::on_BinaryToSvgAction_triggered);
    connect(AllToSVG, &QAction::triggered, this, &MainWindow::on_AllToSVGAction_triggered);
    connect(AllToSHP, &QAction::triggered, this, &MainWindow::on_AllToSHPAction_triggered);
    connect(SaveAllBinaryImage, &QAction::triggered, this, &MainWindow::on_SaveAllBinaryImageAction_triggered);
    //connect(SaveWithDilate3x3, &QAction::triggered, this, &MainWindow::on_SaveWithDilate3x3Action_triggered);
    //connect(SaveWithDilate5x5, &QAction::triggered, this, &MainWindow::on_SaveWithDilate5x5Action_triggered);

    settingLabel = new QLabel;
    settingLabel->setAlignment(Qt::AlignCenter);
    settingLabel->setText(tr("Parms: "));
    settingLabel->setStyleSheet("font-size : 16px");

    minColorAreaRatioLabel = new QLabel;
    minColorAreaRatioLabel->setAlignment(Qt::AlignCenter);
    minColorAreaRatioLabel->setText(tr("MinColorAreaRatio: "));
    minColorAreaRatioLabel->setStyleSheet("font-size : 16px");
    minColorAreaRatio = new QLineEdit;
    minColorAreaRatio->setMaxLength(7);
    minColorAreaRatio->setText("0.00002");
    minColorAreaRatio->setStyleSheet("font-size : 16px");
    QDoubleValidator *minColorAreaRatioValidator = new QDoubleValidator(0.00000, 0.5, 5);
    minColorAreaRatio->setValidator(minColorAreaRatioValidator);

    rightShiftNumLabel = new QLabel;
    rightShiftNumLabel->setAlignment(Qt::AlignCenter);
    rightShiftNumLabel->setText(tr("RightShiftNum: "));
    rightShiftNumLabel->setStyleSheet("font-size : 16px");
    rightShiftNum = new QLineEdit;
    rightShiftNum->setMaxLength(1);
    rightShiftNum->setText("4");
    rightShiftNum->setStyleSheet("font-size : 16px");
    QIntValidator *rightShiftValidator = new QIntValidator(2, 7);
    rightShiftNum->setValidator(rightShiftValidator);

    maxRangeToMergeColorLabel = new QLabel;
    maxRangeToMergeColorLabel->setAlignment(Qt::AlignCenter);
    maxRangeToMergeColorLabel->setText(tr("maxRangeToMergeColor: "));
    maxRangeToMergeColorLabel->setStyleSheet("font-size : 16px");
    maxRangeToMergeColor = new QLineEdit;
    maxRangeToMergeColor->setMaxLength(2);
    maxRangeToMergeColor->setText("16");
    maxRangeToMergeColor->setStyleSheet("font-size : 16px");
    QIntValidator *maxRangeValidator = new QIntValidator(2, 128);
    maxRangeToMergeColor->setValidator(maxRangeValidator);

    maxMainColorNumLabel = new QLabel;
    maxMainColorNumLabel->setAlignment(Qt::AlignCenter);
    maxMainColorNumLabel->setText(tr("MaxEraseWindowSize: "));
    maxMainColorNumLabel->setStyleSheet("font-size : 16px");
    maxMainColorNum = new QLineEdit;
    maxMainColorNum->setMaxLength(2);
    maxMainColorNum->setText("24");
    maxMainColorNum->setStyleSheet("font-size : 16px");
    QIntValidator *maxMainColorValidator = new QIntValidator(1, 50);
    maxMainColorNum->setValidator(maxMainColorValidator);

    notDilateLabel = new QLabel;
    notDilateLabel->setAlignment(Qt::AlignCenter);
    notDilateLabel->setText(tr("SaveContourNum: "));
    notDilateLabel->setStyleSheet("font-size : 16px");
    notDilateNum = new QLineEdit;
    notDilateNum->setMaxLength(2);
    notDilateNum->setText("1");
    notDilateNum->setStyleSheet("font-size : 16px");
    QIntValidator *notDilateValidator = new QIntValidator(1, 200);
    notDilateNum->setValidator(notDilateValidator);

    OK = new QPushButton;
    OK->setText("Change");
    OK->setStyleSheet("background: rgb(28,134,240); color: black; font-size : 16px");
    connect(OK, &QPushButton::clicked, this, &MainWindow::change_setting);

    convertColorLabel = new QLabel;
    convertColorLabel->setAlignment(Qt::AlignCenter);
    convertColorLabel->setText(tr(" ConvertRGB:"));
    convertColorLabel->setStyleSheet("font-size : 16px");
    convertColorR = new QLineEdit;
    convertColorR->setMaxLength(3);
    convertColorR->setText("255");
    convertColorR->setStyleSheet("font-size : 16px");
    convertColorG = new QLineEdit;
    convertColorG->setMaxLength(3);
    convertColorG->setText("255");
    convertColorG->setStyleSheet("font-size : 16px");
    convertColorB = new QLineEdit;
    convertColorB->setMaxLength(3);
    convertColorB->setText("255");
    convertColorB->setStyleSheet("font-size : 16px");
    QIntValidator *ColorValidator = new QIntValidator(0, 255);
    convertColorR->setValidator(ColorValidator);
    convertColorG->setValidator(ColorValidator);
    convertColorB->setValidator(ColorValidator);

    eraseColorLabel = new QLabel;
    eraseColorLabel->setAlignment(Qt::AlignCenter);
    eraseColorLabel->setText(tr(" EraseRGB:"));
    eraseColorLabel->setStyleSheet("font-size : 16px");
    eraseColorR = new QLineEdit;
    eraseColorR->setMaxLength(3);
    eraseColorR->setText("255");
    eraseColorR->setStyleSheet("font-size : 16px");
    eraseColorG = new QLineEdit;
    eraseColorG->setMaxLength(3);
    eraseColorG->setText("255");
    eraseColorG->setStyleSheet("font-size : 16px");
    eraseColorB = new QLineEdit;
    eraseColorB->setMaxLength(3);
    eraseColorB->setText("255");
    eraseColorB->setStyleSheet("font-size : 16px");
    eraseColorR->setValidator(ColorValidator);
    eraseColorG->setValidator(ColorValidator);
    eraseColorB->setValidator(ColorValidator);

    QToolBar *setting = this->addToolBar(tr("file"));
    setting->addWidget(settingLabel);
    setting->addSeparator();
    setting->addWidget(minColorAreaRatioLabel);
    setting->addWidget(minColorAreaRatio);
    setting->addSeparator();
    setting->addWidget(rightShiftNumLabel);
    setting->addWidget(rightShiftNum);
    setting->addSeparator();
    setting->addWidget(maxRangeToMergeColorLabel);
    setting->addWidget(maxRangeToMergeColor);
    setting->addSeparator();
    setting->addWidget(maxMainColorNumLabel);
    setting->addWidget(maxMainColorNum);
    setting->addSeparator();
    setting->addWidget(notDilateLabel);
    setting->addWidget(notDilateNum);
    setting->addSeparator();
    setting->addWidget(OK);
    setting->addSeparator();
    setting->addWidget(convertColorLabel);
    setting->addWidget(convertColorR);
    setting->addWidget(convertColorG);
    setting->addWidget(convertColorB);
    setting->addSeparator();
    setting->addWidget(eraseColorLabel);
    setting->addWidget(eraseColorR);
    setting->addWidget(eraseColorG);
    setting->addWidget(eraseColorB);
    setting->setFixedWidth(1850);

    connect(OpenAction, &QAction::triggered, this, &MainWindow::on_OpenAction_triggered);
    connect(SaveAction, &QAction::triggered, this, &MainWindow::on_SaveAction_triggered);

    connect(median_3_3, &QAction::triggered, this, &MainWindow::on_median_3_3_triggered);
    connect(median_5_5, &QAction::triggered, this, &MainWindow::on_median_5_5_triggered);
    connect(median_7_7, &QAction::triggered, this, &MainWindow::on_median_7_7_triggered);
    connect(median_9_9, &QAction::triggered, this, &MainWindow::on_median_9_9_triggered);
    connect(median_11_11, &QAction::triggered, this, &MainWindow::on_median_11_11_triggered);
    connect(major_3_3, &QAction::triggered, this, &MainWindow::on_major_3_3_triggered);
    connect(major_5_5, &QAction::triggered, this, &MainWindow::on_major_5_5_triggered);
    connect(major_7_7, &QAction::triggered, this, &MainWindow::on_major_7_7_triggered);
    connect(major_9_9, &QAction::triggered, this, &MainWindow::on_major_9_9_triggered);
    connect(major_11_11, &QAction::triggered, this, &MainWindow::on_major_11_11_triggered);
    connect(major_13_13, &QAction::triggered, this, &MainWindow::on_major_13_13_triggered);
    connect(major_15_15, &QAction::triggered, this, &MainWindow::on_major_15_15_triggered);

    connect(FindColorAction, &QAction::triggered, this, &MainWindow::on_FindColorAction_triggered);
    connect(EraseColorByNearestColorAction, &QAction::triggered, this, &MainWindow::on_EraseColorByNearestColorAction_triggered);
    connect(EraseSpecificColorAction, &QAction::triggered, this, &MainWindow::on_EraseSpecificColorAction_triggered);
    connect(EraseAllBelowColorAction, &QAction::triggered, this, &MainWindow::on_EraseAllBelowColorAction_triggered);
    connect(EraseAllUnshowColorAction, &QAction::triggered, this, &MainWindow::on_EraseAllUnshowColorAction_triggered);
    connect(ExtractColorAction, &QAction::triggered, this, &MainWindow::on_ExtractColorAction_triggered);
    connect(ConvertColorAction, &QAction::triggered, this, &MainWindow::on_ConvertColorAction_triggered);
    connect(EraseAllSmallContourAction, &QAction::triggered, this, &MainWindow::on_EraseAllSmallContourAction_triggered);
    connect(AddColorFromBinaryImageAction, &QAction::triggered, this, &MainWindow::on_AddColorFromBinaryImageAction_triggered);

    connect(see_white, &QAction::triggered, this, &MainWindow::see_white_areas);
    connect(disk_11_11, &QAction::triggered, this, &MainWindow::on_disk_11_11_triggered);
    connect(disk_7_7, &QAction::triggered, this, &MainWindow::on_disk_7_7_triggered);
    connect(disk_9_9, &QAction::triggered, this, &MainWindow::on_disk_9_9_triggered);
    connect(square_11_11, &QAction::triggered, this, &MainWindow::on_square_11_11_triggered);
    connect(square_7_7, &QAction::triggered, this, &MainWindow::on_square_7_7_triggered);
    connect(square_9_9, &QAction::triggered, this, &MainWindow::on_square_9_9_triggered);

    connect(close_open_disk_5_5, &QAction::triggered, this, &MainWindow::on_close_open_disk_5_5_triggered);
    connect(close_open_disk_7_7, &QAction::triggered, this, &MainWindow::on_close_open_disk_7_7_triggered);
    connect(close_open_disk_9_9, &QAction::triggered, this, &MainWindow::on_close_open_disk_9_9_triggered);
    connect(open_disk_3_3, &QAction::triggered, this, &MainWindow::on_open_disk_3_3_triggered);
    connect(open_disk_5_5, &QAction::triggered, this, &MainWindow::on_open_disk_5_5_triggered);
    connect(open_disk_7_7, &QAction::triggered, this, &MainWindow::on_open_disk_7_7_triggered);
    connect(open_square_3_3, &QAction::triggered, this, &MainWindow::on_open_square_3_3_triggered);
    connect(open_square_5_5, &QAction::triggered, this, &MainWindow::on_open_square_5_5_triggered);
    connect(open_square_7_7, &QAction::triggered, this, &MainWindow::on_open_square_7_7_triggered);
    connect(close_disk_3_3, &QAction::triggered, this, &MainWindow::on_close_disk_3_3_triggered);
    connect(close_disk_5_5, &QAction::triggered, this, &MainWindow::on_close_disk_5_5_triggered);
    connect(close_disk_7_7, &QAction::triggered, this, &MainWindow::on_close_disk_7_7_triggered);
    connect(close_square_3_3, &QAction::triggered, this, &MainWindow::on_close_square_3_3_triggered);
    connect(close_square_5_5, &QAction::triggered, this, &MainWindow::on_close_square_5_5_triggered);
    connect(close_square_7_7, &QAction::triggered, this, &MainWindow::on_close_square_7_7_triggered);

    connect(ReturnAction, &QAction::triggered, this, &MainWindow::on_ReturnAction_triggered);
    connect(NextAction, &QAction::triggered, this, &MainWindow::on_NextAction_triggered);

    QVBoxLayout *MainLayout = new QVBoxLayout;
    QVBoxLayout *ColorVLayout = new QVBoxLayout;
    QHBoxLayout *ColorHLayout[static_cast<int>(display_num_color/display_per_row)];
    for (int i=0; i<static_cast<int>(display_num_color/display_per_row); i++)
    {
        ColorHLayout[i] = new QHBoxLayout;
    }
    noneColor.load(":/none.png");
    QSize pixmap1Size(square_size, square_size);
    noneColor = noneColor.scaled(pixmap1Size,Qt::KeepAspectRatio);
    QVBoxLayout *ColorMulLayout[display_num_color];
    for (int i=0; i<display_num_color; i++)
    {
        ColorMulLayout[i] = new QVBoxLayout;
        rgb[i] = new QLabel();
        QFont ft;
        if (square_size >= 60)
            ft.setPointSize(10);
        else
            ft.setPointSize(8);
        rgb[i]->setFont(ft);
        rgb[i]->setText(tr("000,000,000")); //QString::number(0)
        select[i] = new QCheckBox();
        color[i] = new QLabel();
        color[i]->setFixedSize(square_size,square_size);
        //color[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        //color[i]->setStyleSheet("QLabel{background:brown;}");
        color[i]->setPixmap(noneColor);
        ColorMulLayout[i]->addWidget(color[i], 0, Qt::AlignCenter);
        ColorMulLayout[i]->addWidget(rgb[i], 0, Qt::AlignCenter);
        ColorMulLayout[i]->addWidget(select[i], 0, Qt::AlignCenter);

        ColorHLayout[static_cast<int>(i/display_per_row)]->addLayout(ColorMulLayout[i]);
    }
    for (int i=0; i<static_cast<int>(display_num_color/display_per_row); i++)
    {
        ColorVLayout->addLayout(ColorHLayout[i]);
    }

    ImageLabel = new QLabel();
    //ImageLabel->setFixedSize(400,400);
    ImageLabel->setAlignment(Qt::AlignCenter);
    MainLayout->addLayout(ColorVLayout);
    MainLayout->addStretch();
    MainLayout->addWidget(ImageLabel);
    MainLayout->addStretch();
    QWidget* MainWidget = new QWidget();
    MainWidget->setLayout(MainLayout);
    MainWidget->setAttribute(Qt::WA_DeleteOnClose);
    setCentralWidget(MainWidget);

    for (unsigned int i=0; i<max_history_step; i++)
    {
        image_history[i].image_array = nullptr;
        image_history[i].image_type = "nullptr";
    }
}

void MainWindow::change_setting()
{
    min_color_area_ratio = minColorAreaRatio->text().toDouble();
    int a = rightShiftNum->text().toInt();
    bit_mov_right = static_cast<short>(a);
    max_range_to_merge_color = static_cast<unsigned int>(maxRangeToMergeColor->text().toInt());
    int c = maxMainColorNum->text().toInt();
    max_erase_win_size = static_cast<unsigned char>(c);
    notdilate = static_cast<int>(notDilateNum->text().toInt());
}

void MainWindow::on_SaveAllBinaryImageAction_triggered()
{
    QString dir_str = "binary_image";
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = false;
        while(!res) {
            res = dir.mkpath(dir_str);
        }
        qDebug() << "make new dir: ./binary_image";
    }
    unsigned int color_index;
    unsigned char r,g,b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
        {
            std::vector<int> save_color;
            save_color.push_back(i);
            selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);
            color_index = sorted_color[i].at(0).color_index;
            b = static_cast<unsigned char>(color_index / (color_size*color_size));
            g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
            r = static_cast<unsigned char>(color_index % color_size);
            delete bin_image;
            bin_image = new QImage(binary_color, img_width, img_height, QImage::Format_Grayscale8);
            QString BinaryName = QString("binary_image/%1_%2_%3.png").arg(r*ratio).arg(g*ratio).arg(b*ratio);
            bin_image->save(BinaryName, "PNG");
        }
    }
    delete [] binary_color;
    return;
}

void MainWindow::on_SaveWithDilate3x3Action_triggered()
{
    QString dir_str = "binary_image";
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = false;
        while(!res) {
            res = dir.mkpath(dir_str);
        }
        qDebug() << "make new dir: ./binary_image";
    }
    unsigned int color_index;
    unsigned char r,g,b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
        {
            std::vector<int> save_color;
            save_color.push_back(i);
            selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);
            //if (i >= notdilate)
            dilate(binary_color, 0, img_height, img_width, 3);
            color_index = sorted_color[i].at(0).color_index;
            b = static_cast<unsigned char>(color_index / (color_size*color_size));
            g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
            r = static_cast<unsigned char>(color_index % color_size);
            delete bin_image;
            bin_image = new QImage(binary_color, img_width, img_height, QImage::Format_Grayscale8);
            QString BinaryName = QString("binary_image/%1_%2_%3.png").arg(r*ratio).arg(g*ratio).arg(b*ratio);
            bin_image->save(BinaryName, "PNG");
        }
    }
    delete [] binary_color;
    return;
}
void MainWindow::on_SaveWithDilate5x5Action_triggered()
{
    QString dir_str = "binary_image";
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = false;
        while(!res) {
            res = dir.mkpath(dir_str);
        }
        qDebug() << "make new dir: ./binary_image";
    }
    unsigned int color_index;
    unsigned char r,g,b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
        {
            std::vector<int> save_color;
            save_color.push_back(i);
            selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);
            //if (i >= notdilate)
            //{
            dilate(binary_color, 0, img_height, img_width, 5);
            //}
            color_index = sorted_color[i].at(0).color_index;
            b = static_cast<unsigned char>(color_index / (color_size*color_size));
            g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
            r = static_cast<unsigned char>(color_index % color_size);
            delete bin_image;
            bin_image = new QImage(binary_color, img_width, img_height, QImage::Format_Grayscale8);
            QString BinaryName = QString("binary_image/%1_%2_%3.png").arg(r*ratio).arg(g*ratio).arg(b*ratio);
            bin_image->save(BinaryName, "PNG");
        }
    }
    delete [] binary_color;
    return;
}

void MainWindow::on_OpenAction_triggered()
{
    OpenFileName = QFileDialog::getOpenFileName(this, tr("open file"), "pic", tr("img(*png *jpg *jpeg);;All files (*.*)"));
    if (OpenFileName != "")
    {
        win_width = this->geometry().width();
        win_height = this->geometry().height();
        image = new QImage;
        bin_image = new QImage;
        if (image->load(OpenFileName))
        {
            unsigned char *ImgPointer=image->bits();
            img_width = image->width();
            img_height = image->height();
            if (img_width % 4 != 0)
            {
                int bits_per_line = image->bytesPerLine();
                int resized_width = static_cast<int>(img_width / 4) * 4;
                unsigned char *resize_image = new unsigned char [resized_width*img_height*4];
                for (int i=0; i<img_height; i++)
                {
                    for (int j=0; j<resized_width; j++)
                    {
                        resize_image[i*resized_width*4+j*4+0] = ImgPointer[i*bits_per_line+j*4];
                        resize_image[i*resized_width*4+j*4+1] = ImgPointer[i*bits_per_line+j*4+1];
                        resize_image[i*resized_width*4+j*4+2] = ImgPointer[i*bits_per_line+j*4+2];
                        resize_image[i*resized_width*4+j*4+3] = ImgPointer[i*bits_per_line+j*4+3];
                    }
                }
                delete image;
                image = new QImage(resize_image, resized_width, img_height, QImage::Format_ARGB32);
            }

            for (int i=0; i<display_num_color; i++)
            {
                color[i]->clear();
                color[i]->setPixmap(noneColor);
            }
            //qDebug()<<win_height<<win_width;
            img_width = image->width();
            img_height = image->height();
            int real_win_width = static_cast<int>(0.9 * win_width);
            int real_win_height = static_cast<int>(0.9 * abs(win_height-(40+square_size)*static_cast<int>(display_num_color/display_per_row)));
            float img_ratio = static_cast<float>(img_width) / static_cast<float>(img_height);
            float win_ratio = static_cast<float>(real_win_width) / static_cast<float>(real_win_height);
            if (img_ratio > win_ratio)
            {
                display_img_width = real_win_width;
                display_img_height = static_cast<int>(real_win_width / img_ratio);
            }
            else
            {
                display_img_height = real_win_height;
                display_img_width = static_cast<int>(real_win_height * img_ratio);
            }
            //qDebug()<<display_img_height<<display_img_width;
            QImage resizedImg = image->scaled(display_img_width, display_img_height);
            ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
            SaveImage = image;
        }
    }
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    for (int n=0; n<display_num_color; n++) {sorted_color[n].clear(); std::vector<color_node>().swap(sorted_color[n]);}
    for (unsigned int n=0; n<max_history_step; n++)
    {
        image_history[n].image_type= "nullptr";
        delete [] image_history[n].image_array;
        image_history[n].image_array = nullptr;
    }
    first_find_color = true;
    return;
}

void MainWindow::on_SaveAction_triggered()
{
    SaveFileName = QFileDialog::getSaveFileName(this, tr("save file"), "x_x_x.png", tr("img(*png);;All files (*.*)"));
    SaveImage->save(SaveFileName, "PNG");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}

void MainWindow::autosave(QString name="tmp.png")
{
    if (name == "tmp.png")
    {
        save_count++;
        if (save_count % 2 == 0)
        {
            SaveImage->save(name, "PNG");
            qDebug() << "saved current image as " << name;
            qDebug() << "--------------";
        }
    }
    else
    {
        binary_save_count++;
        if (binary_save_count % 2 == 0)
        {
            SaveImage->save(name, "PNG");
            qDebug() << "saved current image as " << name;
            qDebug() << "--------------";
        }
    }
    return;
}

void MainWindow::on_FindColorAction_triggered()
{
    delete [] ImgArray;
    ImgArray = nullptr;
    delete [] color_histogram;
    color_histogram = nullptr;
    unsigned char *ImgPointer=image->bits();
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned int total_color = static_cast<unsigned int>(pow(color_size, 3));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    ImgArray = new unsigned char [img_height*img_width*3];
    color_histogram = new unsigned int [total_color];
    memset(color_histogram, 0, total_color*sizeof(unsigned int));
    unsigned char r,g,b;

    unsigned char max_color = 0;
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (ImgPointer[0] > max_color) max_color = ImgPointer[0];
            if (ImgPointer[1] > max_color) max_color = ImgPointer[1];
            if (ImgPointer[2] > max_color) max_color = ImgPointer[2];
            ImgPointer += 4;
        }
    }
    ImgPointer=image->bits();
    if (max_color+ ratio/2 > 255)
        need_sub16 = 1;
    else
        need_sub16 = 0;

    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            b = (ImgPointer[0] + ratio/2) / ratio;
            g = (ImgPointer[1] + ratio/2) / ratio;
            r = (ImgPointer[2] + ratio/2) / ratio;
            if (first_find_color && need_sub16) // only sub once during processing
            {
                if (b==0) b = 1;
                if (g==0) g = 1;
                if (r==0) r = 1;
                r--; g--; b--;
            }
            if (r >= white_threshold/ratio && g >= white_threshold/ratio && b >= white_threshold/ratio)
            {
                ImgArray[i*img_width*3+j*3] = color_size-1;
                ImgArray[i*img_width*3+j*3+1] = color_size-1;
                ImgArray[i*img_width*3+j*3+2] = color_size-1;
            }
            else
            {
                *(color_histogram + b*color_size*color_size + g*color_size + r) += 1;
                ImgArray[i*img_width*3+j*3] = b;
                ImgArray[i*img_width*3+j*3+1] = g;
                ImgArray[i*img_width*3+j*3+2] = r;
            }
            ImgPointer += 4;
        }
    }
    first_find_color = false;
    std::priority_queue<color_node, std::vector<color_node>, cmp> queue;
    for (unsigned int i=0; i<total_color; i++)
    {
        if (*(color_histogram+i) > img_height*img_width*min_color_area_ratio){
            color_node tmp;
            tmp.color_freq = *(color_histogram+i);
            tmp.color_index = i;
            queue.push(tmp);
        }
    }
    delete [] color_histogram;
    color_histogram = nullptr;
    qDebug() << "queue_size: " << queue.size();
    //qDebug() << queue.top().color_index << queue.top().color_freq << queue.top().color_freq / (img_height*img_width*1.0);

    for (int n=0; n<display_num_color; n++) {sorted_color[n].clear(); std::vector<color_node>().swap(sorted_color[n]);}
    int i = 0;
    color_node tmp, benchmarker;
    unsigned char r0,g0,b0;
    while(i<display_num_color)
    {
        if (queue.empty()) break;
        tmp = queue.top();
        b = static_cast<unsigned char>(tmp.color_index / (color_size*color_size));
        g = static_cast<unsigned char>((tmp.color_index - b * (color_size * color_size)) / color_size);
        r = static_cast<unsigned char>(tmp.color_index % color_size);
        bool save = true;
        //if ((r >= white_threshold/ratio && g >= white_threshold/ratio && b >= white_threshold/ratio) || (r <= 48/ratio && g <= 48/ratio && b <= 48/ratio) || (r+g+b <= 96/ratio))
        if (r >= white_threshold/ratio && g >= white_threshold/ratio && b >= white_threshold/ratio)
        {
            save = false;
        }
        for (int j=0; j < i; j++)
        {
            unsigned int num_same = sorted_color[j].size();
            if (num_same >2)
                num_same = 2;
            for (unsigned int k=0; k<num_same; ++k)
            {
                benchmarker = sorted_color[j].at(k);
                b0 = static_cast<unsigned char>(benchmarker.color_index / (color_size*color_size));
                g0 = static_cast<unsigned char>((benchmarker.color_index - b0 * (color_size * color_size)) / color_size);
                r0 = static_cast<unsigned char>(benchmarker.color_index % color_size);
                if (static_cast<unsigned int>(abs(r - r0) + abs(g-g0) + abs(b-b0)) <= max_range_to_merge_color/ratio)
                {
                    save = false;
                    sorted_color[j].push_back(tmp);
                    break;
                }
            }
            if (!save) break;
        }
        if (save)
        {
            sorted_color[i].push_back(tmp);
            queue.pop();
            i++;
        }
        else queue.pop();
    }
    qDebug() << "new_queue_size: " << i;
    qDebug() << "--------------";

    for (int k=0; k<display_num_color; k++)
    {
        unsigned int sorted_color_k_size = sorted_color[k].size();
        if (sorted_color_k_size == 0)
            break;
        if (sorted_color_k_size == 1)
            continue;
        unsigned int color_index = sorted_color[k].at(0).color_index;
        b0 = static_cast<unsigned char>(color_index / (color_size*color_size));
        g0 = static_cast<unsigned char>((color_index - b0 * (color_size * color_size)) / color_size);
        r0 = static_cast<unsigned char>(color_index % color_size);
        for (unsigned int m=1; m<sorted_color_k_size; ++m)
        {
            color_index = sorted_color[k].at(m).color_index;
            b = static_cast<unsigned char>(color_index / (color_size*color_size));
            g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
            r = static_cast<unsigned char>(color_index % color_size);
            for (int i=0; i<img_height; i++)
            {
                for (int j=0; j<img_width; j++)
                {
                    if (b==ImgArray[i*img_width*3+j*3] && g==ImgArray[i*img_width*3+j*3+1] && r==ImgArray[i*img_width*3+j*3+2])
                    {
                        ImgArray[i*img_width*3+j*3] = b0;
                        ImgArray[i*img_width*3+j*3+1] = g0;
                        ImgArray[i*img_width*3+j*3+2] = r0;
                    }
                }
            }
        }
    }

    for (int i=0; i<display_num_color; ++i)
    {
        unsigned int num_same = sorted_color[i].size();
        if (num_same == 0)
        {
            color[i]->clear();
            color[i]->setFixedSize(square_size,square_size);
            color[i]->setPixmap(noneColor);
            continue;
        }

        unsigned int benchmarker_color = sorted_color[i].at(0).color_index;
        b = static_cast<unsigned char>(benchmarker_color / (color_size*color_size) * ratio);
        g = static_cast<unsigned char>((benchmarker_color - b * (color_size * color_size)) / color_size * ratio);
        r = static_cast<unsigned char>(benchmarker_color % color_size * ratio);

        color[i]->clear();
        QPalette palette;
        palette.setColor(QPalette::Background, QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b)));
        color[i]->setAutoFillBackground(true);
        color[i]->setPalette(palette);
        QString rgb_value = QString("%1,%2,%3").arg(r).arg(g).arg(b);
        rgb[i]->setText(rgb_value);
    }
    delete [] approximate_img;
    approximate_img = new unsigned char [img_height*img_width*4];
    for (int i=0; i<img_height*img_width; i++)
    {
        approximate_img[i*4+0] = ImgArray[i*3] * ratio;
        approximate_img[i*4+1] = ImgArray[i*3+1] * ratio;
        approximate_img[i*4+2] = ImgArray[i*3+2] * ratio;
        approximate_img[i*4+3] = 255;
    }
    delete image;
    image = new QImage(approximate_img, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    autosave();

    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }

    if (max_history_step <= current_image_index+1)
    {
        delete [] image_history[0].image_array;
        image_history[0].image_array = nullptr;
        for (int i=1; i<max_history_step; i++)
        {
            image_history[i-1].image_type = image_history[i].image_type;
            image_history[i-1].image_array = image_history[i].image_array;
        }
        image_history[max_history_step-1].image_type = "nullptr";
        image_history[max_history_step-1].image_array = nullptr;
        current_image_index -= 1;
    }

    unsigned char * record_image_array = new unsigned char [img_width*img_height*3];
    memcpy(record_image_array, ImgArray, static_cast<unsigned int>(img_height*img_width*3));
    image_history[current_image_index + 1].image_type = "rgb";
    if (image_history[current_image_index + 1].image_array != nullptr)
        delete [] image_history[current_image_index + 1].image_array;
    image_history[current_image_index + 1].image_array = record_image_array;
    current_image_index += 1;
    return;
}

void MainWindow::on_EraseAllUnshowColorAction_triggered()
{
    std::vector<int> main_color;
    int main_color_index[256] = {0};
    memset(main_color_index, 0, 256*sizeof(int));
    unsigned int color_index;
    unsigned char r,g,b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
        {
            main_color.push_back(i);
            color_index = sorted_color[i].at(0).color_index;
            b = static_cast<unsigned char>(color_index / (color_size*color_size));
            g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
            r = static_cast<unsigned char>(color_index % color_size);
            unsigned char tmp_gray = static_cast<unsigned char>(r*ratio*0.114 + g*ratio*0.586 + b*ratio*0.3);
            main_color_index[tmp_gray] = 1;
        }
    }
    unsigned char *EraseArea = new unsigned char [img_height * img_width];
    memset(EraseArea, 255, static_cast<unsigned int>(img_height * img_width));
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    selected_colors_to_binary_array(binary_color, main_color, img_height, img_width, 1);
    for (int i=0; i<img_height*img_width; i++)
    {
        if (binary_color[i] == 1) EraseArea[i] = 0;
    }
    delete [] binary_color;
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            EraseArea[i] = 0;
    }

    unsigned char *grayImgArray = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgArray[i*3]*ratio*0.114 + ImgArray[i*3+1]*ratio*0.586 + ImgArray[i*3+2]*ratio*0.3);
    }
    const int histogramSize  = 256;
    int Hist[histogramSize] = {0};
    int HistIndex[histogramSize] = {0};
    int x0, y0, x1, y1;
    int nx0, ny0, nx1, ny1;
    unsigned char value, mid_value;
    unsigned int max_index;
    int replace_color_place;
    unsigned int erase_count = 0;
    unsigned int count = 0;
    int win_size = 4;
    bool find_nearest_main_color = false;
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (EraseArea[i*img_width+j] == 255)
            {
                mid_value = grayImgArray[i*img_width+j];
                erase_count += 1;
                win_size = 3;
                find_nearest_main_color = false;
                memset(Hist, 0, histogramSize*sizeof(int));
                (i-win_size > 0)?x0=i-win_size:x0=0;
                (j-win_size > 0)?y0=j-win_size:y0=0;
                (i+win_size < img_height)?x1=i+win_size:x1=img_height-1;
                (j+win_size < img_width)?y1=j+win_size:y1=img_width-1;
                for (int m=x0; m<x1+1; m++)
                {
                    for (int n=y0; n<y1+1; n++)
                    {
                        if (EraseArea[m*img_width+n] == 255)
                            continue;
                        value = grayImgArray[m*img_width+n];
                        Hist[value]++;
                        HistIndex[value] = m*img_width+n;
                    }
                }
                while(!find_nearest_main_color && win_size < max_erase_win_size)
                {
                    win_size += 2;
                    (i-win_size > 0)?nx0=i-win_size:nx0=0;
                    (j-win_size > 0)?ny0=j-win_size:ny0=0;
                    (i+win_size < img_height)?nx1=i+win_size:nx1=img_height-1;
                    (j+win_size < img_width)?ny1=j+win_size:ny1=img_width-1;
                    for (int n=ny0; n<ny1+1; n++)
                    {
                        for (int m=nx0; m<x0; m++)
                        {
                            if (EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                        for (int m=x1+1; m<nx1+1; m++)
                        {
                            if (EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                    }
                    for (int m=x0; m<x1+1; m++)
                    {
                        for (int n=ny0; n<y0; n++)
                        {
                            if (EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                        for (int n=y1+1; n<ny1+1; n++)
                        {
                            if (EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                    }
                    /*for (int pixel=0; pixel < histogramSize; pixel++)
                    {
                        Hist[pixel] = Hist[pixel]*main_color_index[pixel];
                    }*/

                    max_index = 0;
                    for (unsigned int k=1; k<histogramSize; k++)
                    {
                        if (Hist[k] >= Hist[max_index])
                            max_index = k;
                    }
                    replace_color_place = HistIndex[max_index];
                    if (Hist[max_index] > (nx1-nx0)*(ny1-ny0)*min_area_ratio_to_replace)
                    {
                        find_nearest_main_color = true;
                        count += 1;
                        ImgArray[i*img_width*3+j*3] = ImgArray[replace_color_place*3];
                        ImgArray[i*img_width*3+j*3+1] = ImgArray[replace_color_place*3+1];
                        ImgArray[i*img_width*3+j*3+2] = ImgArray[replace_color_place*3+2];
                    }
                    x0=nx0; y0=ny0; x1=nx1; y1=ny1;
                }
            }
        }
    }
    delete [] EraseArea;
    delete [] grayImgArray;

    qDebug() << "total pixels to erase:" << erase_count << " erased_num:" << count << " left_ratio:" << (erase_count-count)/(img_height*img_width*1.0);
    delete [] erased_img;
    erased_img = new unsigned char [img_height*img_width*4];
    for (int i=0; i<img_height*img_width; i++)
    {
        erased_img[i*4+0] = ImgArray[i*3] * ratio;
        erased_img[i*4+1] = ImgArray[i*3+1] * ratio;
        erased_img[i*4+2] = ImgArray[i*3+2] * ratio;
        erased_img[i*4+3] = 255;
    }
    delete image;
    image = new QImage(erased_img, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    on_FindColorAction_triggered();
}
void MainWindow::on_EraseAllBelowColorAction_triggered()
{
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char r = static_cast<unsigned char>(eraseColorR->text().toInt() / ratio);
    unsigned char g = static_cast<unsigned char>(eraseColorG->text().toInt() / ratio);
    unsigned char b = static_cast<unsigned char>(eraseColorB->text().toInt() / ratio);
    if (r==color_size-1 && g==color_size-1 && b==color_size-1)
        return;
    unsigned char *grayImgArray = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgArray[i*3]*ratio*0.114 + ImgArray[i*3+1]*ratio*0.586 + ImgArray[i*3+2]*ratio*0.3);
    }
    unsigned char *EraseArea = new unsigned char [img_height*img_width];
    memset(EraseArea, 0, static_cast<unsigned int>(img_height*img_width));
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (b>=ImgArray[i*img_width*3+j*3] && g>=ImgArray[i*img_width*3+j*3+1] && r>=ImgArray[i*img_width*3+j*3+2])
            {
                *(EraseArea + i*img_width+j) = 255;
            }
        }
    }
    erase_select_area_with_main_color(EraseArea, false);
    delete [] grayImgArray;
    delete [] EraseArea;
    return;
}
void MainWindow::on_EraseSpecificColorAction_triggered()
{
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char r = static_cast<unsigned char>(eraseColorR->text().toInt() / ratio);
    unsigned char g = static_cast<unsigned char>(eraseColorG->text().toInt() / ratio);
    unsigned char b = static_cast<unsigned char>(eraseColorB->text().toInt() / ratio);
    if (r==color_size-1 && g==color_size-1 && b==color_size-1)
        return;
    unsigned char *grayImgArray = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgArray[i*3]*ratio*0.114 + ImgArray[i*3+1]*ratio*0.586 + ImgArray[i*3+2]*ratio*0.3);
    }
    unsigned char *EraseArea = new unsigned char [img_height*img_width];
    memset(EraseArea, 0, static_cast<unsigned int>(img_height)*static_cast<unsigned int>(img_width));
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (b==ImgArray[i*img_width*3+j*3] && g==ImgArray[i*img_width*3+j*3+1] && r==ImgArray[i*img_width*3+j*3+2])
            {
                *(EraseArea + i*img_width+j) = 255;
            }
        }
    }
    erase_select_area_with_main_color(EraseArea, false);
    delete [] grayImgArray;
    delete [] EraseArea;
    return;
}

void MainWindow::on_EraseColorByNearestColorAction_triggered()
{
    std::vector<int> erase_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            erase_color.push_back(i);
    }
    unsigned int erase_num = erase_color.size();
    if (erase_num != 0)
    {
        unsigned char *EraseArea = new unsigned char [img_height*img_width];
        selected_colors_to_binary_array(EraseArea, erase_color, img_height, img_width, 255);
        erase_select_area_with_main_color(EraseArea, false);
        delete [] EraseArea;
    }
    return;
}

void MainWindow::on_ExtractColorAction_triggered()
{
    std::vector<int> select_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            select_color.push_back(i);
    }
    if (!select_color.empty())
    {
        delete [] binary_img;
        binary_img = new unsigned char [img_height*img_width];
        selected_colors_to_binary_array(binary_img, select_color, img_height, img_width, 255);
        delete bin_image;
        bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
        QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);
        ImageLabel->clear();
        ImageLabel->setAlignment(Qt::AlignCenter);
        ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
        SaveImage = bin_image;
        autosave("tmp_binary.png");
    }
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }

    if (max_history_step <= current_image_index+1)
    {
        delete [] image_history[0].image_array;
        image_history[0].image_array = nullptr;
        for (int i=1; i<max_history_step; i++)
        {
            image_history[i-1].image_type = image_history[i].image_type;
            image_history[i-1].image_array = image_history[i].image_array;
        }
        image_history[max_history_step-1].image_type = "nullptr";
        image_history[max_history_step-1].image_array = nullptr;
        current_image_index -= 1;
    }

    unsigned char * record_image_array = new unsigned char [img_width*img_height];
    memcpy(record_image_array, binary_img, static_cast<unsigned int>(img_height*img_width));
    image_history[current_image_index + 1].image_type = "binary";
    if (image_history[current_image_index + 1].image_array != nullptr)
        delete [] image_history[current_image_index + 1].image_array;
    image_history[current_image_index + 1].image_array = record_image_array;
    current_image_index += 1;
    return;
}

void MainWindow::see_white_areas()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    delete [] binary_img;
    binary_img = new unsigned char [img_height*img_width];
    memset(binary_img, 0, static_cast<unsigned int>(img_height*img_width));
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            binary_img[i] = 255;
    }
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");

    if (max_history_step <= current_image_index+1)
    {
        delete [] image_history[0].image_array;
        image_history[0].image_array = nullptr;
        for (int i=1; i<max_history_step; i++)
        {
            image_history[i-1].image_type = image_history[i].image_type;
            image_history[i-1].image_array = image_history[i].image_array;
        }
        image_history[max_history_step-1].image_type = "nullptr";
        image_history[max_history_step-1].image_array = nullptr;
        current_image_index -= 1;
    }

    unsigned char * record_image_array = new unsigned char [img_width*img_height];
    memcpy(record_image_array, binary_img, static_cast<unsigned int>(img_height*img_width));
    image_history[current_image_index + 1].image_type = "binary";
    if (image_history[current_image_index + 1].image_array != nullptr)
        delete [] image_history[current_image_index + 1].image_array;
    image_history[current_image_index + 1].image_array = record_image_array;
    current_image_index += 1;
    return;
}

void MainWindow::on_ConvertColorAction_triggered()
{
    //max_main_color_num = static_cast<unsigned char>(r);
    std::vector<int> select_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            select_color.push_back(i);
    }
    if (select_color.empty())
        return;
    unsigned int convert_num = select_color.size();
    unsigned int color_index = 0;
    unsigned char r, g, b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));

    unsigned char r0 = static_cast<unsigned char>(convertColorR->text().toInt() / ratio);
    unsigned char g0 = static_cast<unsigned char>(convertColorG->text().toInt() / ratio);
    unsigned char b0 = static_cast<unsigned char>(convertColorB->text().toInt() / ratio);

    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            for (unsigned int k=0; k<convert_num; k++)
            {
                color_index = sorted_color[select_color.at(k)].at(0).color_index;
                b = static_cast<unsigned char>(color_index / (color_size*color_size));
                g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
                r = static_cast<unsigned char>(color_index % color_size);
                if (b==ImgArray[i*img_width*3+j*3] && g==ImgArray[i*img_width*3+j*3+1] && r==ImgArray[i*img_width*3+j*3+2])
                {
                    ImgArray[i*img_width*3+j*3] = b0;
                    ImgArray[i*img_width*3+j*3+1] = g0;
                    ImgArray[i*img_width*3+j*3+2] = r0;
                }
            }
        }
    }
    delete [] approximate_img;
    approximate_img = new unsigned char [img_height*img_width*4];
    for (int i=0; i<img_height*img_width; i++)
    {
        approximate_img[i*4+0] = ImgArray[i*3] * ratio;
        approximate_img[i*4+1] = ImgArray[i*3+1] * ratio;
        approximate_img[i*4+2] = ImgArray[i*3+2] * ratio;
        approximate_img[i*4+3] = 255;
    }
    delete image;
    image = new QImage(approximate_img, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    on_FindColorAction_triggered();
    return;
}

void MainWindow::on_EraseAllSmallContourAction_triggered()
{
    std::vector<int> select_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
        {
            select_color.push_back(i);
            break;
        }
    }
    if (select_color.empty())
        return;
    unsigned char * tmp_binary_img = new unsigned char [img_height*img_width];
    selected_colors_to_binary_array(tmp_binary_img, select_color, img_height, img_width, 255);
    /*
    unsigned char * flag = new unsigned char [img_height*img_width];
    memset(flag, 0, static_cast<unsigned int>(img_height*img_width));

    std::vector<contour> contour_vector;
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (tmp_binary_img[i*img_width + j] > 0 && flag[i*img_width + j] == 0)
            {
                contour new_contour(i, j, static_cast<int>(contour_vector.size()), 1);
                std::stack <std::pair<int, int>> contour_stack;
                contour_stack.push(std::make_pair(i, j));
                flag[i*img_width + j] = 1;
                while (! contour_stack.empty()) {

                }

            }
        }
    }
    */
    //完成所有团的查找与记录
    std::vector<int> startRun;
    std::vector<int> endRun;
    std::vector<int> rowRun;
    unsigned char * rowPoint;
    int numRun = 0;
    for (int i=0; i<img_height; i++)
    {
        rowPoint = tmp_binary_img + i * img_width;
        if (rowPoint[0] > 0)
        {
            numRun++;
            startRun.push_back(0);
            rowRun.push_back(i);
        }
        for (int j=1; j<img_width; j++)
        {
            if (rowPoint[j-1] == 0 && rowPoint[j] > 0)
            {
                numRun++;
                startRun.push_back(j);
                rowRun.push_back(i);
            }
            else if (rowPoint[j-1] > 0 && rowPoint[j] == 0)
            {
                endRun.push_back(j-1);
            }
        }
        if (rowPoint[img_width-1] > 0)
        {
            endRun.push_back(img_width-1);
        }
    }
    //完成团的标记与等价对列表的生成
    std::vector<std::pair<int, int>> equivalence;
    std::vector<int> runLabels;
    runLabels.assign(static_cast<unsigned int>(numRun), 0);
    int idxLabel = 1;
    int curRowIdx = 0;
    int firstRunOnCur = 0;
    int firstRunOnPre = 0;
    int lastRunOnPre = -1;
    int offset = 0;
    for (int i=0; i<numRun; i++)
    {
        unsigned int u_i = static_cast<unsigned int>(i);
        if (rowRun[u_i] != curRowIdx)
        {
            curRowIdx = rowRun[u_i];
            firstRunOnPre = firstRunOnCur; // 上一行的第一个团的索引
            lastRunOnPre = i - 1; // 上一行的最后一个团的索引
            firstRunOnCur = i; // 当前行的第一个团的索引
        }
        for (int j = firstRunOnPre; j <= lastRunOnPre; j++)
        {
            unsigned int u_j = static_cast<unsigned int>(j);
            if (startRun[u_i] <= endRun[u_j] + offset && endRun[u_i] >= startRun[u_j] - offset && rowRun[u_i] == rowRun[u_j] + 1)
            {
                if (runLabels[u_i] == 0) // 没有被标号过
                    runLabels[u_i] = runLabels[u_j];
                else if (runLabels[u_i] != runLabels[u_j])// 已经被标号
                    equivalence.push_back(std::make_pair(runLabels[u_i], runLabels[u_j])); // 保存等价对
            }
        }
        if (runLabels[u_i] == 0) // 没有与前一列的任何run重合
        {
            runLabels[u_i] = idxLabel;
            idxLabel++;
        }
    }
    /* 等价对的处理，我们需要将它转化为若干个等价序列。比如有如下等价对:
     * (1,2),(1,6),(3,7),(9-3),(8,1),(8,10),(11,5),(11,8),(11,12),(11,13),(11,14),(15,11)
     * 我们需要得到最终序列是：
     * list1:1-2-5-6-8-10-11-12-13-14-15
     * list2:3-7-9
     * list3:4*/
    unsigned int maxLabel = static_cast<unsigned int>(*std::max_element(runLabels.begin(), runLabels.end())); // find the max value of runLabels
    std::vector<std::vector<bool>> eqTab(maxLabel, std::vector<bool>(maxLabel, false));
    std::vector<std::pair<int, int>>::iterator vecPairIt = equivalence.begin();
    while (vecPairIt != equivalence.end())
    {
        eqTab[static_cast<unsigned int>(vecPairIt->first - 1)][static_cast<unsigned int>(vecPairIt->second - 1)] = true;
        eqTab[static_cast<unsigned int>(vecPairIt->second - 1)][static_cast<unsigned int>(vecPairIt->first - 1)] = true;
        vecPairIt++;
    }
    std::vector<unsigned int> labelFlag(maxLabel, 0);
    std::vector<std::vector<unsigned int>> equaList;
    std::vector<unsigned int> tempList;
    for (unsigned int i = 1; i <= maxLabel; i++)
    {
        if (labelFlag[i - 1] > 0)
        {
            continue;
        }
        labelFlag[i - 1] = equaList.size() + 1;
        tempList.push_back(i);
        for (unsigned int j = 0; j < tempList.size(); j++)
        {
            for (unsigned int k = 0; k != eqTab[tempList[j] - 1].size(); k++)
            {
                if (eqTab[tempList[j] - 1][k] && !labelFlag[k])
                {
                    tempList.push_back(k + 1);
                    labelFlag[k] = equaList.size() + 1;
                }
            }
        }
        equaList.push_back(tempList);
        tempList.clear();
    }
    for (unsigned int i = 0; i < runLabels.size(); i++)
    {
        runLabels[i] = static_cast<int>(labelFlag[static_cast<unsigned int>(runLabels[i]) - 1]);
    }
    maxLabel = equaList.size();
    qDebug() << endl << "Total Contour Number: " << maxLabel;

    std::vector<unsigned int> contourArea(maxLabel, 0);
    for (unsigned int i = 0; i != runLabels.size(); i++)
    {
        contourArea[static_cast<unsigned int>(runLabels[i]) - 1] += static_cast<unsigned int>(endRun[i] - startRun[i] + 1);
    }
    unsigned int erased_top_contour_num = static_cast<unsigned int>(notDilateNum->text().toInt());;

    for (unsigned int i = 0; i < erased_top_contour_num; i++)
    {
        unsigned int max_index = 0;
        for (unsigned int j = 1; j < maxLabel; j++)
        {
            if (contourArea[j] > contourArea[max_index])
                max_index = j;
        }
        contourArea[max_index] = 0;
        for (unsigned int k = 0; k < runLabels.size(); k++)
        {
            if (runLabels[k] == static_cast<int>(max_index + 1))
            {
                for (int m = startRun[k]; m <= endRun[k]; m++)
                {
                    tmp_binary_img[rowRun[k] * img_width + m] = 0;
                }
            }
        }
    }
    erase_select_area_with_main_color(tmp_binary_img, false);
    delete[] tmp_binary_img;
    return;
}
void MainWindow::on_AddColorFromBinaryImageAction_triggered()
{
    QString FileName = QFileDialog::getOpenFileName(this, tr("open binary file"), "pic", tr("img(*png);;All files (*.*)"));
    if (FileName != "")
    {
        QImage *binary_image = new QImage;
        if (binary_image->load(FileName)) //根据文件名打开图像，如果图像本身是32、24位的，程序中图像是32位的，如果图像本身是8位、1位的，程序中对应为8位、1位。
        {
            int binary_img_width = binary_image->width();
            int binary_img_height = binary_image->height();
            if (img_height == 0 || binary_img_height != img_height || binary_img_width != img_width)
            {
                qDebug() << "the size of the binary image is not equal with current image!" << endl;
                delete binary_image;
                return;
            }
            unsigned char *BinaryImgPointer = binary_image->bits();
            int bits_per_line = binary_image->bytesPerLine();
            int channel = bits_per_line / binary_img_width;
            qDebug() << "channels of the binary image: " << channel;
            unsigned char *ImgPointer=image->bits();
            unsigned char r0 = static_cast<unsigned char>(convertColorR->text().toInt());
            unsigned char g0 = static_cast<unsigned char>(convertColorG->text().toInt());
            unsigned char b0 = static_cast<unsigned char>(convertColorB->text().toInt());
            for (int i=0; i<img_height; i++)
            {
                for (int j=0; j<img_width; j++)
                {
                    if (BinaryImgPointer[0] > 0)
                    {
                        ImgPointer[0] = b0; // B G R A
                        ImgPointer[1] = g0;
                        ImgPointer[2] = r0;
                    }
                    BinaryImgPointer += channel;
                    ImgPointer += 4;
                }
            }
            delete binary_image;
            on_FindColorAction_triggered();
        }
    }
    return;
}

void MainWindow::on_AllToSVGAction_triggered()
{
    QString svgFileName = QFileDialog::getSaveFileName(this, tr("save svg file"), "x.svg", tr("svg(*svg)"));
    QString endfix = ".svg";
    if (! svgFileName.endsWith(endfix))
    {
        svgFileName.append(endfix);
    }
    unsigned int color_index;
    unsigned char r,g,b;
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    int color_count = 0;
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
            color_count++;
    }

    //write svg head
    QFile file(svgFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString head = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    head.append("<svg width=\"100%\" height=\"100%\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=");
    QString rectStr;
    rectStr.sprintf("\"%d %d %d %d\">", 0, 0, img_width, img_height);
    head.append(rectStr);
    file.write(head.toUtf8());
    QString style = "\n<defs><style>";
    QString end = "</style></defs><title>SVG</title>\n";

    for (int i=0; i<color_count; i++)
    {
        color_index = sorted_color[i].at(0).color_index;
        b = static_cast<unsigned char>(color_index / (color_size*color_size));
        g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
        r = static_cast<unsigned char>(color_index % color_size);
        QString layerName = QByteArray::number(r*16, 10);
        layerName.append("_");
        layerName.append(QByteArray::number(g*16, 10));
        layerName.append("_");
        layerName.append(QByteArray::number(b*16, 10));
        QString colorStr = "#";
        //key =  QString("%1").arg(r*16, 2, 16, vQLatin1Char('0'))
        colorStr.append(QString("%1").arg(r*16, 2, 16, QLatin1Char('0')));
        colorStr.append(QString("%1").arg(g*16, 2, 16, QLatin1Char('0')));
        colorStr.append(QString("%1").arg(b*16, 2, 16, QLatin1Char('0')));
        QString classColor;
        classColor.sprintf(".cls-%d{fill:", i+1);
        classColor.append(colorStr);
        classColor.append(";}");
        style.append(classColor);
    }

    style.append(end);
    file.write(style.toUtf8());

    unsigned int totalOutlineNum = 0;
    unsigned int totalInlineNum = 0;
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    on_major_3_3_triggered();
    on_major_3_3_triggered();
    std::vector<std::vector<std::pair<float, float>>> outContours;
    std::vector<std::vector<std::pair<float, float>>> inContours;
    std::vector<std::vector<unsigned int>> holeIndexs;
    for (int i=0; i<color_count; i++)
    {
        std::vector<int> save_color;
        save_color.push_back(i);
        selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);

        color_index = sorted_color[i].at(0).color_index;
        b = static_cast<unsigned char>(color_index / (color_size*color_size));
        g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
        r = static_cast<unsigned char>(color_index % color_size);

        outContours.clear();
        inContours.clear();
        holeIndexs.clear();
        extractContourFromBinaryImg(binary_color, outContours, inContours, holeIndexs);
        totalOutlineNum += outContours.size();
        totalInlineNum += inContours.size();
        //write to svg
        QString layerName = QByteArray::number(r*16, 10);
        layerName.append("_");
        layerName.append(QByteArray::number(g*16, 10));
        layerName.append("_");
        layerName.append(QByteArray::number(b*16, 10));
        contourTwoSVG(outContours, inContours, holeIndexs, file, layerName, i+1);
    }

    end = "</svg>";
    file.write(end.toUtf8());
    file.close();
    qDebug() << "Done writen to svg! total outlines: " << totalOutlineNum << "; total inlines: " << totalInlineNum;
    qDebug() << "--------------------------------------";
    delete [] binary_color;
    //delete [] ProcessedExtractColor;
    return;
}

void MainWindow::getCoordinatePage(float* coor)
{
    getValue = new Dialog(coor, this);
    getValue->exec();
    delete getValue;
}

void MainWindow::on_AllToSHPAction_triggered()
{
    float* coor = new float [4];
    memset(coor, 0, 4*sizeof(float));
    getCoordinatePage(coor);
    qDebug() << coor[0] << coor[1] << coor[2] << coor[3];
    if (coor[0] >= coor[2] || coor[1] <= coor[3])
    {
        qDebug() << "wrong format of longtitude and latitude format";
        return;
    }
    QString shpFileName = QFileDialog::getSaveFileName(this, tr("save shp file"), "xxx", tr("shp(*shp)"));
    QString endfix = ".shp";
    if (shpFileName.endsWith(endfix))
    {
        shpFileName = shpFileName.left(shpFileName.length()-4);
    }
    QByteArray ba = shpFileName.toLatin1();
    char* fileName = ba.data();

    int color_count = 0;
    for (int i=0; i<display_num_color; i++)
    {
        if (sorted_color[i].size() != 0)
            color_count++;
    }

    unsigned int totalOutlineNum = 0;
    unsigned int totalInlineNum = 0;
    unsigned char *binary_color = new unsigned char [img_height * img_width];
    on_major_3_3_triggered();
    on_major_3_3_triggered();
    std::vector<std::vector<std::pair<float, float>>> totalOutContours;
    std::vector<std::vector<std::pair<float, float>>> totalInContours;
    std::vector<std::vector<unsigned int>> totalHoleIndexs;
    std::vector<int> label;
    totalOutContours.reserve(100);
    totalInContours.reserve(100);
    totalHoleIndexs.reserve(100);

    if (color_count > 0)
    {
        std::vector<int> save_color;
        save_color.push_back(0);
        selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);
        extractContourFromBinaryImg(binary_color, totalOutContours, totalInContours, totalHoleIndexs);
        totalOutlineNum += totalOutContours.size();
        totalInlineNum += totalInContours.size();
        for (unsigned int i=0; i<totalOutContours.size(); i++)
        {
            label.push_back(0);
        }
    }

    std::vector<std::vector<std::pair<float, float>>> outContours;
    std::vector<std::vector<std::pair<float, float>>> inContours;
    std::vector<std::vector<unsigned int>> holeIndexs;
    outContours.reserve(50);
    inContours.reserve(50);
    holeIndexs.reserve(50);

    for (int i=1; i<color_count; i++)
    {
        std::vector<int> save_color;
        save_color.push_back(i);
        selected_colors_to_binary_array(binary_color, save_color, img_height, img_width, 255);
        extractContourFromBinaryImg(binary_color, outContours, inContours, holeIndexs);

        totalOutContours.insert(totalOutContours.end(), outContours.begin(), outContours.end());
        totalInContours.insert(totalInContours.end(), inContours.begin(), inContours.end());
        for (unsigned int j=0; j<holeIndexs.size(); j++)
        {
            for (unsigned int k=1; k<holeIndexs[j].size(); k++)
            {
                holeIndexs[j][k] += totalInlineNum;
            }
            label.push_back(i);
        }
        totalHoleIndexs.insert(totalHoleIndexs.end(), holeIndexs.begin(), holeIndexs.end());
        totalOutlineNum += outContours.size();
        totalInlineNum += inContours.size();
        outContours.clear();
        inContours.clear();
        holeIndexs.clear();
        //std::vector<std::vector<std::pair<float, float>>>().swap(outContours);
        //std::vector<std::vector<std::pair<float, float>>>().swap(inContours);
        //std::vector<std::vector<unsigned int>>().swap(holeIndexs);
        /*color_index = sorted_color[i].at(0).color_index;
        b = static_cast<unsigned char>(color_index / (color_size*color_size));
        g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
        r = static_cast<unsigned char>(color_index % color_size);
        QString layerName = QByteArray::number(r*16, 10);
        layerName.append("_");
        layerName.append(QByteArray::number(g*16, 10));
        layerName.append("_");
        layerName.append(QByteArray::number(b*16, 10));*/
    }
    qDebug() << "find all outlines and inlines";
    for (unsigned int i=0; i<totalOutContours.size(); i++)
    {
        for (unsigned int j=0; j<totalOutContours[i].size()-1; j++)
        {
            totalOutContours[i][j].second = coor[0] + (coor[2] - coor[0])/img_width*totalOutContours[i][j].second; // topleft longtitude
            totalOutContours[i][j].first = coor[1] + (coor[3] - coor[1])/img_height*totalOutContours[i][j].first;// topleft latitude
        }
        totalOutContours[i][totalOutContours[i].size()-1].first = totalOutContours[i][0].first;
        totalOutContours[i][totalOutContours[i].size()-1].second = totalOutContours[i][0].second;
    }
    for (unsigned int i=0; i<totalInContours.size(); i++)
    {
        for (unsigned int j=0; j<totalInContours[i].size()-1; j++)
        {
            totalInContours[i][j].second = coor[0] + (coor[2] - coor[0])/img_width*totalInContours[i][j].second; // topleft longtitude
            totalInContours[i][j].first = coor[1] + (coor[3] - coor[1])/img_height*totalInContours[i][j].first;// topleft latitude
        }
        totalInContours[i][totalInContours[i].size()-1].first = totalInContours[i][0].first;
        totalInContours[i][totalInContours[i].size()-1].second = totalInContours[i][0].second;
    }
    delete [] binary_color;
    delete [] coor;
    qDebug() << "finish corrdinate mapping";

    writeSHPpolygon(fileName, totalOutContours, totalInContours, totalHoleIndexs, label);
    std::vector<std::vector<std::pair<float, float>>>().swap(totalOutContours);
    std::vector<std::vector<std::pair<float, float>>>().swap(totalInContours);
    std::vector<std::vector<unsigned int>>().swap(totalHoleIndexs);
    qDebug() << "complete written to shp";
    return;
}

void MainWindow::on_BinaryToSvgAction_triggered()
{
    if (bin_image == nullptr)
        return;
    QString svgFileName = QFileDialog::getSaveFileName(this, tr("save svg file"), "x.svg", tr("svg(*svg)"));
    QString endfix = ".svg";
    if (! svgFileName.endsWith(endfix))
    {
        svgFileName.append(endfix);
    }
    std::vector<std::vector<std::pair<float, float>>> out_contours;
    std::vector<std::vector<std::pair<float, float>>> in_contours;
    std::vector<std::vector<unsigned int>> hole_indexs;
    extractContourFromBinaryImg(bin_image->bits(), out_contours, in_contours, hole_indexs);
    //write svg head
    QFile file(svgFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString head = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    head.append("<svg width=\"100%\" height=\"100%\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=");
    QString rectStr;
    rectStr.sprintf("\"%d %d %d %d\">", 0, 0, img_width, img_height);
    head.append(rectStr);
    file.write(head.toUtf8());
    QString style = "\n<defs><style>";
    QString end = "</style></defs><title>SVG</title>\n";

    QString layerName = "0_0_0";
    QString colorStr = "#000000";
    QString classColor;
    classColor.sprintf(".cls-%d{fill:", 1);
    classColor.append(colorStr);
    classColor.append(";}");
    style.append(classColor);
    style.append(end);
    file.write(style.toUtf8());
    //write svg body
    contourTwoSVG(out_contours, in_contours, hole_indexs, file, layerName, 1);
    end = "</svg>";
    file.write(end.toUtf8());
    file.close();
    qDebug() << "Done writen to svg!";
    qDebug() << "--------------------------------------";
    return;
}

void MainWindow::extractContourFromBinaryImg(unsigned char * biPointer,
                                             std::vector<std::vector<std::pair<float, float>>>& outContours,
                                             std::vector<std::vector<std::pair<float, float>>>& inContours,
                                             std::vector<std::vector<unsigned int>>& holeIndexs)
{
    // make zero borders and convert to 0-1
    unsigned int pad_img_width = static_cast<unsigned int>(img_width + 2);
    unsigned int pad_img_height = static_cast<unsigned int>(img_height + 2);
    unsigned char *biImg0Pad = new unsigned char [pad_img_width * pad_img_height];
    memset(biImg0Pad, 0, pad_img_width * pad_img_height);
    short *flag = new short [pad_img_width * pad_img_height];
    memset(flag, 0, pad_img_width * pad_img_height *sizeof(short));
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (biPointer[i * img_width + j] > 0)
                biImg0Pad[(i + 1) * (img_width + 2) + j + 1] = 1;
        }
    }

    int offset[8] = {1, img_width + 3, img_width+2, img_width+1, -1, -img_width-3, -img_width-2, -img_width-1};// 0-1-2-3-4-5-6-7
    short label = 1;
    int curPos = 0;
    int newPos = 0;
    unsigned int pointH = 0;
    unsigned int pointW = 0;

    std::vector<unsigned int> insideLineLabel;
    std::vector<std::pair<float, float>> simLine;
    std::vector<std::pair<unsigned int, unsigned int>> outline;
    std::vector<std::pair<unsigned int, unsigned int>> insideline;

    for (unsigned int i=0; i<pad_img_height; i++)
    {
        for (unsigned int j=0; j<pad_img_width; j++)
        {
            curPos = static_cast<int>(i*pad_img_width + j);
            if (biImg0Pad[curPos] == 0)
                continue;
            if (label > maxContourNum)
            {
                qDebug() << "too many contours, exceed " << maxContourNum << "please filter noises first!";
                delete [] biImg0Pad;
                delete [] flag;
                return;
            }
            // find a new outline point
            if (biImg0Pad[curPos] > 0 && flag[curPos] == 0 && biImg0Pad[(i-1)*pad_img_width + j] == 0)
            {
                //std::vector<std::pair<unsigned int, unsigned int>> outline;
                outline.clear();
                outline.push_back(std::make_pair(i-1, j-1));
                // find outline points in clockwise direction: 5 6 7 / 4 p 0 / 3 2 1  7-0-1-2-3-4-5-6
                bool isfinish = false;
                int startIndex = 7;
                int intoIndex = 0;
                int k = 0;
                for (k=0; k<8; k++)
                {
                    newPos = curPos+offset[(startIndex + k) % 8];
                    if (biImg0Pad[newPos] > 0)
                    {
                        pointH = static_cast<unsigned int>(newPos) / pad_img_width;
                        pointW = static_cast<unsigned int>(newPos) % pad_img_width;
                        outline.push_back(std::make_pair(pointH-1, pointW-1));
                        flag[newPos] = label;
                        intoIndex = (startIndex + k + 4) % 8;
                        break;
                    }
                    else
                    {
                        flag[newPos] = -1;
                    }
                }
                if (k == 8) // single point
                    continue;

                curPos = newPos;
                while(! isfinish) {
                    for (k=0; k<8; k++)
                    {
                        newPos = curPos+offset[(intoIndex + 2 + k) % 8];
                        if (biImg0Pad[newPos] > 0)
                        {
                            intoIndex = (intoIndex + 2 + k + 4) % 8;
                            pointH = static_cast<unsigned int>(newPos) / pad_img_width;
                            pointW = static_cast<unsigned int>(newPos) % pad_img_width;
                            if (pointH == i && pointW == j)
                            {
                                isfinish = true;
                                /*for (int m=0; m<8; m++)
                                {
                                    if (flag[newPos + offset[m]] == 0)
                                        isfinish = false;
                                }*/
                            }
                            outline.push_back(std::make_pair(pointH-1, pointW-1));
                            flag[newPos] = label;
                            break;
                        }
                        else
                        {
                            flag[newPos] = -1;
                        }
                    }
                    curPos = newPos;
                }
                if (outline.size() >= 4 && calCoutourArea(outline) > minContourArea)
                {
                    //qDebug() << outline;
                    //qDebug() << calCoutourArea(outline);
                    //std::vector<std::pair<double, double>> simLine;
                    simLine.clear();
                    simplifyLine(outline, simLine, outLine);
                    //qDebug() << simLine;
                    outContours.push_back(simLine);
                    label++;
                }
            }
            // find a new inline point
            else if (biImg0Pad[curPos] > 0 && biImg0Pad[(i+1)*pad_img_width + j] == 0
                     && flag[(i+1)*pad_img_width + j]==0)
            {
                //std::vector<std::pair<unsigned int, unsigned int>> insideline;
                insideline.clear();
                insideline.push_back(std::make_pair(i-1, j-1));
                // find outline points in clockwise direction: 5 6 7 / 4 p 0 / 3 2 1
                bool isfinish = false;
                int startIndex = 3;
                int intoIndex = 0;
                int k = 0;
                short insideLabel = 0;
                if (flag[curPos] > 0)
                {
                    insideLabel = flag[curPos];
                }
                else
                {
                    insideLabel = flag[curPos-1];
                }

                for (k = 0; k < 8; k++)
                {
                    newPos = curPos+offset[(startIndex + k) % 8];
                    if (biImg0Pad[newPos] > 0)
                    {
                        pointH = static_cast<unsigned int>(newPos) / pad_img_width;
                        pointW = static_cast<unsigned int>(newPos) % pad_img_width;
                        insideline.push_back(std::make_pair(pointH-1, pointW-1));
                        flag[newPos] = insideLabel;
                        intoIndex = (startIndex + k + 4) % 8;
                        break;
                    }
                    else
                    {
                        flag[newPos] = -1;
                    }
                }
                if (k == 8) // single point
                    continue;

                curPos = newPos;
                while(! isfinish) {
                    for (k = 0; k < 8; k++)
                    {
                        newPos = curPos+offset[(intoIndex + 2 + k) % 8];
                        if (biImg0Pad[newPos] > 0)
                        {
                            intoIndex = (intoIndex + 2 + k + 4) % 8;
                            pointH = static_cast<unsigned int>(newPos) / pad_img_width;
                            pointW = static_cast<unsigned int>(newPos) % pad_img_width;
                            if (pointH == i && pointW == j)
                            {
                                isfinish = true;
                                //for (int m=0; m<8; m++)
                                //{
                                //    if (flag[newPos + offset[m]] == 0)
                                //        isfinish = false;
                                //}
                            }
                            insideline.push_back(std::make_pair(pointH-1, pointW-1));
                            flag[newPos] = insideLabel;
                            break;
                        }
                        else
                        {
                            flag[newPos] = -1;
                        }
                    }
                    curPos = newPos;
                }
                if (insideline.size() >= 4 && calCoutourArea(insideline) > minContourArea)
                {
                    //qDebug() << insideline;
                    //qDebug() << calCoutourArea(insideline);
                    //qDebug() << "find inline of label: " << insideLabel;
                    //std::vector<std::pair<double, double>> simLine;
                    simLine.clear();
                    simplifyLine(insideline, simLine, inLine);
                    inContours.push_back(simLine);
                    insideLineLabel.push_back(static_cast<unsigned int>(insideLabel));
                }
            }
            else if (biImg0Pad[curPos] > 0 && flag[curPos] == 0)
            {
                flag[curPos] = flag[curPos-1];
            }
        }
    }
    delete [] biImg0Pad;
    delete [] flag;

    std::vector<unsigned int> emptyIndex;
    emptyIndex.push_back(0);
    for (unsigned int i=0; i<outContours.size(); i++)
    {
        holeIndexs.push_back(emptyIndex);
    }
    for (unsigned int i=0; i<insideLineLabel.size(); i++)
    {
        holeIndexs[insideLineLabel[i]-1].push_back(i);
    }
    return;
}

double MainWindow::calCoutourArea(std::vector<std::pair<unsigned int, unsigned int>> &points)
{
    unsigned int point_num = points.size();
    double s = static_cast<int>(points[0].first) * (static_cast<int>(points[point_num-2].second) - static_cast<int>(points[1].second));
    for (unsigned int i = 1; i < point_num-1; ++i)
        s += static_cast<int>(points[i].first) * (static_cast<int>(points[i-1].second) - static_cast<int>(points[i+1].second));
    return fabs(s/2.0) - static_cast<int>(point_num) + 1;

    /*
    std::vector<std::pair<unsigned int, unsigned int>> linePoints;
    unsigned int point_num = points.size() - 1;
    if (points[0].first != points[point_num-1].first || points[0].second != points[point_num - 1].second) // not close
        return 0.0;
    for (unsigned int i = 0; i < point_num; i++)
    {
        if (points[i].first != points[i + 1].first) // check whether two points at the same line (height)
        {
            if (points[i].first > points[i + 1].first && points[i].second < points[i + 1].second) // code 1
            {
                if (((points[i - 1].first - points[i + 1].first)*(points[i - 1].first - points[i + 1].first)) > 1) //single point ar line x, haven't insert: insert i
                    linePoints.push_back(points[i]);
                // multiple points at the same line, only save the leftmost point at i+1
                linePoints.push_back(points[i + 1]);
            }
            else
            {
                linePoints.push_back(points[i]);
            }
        }
    }
    points.clear();
    if (linePoints.back().first != linePoints[0].first || linePoints.back().second != linePoints[0].second)
    {
        linePoints.push_back(linePoints[0]);
    }*/
}

void MainWindow::simplifyLine(std::vector<std::pair<unsigned int, unsigned int>> &points,
                              std::vector<std::pair<float, float>> &finalPoints, LineType line)
{
    unsigned char code8[9] = {3, 2, 1, 4, 8, 0, 5, 6, 7};
    unsigned int point_num = points.size() - 1;
    std::vector<unsigned char> codeLine;
    int deltaH, deltaW;
    for (unsigned int i = 0; i < point_num; i++)
    {
        deltaH = static_cast<int>(points[i+1].first) - static_cast<int>(points[i].first);
        deltaW = static_cast<int>(points[i+1].second) - static_cast<int>(points[i].second);
        //if (deltaH == 0 && deltaW == 0)
        //    continue;
        codeLine.push_back(code8[(deltaH+1)*3 + (deltaW+1)]);
    }

    // find the envelope of contour points (in this way, boundary of adjoin colors is unique and same) 包络
    std::vector<std::pair<int, int>> envelopePoints;
    unsigned int offset[4][2] = {{0,1}, {0,0}, {1,0}, {1,1}};
    unsigned int addNum, beginPos, curPos, Minus1;
    for (unsigned int i = 0; i < point_num; i++)
    {
        addNum = 0; beginPos = 1;
        Minus1 = (i - 1 + point_num) % point_num;
        for (unsigned int j = (codeLine[Minus1] + 4) % 8; j != codeLine[i]; j = (j - 1 + 8) % 8)
        {
            if (j % 2 == 1)
            {
                if (addNum == 0)
                    beginPos = j / 2;
                addNum++;
            }
        }
        if (codeLine[i] % 2 == 1)
        {
            if (addNum == 0)
                beginPos = codeLine[i] / 2;
            addNum++;
        }
        addNum--;
        for (unsigned int j = 0; j < addNum; j++)
        {
            curPos = (beginPos - j + 4) % 4;
            envelopePoints.push_back(std::make_pair(points[i].first+offset[curPos][0], points[i].second+offset[curPos][1]));
        }
    }
    if (envelopePoints.back().first != envelopePoints[0].first || envelopePoints.back().second != envelopePoints[0].second)
    {
        envelopePoints.push_back(envelopePoints[0]);
    }
    //points.clear();
    std::vector<std::pair<unsigned int, unsigned int>>().swap(points);
    // simplify envelopePoints, only save point whose three adjacent points have 90/270 degree
    std::vector<std::pair<int, int>> simPoints;
    point_num = envelopePoints.size() - 1;
    for (unsigned int i = 0; i < point_num; i++)
    {
        Minus1 = (i - 1 + point_num) % point_num;
        if ((envelopePoints[Minus1].first-envelopePoints[i].first)*(envelopePoints[i+1].first-envelopePoints[i].first) +
            (envelopePoints[Minus1].second-envelopePoints[i].second)*(envelopePoints[i+1].second-envelopePoints[i].second) == 0)
        {
            simPoints.push_back(envelopePoints[i]); // two vector are orthogonal
        }
    }
    simPoints.push_back(simPoints[0]);
    //envelopePoints.clear();
    std::vector<std::pair<int, int>>().swap(envelopePoints);

    // merge points that only differ one/two in X or Y axis
    std::vector<std::pair<int, int>> convexPoints;
    point_num = simPoints.size() - 1;
    for (unsigned int i = 0; i < point_num; i++)
    {
        Minus1 = (i - 1 + point_num) % point_num;
        //s=(x1-x3)*(y2-y3)-(x2-x3)*(y1-y3)
        //当s>0时,p1,p2,p3三个点呈顺时针; 当s<0时,逆时针
        int s = (simPoints[Minus1].second - simPoints[i+1].second)*(simPoints[i].first-simPoints[i+1].first) -
                (simPoints[i].second - simPoints[i+1].second)*(simPoints[Minus1].first-simPoints[i+1].first);
        if (s > 0) // if clockwise
        {
            convexPoints.push_back(simPoints[i]);
        }
        else if (abs(simPoints[Minus1].first - simPoints[i+1].first) > 2 &&
                 abs(simPoints[Minus1].second - simPoints[i+1].second) > 2)
        {
            convexPoints.push_back(simPoints[i]);
        }
    }
    convexPoints.push_back(convexPoints[0]);
    //simPoints.clear();
    std::vector<std::pair<int, int>>().swap(simPoints);

    // simplify by erase points with continuous same direction code
    point_num = convexPoints.size() - 1;
    finalPoints.push_back(convexPoints[0]);
    int prevCode[2] = {convexPoints[1].first-convexPoints[0].first, convexPoints[1].second-convexPoints[0].second};

    for (unsigned int i = 1; i < point_num; i++)
    {
        deltaH = convexPoints[i+1].first - convexPoints[i].first;
        deltaW = convexPoints[i+1].second - convexPoints[i].second;
        if (deltaH != prevCode[0] || deltaW != prevCode[1])
        {
            prevCode[0] = deltaH; prevCode[1] = deltaW;
            finalPoints.push_back(convexPoints[i]);
        }
    }
    finalPoints.push_back(finalPoints[0]);
    //convexPoints.clear();
    std::vector<std::pair<int, int>>().swap(convexPoints);
    return;
}

void MainWindow::contourTwoSVG(std::vector<std::vector<std::pair<float, float>>>& outContour,
                               std::vector<std::vector<std::pair<float, float>>>& inContour,
                               std::vector<std::vector<unsigned int>>& holeIndexs, QFile &file,
                               QString layerName, int layerIndex)
{
    unsigned int outContourNum = outContour.size();
    QString contour, end;
    contour.sprintf("<g id=\"_");
    contour.append(layerName);
    contour.append("\" data-name=\"");
    contour.append(layerName);
    contour.append("\">\n");

    for (unsigned int j=0; j<outContourNum; j++)
    {
        end = " Z";
        QString path, pathSeq;
        unsigned int pointNum = outContour[j].size() - 1;
        pathSeq.sprintf("M%.1f %.1f", double(outContour[j][0].second), double(outContour[j][0].first));
        for (unsigned int k=1; k<pointNum; k++)
        {
            QString linePoint;
            linePoint.sprintf(" L%.1f %.1f", double(outContour[j][k].second), double(outContour[j][k].first));
            pathSeq.append(linePoint);
        }
        pathSeq.append(end);

        for (unsigned int m=1; m<holeIndexs[j].size(); m++)
        {
            QString inPathSeq;
            inPathSeq.sprintf(" M%.1f %.1f", double(inContour[holeIndexs[j][m]][0].second),
                    double(inContour[holeIndexs[j][m]][0].first));
            unsigned int inPointNum = inContour[holeIndexs[j][m]].size() - 1;
            for (unsigned int n=1; n<inPointNum; n++)
            {
                QString inLinePoint;
                inLinePoint.sprintf(" L%.1f %.1f", double(inContour[holeIndexs[j][m]][n].second),
                        double(inContour[holeIndexs[j][m]][n].first));
                inPathSeq.append(inLinePoint);
            }
            inPathSeq.append(end);
            pathSeq.append(inPathSeq);
        }
        path.sprintf("<path class=\"cls-%d\" d=\"", layerIndex);
        path.append(pathSeq);
        end = "\"/>\n";
        path.append(end);
        contour.append(path);
    }
    end = "</g>\n";
    contour.append(end);
    file.write(contour.toUtf8());
    return;
}

void MainWindow::erase_select_area_with_main_color(unsigned char * EraseArea, bool use_self_color=true)
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *grayImgArray = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgArray[i*3]*ratio*0.114 + ImgArray[i*3+1]*ratio*0.586 + ImgArray[i*3+2]*ratio*0.3);
    }

    const int histogramSize  = 256;
    int Hist[histogramSize] = {0};
    int HistIndex[histogramSize] = {0};
    int x0, y0, x1, y1;
    int nx0, ny0, nx1, ny1;
    unsigned char value, mid_value;
    unsigned int max_index;
    int replace_color_place;
    unsigned int erase_count = 0;
    unsigned int count = 0;
    int win_size = 4;
    bool find_nearest_main_color = false;
    for (int i=0; i<img_height; i++)
    {
        for (int j=0; j<img_width; j++)
        {
            if (EraseArea[i*img_width+j] == 255)
            {
                mid_value = grayImgArray[i*img_width+j];
                erase_count += 1;
                win_size = 3;
                find_nearest_main_color = false;
                memset(Hist, 0, histogramSize*sizeof(int));
                (i-win_size > 0)?x0=i-win_size:x0=0;
                (j-win_size > 0)?y0=j-win_size:y0=0;
                (i+win_size < img_height)?x1=i+win_size:x1=img_height-1;
                (j+win_size < img_width)?y1=j+win_size:y1=img_width-1;
                for (int m=x0; m<x1+1; m++)
                {
                    for (int n=y0; n<y1+1; n++)
                    {
                        if (!use_self_color && EraseArea[m*img_width+n] == 255)
                            continue;
                        value = grayImgArray[m*img_width+n];
                        Hist[value]++;
                        HistIndex[value] = m*img_width+n;
                    }
                }
                while(!find_nearest_main_color && win_size < max_erase_win_size)
                {
                    win_size += 2;
                    (i-win_size > 0)?nx0=i-win_size:nx0=0;
                    (j-win_size > 0)?ny0=j-win_size:ny0=0;
                    (i+win_size < img_height)?nx1=i+win_size:nx1=img_height-1;
                    (j+win_size < img_width)?ny1=j+win_size:ny1=img_width-1;
                    for (int n=ny0; n<ny1+1; n++)
                    {
                        for (int m=nx0; m<x0; m++)
                        {
                            if (!use_self_color && EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                        for (int m=x1+1; m<nx1+1; m++)
                        {
                            if (!use_self_color && EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                    }
                    for (int m=x0; m<x1+1; m++)
                    {
                        for (int n=ny0; n<y0; n++)
                        {
                            if (!use_self_color && EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                        for (int n=y1+1; n<ny1+1; n++)
                        {
                            if (!use_self_color && EraseArea[m*img_width+n] == 255)
                                continue;
                            value = grayImgArray[m*img_width+n];
                            Hist[value]++;
                            HistIndex[value] = m*img_width+n;
                        }
                    }
                    if (!use_self_color) {Hist[mid_value] = 0;}
                    //if (!use_white)
                    //{
                    //    for (int pixel=white_threshold; pixel < 256; pixel++) {Hist[pixel] = 0;}
                    //}
                    max_index = 0;
                    for (unsigned int k=1; k<histogramSize; k++)
                    {
                        if (Hist[k] >= Hist[max_index])
                            max_index = k;
                    }
                    replace_color_place = HistIndex[max_index];
                    if (Hist[max_index] > (nx1-nx0)*(ny1-ny0)*min_area_ratio_to_replace)
                    {
                        find_nearest_main_color = true;
                        count += 1;
                        ImgArray[i*img_width*3+j*3] = ImgArray[replace_color_place*3];
                        ImgArray[i*img_width*3+j*3+1] = ImgArray[replace_color_place*3+1];
                        ImgArray[i*img_width*3+j*3+2] = ImgArray[replace_color_place*3+2];
                    }
                    x0=nx0; y0=ny0; x1=nx1; y1=ny1;
                }
            }
        }
    }
    delete [] grayImgArray;

    qDebug() << "total pixels to erase:" << erase_count << " erased_num:" << count << " left_ratio:" << (erase_count-count)/(img_height*img_width*1.0);
    delete [] erased_img;
    erased_img = new unsigned char [img_height*img_width*4];
    for (int i=0; i<img_height*img_width; i++)
    {
        erased_img[i*4+0] = ImgArray[i*3] * ratio;
        erased_img[i*4+1] = ImgArray[i*3+1] * ratio;
        erased_img[i*4+2] = ImgArray[i*3+2] * ratio;
        erased_img[i*4+3] = 255;
    }
    delete image;
    image = new QImage(erased_img, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    on_FindColorAction_triggered();
    return;
}

void MainWindow::on_close_open_disk_5_5_triggered()
{
    std::vector<int> extract_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            extract_color.push_back(i);
    }
    unsigned int erase_num = extract_color.size();
    if (erase_num == 0)
        return;
    //std::vector<int> selected_main_color = Find_suitable_main_color(select_color);
    unsigned char *ExtractColor = new unsigned char [img_height*img_width];
    unsigned char *ProcessedExtractColor = new unsigned char [img_height*img_width];
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    dilate(ProcessedExtractColor, 0, img_height, img_width, 5);
    erode(ProcessedExtractColor, 0, img_height, img_width, 5);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ProcessedExtractColor[i] - ExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, true);
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    erode(ProcessedExtractColor, 0, img_height, img_width, 5);
    dilate(ProcessedExtractColor, 0, img_height, img_width, 5);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ExtractColor[i] - ProcessedExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, false);
    delete [] ExtractColor;
    delete [] ProcessedExtractColor;
    return;
}
void MainWindow::on_close_open_disk_7_7_triggered()
{
    std::vector<int> extract_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            extract_color.push_back(i);
    }
    unsigned int erase_num = extract_color.size();
    if (erase_num == 0)
        return;
    //std::vector<int> selected_main_color = Find_suitable_main_color(select_color);
    unsigned char *ExtractColor = new unsigned char [img_height*img_width];
    unsigned char *ProcessedExtractColor = new unsigned char [img_height*img_width];
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    dilate(ProcessedExtractColor, 0, img_height, img_width, 7);
    erode(ProcessedExtractColor, 0, img_height, img_width, 7);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ProcessedExtractColor[i] - ExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, true);
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    erode(ProcessedExtractColor, 0, img_height, img_width, 7);
    dilate(ProcessedExtractColor, 0, img_height, img_width, 7);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ExtractColor[i] - ProcessedExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, false);
    delete [] ExtractColor;
    delete [] ProcessedExtractColor;
    return;
}
void MainWindow::on_close_open_disk_9_9_triggered()
{
    // extract selected colors and convert to binary image1
    // erode and dilate the binary image using the same kernel, then get binary image2
    // pixels to erase = image1 - image2
    std::vector<int> extract_color;
    for (int i=0; i<display_num_color; i++)
    {
        if (select[i]->isChecked() == true && sorted_color[i].size() != 0)
            extract_color.push_back(i);
    }
    unsigned int erase_num = extract_color.size();
    if (erase_num == 0)
        return;
    unsigned char *ExtractColor = new unsigned char [img_height*img_width];
    unsigned char *ProcessedExtractColor = new unsigned char [img_height*img_width];
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    dilate(ProcessedExtractColor, 0, img_height, img_width, 9);
    erode(ProcessedExtractColor, 0, img_height, img_width, 9);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ProcessedExtractColor[i] - ExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, true);
    selected_colors_to_binary_array(ExtractColor, extract_color, img_height, img_width, 255);
    memcpy(ProcessedExtractColor, ExtractColor, static_cast<unsigned int>(img_height*img_width));
    erode(ProcessedExtractColor, 0, img_height, img_width, 9);
    dilate(ProcessedExtractColor, 0, img_height, img_width, 9);
    for (int i=0; i<img_height*img_width; i++)
    {
        ProcessedExtractColor[i] = ExtractColor[i] - ProcessedExtractColor[i];
    }
    erase_select_area_with_main_color(ProcessedExtractColor, false);
    delete [] ExtractColor;
    delete [] ProcessedExtractColor;
    return;
}

void MainWindow::on_open_disk_3_3_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 0, img_height, img_width, 3);
    dilate(binary_img, 0, img_height, img_width, 3);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);
    /*if (max_history_step <= current_image_index+1)
    {
        delete [] image_history[0].image_array;
        image_history[0].image_array = nullptr;
        for (int i=1; i<current_image_index; i++)
        {
            image_history[i-1].image_type = image_history[i].image_type;
            image_history[i-1].image_array = image_history[i].image_array;
        }
        current_image_index -= 1;
    }
    unsigned char * record_image_array = new unsigned char [img_width*img_height];
    memcpy(record_image_array, binary_img, static_cast<unsigned int>(img_height*img_width));
    image_history[current_image_index + 1].image_type = "binary";
    image_history[current_image_index + 1].image_array = record_image_array;
    current_image_index += 1;*/
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_open_disk_5_5_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 0, img_height, img_width, 5);
    dilate(binary_img, 0, img_height, img_width, 5);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_open_disk_7_7_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 0, img_height, img_width, 7);
    dilate(binary_img, 0, img_height, img_width, 7);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_disk_3_3_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 0, img_height, img_width, 3);
    erode(binary_img, 0, img_height, img_width, 3);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_disk_5_5_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 0, img_height, img_width, 5);
    erode(binary_img, 0, img_height, img_width, 5);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_disk_7_7_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 0, img_height, img_width, 7);
    erode(binary_img, 0, img_height, img_width, 7);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}

void MainWindow::on_open_square_3_3_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 1, img_height, img_width, 3);
    dilate(binary_img, 1, img_height, img_width, 3);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_open_square_5_5_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 1, img_height, img_width, 5);
    dilate(binary_img, 1, img_height, img_width, 5);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_open_square_7_7_triggered()
{
    if (binary_img == nullptr)
        return;
    erode(binary_img, 1, img_height, img_width, 7);
    dilate(binary_img, 1, img_height, img_width, 7);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_square_3_3_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 1, img_height, img_width, 3);
    erode(binary_img, 1, img_height, img_width, 3);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_square_5_5_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 1, img_height, img_width, 5);
    erode(binary_img, 1, img_height, img_width, 5);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}
void MainWindow::on_close_square_7_7_triggered()
{
    if (binary_img == nullptr)
        return;
    dilate(binary_img, 1, img_height, img_width, 7);
    erode(binary_img, 1, img_height, img_width, 7);
    delete bin_image;
    bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
    QImage resizedImg = bin_image->scaled(display_img_width, display_img_height);

    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = bin_image;
    autosave("tmp_binary.png");
    for (int i=0; i<display_num_color; i++)
    {
        select[i]->setCheckState(Qt::Unchecked);
    }
    return;
}

void MainWindow::generate_intergation_array(unsigned int *intergation_array, unsigned char *binary_img, int height, int width)
{
    memset(intergation_array, 0, static_cast<unsigned int>(width*height)*sizeof(unsigned int));
    intergation_array[0] = binary_img[0];
    for (int i=1; i<width; i++) {intergation_array[i] = intergation_array[i-1] + binary_img[i];}
    for (int i=1; i<height; i++) {intergation_array[i*width] = intergation_array[(i-1)*width] + binary_img[i*width];}
    for (int i=1; i<height; i++)
    {
        for (int j=1; j<width; j++)
        {
            intergation_array[i*width+j] = binary_img[i*width+j] + intergation_array[i*width+j-1] + intergation_array[(i-1)*width+j] - intergation_array[(i-1)*width+j-1];
        }
    }
    return;
}

void MainWindow::selected_colors_to_binary_array(unsigned char *binary_img, std::vector<int> select_color, int height, int width, unsigned char fill_num=1)
{
    memset(binary_img, 0, static_cast<unsigned int>(height*width));
    unsigned char color_size = static_cast<unsigned char>(256/pow(2, bit_mov_right));
    unsigned char r,g,b;
    for (unsigned int k=0; k<select_color.size(); k++)
    {
        unsigned int color_index = sorted_color[select_color.at(k)].at(0).color_index;
        b = static_cast<unsigned char>(color_index / (color_size*color_size));
        g = static_cast<unsigned char>((color_index - b * (color_size * color_size)) / color_size);
        r = static_cast<unsigned char>(color_index % color_size);
        for (int i=0; i<height; i++)
        {
            for (int j=0; j<width; j++)
            {
                if (b==ImgArray[i*width*3+j*3] && g==ImgArray[i*width*3+j*3+1] && r==ImgArray[i*width*3+j*3+2])
                {
                    *(binary_img + i*width+j) = fill_num;
                }
            }
        }
    }
    return;
}

void MainWindow::on_median_3_3_triggered()
{
    unsigned char *ImgPointer=image->bits();
    median_filter(ImgPointer, 3, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_median_5_5_triggered()
{
    unsigned char *ImgPointer=image->bits();
    median_filter(ImgPointer, 5, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_median_7_7_triggered()
{
    unsigned char *ImgPointer=image->bits();
    median_filter(ImgPointer, 7, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_median_9_9_triggered()
{
    unsigned char *ImgPointer=image->bits();
    median_filter(ImgPointer, 9, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_median_11_11_triggered()
{
    unsigned char *ImgPointer=image->bits();
    median_filter(ImgPointer, 11, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}

void MainWindow::on_major_3_3_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 3, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_5_5_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 5, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_7_7_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 7, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_9_9_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 9, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_11_11_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 11, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_13_13_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 13, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}
void MainWindow::on_major_15_15_triggered()
{
    unsigned char *ImgPointer=image->bits();
    major_filter(ImgPointer, 15, img_height, img_width);
    on_FindColorAction_triggered();
    return;
}

void MainWindow::median_filter(unsigned char *ImgPointer, unsigned char size, int height, int width)
{
    unsigned char *ImgArray1 = new unsigned char [height*width];
    unsigned char *ImgArray2 = new unsigned char [height*width];
    unsigned char *ImgArray3 = new unsigned char [height*width];
    for (int i=0; i<height*width; i++)
    {
        ImgArray1[i] = ImgPointer[i*4+0];
        ImgArray2[i] = ImgPointer[i*4+1];
        ImgArray3[i] = ImgPointer[i*4+2];
    }
    while(FilterImgArray == nullptr) {FilterImgArray = new unsigned char [height*width*4];}
    int radius = static_cast<int>((size - 1)/2);
    int win_size = size * size;
    int mid_pos = win_size / 2 + 1;
    const int histogramSize  = 256;

    int right = width - radius;
    int bottom = height - radius;
    int begin, end;

    int sum1 = 0;
    int sum2 = 0;
    int sum3 = 0;

    int Hist1[histogramSize] = {0};
    int Hist2[histogramSize] = {0};
    int Hist3[histogramSize] = {0};

    unsigned char value1 = 0;
    unsigned char value2 = 0;
    unsigned char value3 = 0;

    for (int j=radius; j<bottom; j++)
    {
        for (int i=radius; i<right; i++)
        {
            if (i==radius)
            {
                memset(Hist1, 0, histogramSize*sizeof(int));
                memset(Hist2, 0, histogramSize*sizeof(int));
                memset(Hist3, 0, histogramSize*sizeof(int));
                for (int y=j-radius; y<=j+radius; y++)
                {
                    for (int x=i-radius; x<=i+radius; x++)
                    {
                        value1 = ImgArray1[y*width+x];
                        ++Hist1[value1];

                        value2 = ImgArray2[y*width+x];
                        ++Hist2[value2];

                        value3 = ImgArray3[y*width+x];
                        ++Hist3[value3];
                    }
                }
            }
            else
            {
                begin = i - radius - 1;
                end = i + radius;
                for (int y=j-radius; y<=j+radius; y++)
                {
                    value1 = ImgArray1[y*width+begin];
                    --Hist1[value1];
                    value1 = ImgArray1[y*width+end];
                    ++Hist1[value1];

                    value2 = ImgArray2[y*width+begin];
                    --Hist2[value2];
                    value2 = ImgArray2[y*width+end];
                    ++Hist2[value2];

                    value3 = ImgArray3[y*width+begin];
                    --Hist3[value3];
                    value3 = ImgArray3[y*width+end];
                    ++Hist3[value3];
                }
            }
            sum1 = 0;
            for (int k=0; k<histogramSize; k++)
            {
                sum1 += Hist1[k];
                if (sum1 >= mid_pos)
                {
                    FilterImgArray[(j*width+i)*4] = static_cast<unsigned char>(k);
                    break;
                }
            }
            sum2 = 0;
            for (int k=0; k<histogramSize; k++)
            {
                sum2 += Hist2[k];
                if (sum2 >= mid_pos)
                {
                    FilterImgArray[(j*width+i)*4+1] = static_cast<unsigned char>(k);
                    break;
                }
            }
            sum3 = 0;
            for (int k=0; k<histogramSize; k++)
            {
                sum3 += Hist3[k];
                if (sum3 >= mid_pos)
                {
                    FilterImgArray[(j*width+i)*4+2] = static_cast<unsigned char>(k);
                    break;
                }
            }
            FilterImgArray[(j*width+i)*4+3] = ImgPointer[(j * width + i) * 4 + 3];
        }
    }

    int idxTop, idxBot;
    for (int i=0; i<width; i++)
    {
        for (int j=2; j<radius; j++)
        {
            idxTop = j*width + i;
            FilterImgArray[idxTop*4] = ImgPointer[idxTop*4];
            FilterImgArray[idxTop*4+1] = ImgPointer[idxTop*4+1];
            FilterImgArray[idxTop*4+2] = ImgPointer[idxTop*4+2];
            FilterImgArray[idxTop*4+3] = ImgPointer[idxTop*4+3];
            idxBot = (height-j-1)*width + i;
            FilterImgArray[idxBot*4] = ImgPointer[idxBot*4];
            FilterImgArray[idxBot*4+1] = ImgPointer[idxBot*4+1];
            FilterImgArray[idxBot*4+2] = ImgPointer[idxBot*4+2];
            FilterImgArray[idxBot*4+3] = ImgPointer[idxBot*4+3];
        }
    }

    int idxLeft, idxRight;
    for (int j=radius; j<height-radius-1; j++)
    {
        for (int i=2; i<radius; i++)
        {
            idxLeft = j*width + i;
            FilterImgArray[idxLeft*4] = ImgPointer[idxLeft*4];
            FilterImgArray[idxLeft*4+1] = ImgPointer[idxLeft*4+1];
            FilterImgArray[idxLeft*4+2] = ImgPointer[idxLeft*4+2];
            FilterImgArray[idxLeft*4+3] = ImgPointer[idxLeft*4+3];
            idxRight = j*width + width - i - 1;
            FilterImgArray[idxRight*4] = ImgPointer[idxRight*4];
            FilterImgArray[idxRight*4+1] = ImgPointer[idxRight*4+1];
            FilterImgArray[idxRight*4+2] = ImgPointer[idxRight*4+2];
            FilterImgArray[idxRight*4+3] = ImgPointer[idxRight*4+3];
        }
    }

    for (int i = 2; i < width - 2; i++)
    {
        FilterImgArray[(0 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(0 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(0 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(0 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];
        FilterImgArray[(1 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(1 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(1 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(1 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];

        FilterImgArray[((height - 1) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 1) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 1) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 1) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
        FilterImgArray[((height - 2) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 2) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 2) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 2) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
    }
    for (int i = 0; i < height; i++)
    {
        FilterImgArray[(i*width + 0) * 4] = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 0) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 0) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 0) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];
        FilterImgArray[(i*width + 1) * 4] = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 1) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 1) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 1) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];

        FilterImgArray[(i*width + width - 1) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 1) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 1) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 1) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
        FilterImgArray[(i*width + width - 2) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 2) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 2) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 2) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
    }

    delete [] ImgArray1;
    delete [] ImgArray2;
    delete [] ImgArray3;
    delete image;
    image = new QImage(FilterImgArray, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    delete [] grayImgArray;
    grayImgArray = nullptr;
    return;
}
void MainWindow::major_filter(unsigned char *ImgPointer, unsigned char size, int height, int width)
{
    unsigned char *ImgArray1 = new unsigned char [height*width];
    unsigned char *ImgArray2 = new unsigned char [height*width];
    unsigned char *ImgArray3 = new unsigned char [height*width];
    for (int i=0; i<height*width; i++)
    {
        ImgArray1[i] = ImgPointer[i*4+0];
        ImgArray2[i] = ImgPointer[i*4+1];
        ImgArray3[i] = ImgPointer[i*4+2];
    }
    while(FilterImgArray == nullptr) {FilterImgArray = new unsigned char [height*width*4];}
    int radius = static_cast<int>((size - 1)/2);

    const unsigned int histogramSize  = 256;
    int Hist1[histogramSize] = {0};
    int Hist2[histogramSize] = {0};
    int Hist3[histogramSize] = {0};

    int right = width - radius;
    int bottom = height - radius;
    int begin, end;

    unsigned int max_index1 = 0;
    unsigned int max_index2 = 0;
    unsigned int max_index3 = 0;
    unsigned char value1 = 0;
    unsigned char value2 = 0;
    unsigned char value3 = 0;

    for (int j=radius; j<bottom; j++)
    {
        for (int i=radius; i<right; i++)
        {
            if (i==radius)
            {
                memset(Hist1, 0, histogramSize*sizeof(int));
                memset(Hist2, 0, histogramSize*sizeof(int));
                memset(Hist3, 0, histogramSize*sizeof(int));
                for (int y=j-radius; y<=j+radius; y++)
                {
                    for (int x=i-radius; x<=i+radius; x++)
                    {
                        value1 = ImgArray1[y*width+x];
                        ++Hist1[value1];

                        value2 = ImgArray2[y*width+x];
                        ++Hist2[value2];

                        value3 = ImgArray3[y*width+x];
                        ++Hist3[value3];
                    }
                }
            }
            else
            {
                begin = i - radius - 1;
                end = i + radius;
                for (int y=j-radius; y<=j+radius; y++)
                {
                    value1 = ImgArray1[y*width+begin];
                    --Hist1[value1];
                    value1 = ImgArray1[y*width+end];
                    ++Hist1[value1];

                    value2 = ImgArray2[y*width+begin];
                    --Hist2[value2];
                    value2 = ImgArray2[y*width+end];
                    ++Hist2[value2];

                    value3 = ImgArray3[y*width+begin];
                    --Hist3[value3];
                    value3 = ImgArray3[y*width+end];
                    ++Hist3[value3];
                }
            }
            max_index1 = 0;
            max_index2 = 0;
            max_index3 = 0;
            for (unsigned int k=1; k < histogramSize; k++)
            {
                if (Hist1[k] >= Hist1[max_index1])
                    max_index1 = k;
                if (Hist2[k] >= Hist2[max_index2])
                    max_index2 = k;
                if (Hist3[k] >= Hist3[max_index3])
                    max_index3 = k;
            }
            FilterImgArray[(j*width+i)*4]  = static_cast<unsigned char>(max_index1);
            FilterImgArray[(j*width+i)*4+1] = static_cast<unsigned char>(max_index2);
            FilterImgArray[(j*width+i)*4+2] = static_cast<unsigned char>(max_index3);
            FilterImgArray[(j*width+i)*4+3] = ImgPointer[(j * width + i) * 4 + 3];
        }
    }
    int idxTop, idxBot;
    for (int i=0; i<width; i++)
    {
        for (int j=2; j<radius; j++)
        {
            idxTop = j*width + i;
            FilterImgArray[idxTop*4] = ImgPointer[idxTop*4];
            FilterImgArray[idxTop*4+1] = ImgPointer[idxTop*4+1];
            FilterImgArray[idxTop*4+2] = ImgPointer[idxTop*4+2];
            FilterImgArray[idxTop*4+3] = ImgPointer[idxTop*4+3];
            idxBot = (height-j-1)*width + i;
            FilterImgArray[idxBot*4] = ImgPointer[idxBot*4];
            FilterImgArray[idxBot*4+1] = ImgPointer[idxBot*4+1];
            FilterImgArray[idxBot*4+2] = ImgPointer[idxBot*4+2];
            FilterImgArray[idxBot*4+3] = ImgPointer[idxBot*4+3];
        }
    }
    int idxLeft, idxRight;
    for (int j=radius; j<height-radius-1; j++)
    {
        for (int i=2; i<radius; i++)
        {
            idxLeft = j*width + i;
            FilterImgArray[idxLeft*4] = ImgPointer[idxLeft*4];
            FilterImgArray[idxLeft*4+1] = ImgPointer[idxLeft*4+1];
            FilterImgArray[idxLeft*4+2] = ImgPointer[idxLeft*4+2];
            FilterImgArray[idxLeft*4+3] = ImgPointer[idxLeft*4+3];
            idxRight = j*width + width - i - 1;
            FilterImgArray[idxRight*4] = ImgPointer[idxRight*4];
            FilterImgArray[idxRight*4+1] = ImgPointer[idxRight*4+1];
            FilterImgArray[idxRight*4+2] = ImgPointer[idxRight*4+2];
            FilterImgArray[idxRight*4+3] = ImgPointer[idxRight*4+3];
        }
    }

    for (int i = 2; i < width-2; i++)
    {
        FilterImgArray[(0 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(0 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(0 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(0 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];
        FilterImgArray[(1 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(1 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(1 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(1 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];

        FilterImgArray[((height - 1) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 1) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 1) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 1) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
        FilterImgArray[((height - 2) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 2) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 2) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 2) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
    }
    for (int i = 0; i < height; i++)
    {
        FilterImgArray[(i*width + 0) * 4]     = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 0) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 0) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 0) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];
        FilterImgArray[(i*width + 1) * 4]     = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 1) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 1) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 1) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];

        FilterImgArray[(i*width + width - 1) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 1) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 1) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 1) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
        FilterImgArray[(i*width + width - 2) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 2) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 2) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 2) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
    }

    delete [] ImgArray1;
    delete [] ImgArray2;
    delete [] ImgArray3;
    delete image;
    image = new QImage(FilterImgArray, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    return;
}

/*
void MainWindow::median_filter(unsigned char *ImgPointer, unsigned char size, int height, int width)
{
    //unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *grayImgArray = new unsigned char [height*width];
    for (int i=0; i<height*width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgPointer[i*4]*0.114 + ImgPointer[i*4+1]*0.586 + ImgPointer[i*4+2]*0.3);
    }
    while(FilterImgArray == nullptr) {FilterImgArray = new unsigned char [height*width*4];}
    int radius = static_cast<int>((size - 1)/2);
    int win_size = size * size;
    int mid_pos = win_size / 2 + 1;
    const int histogramSize  = 256;
    int Hist[histogramSize] = {0};
    int HistIndex[histogramSize] = {0};

    int right = width - radius;
    int bottom = height - radius;
    int begin, end;
    int sum = 0;
    int tmp = 0;
    unsigned char value = 0;
    for (int j=radius; j<bottom; j++)
    {
        for (int i=radius; i<right; i++)
        {
            if (i==radius)
            {
                memset(Hist, 0, histogramSize*sizeof(int));
                for (int y=j-radius; y<=j+radius; y++)
                {
                    for (int x=i-radius; x<=i+radius; x++)
                    {
                        value = grayImgArray[y*width+x];
                        Hist[value]++;
                        HistIndex[value] = y*width+x;
                    }
                }
            }
            else
            {
                begin = i - radius - 1;
                end = i + radius;
                for (int y=j-radius; y<=j+radius; y++)
                {
                    value = grayImgArray[y*width+begin];
                    Hist[value]--;
                    HistIndex[value] = y*width+begin;
                    value = grayImgArray[y*width+end];
                    Hist[value]++;
                    HistIndex[value] = y*width+end;
                }
            }
            sum = 0;
            for (int k=0; k<histogramSize; k++)
            {
                sum += Hist[k];
                if (sum >= mid_pos)
                {
                    tmp = HistIndex[k];
                    FilterImgArray[(j*width+i)*4] = ImgPointer[tmp*4];
                    FilterImgArray[(j*width+i)*4+1] = ImgPointer[tmp*4+1];
                    FilterImgArray[(j*width+i)*4+2] = ImgPointer[tmp*4+2];
                    FilterImgArray[(j*width+i)*4+3] = ImgPointer[tmp*4+3];
                    break;
                }
            }
        }
    }

    int idxTop, idxBot;
    for (int i=0; i<width; i++)
    {
        for (int j=2; j<radius; j++)
        {
            idxTop = j*width + i;
            FilterImgArray[idxTop*4] = ImgPointer[idxTop*4];
            FilterImgArray[idxTop*4+1] = ImgPointer[idxTop*4+1];
            FilterImgArray[idxTop*4+2] = ImgPointer[idxTop*4+2];
            FilterImgArray[idxTop*4+3] = ImgPointer[idxTop*4+3];
            idxBot = (height-j-1)*width + i;
            FilterImgArray[idxBot*4] = ImgPointer[idxBot*4];
            FilterImgArray[idxBot*4+1] = ImgPointer[idxBot*4+1];
            FilterImgArray[idxBot*4+2] = ImgPointer[idxBot*4+2];
            FilterImgArray[idxBot*4+3] = ImgPointer[idxBot*4+3];
        }
    }

    int idxLeft, idxRight;
    for (int j=radius; j<height-radius-1; j++)
    {
        for (int i=2; i<radius; i++)
        {
            idxLeft = j*width + i;
            FilterImgArray[idxLeft*4] = ImgPointer[idxLeft*4];
            FilterImgArray[idxLeft*4+1] = ImgPointer[idxLeft*4+1];
            FilterImgArray[idxLeft*4+2] = ImgPointer[idxLeft*4+2];
            FilterImgArray[idxLeft*4+3] = ImgPointer[idxLeft*4+3];
            idxRight = j*width + width - i - 1;
            FilterImgArray[idxRight*4] = ImgPointer[idxRight*4];
            FilterImgArray[idxRight*4+1] = ImgPointer[idxRight*4+1];
            FilterImgArray[idxRight*4+2] = ImgPointer[idxRight*4+2];
            FilterImgArray[idxRight*4+3] = ImgPointer[idxRight*4+3];
        }
    }

    for (int i = 2; i < width - 2; i++)
    {
        FilterImgArray[(0 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(0 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(0 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(0 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];
        FilterImgArray[(1 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(1 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(1 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(1 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];

        FilterImgArray[((height - 1) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 1) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 1) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 1) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
        FilterImgArray[((height - 2) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 2) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 2) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 2) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
    }
    for (int i = 0; i < height; i++)
    {
        FilterImgArray[(i*width + 0) * 4] = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 0) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 0) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 0) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];
        FilterImgArray[(i*width + 1) * 4] = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 1) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 1) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 1) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];

        FilterImgArray[(i*width + width - 1) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 1) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 1) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 1) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
        FilterImgArray[(i*width + width - 2) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 2) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 2) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 2) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
    }

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            FilterImgArray[(i*width + j) * 4 + 0] = FilterImgArray[(0 * width + 2) * 4 + 0];
            FilterImgArray[(i*width + j) * 4 + 1] = FilterImgArray[(0 * width + 2) * 4 + 1];
            FilterImgArray[(i*width + j) * 4 + 2] = FilterImgArray[(0 * width + 2) * 4 + 2];
            FilterImgArray[(i*width + j) * 4 + 3] = FilterImgArray[(0 * width + 2) * 4 + 3];

            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 0] = FilterImgArray[(0 * width + (width - 3)) * 4 + 0];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 1] = FilterImgArray[(0 * width + (width - 3)) * 4 + 1];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 2] = FilterImgArray[(0 * width + (width - 3)) * 4 + 2];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 3] = FilterImgArray[(0 * width + (width - 3)) * 4 + 3];

            FilterImgArray[((height - 1 - i)*width + j) * 4 + 0] = FilterImgArray[((height - 1) * width + 2) * 4 + 0];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 1] = FilterImgArray[((height - 1) * width + 2) * 4 + 1];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 2] = FilterImgArray[((height - 1) * width + 2) * 4 + 2];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 3] = FilterImgArray[((height - 1) * width + 2) * 4 + 3];

            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 0] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 0];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 1] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 1];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 2] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 2];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 3] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 3];
        }
    }

    delete image;
    image = new QImage(FilterImgArray, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    delete [] grayImgArray;
    grayImgArray = nullptr;
    return;
}
*/
/*
void MainWindow::major_filter(unsigned char *ImgPointer, unsigned char size, int height, int width)
{
    //unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *grayImgArray = new unsigned char [height*width];
    for (int i=0; i<height*width; i++)
    {
        grayImgArray[i] = static_cast<unsigned char>(ImgPointer[i*4]*0.114 + ImgPointer[i*4+1]*0.586 + ImgPointer[i*4+2]*0.3);
    }
    while(FilterImgArray == nullptr) {FilterImgArray = new unsigned char [height*width*4];}
    int radius = static_cast<int>((size - 1)/2);
    //int win_size = size * size;
    //int mid_pos = win_size / 2 + 1;
    const int histogramSize  = 256;
    int Hist[histogramSize] = {0};
    int HistIndex[histogramSize] = {0};

    int right = width - radius;
    int bottom = height - radius;
    int begin, end;
    int max_index = 0;
    int tmp = 0;
    unsigned char value = 0;
    for (int j=radius; j<bottom; j++)
    {
        for (int i=radius; i<right; i++)
        {
            if (i==radius)
            {
                memset(Hist, 0, histogramSize*sizeof(int));
                for (int y=j-radius; y<=j+radius; y++)
                {
                    for (int x=i-radius; x<=i+radius; x++)
                    {
                        value = grayImgArray[y*width+x];
                        Hist[value]++;
                        HistIndex[value] = y*width+x;
                    }
                }
            }
            else
            {
                begin = i - radius - 1;
                end = i + radius;
                for (int y=j-radius; y<=j+radius; y++)
                {
                    value = grayImgArray[y*width+begin];
                    Hist[value]--;
                    HistIndex[value] = y*width+begin;
                    value = grayImgArray[y*width+end];
                    Hist[value]++;
                    HistIndex[value] = y*width+end;
                }
            }
            max_index = 0;
            for (int k=1; k<histogramSize; k++)
            {
                if (Hist[k] >= Hist[max_index])
                    max_index = k;
            }
            tmp = HistIndex[max_index];
            FilterImgArray[(j*width+i)*4] = ImgPointer[tmp*4];
            FilterImgArray[(j*width+i)*4+1] = ImgPointer[tmp*4+1];
            FilterImgArray[(j*width+i)*4+2] = ImgPointer[tmp*4+2];
            FilterImgArray[(j*width+i)*4+3] = ImgPointer[tmp*4+3];
        }
    }
    int idxTop, idxBot;
    for (int i=0; i<width; i++)
    {
        for (int j=2; j<radius; j++)
        {
            idxTop = j*width + i;
            FilterImgArray[idxTop*4] = ImgPointer[idxTop*4];
            FilterImgArray[idxTop*4+1] = ImgPointer[idxTop*4+1];
            FilterImgArray[idxTop*4+2] = ImgPointer[idxTop*4+2];
            FilterImgArray[idxTop*4+3] = ImgPointer[idxTop*4+3];
            idxBot = (height-j-1)*width + i;
            FilterImgArray[idxBot*4] = ImgPointer[idxBot*4];
            FilterImgArray[idxBot*4+1] = ImgPointer[idxBot*4+1];
            FilterImgArray[idxBot*4+2] = ImgPointer[idxBot*4+2];
            FilterImgArray[idxBot*4+3] = ImgPointer[idxBot*4+3];
        }
    }
    int idxLeft, idxRight;
    for (int j=radius; j<height-radius-1; j++)
    {
        for (int i=2; i<radius; i++)
        {
            idxLeft = j*width + i;
            FilterImgArray[idxLeft*4] = ImgPointer[idxLeft*4];
            FilterImgArray[idxLeft*4+1] = ImgPointer[idxLeft*4+1];
            FilterImgArray[idxLeft*4+2] = ImgPointer[idxLeft*4+2];
            FilterImgArray[idxLeft*4+3] = ImgPointer[idxLeft*4+3];
            idxRight = j*width + width - i - 1;
            FilterImgArray[idxRight*4] = ImgPointer[idxRight*4];
            FilterImgArray[idxRight*4+1] = ImgPointer[idxRight*4+1];
            FilterImgArray[idxRight*4+2] = ImgPointer[idxRight*4+2];
            FilterImgArray[idxRight*4+3] = ImgPointer[idxRight*4+3];
        }
    }

    for (int i = 2; i < width-2; i++)
    {
        FilterImgArray[(0 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(0 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(0 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(0 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];
        FilterImgArray[(1 * width + i) * 4]     = FilterImgArray[(2 * width + i) * 4];
        FilterImgArray[(1 * width + i) * 4 + 1] = FilterImgArray[(2 * width + i) * 4 + 1];
        FilterImgArray[(1 * width + i) * 4 + 2] = FilterImgArray[(2 * width + i) * 4 + 2];
        FilterImgArray[(1 * width + i) * 4 + 3] = FilterImgArray[(2 * width + i) * 4 + 3];

        FilterImgArray[((height - 1) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 1) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 1) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 1) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
        FilterImgArray[((height - 2) * width + i) * 4]     = FilterImgArray[((height - 3) * width + i) * 4];
        FilterImgArray[((height - 2) * width + i) * 4 + 1] = FilterImgArray[((height - 3) * width + i) * 4 + 1];
        FilterImgArray[((height - 2) * width + i) * 4 + 2] = FilterImgArray[((height - 3) * width + i) * 4 + 2];
        FilterImgArray[((height - 2) * width + i) * 4 + 3] = FilterImgArray[((height - 3) * width + i) * 4 + 3];
    }
    for (int i = 0; i < height; i++)
    {
        FilterImgArray[(i*width + 0) * 4]     = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 0) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 0) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 0) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];
        FilterImgArray[(i*width + 1) * 4]     = FilterImgArray[(i*width + 2) * 4];
        FilterImgArray[(i*width + 1) * 4 + 1] = FilterImgArray[(i*width + 2) * 4 + 1];
        FilterImgArray[(i*width + 1) * 4 + 2] = FilterImgArray[(i*width + 2) * 4 + 2];
        FilterImgArray[(i*width + 1) * 4 + 3] = FilterImgArray[(i*width + 2) * 4 + 3];

        FilterImgArray[(i*width + width - 1) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 1) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 1) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 1) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
        FilterImgArray[(i*width + width - 2) * 4]     = FilterImgArray[(i*width + width - 3) * 4];
        FilterImgArray[(i*width + width - 2) * 4 + 1] = FilterImgArray[(i*width + width - 3) * 4 + 1];
        FilterImgArray[(i*width + width - 2) * 4 + 2] = FilterImgArray[(i*width + width - 3) * 4 + 2];
        FilterImgArray[(i*width + width - 2) * 4 + 3] = FilterImgArray[(i*width + width - 3) * 4 + 3];
    }

    //for (int i = 0; i < 2; i++)
    {
    //    for (int j = 0; j < 2; j++)
        {
            FilterImgArray[(i*width + j) * 4 + 0] = FilterImgArray[(0 * width + 2) * 4 + 0];
            FilterImgArray[(i*width + j) * 4 + 1] = FilterImgArray[(0 * width + 2) * 4 + 1];
            FilterImgArray[(i*width + j) * 4 + 2] = FilterImgArray[(0 * width + 2) * 4 + 2];
            FilterImgArray[(i*width + j) * 4 + 3] = FilterImgArray[(0 * width + 2) * 4 + 3];

    //        FilterImgArray[(i*width + (width - 1 - j)) * 4 + 0] = FilterImgArray[(0 * width + (width - 3)) * 4 + 0];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 1] = FilterImgArray[(0 * width + (width - 3)) * 4 + 1];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 2] = FilterImgArray[(0 * width + (width - 3)) * 4 + 2];
            FilterImgArray[(i*width + (width - 1 - j)) * 4 + 3] = FilterImgArray[(0 * width + (width - 3)) * 4 + 3];

    //        FilterImgArray[((height - 1 - i)*width + j) * 4 + 0] = FilterImgArray[((height - 1) * width + 2) * 4 + 0];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 1] = FilterImgArray[((height - 1) * width + 2) * 4 + 1];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 2] = FilterImgArray[((height - 1) * width + 2) * 4 + 2];
            FilterImgArray[((height - 1 - i)*width + j) * 4 + 3] = FilterImgArray[((height - 1) * width + 2) * 4 + 3];

    //        FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 0] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 0];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 1] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 1];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 2] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 2];
            FilterImgArray[((height - 1 - i)*width + (width - 1 - j)) * 4 + 3] = FilterImgArray[((height - 1) * width + width - 3) * 4 + 3];
        }
    //}

    delete image;
    image = new QImage(FilterImgArray, img_width, img_height, QImage::Format_ARGB32);
    QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    SaveImage = image;
    delete [] grayImgArray;
    grayImgArray = nullptr;
    return;
}
*/

void MainWindow::dilate(unsigned char *binary_img, int kind, int height, int width, int kernel_size=3)
{
    unsigned char *dilateBuffer = new unsigned char [height * width];
    memset(dilateBuffer, 0, static_cast<unsigned int>(height * width));
    unsigned char *kernel = new unsigned char [kernel_size*kernel_size];
    int mid_pos = (kernel_size - 1) / 2;
    if (kind == 1) // square
    {
        for (int i=0; i<kernel_size*kernel_size; i++)
            kernel[i] = 1;
    }
    else
    {
        for (int i=0; i<kernel_size; i++)
        {
            for (int j=0; j<kernel_size; j++)
            {
                if ((i-mid_pos)*(i-mid_pos) + (j-mid_pos)*(j-mid_pos) <= mid_pos*mid_pos)
                    kernel[i*kernel_size+j] = 1;
                else
                    kernel[i*kernel_size+j] = 0;
            }
        }
    }
    unsigned char tempNum = 0;
    int index = 0;
    for (int i=mid_pos; i<height-mid_pos; i++)
    {
        for (int j=mid_pos; j<width-mid_pos; j++)
        {
            tempNum = 0;
            for (int m=-mid_pos; m<mid_pos+1; m++)
            {
                for (int n=-mid_pos; n<mid_pos+1; n++)
                {
                    index = (i+m)*width+(j+n);
                    if (binary_img[index] > tempNum && kernel[(m+mid_pos)*kernel_size+(n+mid_pos)] == 1)
                    {
                        tempNum = binary_img[index];
                    }
                }
            }
            dilateBuffer[i * width + j] = tempNum;
        }
    }
    for (int i=0; i<height*width; i++)
    {
        binary_img[i] = dilateBuffer[i];
    }
    delete [] kernel;
    kernel = nullptr;
    delete [] dilateBuffer;
    dilateBuffer = nullptr;
    return;
}

void MainWindow::erode(unsigned char *binary_img, int kind, int height, int width, int kernel_size=3)
{
    unsigned char *erodeBuffer = new unsigned char [height * width];
    memset(erodeBuffer, 0, static_cast<unsigned int>(height * width));
    unsigned char *kernel = new unsigned char [kernel_size*kernel_size];
    int mid_pos = (kernel_size - 1) / 2;
    if (kind == 1) // square
    {
        for (int i=0; i<kernel_size*kernel_size; i++)
            kernel[i] = 1;
    }
    else  // disk
    {
        for (int i=0; i<kernel_size; i++)
        {
            for (int j=0; j<kernel_size; j++)
            {
                if ((i-mid_pos)*(i-mid_pos) + (j-mid_pos)*(j-mid_pos) <= mid_pos*mid_pos)
                    kernel[i*kernel_size+j] = 1;
                else
                    kernel[i*kernel_size+j] = 0;
            }
        }
    }
    unsigned char tempNum = 0;
    int index = 0;
    for (int i=mid_pos; i<height-mid_pos; i++)
    {
        for (int j=mid_pos; j<width-mid_pos; j++)
        {
            tempNum = 255;
            for (int m=-mid_pos; m<mid_pos+1; m++)
            {
                for (int n=-mid_pos; n<mid_pos+1; n++)
                {
                    index = (i+m)*width+(j+n);
                    if (binary_img[index] < tempNum && kernel[(m+mid_pos)*kernel_size+(n+mid_pos)] == 1)
                    {
                        tempNum = binary_img[index];
                    }
                }
            }
            erodeBuffer[i * width + j] = tempNum;
        }
    }
    for (int i=0; i<height*width; i++)
    {
        binary_img[i] = erodeBuffer[i];
    }
    delete [] kernel;
    kernel = nullptr;
    delete [] erodeBuffer;
    erodeBuffer = nullptr;
    return;
}

void MainWindow::on_ReturnAction_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    if (current_image_index <= 0)
        return;
    if (image_history[current_image_index-1].image_array == nullptr)
        return;
    current_image_index -= 1;
    QImage resizedImg;
    if (image_history[current_image_index].image_type == "rgb")
    {
        delete [] approximate_img;
        approximate_img = new unsigned char [img_height*img_width*4];
        for (int i=0; i<img_height*img_width; i++)
        {
            approximate_img[i*4+0] = image_history[current_image_index].image_array[i*3] * ratio;
            approximate_img[i*4+1] = image_history[current_image_index].image_array[i*3+1] * ratio;
            approximate_img[i*4+2] = image_history[current_image_index].image_array[i*3+2] * ratio;
            approximate_img[i*4+3] = 255;
        }
        delete image;
        image = new QImage(approximate_img, img_width, img_height, QImage::Format_ARGB32);
        SaveImage = image;
        current_image_index -= 1;
        on_FindColorAction_triggered();
        resizedImg = image->scaled(display_img_width, display_img_height);
    }
    else if (image_history[current_image_index].image_type == "binary")
    {
        memcpy(binary_img, image_history[current_image_index].image_array, static_cast<unsigned int>(img_height*img_width));
        delete bin_image;
        bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
        SaveImage = bin_image;
        autosave("tmp_binary.png");
        resizedImg = bin_image->scaled(display_img_width, display_img_height);
    }
    //QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    return;
}

void MainWindow::on_NextAction_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    if (image_history[current_image_index].image_array == nullptr || current_image_index + 1 >= max_history_step)
        return;
    if (image_history[current_image_index+1].image_array == nullptr)
        return;
    current_image_index += 1;
    QImage resizedImg;
    if (image_history[current_image_index].image_type == "rgb")
    {
        delete [] approximate_img;
        approximate_img = new unsigned char [img_height*img_width*4];
        for (int i=0; i<img_height*img_width; i++)
        {
            approximate_img[i*4+0] = image_history[current_image_index].image_array[i*3] * ratio;
            approximate_img[i*4+1] = image_history[current_image_index].image_array[i*3+1] * ratio;
            approximate_img[i*4+2] = image_history[current_image_index].image_array[i*3+2] * ratio;
            approximate_img[i*4+3] = 255;
        }
        delete image;
        image = new QImage(approximate_img, img_width, img_height, QImage::Format_ARGB32);
        SaveImage = image;
        current_image_index -= 1;
        on_FindColorAction_triggered();
        resizedImg = image->scaled(display_img_width, display_img_height);
    }
    else if (image_history[current_image_index].image_type == "binary")
    {
        memcpy(binary_img, image_history[current_image_index].image_array, static_cast<unsigned int>(img_height*img_width));
        delete bin_image;
        bin_image = new QImage(binary_img, img_width, img_height, QImage::Format_Grayscale8);
        SaveImage = bin_image;
        autosave("tmp_binary.png");
        resizedImg = bin_image->scaled(display_img_width, display_img_height);
    }
    //QImage resizedImg = image->scaled(display_img_width, display_img_height);
    ImageLabel->clear();
    ImageLabel->setAlignment(Qt::AlignCenter);
    ImageLabel->setPixmap(QPixmap::fromImage(resizedImg));
    return;
}

void MainWindow::on_disk_5_5_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 0, img_height, img_width, 5);
    dilate(openWhiteArea, 0, img_height, img_width, 5);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}
void MainWindow::on_disk_7_7_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 0, img_height, img_width, 7);
    dilate(openWhiteArea, 0, img_height, img_width, 7);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}
void MainWindow::on_disk_9_9_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 0, img_height, img_width, 9);
    dilate(openWhiteArea, 0, img_height, img_width, 9);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    WhiteArea = nullptr;
    delete [] openWhiteArea;
    openWhiteArea = nullptr;
    return;
}
void MainWindow::on_disk_11_11_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 0, img_height, img_width, 11);
    dilate(openWhiteArea, 0, img_height, img_width, 11);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}

void MainWindow::on_square_5_5_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 1, img_height, img_width, 5);
    dilate(openWhiteArea, 1, img_height, img_width, 5);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}
void MainWindow::on_square_7_7_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 1, img_height, img_width, 7);
    dilate(openWhiteArea, 1, img_height, img_width, 7);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}
void MainWindow::on_square_9_9_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 1, img_height, img_width, 9);
    dilate(openWhiteArea, 1, img_height, img_width, 9);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}
void MainWindow::on_square_11_11_triggered()
{
    unsigned char ratio = static_cast<unsigned char>(pow(2, bit_mov_right));
    unsigned char *WhiteArea = new unsigned char [img_height*img_width];
    unsigned char *openWhiteArea = new unsigned char [img_height*img_width];
    for (int i=0; i<img_height*img_width; i++)
    {
        if (ImgArray[i*3] >= white_threshold/ratio && ImgArray[i*3+1] >= white_threshold/ratio && ImgArray[i*3+2] >= white_threshold/ratio)
            WhiteArea[i] = 255;
        else
            WhiteArea[i] = 0;
    }
    memcpy(openWhiteArea, WhiteArea, static_cast<unsigned int>(img_height*img_width));
    erode(openWhiteArea, 1, img_height, img_width, 11);
    dilate(openWhiteArea, 1, img_height, img_width, 11);
    for (int i=0; i<img_height*img_width; i++)
    {
        WhiteArea[i] = WhiteArea[i] - openWhiteArea[i];
    }
    erase_select_area_with_main_color(WhiteArea, false);
    delete [] WhiteArea;
    delete [] openWhiteArea;
    return;
}

MainWindow::~MainWindow()
{
    delete [] ImgArray;
    delete [] grayImgArray;
    delete [] FilterImgArray;
    delete [] color_histogram;
    delete [] binary_img;
    delete [] approximate_img;
    delete [] erased_img;
    for (int i=0; i<max_history_step; i++)
        delete [] image_history[i].image_array;
    /*
    delete image;
    delete bin_image;
    delete ImageLabel;
    for (int i=0; i<display_num_color; i++)
    {
        delete color[i];
        delete rgb[i];
    }*/
    qDebug()<<"Executed destructor";
}
