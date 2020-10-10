#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>
#include <utility>
#include <iomanip>
#include "wtypes.h"


/* Globals */
int ScreenX = 0;
int ScreenY = 0;
BYTE* ScreenData = 0;
static bool th_finished=0;

void ScreenCap()              // capture screen
{
    HDC hScreen = GetDC(NULL);
    ScreenX = GetDeviceCaps(hScreen, HORZRES);
    ScreenY = GetDeviceCaps(hScreen, VERTRES);

    HDC hdcMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, ScreenX, ScreenY, hScreen, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOld);

    BITMAPINFOHEADER bmi = {0};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = ScreenX;
    bmi.biHeight = -ScreenY;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

    if(ScreenData)
        free(ScreenData);
    ScreenData = (BYTE*)malloc(4 * ScreenX * ScreenY);

    GetDIBits(hdcMem, hBitmap, 0, ScreenY, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    ReleaseDC(GetDesktopWindow(),hScreen);
    DeleteDC(hdcMem);
    DeleteObject(hBitmap);
}                                  //end of capture screen

inline int PosB(int x, int y)
{
    return ScreenData[4*((y*ScreenX)+x)];
}    //get blue

inline int PosG(int x, int y)
{
    return ScreenData[4*((y*ScreenX)+x)+1];
}    //get green

inline int PosR(int x, int y)
{
    return ScreenData[4*((y*ScreenX)+x)+2];
}    //get red

bool ButtonPress(int Key)   //check for right button press
{
    bool button_pressed = false;

    while(GetAsyncKeyState(Key))
        button_pressed = true;

    return button_pressed;
}

namespace patch   //to_string_patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class statuses{    // statuses

public:
    std::vector < std::vector <int> > mtrx;
    std::vector < std::string > output;
    int len=0;

    statuses(){}
    statuses(int a)
        {
        len=a;
        }

    void getall();
    bool check(int R, int G, int B);
    void addrgb(int R, int G, int B);
    void addstatuses(bool choice);

};             // end of statuses

void statuses::getall()      // get other rgb
{
    int num,r,g,b;
    std::string line;
    std::cout <<"Number of known RGB statuses: ";
    std::cin >> num;

    for(int i=0;i<num;i++)
    {
        std::cout <<"Status " << i+1 << " rgb (3 int): ";
        std::cin >> r >> g >> b;
        std::vector<int> rgb;
        rgb.push_back(r);
        rgb.push_back(g);
        rgb.push_back(b);
        this->mtrx.push_back(rgb);
        std::cout << "Enter status line: ";
        std::cin >> line;
        this->output.push_back(line);
        this->len++;

    }
    std::cout<< std::endl;
return;
}                                          // end of get other rgb

bool statuses::check(int R, int G, int B)    //check if the status is declared in statuses
{
    int i;

    for(i=0;i<this->len;i++)
    {
    if(this->mtrx[i][0]==R && this->mtrx[i][1]==G && this->mtrx[i][2]==B)
        return true;
    }
return false;
}

void statuses::addrgb(int R, int G, int B)          //adds a status
{
    std::vector<int> new_rgb;
    new_rgb.push_back(R);
    new_rgb.push_back(G);
    new_rgb.push_back(B);
this->mtrx.push_back(new_rgb);
this->len+=1;
}

void statuses::addstatuses(bool choice)
{
    if(choice==0)
        {
        for (int i=0;i<this->len;i++)
            {
            this->output.push_back(patch::to_string((i*(-1))-1));
            }
         return;
        }
    else
        {
        for (int i=0;i<this->len;i++)
            {
            std::string line;
            std::cout << "Enter status (" << this->mtrx[i][0] << "," << this->mtrx[i][1] << " " << this->mtrx[i][2] << ") line: ";
            std::cin >> line;
            this->output.push_back(line);
            }
        return;
        }
}

void mainth()                               //thread that ends the main loop
{
    while (true)
    {
    if (ButtonPress(VK_SHIFT))
        break;
    }
    th_finished=!th_finished;
}

std::pair<int,int> getmainpos (int horizontal, int vertical)              //get the desired position
{
    char answer;
    std::cout<< std::endl;
    std::cout<< "Click on desired position: 1" << std::endl
               << "Enter desired pixel location: 0" << std::endl
                << "Your answer: ";
                std::cin>> answer;
    while(answer!='0' && answer != '1')
        {
        std::cout<< "Error: Answer has to be either 0 or 1. Enter again: ";
        std::cin >> answer;
        }
        std::cout<< std::endl;

    std::pair<int,int> to_push;
    int x,y;
    if (answer=='0')
        {
        std::cout<< std::endl << "Enter desired x,y: { (0,0) = top-left } ";
        std::cin>> x >> y;
        while(x<0 || x>horizontal || y<0 || y>vertical)
            {
            std::cout<< std::endl << "Error - expected input: "<< "0 < x < " << horizontal << ", 0 < y < " << vertical << std::endl
                            << "Enter desired x,y: { (0,0) = top-left } ";
            std::cin>> x >> y;
            }
        }
    else
    {
        std::cout<< "Put cursor on desired location and CLICK or press SPACE"<<std::endl;
        while (true)
            {
            if (ButtonPress(VK_SPACE)|| ButtonPress(VK_LBUTTON))
                {
                POINT p;
                GetCursorPos(&p);
                x=p.x;
                y=p.y;
                std::cout<< "Position selected: "<< x << ", " << y << std::endl;
                break;
                }
            }
    }
    std::cout<<std::endl;
    to_push= std::make_pair(x,y);

return to_push;
}

