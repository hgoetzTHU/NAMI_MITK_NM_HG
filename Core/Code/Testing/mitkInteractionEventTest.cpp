/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkMousePressEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkVtkPropRenderer.h"
#include "mitkInteractionEventConst.h"
#include "mitkTestingMacros.h"

int mitkInteractionEventTest(int /*argc*/, char* /*argv*/[])
{
  /*
   * Create different Events, fill them with data.
   * And check if isEqual method is implemented properly.
   */


  MITK_TEST_BEGIN("InteractionEvent")

  mitk::VtkPropRenderer::Pointer renderer = NULL;

  mitk::EButtonStates buttonStates =  mitk::BS_LeftButton | mitk::BS_RightButton;
  mitk::EButtonStates eventButton = mitk::BS_LeftButton;
  mitk::EButtonStates modifiers = mitk::BS_ControlButton | mitk::BS_AltButton;

  mitk::Point2D point;
  point[0] = 17;
  point[1] = 170;

  // MousePress Events
  mitk::MousePressEvent::Pointer me1 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  mitk::MousePressEvent::Pointer me2 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MousePressEvent::Pointer me3 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  modifiers = mitk::BS_ControlButton;
  mitk::MousePressEvent::Pointer me4 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);


  MITK_TEST_CONDITION_REQUIRED(
      me1->isEqual(me2.GetPointer()) &&
      me1->isEqual(me3.GetPointer()) &&
      (me2->isEqual(me3.GetPointer())) &&
      !(me3->isEqual(me4.GetPointer()))
      , "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MousePressEvent");

  // MouseReleaseEvents
  mitk::MouseReleaseEvent::Pointer mr1 = mitk::MouseReleaseEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  mitk::MouseReleaseEvent::Pointer mr2 = mitk::MouseReleaseEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MouseReleaseEvent::Pointer mr3 = mitk::MouseReleaseEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  eventButton = mitk::BS_RightButton;
  mitk::MouseReleaseEvent::Pointer mr4 = mitk::MouseReleaseEvent::New(renderer,point, buttonStates, modifiers, eventButton);


  MITK_TEST_CONDITION_REQUIRED(
      mr1->isEqual(mr2.GetPointer()) &&
      mr1->isEqual(mr3.GetPointer()) &&
      (mr2->isEqual(mr3.GetPointer())) &&
      !(mr3->isEqual(mr4.GetPointer()))
      , "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseReleaseEvent");


  // MouseMoveEvents
  mitk::MouseMoveEvent::Pointer mm1 = mitk::MouseMoveEvent::New(renderer,point, buttonStates, modifiers);
    point[0] = 178;
  point[1] = 170;
  mitk::MouseMoveEvent::Pointer mm3 = mitk::MouseMoveEvent::New(renderer,point, buttonStates, modifiers);
  modifiers = mitk::BS_LeftButton;
  mitk::MouseMoveEvent::Pointer mm4 = mitk::MouseMoveEvent::New(renderer,point, buttonStates, modifiers);



  MITK_TEST_CONDITION_REQUIRED(
      mm1->isEqual(mm3.GetPointer()) &&
      !(mm3->isEqual(mm4.GetPointer()))
      , "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseMoveEvent");


  // always end with this!
  MITK_TEST_END()

}
