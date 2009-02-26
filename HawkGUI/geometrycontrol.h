#ifndef _GEOMETRY_CONTROL_H_
#define _GEOMETRY_CONTROL_H_ 1

#include <QSize>


class GeometryControl
{
 public:
  GeometryControl();
  QSize getGeometry();
 private:
  QSize geometry;
};

#endif