bool manual_status_add(statuses unrecognized)
{
    char desired;
    std::cout<< std::endl;
    std::cout<< "Number of unrecognized statuses: " << unrecognized.len << std::endl;
    if(unrecognized.len==0) return false;
    std::cout<< "To add the default number as status (-1,-2,...) - enter 0" << std::endl
                    << "To add statuses manually - enter 1" << std::endl
                    << "Your input: ";
    std::cin >> desired;
    while(desired!= '0' && desired!='1')
        {
        std::cout<< "Error: input is element of {0,1}. Enter again: ";
        std::cin>> desired;
        }
return desired-'0';
}

void giveintro()   //program info
{
    std::cout<< "Captures the screen every n seconds. Converts to a Bitmap." << std::endl
                << "Reads the pixel at desired location." << std::endl
                << "Outputs a status to to_save_1.txt every n seconds." << std::endl
                << "Cleans it up and outputs to to_save_2.txt file." << std::endl
                << "Main loop of the program is stopped by pressing SHIFT." << std::endl
                << "Note: status line is type string." << std::endl << std::endl;
    return;
}

void GetDesktopResolution(int& horizontal, int& vertical)          // Get the horizontal and vertical screen sizes in pixel
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

int to_log()   //ask to log the output in terminal
{
    char put;
    std::cout<< "Log output (Y=1,N=0): ";
    std::cin>> put;
    while(put!='0' && put!='1')
    {
        std::cout<< "Error, possible input is 0/1. Enter again: ";
        std::cin>> put;
    }
    std::cout<<std::endl;

 return put-'0';
}

double get_wait()           //get wait time between screencaps
{
    double secs;
  std::cout<<"Enter the number of seconds between screenshots: ";
  std::cin>> secs;
  while(secs<=0)
  {
      std::cout<<"Error: number of seconds must be positive. Enter again: ";
      std::cin>> secs;
  }
std::cout<< std::endl;

return secs;
}

std::string find_status(statuses allst,statuses unrecognized,int R, int G, int B)   //finds the correct RGB in statuses
{
    for (int i=0;i<allst.len;i++)
        {
        if (allst.mtrx[i][0]==R && allst.mtrx[i][0]==G && allst.mtrx[i][2]==B)
            return allst.output[i];
        }
    for (int i=0;i<unrecognized.len;i++)
        {
        if (unrecognized.mtrx[i][0]==R && unrecognized.mtrx[i][1]==G && unrecognized.mtrx[i][2]==B)
            return unrecognized.output[i];
        }
    return "0";
}


