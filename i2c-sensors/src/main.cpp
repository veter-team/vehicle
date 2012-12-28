#include <stdexcept>
#include "IceSvc.h"
#include "from-u-boot/i2c_interface.h"
#include "from-u-boot/enable_i2c_clocks.h"

using namespace std;


int
main(int argc, char* argv[])
{
  int res = enable_i2c_clocls();
  if(res)
    {
      cerr << "Error enabling I2C clocks: %i" << res << endl;
      return res;
    }
  i2c_init(100000, 1);
  cout << "I2C bus initialized\n";

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
  i2c_uninit();

  return res;
}
