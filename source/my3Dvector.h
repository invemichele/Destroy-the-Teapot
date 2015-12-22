// File: my3Dvector.h

#ifndef MY3DVECTOR_H
#define MY3DVECTOR_H

//////////////////////////////////////////////////////////////////////////////////
// data type that implements the basic characteristics of a physic vector in 3D //
//////////////////////////////////////////////////////////////////////////////////

#include <cmath> //sqrt(), acos(), atan2(), cos(), sin()

struct my3Dvector 
{
  double x, y, z;
  my3Dvector (double first, double second, double third) {x=first; y=second; z=third;};
  my3Dvector () {x=0; y=0; z=0;};
  
  double norm() const;
  double elevation() const; //[M_PI/2, -M_PI/2]. use 180/M_PI for conversion.
  double azimuth() const; //[-M_PI, M_PI]
  
  void set_spherical(double, double, double); //norm, elevation, azimuth (in radians!)
  void set_norm(double);
  void set_elevation(double);
  void set_azimuth(double);
  void set(double first, double second, double third) {x=first; y=second; z=third;};
  
  void rotate(double, my3Dvector); //radians!
  void rotate(double, double, double, double);
  
  my3Dvector operator= (double); //a little inappropriate, but useful
  my3Dvector operator= (const my3Dvector&);
  my3Dvector operator+ (const my3Dvector&) const;
  my3Dvector operator- (const my3Dvector&) const;
  my3Dvector operator+= (const my3Dvector&);
  my3Dvector operator-= (const my3Dvector&);
  double operator* (const my3Dvector&) const; //scalar product
  my3Dvector operator^ (const my3Dvector&) const; //cross product. Caution, always use brackets for priority
  my3Dvector operator* (const double) const;
  my3Dvector operator/ (const double) const;
};

//some non-member functions needed
my3Dvector operator* (const double scalar, const my3Dvector& vect);

std::ostream& operator<<( std::ostream& os, const my3Dvector& vect );
std::istream& operator>>( std::istream& is, my3Dvector& vect );

#endif