int main (){      // main

giveintro();


   int horizontal = 0;
   int vertical = 0;
   GetDesktopResolution(horizontal, vertical);

std::pair<int,int> main_pos=getmainpos(horizontal,vertical);

statuses allst,unrecognized;
allst.getall();

int log_output=to_log();
double time_to_wait=get_wait();


std::ofstream ofile;
ofile.open("to_save_1.txt", std::ofstream::out | std::ofstream::trunc);





auto start = std::chrono::system_clock::now();
std::time_t start_time = std::chrono::system_clock::to_time_t(start);  // get start time

std::thread maketh(mainth);


while(!th_finished)
{
    ScreenCap();

    int R,G,B;
    R=PosR(main_pos.first, main_pos.second);
    G=PosG(main_pos.first, main_pos.second);
    B=PosB(main_pos.first, main_pos.second);

    auto time_ = std::chrono::system_clock::now();
    std::time_t time_is = std::chrono::system_clock::to_time_t(time_);  // get current time


    if(log_output==1)
        {
        std::cout << "Bitmap: r: " << std::setw (3) << std::right << R << " g: "
                                                << std::setw (3) << std::right << G << " b: "
                                                << std::setw (3) << std::right << B << "  "
                                                << std::ctime(&time_is);
        }

    bool check1=allst.check(R,G,B);
    if(check1==false)
        {
        bool check2=unrecognized.check(R,G,B);
        if(check2==false)
            {
            unrecognized.addrgb(R,G,B);
            }
        }

    ofile << R << " " << G << " " << B << " " << std::ctime(&time_is);

    Sleep(time_to_wait*1000);
}
maketh.join();
ofile.close();

auto end = std::chrono::system_clock::now();      // get end time
std::chrono::duration<double> elapsed_seconds = end-start;
std::time_t end_time = std::chrono::system_clock::to_time_t(end);

bool desired=manual_status_add(unrecognized);
unrecognized.addstatuses(desired);

std::ifstream infile;

infile.open("to_save_1.txt", std::ios::in);
ofile.open("to_save_2.txt", std::ofstream::out | std::ofstream::trunc);


std::string st_line_1;
int is_set;

while (std::getline(infile, st_line_1))
{
    std::istringstream iss(st_line_1);
    int line_1_R, line_1_G, line_1_B, line_1_num, line_1_year, line_1_hour, line_1_min, line_1_sec;
    std::string line_1_day,line_1_month;

    int start_num, start_hour, start_min, start_year, start_sec, end_num, end_hour, end_min, end_year, end_sec;
    std::string start_day, start_month, end_day, end_month;

    is_set=0;

    //reading first line
    char t1,t2;
    if (!(iss >> line_1_R >> line_1_G >> line_1_B >> line_1_day >> line_1_month >> line_1_num >> line_1_hour
                >> t1 >> line_1_min >> t2 >> line_1_sec >> line_1_year))
    {
        std::cout<< "Error";
        break;
    }  // error


    std::string st_line_2;
    while (std::getline(infile, st_line_2))
        {
            std::istringstream iss(st_line_2);
            int line_2_R, line_2_G, line_2_B, line_2_num, line_2_year, line_2_hour, line_2_min, line_2_sec;
            std::string line_2_day,line_2_month;

             //reading second line
             char tt1,tt2;
             if (!(iss >> line_2_R >> line_2_G >> line_2_B >> line_2_day >> line_2_month >> line_2_num >> line_2_hour
                        >> tt1 >> line_2_min >> tt2 >> line_2_sec >> line_2_year))
                {
                    std::cout<< "Error";
                    break;
                }  // error

        if (line_1_R==line_2_R && line_1_G==line_2_G && line_1_B==line_2_B)
            {
            start_day=line_1_day;
            start_month=line_1_month;
            start_num=line_1_num;
            start_hour=line_1_hour;
            start_min=line_1_min;
            start_year=line_1_year;
            start_sec=line_1_sec;
            end_day=line_2_day;
            end_month=line_2_month;
            end_num=line_2_num;
            end_hour=line_2_hour;
            end_min=line_2_min;
            end_year=line_2_year;
            end_sec=line_2_sec;
            is_set=1;
            }
        else
            {
            std::string s_t;
            s_t=find_status(allst,unrecognized,line_1_R,line_1_G,line_1_B);
            if (is_set==0)
                {
                ofile << line_1_day << " " << line_1_month << " " << line_1_num << " "
                        << line_1_year << " ... " << line_1_hour << ":" << line_1_min << ":"
                        << line_1_sec << " ... " << "Status: " << s_t << std::endl;
                }
            else if (start_day==end_day && start_month==end_month && start_year==end_year && start_num==end_num)
                {
                ofile << start_day << " " << start_month << " " << start_num << " "
                        << start_year << " ... " << start_hour << ":" << start_min << ":"
                        << start_sec << " - " << end_hour << ":" << end_min << ":" << end_sec
                        << " ... " << "Status: " << s_t << std::endl;
                }
            else
                {
                ofile << start_day << " " << start_month << " " << start_num << " "
                        << start_year << " ... " <<  start_hour << ":" << start_min << ":"
                        << start_sec << " - " << end_day << " " << end_month << " " << end_num
                        << " " << end_year << " ... " << end_hour << ":" << end_min << ":"
                        << end_sec << " ... " << "Status: " << s_t << std::endl;
                }
                is_set=0;

            line_1_day=line_2_day;
            line_1_month=line_2_month;
            line_1_num=line_2_num;
            line_1_hour=line_2_hour;
            line_1_min=line_2_min;
            line_1_year=line_2_year;
            line_1_sec=line_2_sec;
            line_1_R=line_2_R;
            line_1_G=line_2_G;
            line_1_B=line_2_B;

            }

        }

        std::string s_t;
        s_t=find_status(allst,unrecognized,line_1_R,line_1_G,line_1_B);
        if (is_set==0)
            {
            ofile << line_1_day << " " << line_1_month << " " << line_1_num << " "
                    << line_1_year << " ... " << line_1_hour << ":" << line_1_min << ":"
                    << line_1_sec << " ... " << "Status: " << s_t << std::endl;
            }
        else if (start_day==end_day && start_month==end_month && start_year==end_year && start_num==end_num)
            {
            ofile << start_day << " " << start_month << " " << start_num << " "
                    << start_year << " ... " << start_hour << ":" << start_min << ":"
                    << start_sec << " - " << end_hour << ":" << end_min << ":" << end_sec
                    << " ... " << "Status: " << s_t << std::endl;
            }
        else
            {
            ofile << start_day << " " << start_month << " " << start_num << " "
                    << start_year << " ... " <<  start_hour << ":" << start_min << ":"
                    << start_sec << " - " << end_day << " " << end_month << " " << end_num
                    << " " << end_year << " ... " << end_hour << ":" << end_min << ":"
                    << end_sec << " ... " << "Status: " << s_t << std::endl;
            }
        is_set=0;
}


    std::cout << std::endl << "Starting time: " << std::ctime(&start_time);     // print time + elapsed
    std::cout << "Ending time:   " << std::ctime(&end_time)
                    << "Elapsed time:  " << elapsed_seconds.count() << "s\n";


    return 0;
}                    // end of main

