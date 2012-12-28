#ifndef __ICESVC_H
#define __ICESVC_H

#include <Ice/Service.h>
#include <IceUtil/Mutex.h>
#include "UpdaterThread.h"
#include "SensorGroupI.h"
#include "SensorAdminI.h"

using namespace std;

typedef SensorGroupI<SonarUpdaterThread> SonarGroup;
typedef IceUtil::Handle<SonarGroup> SonarGroupPtr;
typedef SensorAdminI<SonarGroupPtr> SonarAdmin;
typedef IceUtil::Handle<SonarAdmin> SonarAdminPtr;

typedef SensorGroupI<CompassUpdaterThread> CompassGroup;
typedef IceUtil::Handle<CompassGroup> CompassGroupPtr;
typedef SensorAdminI<CompassGroupPtr> CompassAdmin;
typedef IceUtil::Handle<CompassAdmin> CompassAdminPtr;


class IceSvc : public Ice::Service
{
 public:
  IceSvc();

 public:
  virtual bool start(int argc, char *argv[], int &status);
  virtual bool stop();

 private:
  IceUtil::Mutex bus_mutex;
  SonarAdminPtr sadm;
  SonarGroupPtr sonar_group; 
  CompassAdminPtr cadm;
  CompassGroupPtr compass_group; 
};


#endif // __ICESVC_H
