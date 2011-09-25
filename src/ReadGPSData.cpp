/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ReadGPSData.h"
#include <sys/types.h>
#include <sys/select.h>
#include <math.h>
#include <errno.h>
#include <gps.h>

/* pseudo-signals indicating reason for termination */
#define CGPS_QUIT       0       /* voluntary yterminastion */
#define GPS_GONE        -1      /* GPS device went away */
#define GPS_ERROR       -2      /* low-level failure in GPS read */

struct fixsource_t 
/* describe a data source */
{
    const char *spec;         /* pointer to actual storage */
    const char *server;
    const char *port;
    /*@null@*/const char *device;
};

/* standard parsing of a GPS data source spec */
void 
gpsd_source_spec(const char *arg, struct fixsource_t *source)
{
    source->server = "localhost";
    source->port = DEFAULT_GPSD_PORT;
    source->device = NULL;

    if (arg != NULL) {
	char *colon1, *rbrk;
        const char *skipto;
	source->spec = strdup(arg);
	assert(source->spec != NULL);

	skipto = source->spec;
	if (*skipto == '[' && (rbrk = strchr(skipto, ']')) != NULL) {
	    skipto = rbrk;
	}
	colon1 = strchr(skipto, ':');

	if (colon1 != NULL) {
	    char *colon2;
	    *colon1 = '\0';
	    if (colon1 != source->spec) {
		source->server = source->spec;
	    }
	    source->port = colon1 + 1;
	    colon2 = strchr(source->port, ':');
	    if (colon2 != NULL) {
		*colon2 = '\0';
		source->device = colon2 + 1;
	    }
	} else if (strchr(source->spec, '/') != NULL) {
	    source->device = source->spec;
	} else {
	    source->server = source->spec;
	}
    }

    if (*source->server == '[') {
	char *rbrk = strchr(source->server, ']');
	++source->server;
	if (rbrk != NULL)
	    *rbrk = '\0';
    }
}


static struct gps_data_t gpsdata;


void 
InitGPSSensor()
{
  struct fixsource_t source;

  /* Grok the server, port, and device. */
  gpsd_source_spec(NULL, &source);

  /* Open the stream to gpsd. */
#if GPSD_API_MAJOR_VERSION > 4
  if(gps_open(source.server, source.port, &gpsdata) != 0)
#else
  if(gps_open_r(source.server, source.port, &gpsdata) != 0)
#endif
    {
      fprintf(stderr,
              "cgps: no gpsd running or network error: %d, %s\n",
              errno, gps_errstr(errno));
      return;
    }

  gps_stream(&gpsdata, WATCH_ENABLE, NULL);
}


void 
ReadGPSData(vehicle::SensorData &data)
{
  fd_set rfds;
  int res;
  struct timeval timeout;

  /* watch to see when it has input */
  FD_ZERO(&rfds);
  FD_SET(gpsdata.gps_fd, &rfds);

  /* wait up to five seconds. */
  timeout.tv_sec = 0;
  timeout.tv_usec = 100 * 1000; // 100 milliseconds

  /* check if we have new information */
  res = select(gpsdata.gps_fd + 1, &rfds, NULL, NULL, &timeout);

  if(res == -1)
    {
      fprintf(stderr, "cgps: socket error 3\n");
      return;
    } else if(res)
    {
      errno = 0;
      if(gps_read(&gpsdata) == -1)
        {
          fprintf(stderr, "cgps: socket error 4\n");
          /* We're done talking to gpsd. */
          gps_close(&gpsdata);
          return;
        }
      else
        {
          /* Fill in the latitude. */
          if(gpsdata.fix.mode >= MODE_2D && isnan(gpsdata.fix.latitude) == 0)
            data.floatdata.push_back(gpsdata.fix.latitude);

          /* Fill in the longitude. */
          if(gpsdata.fix.mode >= MODE_2D && isnan(gpsdata.fix.longitude) == 0)
            data.floatdata.push_back(gpsdata.fix.longitude);

          /* Fill in the altitude. */
          if(gpsdata.fix.mode == MODE_3D && isnan(gpsdata.fix.altitude) == 0)
            data.floatdata.push_back(gpsdata.fix.altitude);
        }
    }
/*
  for(comtypes::FloatSeq::const_iterator i = data.floatdata.begin();
      i != data.floatdata.end(); ++i)
    printf("%f\t", *i);
  if(!data.floatdata.empty())
    printf("\n");
*/
  //data.floatdata.push_back(47.9975f);
  //data.floatdata.push_back(11.34164f);
  //data.floatdata.push_back(0.0f);
}

