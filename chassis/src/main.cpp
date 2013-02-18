/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <stdexcept>
#include "IceSvc.h"
#include "TracksController.h"
#include "interface.h"

using namespace std;


int
main(int argc, char* argv[])
{
  int res = 0;

  try
    {
      TracksController tracks_ctl(DEVICE_NAME);
      IceSvc svc(&tracks_ctl);

      res = svc.main(argc, argv);
    }
  catch (exception& e)
    {
      cerr << "Exception caught: " << e.what() << endl;
      res = -2;
    }

  return res;
}
