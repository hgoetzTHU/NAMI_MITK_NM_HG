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

#ifndef MITKINTERACTIONPOSITIONEVENT_H_
#define MITKINTERACTIONPOSITIONEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include <MitkExports.h>
#include <string>

namespace mitk
{

  class MITK_CORE_EXPORT InteractionPositionEvent : public InteractionEvent {

  public:
    const Point2D* GetMousePosition();
    const Point3D* GetWorldPosition();
    EButtonStates GetModifiers();
    EButtonStates GetButtonStates();

  protected:
    InteractionPositionEvent(BaseRenderer*, Point2D, EButtonStates, EButtonStates, std::string);
    virtual bool isEqual(InteractionEvent::Pointer) = 0;
    virtual ~InteractionPositionEvent();

  private:
    Point2D m_MousePosition;
    Point3D m_WorldPosition;
    EButtonStates m_ButtonStates;
    EButtonStates m_Modifiers;
  };

} /* namespace mitk */

#endif /* MITKINTERACTIONPOSITIONEVENT_H_ */
