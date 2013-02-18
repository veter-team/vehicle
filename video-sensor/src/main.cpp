/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <stdexcept>
#include "IceSvc.h"

using namespace std;


int
main(int argc, char* argv[])
{
  int res = 0;
  try
    {
      IceSvc svc;
      res = svc.main(argc, argv);
    }
  catch (exception& e)
    {
      cerr << "Exception caught: " << e.what() << endl;
      res = -1;
    }

  return res;
}
