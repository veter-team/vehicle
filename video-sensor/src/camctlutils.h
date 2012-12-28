#ifndef __CAMCTLUTILS_H
#define __CAMCTLUTILS_H

#include <inttypes.h> 
#include <vector>
#include <string>
#include <linux/videodev2.h>


class Control
{
 public:
  Control();
  Control(const Control &c);
  ~Control();

 public:
  v4l2_queryctrl control;
  v4l2_querymenu *menu;
  uint32_t cls;
  int32_t value; //also used for string max size
  int64_t value64;
  char *string;
};

typedef std::vector<Control> ControlList;

int OpenVideoDevice(const char *devfile);
void CloseVideoDevice(int hdevice);
void ListControls(int hdevice, ControlList &ctrls);
void SetControls(int hdevice, 
                 ControlList &ctrl_list, 
                 const std::string &settings);

#endif // __CAMCTLUTILS_H
