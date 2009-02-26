#include "geometrycontrol.h"


GeometryControl::GeometryControl(){  
  geometry = QSize(2,1);
}

QSize GeometryControl::getGeometry(){
  return geometry;
}
