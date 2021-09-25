#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int color_num = 40;
    int num_per_row = 20;
    int block_size = 64;
    int min_white = 224;
    if (argc > 1)
    {
        color_num = atoi(argv[1]);
    }
    if (argc > 2)
    {
        num_per_row = atoi(argv[2]);
    }
    if (argc > 3)
    {
        block_size = atoi(argv[3]);
    }
    if (argc > 4)
    {
        min_white = atoi(argv[4]);
    }
    MainWindow w(color_num, num_per_row, block_size, min_white);
    //w.setAttribute(Qt::WA_DeleteOnClose);
    w.show();

    return a.exec();
}
/*
simplify by erase points with continuous same freeman code
unsigned char code8[9] = {3, 2, 1, 4, 8, 0, 5, 6, 7};
unsigned int point_num = points.size() - 1;
std::vector<unsigned char> codeLine;
//std::vector<unsigned int> eraseIndex;
int deltaH, deltaW;
for (unsigned int i = 0; i < point_num; i++)
{
    deltaH = static_cast<int>(points[i+1].first) - static_cast<int>(points[i].first);
    deltaW = static_cast<int>(points[i+1].second) - static_cast<int>(points[i].second);
    codeLine.push_back(code8[(deltaH+1)*3 + (deltaW+1)]);
}
unsigned char prevCode;
prevCode = codeLine[0];
newPoints.push_back(points[0]);
for (unsigned int i = 1; i < point_num; i++)
{
    if (codeLine[i] != prevCode)
    {
        prevCode = codeLine[i];
        newPoints.push_back(points[i]);
    }
}
newPoints.push_back(points[0]);
points.clear();

std::vector<std::pair<unsigned int, unsigned int>> linePoints;
std::vector<std::pair<int, int>> newPoints;
//linePoints.push_back(points[0]);
unsigned int point_num = points.size() - 1;
for (unsigned int i = 0; i < point_num; i++)
{
    if (points[i].first != points[i+1].first) // check whether two points at the same line (height)
    {
        if (points[i].first > points[i+1].first && points[i].second < points[i+1].second) // code 1
        {
            if (((points[i-1].first - points[i+1].first)*(points[i-1].first - points[i+1].first)) > 1) //single point ar line x, haven't insert: insert i
                linePoints.push_back(points[i]);
            // multiple points at the same line, only save the leftmost point at i+1
            linePoints.push_back(points[i+1]);
        }
        else
        {
            linePoints.push_back(points[i]);
        }
    }
}
points.clear();
// check whether the polygon is closed
if (linePoints.back().first != linePoints[0].first || linePoints.back().second != linePoints[0].second)
{
    linePoints.push_back(linePoints[0]);
}
// simplify by erase points with continuous same direction code
//unsigned int height = static_cast<unsigned int>(img_height);
point_num = linePoints.size() - 1;
newPoints.push_back(linePoints[0]);
int prevCode[2] = {static_cast<int>(linePoints[1].first) - static_cast<int>(linePoints[0].first),
                  static_cast<int>(linePoints[1].second) - static_cast<int>(linePoints[0].second)};
int deltaH, deltaW;
for (unsigned int i = 1; i < point_num; i++)
{
    deltaH = static_cast<int>(linePoints[i+1].first) - static_cast<int>(linePoints[i].first);
    deltaW = static_cast<int>(linePoints[i+1].second) - static_cast<int>(linePoints[i].second);
    if (deltaH != prevCode[0] || deltaW != prevCode[1])
    {
        prevCode[0] = deltaH; prevCode[1] = deltaW;
        newPoints.push_back(linePoints[i]);
    }
}
newPoints.push_back(newPoints[0]);
linePoints.clear();
point_num = newPoints.size() - 1;
// add or sub extral eps so that there is no gap between lines
double Vec[2][2];
double tanTheta, theta, squareSumsqrt;
unsigned int Minus1;
double offset[8][2] = {{0,0.5},{0.5,0.5},{0.5,0},{0.5,-0.5},{0,-0.5},{-0.5,-0.5},{-0.5,0},{-0.5,0.5}};
double newH, newW;
int interval;
for (unsigned int i=0; i<point_num; i++)
{
    Minus1 = (i-1+point_num)%point_num;
    Vec[0][0] = newPoints[Minus1].first - newPoints[i].first;
    Vec[0][1] = newPoints[Minus1].second - newPoints[i].second;
    squareSumsqrt = std::sqrt(Vec[0][0]*Vec[0][0] + Vec[0][1]*Vec[0][1]);
    Vec[0][0] /= squareSumsqrt;
    Vec[0][1] /= squareSumsqrt;
    Vec[1][0] = newPoints[i+1].first - newPoints[i].first;
    Vec[1][1] = newPoints[i+1].second - newPoints[i].second;
    squareSumsqrt = std::sqrt(Vec[1][0]*Vec[1][0] + Vec[1][1]*Vec[1][1]);
    Vec[1][0] /= squareSumsqrt;
    Vec[1][1] /= squareSumsqrt;

    tanTheta = (Vec[0][0]+Vec[1][0]) / (Vec[0][1]+Vec[1][1] + 0.01/img_width);
    theta = std::atan(tanTheta)*180/3.1415926; // (-90-90)
    if (Vec[0][1]+Vec[1][1] + 0.01/img_width < 0) //dx<0 dy>=0
        theta += 180;
    else
        theta += 360; //0~360
    //s=(x1-x3)*(y2-y3)-(x2-x3)*(y1-y3)
    //当s>0时,p1,p2,p3三个点呈逆时针; 当s<0时,p1,p2,p3三个点呈顺时针
    int s = (newPoints[Minus1].second - newPoints[i+1].second)*(newPoints[i].first-newPoints[i+1].first) -
            (newPoints[i].second - newPoints[i+1].second)*(newPoints[Minus1].first-newPoints[i+1].first);
    if (s > 0) // if clockwise, theta += 180
    {
        theta += 180;
        if (theta >= 360)
            theta -= 360;
    }
    if (line == inLine) // inline
    {
        theta += 180;
        if (theta >= 360)
            theta -= 360;
    }
    theta += 22.5;
    if (theta >= 360)
        theta -= 360;
    // assign theta into 8 intervals
    interval = static_cast<int>(theta / 45);
    newH = newPoints[i].first + offset[interval][0];
    newW = newPoints[i].second + offset[interval][1];
    finalPoints.push_back(std::make_pair(newH,newW));
}
finalPoints.push_back(finalPoints[0]);
*/
