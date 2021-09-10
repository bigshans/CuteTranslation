#ifndef XDOTOOL_H
#define XDOTOOL_H

#include <map>
#include "event_monitor.h"
#include "unistd.h"

class Xdotool
{
  public:
    Xdotool();
    ~Xdotool();
    int getMousePosition(int &root_x, int &root_y);
    unsigned long getActiveWindowPID();
    QString getActiveWindowName();
    QString getProcessPathByPID(unsigned long pid);
    EventMonitor eventMonitor;
    std::map<QString, int> getKeyMap();

    int screenWidth;
    int screenHeight;

  private:
    Display *display;
    int screen;
    Window root_window;
    unsigned char *prop;
    void check_status(int status, unsigned long window);
    unsigned char *get_string_property(const char *property_name, Window window);
    unsigned long get_long_property(const char *property_name, Window window);
};

extern Xdotool *xdotool;
#endif // XDOTOOL_H
