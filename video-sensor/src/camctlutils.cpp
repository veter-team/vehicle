/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <stdio.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include "camctlutils.h"

//set ioctl retries to 4 - linux uvc as increased timeout from 1000 to 3000 ms  
#define IOCTL_RETRY 4


Control::Control() 
  : menu(NULL),
    cls(0),
    value(0),
    value64(0L),
    string(NULL) 
{
  memset(&control, 0, sizeof(v4l2_queryctrl));
}


Control::Control(const Control &c)
{
  memcpy(&(this->control), 
         &(c.control), 
         sizeof(v4l2_queryctrl));
  this->cls = (this->control.id & 0xFFFF0000);
  if(c.menu)
    {
      size_t menu_size = (this->control.maximum - this->control.minimum) + 1;
      this->menu = 
        new v4l2_querymenu[menu_size];
      memcpy(this->menu, c.menu, menu_size * sizeof(v4l2_querymenu));
    }
  else
    this->menu = NULL;
  this->cls = c.cls;
  this->value = c.value;
  this->value64 = c.value64;
#ifndef DISABLE_STRING_CONTROLS            
  //allocate a string with max size if needed
  if(c.control.type == V4L2_CTRL_TYPE_STRING)
    {
      this->string = new char[this->control.maximum + 1];
      memcpy(this->string, 
             c.string, 
             this->control.maximum + 1);
    }
  else
#endif
    this->string = NULL;
}


Control::~Control()
{ 
  if(string) delete string;
  if(menu) delete menu;
}


/**
 * ioctl with a number of retries in the case of failure
 * args:
 * fd - device descriptor
 * IOCTL_X - ioctl reference
 * arg - pointer to ioctl data
 * returns - ioctl result
 */
