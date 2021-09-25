#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <vector>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QDoubleValidator>
#include <queue>
#include <utility>
#include <QFile>
#include <math.h>
#include <dialog.h>

//namespace Ui {
//class MainWindow;
//}

struct color_node {
    unsigned int color_freq;
    unsigned int color_index;
};

struct image_node {
    QString image_type="nullptr";
    unsigned char * image_array=nullptr;
};

struct cmp
{
    bool operator()(color_node a, color_node b)
    {
        return  a.color_freq < b.color_freq;
    }
};

/*struct contour{
    std::pair<int, int> seed_coor;
    int index = -1;
    int pixel_num = 0;

    contour(int a, int b, int c, int d)
    {
        seed_coor.first = a;
        seed_coor.second = b;
        index = c;
        pixel_num = d;
    }
};*/

const int max_display_num_color = 80;
const int max_history_step = 6;

enum LineType{inLine, outLine};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int, int, int, int, QWidget *parent = nullptr);
    void change_setting();
    void selected_colors_to_binary_array(unsigned char *, std::vector<int>, int, int, unsigned char);
    void generate_intergation_array(unsigned int *, unsigned char *, int, int);
    void median_filter(unsigned char *, unsigned char, int, int);
    void major_filter(unsigned char *, unsigned char, int, int);
    void dilate(unsigned char *, int, int, int, int);
    void erode(unsigned char *, int, int, int, int);
    void erase_select_area_with_main_color(unsigned char *, bool);
    void see_white_areas();

    void on_OpenAction_triggered();
    void on_SaveAction_triggered();
    void autosave(QString);

    void on_median_3_3_triggered();
    void on_median_5_5_triggered();
    void on_median_7_7_triggered();
    void on_median_9_9_triggered();
    void on_median_11_11_triggered();
    void on_major_3_3_triggered();
    void on_major_5_5_triggered();
    void on_major_7_7_triggered();
    void on_major_9_9_triggered();
    void on_major_11_11_triggered();
    void on_major_13_13_triggered();
    void on_major_15_15_triggered();

    void on_FindColorAction_triggered();
    void on_EraseColorByNearestColorAction_triggered();
    void on_EraseSpecificColorAction_triggered();
    void on_EraseAllBelowColorAction_triggered();
    void on_EraseAllUnshowColorAction_triggered();
    void on_ExtractColorAction_triggered();
    void on_ConvertColorAction_triggered();
    void on_EraseAllSmallContourAction_triggered();
    void on_AddColorFromBinaryImageAction_triggered();

    void on_disk_11_11_triggered(); // for filling white blank
    void on_disk_9_9_triggered();
    void on_disk_7_7_triggered();
    void on_disk_5_5_triggered();
    void on_square_11_11_triggered();
    void on_square_9_9_triggered();
    void on_square_7_7_triggered();
    void on_square_5_5_triggered();

    void on_close_open_disk_5_5_triggered();
    void on_close_open_disk_7_7_triggered();
    void on_close_open_disk_9_9_triggered();
    void on_open_disk_3_3_triggered();
    void on_open_disk_5_5_triggered();
    void on_open_disk_7_7_triggered();
    void on_close_disk_3_3_triggered();
    void on_close_disk_5_5_triggered();
    void on_close_disk_7_7_triggered();
    void on_open_square_3_3_triggered();
    void on_open_square_5_5_triggered();
    void on_open_square_7_7_triggered();
    void on_close_square_3_3_triggered();
    void on_close_square_5_5_triggered();
    void on_close_square_7_7_triggered();

    void on_ReturnAction_triggered();
    void on_NextAction_triggered();

    void on_SaveAllBinaryImageAction_triggered();
    void on_SaveWithDilate3x3Action_triggered();
    void on_SaveWithDilate5x5Action_triggered();
    void on_BinaryToSvgAction_triggered();
    void on_AllToSVGAction_triggered();
    void on_AllToSHPAction_triggered();

    void extractContourFromBinaryImg(unsigned char *,
                                     std::vector<std::vector<std::pair<float, float>>>& outContour,
                                     std::vector<std::vector<std::pair<float, float>>>& inContour,
                                     std::vector<std::vector<unsigned int>>& holeIndexs);
    double calCoutourArea(std::vector<std::pair<unsigned int, unsigned int>>&);
    void simplifyLine(std::vector<std::pair<unsigned int, unsigned int>>&,
                      std::vector<std::pair<float, float>>&, LineType);
    void contourTwoSVG(std::vector<std::vector<std::pair<float, float>>>&,
                       std::vector<std::vector<std::pair<float, float>>>&,
                       std::vector<std::vector<unsigned int>>&, QFile&, QString, int);

    void getCoordinatePage(float* coor);

    ~MainWindow();

