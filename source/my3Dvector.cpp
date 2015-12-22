// File: my3Dvector.cpp

#include <iostream>
#include "my3Dvector.h"

double my3Dvector::norm() const
{  return sqrt(x*x+y*y+z*z);  }

double my3Dvector::elevation() const
{ 
  if (norm()==0)
    return 0; 
  return (M_PI/2.-acos(z/norm()));  
}

double my3Dvector::azimuth() const //CAUTION: better use rotation to set the angle continuously
{
  if (x==0 && y==0)
    return 0;
  return atan2(y,x);
}

void my3Dvector::set_spherical(double norm, double elevation, double azimuth)
{
  x=norm*sin(M_PI/2.-elevation)*cos(azimuth);
  y=norm*sin(M_PI/2.-elevation)*sin(azimuth);
  z=norm*cos(M_PI/2.-elevation);
}

void my3Dvector::set_norm(double new_norm)
{  return set_spherical(new_norm, elevation(), azimuth());  }

void my3Dvector::set_elevation(double new_elevation)
{  return set_spherical(norm(), new_elevation, azimuth());  }

void my3Dvector::set_azimuth(double new_azimuth)
{  return set_spherical(norm(), elevation(), new_azimuth);  }

void my3Dvector::rotate(double angle, double vec_x, double vec_y, double vec_z)
{
  my3Dvector vector(vec_x, vec_y, vec_z);
  rotate(angle, vector);
}

void my3Dvector::rotate(double angle, my3Dvector vector) //Rodrigues' rotation formula
{
  vector=vector/vector.norm();
  (*this)=(*this)*cos(angle)+(vector^(*this))*sin(angle)+vector*(vector*(*this))*(1-cos(angle));
}

my3Dvector my3Dvector::operator= (double value)
{ 
  x=value;
  y=value;
  z=value;
  return *this;
}

my3Dvector my3Dvector::operator= (const my3Dvector& same)
{
  x=same.x;
  y=same.y;
  z=same.z;
  return *this;
}

my3Dvector my3Dvector::operator+ (const my3Dvector& addendum) const
{  return my3Dvector(x+addendum.x, y+addendum.y, z+addendum.z);  }

my3Dvector my3Dvector::operator- (const my3Dvector& subtrahendum) const
{  return my3Dvector(x-subtrahendum.x, y-subtrahendum.y, z-subtrahendum.z);  }

my3Dvector my3Dvector::operator+= (const my3Dvector& addendum)
{
  *this = *this + addendum;
  return *this;
}

my3Dvector my3Dvector::operator-= (const my3Dvector& subtrahendum)
{
  *this = *this - subtrahendum;
  return *this;
}

double my3Dvector::operator* (const my3Dvector& factor) const
{  return (x*factor.x)+(y*factor.y)+(z*factor.z);  }

my3Dvector my3Dvector::operator^ (const my3Dvector& other) const
{  return my3Dvector(y*other.z-z*other.y, z*other.x-x*other.z, x*other.y-y*other.x);  }

my3Dvector my3Dvector::operator* (const double scalar) const
{  return my3Dvector (x*scalar, y*scalar, z*scalar);  }

my3Dvector my3Dvector::operator/ (const double scalar) const
{  return my3Dvector (x/scalar, y/scalar, z/scalar);  }

//non-member functions
my3Dvector operator* (const double scalar, const my3Dvector& vect)
{  return my3Dvector(scalar*vect.x, scalar*vect.y, scalar*vect.z);  }

std::ostream& operator<<( std::ostream& os, const my3Dvector& vect ) 
{
  os<<vect.x<<"\t"<<vect.y<<"\t"<<vect.z;
  return os;
}

std::istream& operator>>( std::istream& is, my3Dvector& vect ) 
{
  is>>vect.x>>vect.y>>vect.z;
  return is;
}
