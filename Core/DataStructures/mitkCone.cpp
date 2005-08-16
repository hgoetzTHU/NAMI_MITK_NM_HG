/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkCone.h"
#include "vtkLinearTransform.h"
#include "mitkVector.h"
#include "vtkConeSource.h"

mitk::Cone::Cone()
{ // Set up Cone Surface. Radius 1.0, height 2.0, , centered around the origin
  GetGeometry()->Initialize();
  vtkConeSource* Cone = vtkConeSource::New();
  Cone->SetRadius(1.0);
  Cone->SetHeight(2.0);
  Cone->SetDirection(0.0, -1.0, 0.0);
  Cone->SetCenter(0.0, 0.0, 0.0);
  Cone->SetResolution(20);
  
  Cone->CappingOn();
  SetVtkPolyData(Cone->GetOutput());
  Cone->Delete(); 
}

mitk::Cone::~Cone()
{
}

bool mitk::Cone::IsInside(const Point3D& worldPoint) const
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;
  GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);  // transform world point to norm-cone coordinate system

  p[1] += 1;  // translate point, so that it fits to the formula below, which describes a cone that has its cone vertex at the origin                                            
  return (sqrt(p[0] * p[0] + p[2] * p[2]) <= p[1] * 0.5) && (p[1] <= 2);  // formula to calculate if a given point is inside a cone that has its cone vertex at the origin, is aligned on the second axis, has a radius of one an a height of two
}

mitk::ScalarType mitk::Cone::GetVolume()
{
  return   GetGeometry()->GetXAxis().GetNorm() * GetGeometry()->GetZAxis().GetNorm() 
         * 2 * GetGeometry()->GetZAxis().GetNorm() * vnl_math::pi / 3.0;
}