protected:
    //void changeEvent(QEvent * event);

private:
    Dialog* getValue;
    bool first_find_color = false;
    int white_threshold = 224;
    short bit_mov_right = 4;
    unsigned int max_range_to_merge_color = 16;
    double min_color_area_ratio = 0.00002;
    //double min_main_color_ratio = 0.05;
    int current_image_index = -1;
    double min_area_ratio_to_replace = 0.15;
    int max_erase_win_size = 24;
    int notdilate = 0;

    int save_count = 0;
    int binary_save_count = 0;
    QAction *OpenAction;
    QAction *SaveAction;

    QAction *FindColorAction;
    QAction *ExtractColorAction;
    QAction *EraseColorAction;
    QAction *EraseColorByNearestColorAction;
    QAction *EraseSpecificColorAction;
    QAction *EraseAllBelowColorAction;
    QAction *EraseAllUnshowColorAction;
    QAction *ConvertColorAction;
    QAction *EraseAllSmallContourAction;
    QAction *AddColorFromBinaryImageAction;

    QAction *ReturnAction;
    QAction *NextAction;

    QAction *median_3_3;
    QAction *median_5_5;
    QAction *median_7_7;
    QAction *median_9_9;
    QAction *median_11_11;
    QAction *major_3_3;
    QAction *major_5_5;
    QAction *major_7_7;
    QAction *major_9_9;
    QAction *major_11_11;
    QAction *major_13_13;
    QAction *major_15_15;

    QAction *see_white;
    QAction *disk_5_5;
    QAction *disk_7_7;
    QAction *disk_9_9;
    QAction *disk_11_11;
    QAction *square_5_5;
    QAction *square_7_7;
    QAction *square_9_9;
    QAction *square_11_11;

    QAction *close_open_disk_5_5;
    QAction *close_open_disk_7_7;
    QAction *close_open_disk_9_9;
    QAction *open_disk_3_3;
    QAction *open_disk_5_5;
    QAction *open_disk_7_7;
    QAction *open_square_3_3;
    QAction *open_square_5_5;
    QAction *open_square_7_7;
    QAction *close_disk_3_3;
    QAction *close_disk_5_5;
    QAction *close_disk_7_7;
    QAction *close_square_3_3;
    QAction *close_square_5_5;
    QAction *close_square_7_7;

    QAction *BinaryToSVG;
    QAction *AllToSVG;
    QAction *AllToSHP;
    QAction *SaveAllBinaryImage;
    QAction *SaveWithDilate3x3;
    QAction *SaveWithDilate5x5;

    QLabel *ImageLabel;
    QImage *image = nullptr;
    QImage *bin_image = nullptr;
    QPixmap noneColor;
    QLabel *color[max_display_num_color];
    QLabel *rgb[max_display_num_color];
    QCheckBox *select[max_display_num_color];
    QImage *SaveImage;

    QLabel *settingLabel;
    QLabel *minColorAreaRatioLabel;
    QLineEdit *minColorAreaRatio;
    QLabel * rightShiftNumLabel;
    QLineEdit* rightShiftNum;
    QLabel * maxRangeToMergeColorLabel;
    QLineEdit* maxRangeToMergeColor;
    QLabel * maxMainColorNumLabel;
    QLineEdit* maxMainColorNum;
    QPushButton* OK;

    QLabel *convertColorLabel;
    QLineEdit *convertColorR;
    QLineEdit *convertColorG;
    QLineEdit *convertColorB;

    QLabel *eraseColorLabel;
    QLineEdit *eraseColorR;
    QLineEdit *eraseColorG;
    QLineEdit *eraseColorB;

    QLabel *notDilateLabel;
    QLineEdit * notDilateNum;

    QString OpenFileName = "";
    QString SaveFileName = "";
    int display_num_color = 40;
    int display_per_row = 20;
    int need_sub16 = 1;
    int square_size = 64;
    int win_width = 0;
    int win_height = 0;
    int img_width = 0;
    int img_height = 0;
    int display_img_width = 0;
    int display_img_height = 0;
    unsigned char *ImgArray = nullptr;
    unsigned char *grayImgArray = nullptr;
    unsigned char *FilterImgArray = nullptr;
    unsigned int *color_histogram = nullptr;
    unsigned char *binary_img = nullptr;
    unsigned char *approximate_img = nullptr;
    unsigned char *erased_img = nullptr;
    std::vector<color_node> sorted_color[max_display_num_color];
    image_node image_history[max_history_step];

    double minContourArea = 5;
    short maxContourNum = 1000;
};

#endif // MAINWINDOW_H