int 
xioctl(int fd, int IOCTL_X, void *arg)
{
  int ret = 0;
  int tries = IOCTL_RETRY;
  do 
    {
      ret = v4l2_ioctl(fd, IOCTL_X, arg);
    } 
  while(ret && tries-- &&
        ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

  if(ret && (tries <= 0)) 
    fprintf(stderr, "ioctl (%i) retried %i times - giving up: %s)\n", 
            IOCTL_X, IOCTL_RETRY, strerror(errno));
        
  return ret;
} 


/**
 * don't use xioctl for control query when using V4L2_CTRL_FLAG_NEXT_CTRL
 */
static int query_ioctl(int hdevice, 
                       int current_ctrl, 
                       v4l2_queryctrl *ctrl)
{
  int ret = 0;
  int tries = 4;
  do 
    {
      if(ret) 
        ctrl->id = current_ctrl | V4L2_CTRL_FLAG_NEXT_CTRL;
      ret = v4l2_ioctl(hdevice, VIDIOC_QUERYCTRL, ctrl);
    } 
  while (ret && tries-- &&
         ((errno == EIO || errno == EPIPE || errno == ETIMEDOUT)));
        
  return(ret);
}


/**
 * List all the device controls with Read/Write permissions.
 * These are the only ones that we can store/restore.
 */
void 
ListControls(int hdevice, ControlList &ctrls)
{
  int ret = 0;
  v4l2_queryctrl queryctrl={0};
  v4l2_querymenu querymenu={0};
  Control control;

  uint32_t currentctrl = 0;
  queryctrl.id = 0 | V4L2_CTRL_FLAG_NEXT_CTRL;
    
  if((ret = query_ioctl(hdevice, currentctrl, &queryctrl)) == 0)
    {
      // The driver supports the V4L2_CTRL_FLAG_NEXT_CTRL flag
      queryctrl.id = 0;
      currentctrl = queryctrl.id;
      queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

      while((ret = query_ioctl(hdevice, currentctrl, &queryctrl)), ret ? errno != EINVAL : 1) 
        {
          v4l2_querymenu *menu = NULL;
            
          // Prevent infinite loop for buggy V4L2_CTRL_FLAG_NEXT_CTRL
          // implementations
          if(ret && queryctrl.id <= currentctrl) 
            {
              printf("buggy V4L2_CTRL_FLAG_NEXT_CTRL flag implementation (workaround enabled)\n");
              currentctrl++;
              goto next_control;
            }
          else if(!ret && queryctrl.id == currentctrl)
            {
              printf("buggy V4L2_CTRL_FLAG_NEXT_CTRL flag implementation (failed enumeration)\n");
              return;
            }
            
          currentctrl = queryctrl.id;
            
          // skip if control failed
          if(ret)
            {
              printf("Control 0x%08x failed to query\n", queryctrl.id);
              goto next_control;
            }
            
          // skip if control is disabled
          if(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            {
              printf("Disabling control 0x%08x\n", queryctrl.id);
              goto next_control;
            }
            
          // check menu items if needed
          if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
              menu = 
                new v4l2_querymenu[(queryctrl.maximum - queryctrl.minimum) + 1];

              int i = 0;
              for (querymenu.index = queryctrl.minimum;
                   querymenu.index <= queryctrl.maximum;
                   querymenu.index++) 
                {
                  querymenu.id = queryctrl.id;
                  if(0 == xioctl(hdevice, VIDIOC_QUERYMENU, &querymenu)) 
                    {
                      memcpy(&(menu[i]), 
                             &querymenu, 
                             sizeof(v4l2_querymenu));
                      i++;
                    }
                  else 
                    {
                      perror("VIDIOC_QUERYMENU");
                      delete menu;
                      menu = NULL;
                      goto next_control;
                    }
                }
            }
          
          // Add the control to the linked list
          memcpy(&(control.control), 
                 &queryctrl, 
                 sizeof(v4l2_queryctrl));
          control.cls = (control.control.id & 0xFFFF0000);
          //add the menu adress (NULL if not a menu)
          control.menu = menu;
#ifndef DISABLE_STRING_CONTROLS            
          //allocate a string with max size if needed
          if(control.control.type == V4L2_CTRL_TYPE_STRING)
            control.string = (char*)calloc(control.control.maximum + 1, 
                                           sizeof(char));
          else
#endif
            control.string = NULL;

          ctrls.push_back(control);
    
        next_control:
          queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
    }
  else
    {
      printf("NEXT_CTRL flag not supported\n");
      int currentctrl;
      for(currentctrl = V4L2_CID_BASE; 
          currentctrl < V4L2_CID_LASTP1; currentctrl++) 
        {
          v4l2_querymenu *menu = NULL;
          queryctrl.id = currentctrl;
          ret = xioctl(hdevice, VIDIOC_QUERYCTRL, &queryctrl);
    
          if(ret || (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) )
            continue;
                
          // check menu items if needed
          if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
              menu = (v4l2_querymenu*)calloc((queryctrl.maximum - queryctrl.minimum) + 1, 
                            sizeof(v4l2_querymenu));
              int i = 0;
              for(querymenu.index = queryctrl.minimum;
                  querymenu.index <= queryctrl.maximum;
                  querymenu.index++) 
                {
                  querymenu.id = queryctrl.id;
                  if(0 == xioctl(hdevice, VIDIOC_QUERYMENU, &querymenu)) 
                    {
                      memcpy(&(menu[i]), 
                             &querymenu, 
                             sizeof(v4l2_querymenu));
                      i++;
                    }
                  else 
                    {
                      perror ("VIDIOC_QUERYMENU");
                      free(menu);
                      menu = NULL;
                      querymenu.index = queryctrl.maximum + 2; //exits loop
                    }
                }
                
              if(querymenu.index > (queryctrl.maximum + 1))
                continue; //query menu failed
            }
            
          // Add the control to the linked list
          memcpy(&(control.control), &queryctrl, sizeof(v4l2_queryctrl));
          control.cls = 0x00980000;
          //add the menu adress (NULL if not a menu)
          control.menu = menu;

          ctrls.push_back(control);
        }
        
      for(queryctrl.id = V4L2_CID_PRIVATE_BASE;;queryctrl.id++) 
        {
          v4l2_querymenu *menu = NULL;
          ret = xioctl(hdevice, VIDIOC_QUERYCTRL, &queryctrl);
          if(ret)
            break;
          else if(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            continue;
                
          //check menu items if needed
          if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
              menu = (v4l2_querymenu*)calloc((queryctrl.maximum - queryctrl.minimum) + 1, 
                            sizeof(v4l2_querymenu));
              int i = 0;
              for(querymenu.index = queryctrl.minimum;
                  querymenu.index <= queryctrl.maximum;
                  querymenu.index++) 
                {
                  querymenu.id = queryctrl.id;
                  if(0 == xioctl(hdevice, VIDIOC_QUERYMENU, &querymenu)) 
                    {
                      memcpy(&(menu[i]), 
                             &querymenu, 
                             sizeof(v4l2_querymenu));
                      i++;
                    }
                  else 
                    {
                      perror ("VIDIOC_QUERYMENU");
                      free(menu);
                      menu = NULL;
                      querymenu.index = queryctrl.maximum + 2; //exits loop
                    }
                }
                
              if(querymenu.index > (queryctrl.maximum + 1))
                continue; //query menu failed
            }
            
          // Add the control to the linked list
          memcpy(&(control.control), &queryctrl, sizeof(v4l2_queryctrl));
          control.cls = 0x00980000;
          //add the menu adress (NULL if not a menu)
          control.menu = menu;

          ctrls.push_back(control);
        }
    }
} 

/**
 * Tries to set the control values passed as string.
 * Expected string format is: id class value, id class value, ... 
 */
void SetControls(int hdevice, 
                 ControlList &ctrl_list, 
                 const std::string &settings)
{
    int ret = 0;
    v4l2_ext_control clist;
    char buff1[1024];
    char buff2[512];
    uint32_t id;
    uint32_t cls;
    
    std::istringstream is1(settings);
    while(!is1.eof())
    {
      is1.getline(buff1, sizeof(buff1), ',');
      std::istringstream is2(buff1);
      is2 >> id >> cls;
      // find control with corresponding id and class
      ControlList::iterator current;
      for(current = ctrl_list.begin(); current != ctrl_list.end(); ++current)
        if(current->control.id == id && current->cls == cls)
          break;

      if(current == ctrl_list.end())
        continue;

      if(current->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        continue;
            
      clist.id = current->control.id;
      switch(current->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS 
        case V4L2_CTRL_TYPE_STRING:
          is2.getline(buff2, sizeof(buff2));
          if(current->string) free(current->string);
          current->string = strdup(buff2);
          current->value = strlen(buff2);
          clist.size = current->value;
          clist.string = current->string;
          break;
#endif
        case V4L2_CTRL_TYPE_INTEGER64:
          is2 >> current->value64;
          clist.value64 = current->value64;
          break;

        default:
          is2 >> current->value;
          clist.value = current->value;
          break;
        }
        
      v4l2_ext_controls ctrls = {0};
      ctrls.ctrl_class = current->cls;
      ctrls.count = 1;
      ctrls.controls = &clist;
      ret = xioctl(hdevice, VIDIOC_S_EXT_CTRLS, &ctrls);
      if(ret)
        {
          printf("VIDIOC_S_EXT_CTRLS for multiple controls failed (error %i)\n", ret);
          v4l2_control ctrl;
          //set the controls one by one
          if(current->cls == V4L2_CTRL_CLASS_USER)
            {
              printf("   using VIDIOC_S_CTRL for user class controls\n");
              ctrl.id = clist.id;
              ctrl.value = clist.value;
              ret = xioctl(hdevice, VIDIOC_S_CTRL, &ctrl);
              if(ret)
                {
                  printf("control(0x%08x) \"%s\" failed to set (error %i)\n",
                         clist.id, current->control.name, ret);
                }
            }
          else
            {
              printf("   using VIDIOC_S_EXT_CTRLS on single controls for class: 0x%08x\n", 
                     current->cls);
              ctrls.count = 1;
              ctrls.controls = &clist;
              ret = xioctl(hdevice, VIDIOC_S_EXT_CTRLS, &ctrls);
              if(ret)
                {
                  printf("control(0x%08x) \"%s\" failed to set (error %i)\n",
                         clist.id, current->control.name, ret);
                }
            }
        }
    }
}


int 
OpenVideoDevice(const char *devfile)
{
  int hdevice = v4l2_open(devfile, O_RDWR | O_NONBLOCK, 0);
  return hdevice;
}


void
CloseVideoDevice(int hdevice)
{
  v4l2_close(hdevice);
}
