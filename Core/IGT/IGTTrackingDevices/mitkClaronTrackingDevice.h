/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkClaronTrackingDevice.h,v $
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

#ifndef MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_


#include <vector>
//ToDoMBISB#include <mitkMBISBConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkClaronTool.h>
#include <itkMultiThreader.h>

//only include MicronTracker if cmake Variable is on
#ifdef MITK_USE_MICRON_TRACKER
#include <mitkClaronInterface.h>
#else
#include <mitkClaronInterfaceStub.h>
#endif

namespace mitk
{
  class ClaronTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(ClaronTrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    /**
    * \brief Starts the tracking.
    * @return Returns true if the tracking is started. Returns false if there was an error.
    */
    virtual bool StartTracking();

    /**
    * \brief Stops the tracking.
    * @return Returns true if the tracking is stoped. Returns false if there was an error.
    */
    virtual bool StopTracking();

    /**
    * @return Returns all tools of the tracking device.
    */
    std::vector<ClaronTool::Pointer> GetAllTools();


    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is startet.
    */
    virtual bool OpenConnection();

    /**
    * \brief Closes the connection and clears all resources.
    */
    virtual bool CloseConnection();


    /**
    * @return Returns the number of tools which have been added to the device.
    */
    virtual unsigned int GetToolCount() const; //wird geerbt

    /**
    * @param toolNumber The number of the tool which should be given back.
    * @return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber); //wird geerbt

    /**
    * \brief Adds a tool to the tracking device.
    * @param tool  The tool which will be added.
    * @return Returns true if the tool has been added, false otherwise.
    */
    bool AddTool(ClaronTool::Pointer tool);

    /**
    * @return Returns wether the MicronTracker is installed (means wether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns false in this case.
    */
    bool IsMicronTrackerInstalled();
  
 
  protected:

    ClaronTrackingDevice(void);

    ~ClaronTrackingDevice(void);



    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means grabbing frames from the camera an updating the tools.
    */
    void TrackTools();

    /**
    * \brief Automatically detects tools in field of measurement of the tracking device. 
    * Tools can only be detected if their calibration file is availiable in the directory 
    * for calibration files.
    * @return Returns all detected Tools.
    */
    std::vector<ClaronTool::Pointer> DetectTools();

    /**
    * @return Gives back the device which is represented by an object of the class ClaronInterface.
    */
    ClaronInterface* GetDevice();        

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);
    std::vector<ClaronTool::Pointer> m_allTools;
    ClaronInterface* m_theDevice;
    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;

    /** \brief The directory where the camera calibrationfiles can be found */        
    std::string m_CalibrationDir;
    /** \brief The directory where the tool calibration files can be found */
    std::string m_ToolfilesDir;              
  };   
}//mitk
#endif /* MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_ */
