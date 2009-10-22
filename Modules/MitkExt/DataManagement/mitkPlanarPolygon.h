/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_POLYGON_H_
#define _MITK_PLANAR_POLYGON_H_

#include "mitkPlanarFigure.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class MITKEXT_CORE_EXPORT PlanarPolygon : public PlanarFigure
{
public:
  mitkClassMacro( PlanarPolygon, PlanarFigure );

  itkNewMacro( Self );


  virtual bool IsClosed() const { return true; };

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();


  /** \brief Polygon has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 2;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 1000;
  }

  
protected:
  PlanarPolygon();
  virtual ~PlanarPolygon();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POLYGON_H_
