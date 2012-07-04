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

#include "QmitkImageStatisticsView.h"

// Qt includes
#include <qclipboard.h>

// berry includes
#include <berryIWorkbenchPage.h>

// mitk includes
#include "mitkNodePredicateDataType.h"
#include "mitkPlanarFigureInteractor.h"

// itk includes
#include "itksys/SystemTools.hxx"

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";

QmitkImageStatisticsView::QmitkImageStatisticsView(QObject* /*parent*/, const char* /*name*/)
: m_Controls( NULL ),
m_TimeStepperAdapter( NULL ),
m_SelectedImage( NULL ),
m_SelectedImageMask( NULL ),
m_SelectedPlanarFigure( NULL ),
m_ImageObserverTag( -1 ),
m_ImageMaskObserverTag( -1 ),
m_PlanarFigureObserverTag( -1 ),
m_CurrentStatisticsValid( false ),
m_StatisticsUpdatePending( false ),
m_DataNodeSelectionChanged ( false ),
m_Visible(false)
{
  this->m_CalculationThread = new QmitkImageStatisticsCalculationThread;
  this->m_SelectedDataNodes = SelectedDataNodeVectorType(2);    // maximum number of selected nodes is exactly two!
}

QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
  if ( m_SelectedImage != NULL )
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
  if ( m_SelectedImageMask != NULL )
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
  if ( m_SelectedPlanarFigure != NULL )
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );

  while(this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }
  delete this->m_CalculationThread;

  //if(m_QThreadMutex != 0)
  //{
  //  delete m_QThreadMutex;
  //}
}

void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkImageStatisticsViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_LineProfileWidget->SetPathModeToPlanarFigure();
  }
}

void QmitkImageStatisticsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(this->m_Controls->m_ButtonCopyHistogramToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(OnClipboardHistogramButtonClicked()) );
    connect( (QObject*)(this->m_Controls->m_ButtonCopyStatisticsToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(OnClipboardStatisticsButtonClicked()) );
    connect( (QObject*)(this->m_Controls->m_IgnoreZerosCheckbox), SIGNAL(clicked()),(QObject*) this, SLOT(OnIgnoreZerosCheckboxClicked()) );
    connect( (QObject*) this->m_CalculationThread, SIGNAL(finished()),this, SLOT( OnThreadedStatisticsCalculationEnds()),Qt::QueuedConnection);
    connect( (QObject*) this, SIGNAL(StatisticsUpdate()),this, SLOT( RequestStatisticsUpdate()), Qt::QueuedConnection); 
  }
}

void QmitkImageStatisticsView::OnIgnoreZerosCheckboxClicked()
{
  emit StatisticsUpdate();
}

void QmitkImageStatisticsView::OnClipboardHistogramButtonClicked()
{
  if ( m_CurrentStatisticsValid )
  {
    typedef mitk::ImageStatisticsCalculator::HistogramType HistogramType;
    const HistogramType *histogram = this->m_CalculationThread->GetTimeStepHistogram().GetPointer();

    QString clipboard( "Measurement \t Frequency\n" );
    for ( HistogramType::ConstIterator it = histogram->Begin();
      it != histogram->End();
      ++it )
    {
      clipboard = clipboard.append( "%L1 \t %L2\n" )
        .arg( it.GetMeasurementVector()[0], 0, 'f', 2 )
        .arg( it.GetFrequency() );
    }

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}

void QmitkImageStatisticsView::OnClipboardStatisticsButtonClicked()
{
  if ( this->m_CurrentStatisticsValid )
  {
    const mitk::ImageStatisticsCalculator::Statistics &statistics =
      this->m_CalculationThread->GetStatisticsData();

    // Copy statistics to clipboard ("%Ln" will use the default locale for
    // number formatting)
    QString clipboard( "Mean \t StdDev \t RMS \t Max \t Min \t N \t V (mm³)\n" );
    clipboard = clipboard.append( "%L1 \t %L2 \t %L3 \t %L4 \t %L5 \t %L6 \t %L7" )
      .arg( statistics.Mean, 0, 'f', 10 )
      .arg( statistics.Sigma, 0, 'f', 10 )
      .arg( statistics.RMS, 0, 'f', 10 )
      .arg( statistics.Max, 0, 'f', 10 )
      .arg( statistics.Min, 0, 'f', 10 )
      .arg( statistics.N )
      .arg( m_Controls->m_StatisticsTable->item( 0, 6 )->text() );

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}

void QmitkImageStatisticsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*part*/,
                                                  const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  this->SelectionChanged( selectedNodes );
}

void QmitkImageStatisticsView::SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  if( this->m_StatisticsUpdatePending )
  {
    this->m_DataNodeSelectionChanged = true;
    return; // not ready for new data now!
  }
  this->ReinitData();
  if(selectedNodes.size() == 1 || selectedNodes.size() == 2)
  {
    for (int i= 0; i< selectedNodes.size(); ++i)
    {
      this->m_SelectedDataNodes.push_back(selectedNodes.at(i));
    }
    this->m_DataNodeSelectionChanged = false;
    this->m_Controls->m_ErrorMessageLabel->setText( "" );
    this->m_Controls->m_ErrorMessageLabel->hide();
    emit StatisticsUpdate();
  }
  else
  {
    this->m_DataNodeSelectionChanged = false;
  }
}

void QmitkImageStatisticsView::ReinitData()
{

  while( this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }

  if(this->m_SelectedImage != NULL)
  {
    this->m_SelectedImage->RemoveObserver( this->m_ImageObserverTag);
    this->m_SelectedImage = NULL;
  }
  if(this->m_SelectedImageMask != NULL)
  {
    this->m_SelectedImageMask->RemoveObserver( this->m_ImageMaskObserverTag);
    this->m_SelectedImageMask = NULL;
  }    
  if(this->m_SelectedPlanarFigure != NULL)
  {
    this->m_SelectedPlanarFigure->RemoveObserver( this->m_PlanarFigureObserverTag);
    this->m_SelectedPlanarFigure = NULL;
  }
  this->m_SelectedDataNodes.clear();
  this->m_StatisticsUpdatePending = false;

  m_Controls->m_ErrorMessageLabel->setText( "" );
  m_Controls->m_ErrorMessageLabel->hide();
  this->InvalidateStatisticsTableView();
  m_Controls->m_HistogramWidget->ClearItemModel();
  m_Controls->m_LineProfileWidget->ClearItemModel();
}

void QmitkImageStatisticsView::OnThreadedStatisticsCalculationEnds()
{
  std::stringstream message;
  message << "";
  m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
  m_Controls->m_ErrorMessageLabel->hide();
  this->WriteStatisticsToGUI();
}

void QmitkImageStatisticsView::UpdateStatistics()
{
  mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
  if ( renderPart == NULL )
  {
    this->m_StatisticsUpdatePending =  false;
    return;
  }
 // classify selected nodes
  mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");

  std::string maskName = std::string();
  std::string maskType = std::string();
  unsigned int maskDimension = 0;

  // reset data from last run
  ITKCommandType::Pointer changeListener = ITKCommandType::New();
  changeListener->SetCallbackFunction( this, &QmitkImageStatisticsView::SelectedDataModified );

  mitk::Image::Pointer selectedImage = mitk::Image::New();
  for( int i= 0 ; i < this->m_SelectedDataNodes.size(); ++i)
  { 
    mitk::PlanarFigure::Pointer planarFig = dynamic_cast<mitk::PlanarFigure*>(this->m_SelectedDataNodes.at(i)->GetData());
    if( imagePredicate->CheckNode(this->m_SelectedDataNodes.at(i)) )
    {
      bool isMask = false;
      this->m_SelectedDataNodes.at(i)->GetPropertyValue("binary", isMask);

      if( this->m_SelectedImageMask == NULL && isMask)
      {
        this->m_SelectedImageMask = dynamic_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
        this->m_ImageMaskObserverTag = this->m_SelectedImageMask->AddObserver(itk::ModifiedEvent(), changeListener);
       
        maskName = this->m_SelectedDataNodes.at(i)->GetName();
        maskType = m_SelectedImageMask->GetNameOfClass();
        maskDimension = 3;                                        
      }
      else if( !isMask )
      {
        if(this->m_SelectedImage == NULL)
        {
          this->m_SelectedImage = static_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
          this->m_ImageObserverTag = this->m_SelectedImage->AddObserver(itk::ModifiedEvent(), changeListener);
        }
      }
    }
    else if (planarFig.IsNotNull())
    {
      if(this->m_SelectedPlanarFigure == NULL)
      {
        this->m_SelectedPlanarFigure = planarFig;
        this->m_PlanarFigureObserverTag  = 
          this->m_SelectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
        maskName = this->m_SelectedDataNodes.at(i)->GetName();
        maskType = this->m_SelectedPlanarFigure->GetNameOfClass();
        maskDimension = 2;
      }
    }
    else
    {
      std::stringstream message;
      message << "<font color='red'>" << "Invalid data node type!" << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }
  }

  if(maskName == "")
  {
    maskName = "None";
    maskType = "";
    maskDimension = 0;
  }

  unsigned int timeStep = renderPart->GetTimeNavigationController()->GetTime()->GetPos();
  
  if ( m_SelectedImage != NULL && m_SelectedImage->IsInitialized())
  {
    // Check if a the selected image is a multi-channel image. If yes, statistics
    // cannot be calculated currently.
    if ( m_SelectedImage->GetPixelType().GetNumberOfComponents() > 1 )
    {
      std::stringstream message;
      message << "<font color='red'>Multi-component images not supported.</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();

      this->InvalidateStatisticsTableView();
      m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;
      this->m_StatisticsUpdatePending = false;
      return;
    }

    std::stringstream maskLabel;
    maskLabel << maskName;
    if ( maskDimension > 0 )
    {
      maskLabel << "  [" << maskDimension << "D " << maskType << "]";
    }
    m_Controls->m_SelectedMaskLabel->setText( maskLabel.str().c_str() );
    
    // check time step validity
    if(m_SelectedImage->GetDimension() <= 3 && timeStep > m_SelectedImage->GetDimension(3)-1)
    {
      timeStep = m_SelectedImage->GetDimension(3)-1;
    }

    //// initialize thread and trigger it
    this->m_CalculationThread->SetIgnoreZeroValueVoxel( m_Controls->m_IgnoreZerosCheckbox->isChecked() );
    this->m_CalculationThread->Initialize( m_SelectedImage, m_SelectedImageMask, m_SelectedPlanarFigure );
    this->m_CalculationThread->SetTimeStep( timeStep );
    std::stringstream message;
    message << "<font color='red'>Calculating statistics...</font>";
    m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
    m_Controls->m_ErrorMessageLabel->show();

    try
    {
      // Compute statistics
      this->m_CalculationThread->start();
    }
    catch ( const std::runtime_error &e )
    {
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>" << e.what() << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
    catch ( const std::exception &e )
    {
      MITK_ERROR << "Caught exception: " << e.what();

      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>Error! Unequal Dimensions of Image and Segmentation. No recompute possible </font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
  }
  else
  {
    this->m_StatisticsUpdatePending = false;
  }
}

void QmitkImageStatisticsView::SelectedDataModified()
{
  if( !m_StatisticsUpdatePending )
  {
    emit StatisticsUpdate();
  }
}

void QmitkImageStatisticsView::RequestStatisticsUpdate()
{
  if ( !m_StatisticsUpdatePending )
  {
    if(this->m_DataNodeSelectionChanged)
    {
      this->SelectionChanged( this->GetDataManagerSelection());  
    }
    else
    {
      this->m_StatisticsUpdatePending = true;
      this->UpdateStatistics();
    }
  }
  this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkImageStatisticsView::WriteStatisticsToGUI()
{
  if(m_DataNodeSelectionChanged)
  {
    this->m_StatisticsUpdatePending = false;
    this->RequestStatisticsUpdate();
    return;    // stop visualization of results and calculate statistics of new selection
  }

  if ( this->m_CalculationThread->GetStatisticsUpdateSuccessFlag())
  {
    if ( this->m_CalculationThread->GetStatisticsChangedFlag() )
    {
      // Do not show any error messages
      m_Controls->m_ErrorMessageLabel->hide();
      m_CurrentStatisticsValid = true;
    }

    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_HistogramWidget->SetHistogramModeToDirectHistogram();
    m_Controls->m_HistogramWidget->UpdateItemModelFromHistogram();
    m_Controls->m_HistogramWidget->SetHistogram( this->m_CalculationThread->GetTimeStepHistogram().GetPointer() );
    int timeStep = this->m_CalculationThread->GetTimeStep();
    this->FillStatisticsTableView( this->m_CalculationThread->GetStatisticsData(), this->m_CalculationThread->GetStatisticsImage());
  }
  else
  {
    m_Controls->m_SelectedMaskLabel->setText( "None" );
    std::stringstream message;
    message << "<font color='red'>Error calculating statistics!</font>";
    m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
    m_Controls->m_ErrorMessageLabel->show();
    // Clear statistics and histogram
    this->InvalidateStatisticsTableView();
    m_Controls->m_HistogramWidget->ClearItemModel();
    m_CurrentStatisticsValid = false;

    // If a (non-closed) PlanarFigure is selected, display a line profile widget
    if ( m_SelectedPlanarFigure != NULL )
    {
      // check whether PlanarFigure is initialized
      const mitk::Geometry2D *planarFigureGeometry2D = m_SelectedPlanarFigure->GetGeometry2D();
      if ( planarFigureGeometry2D == NULL )
      {
        // Clear statistics, histogram, and GUI
        this->InvalidateStatisticsTableView();
        m_Controls->m_HistogramWidget->ClearItemModel();
        m_Controls->m_LineProfileWidget->ClearItemModel();
        m_CurrentStatisticsValid = false;
        m_Controls->m_ErrorMessageLabel->hide();
        m_Controls->m_SelectedMaskLabel->setText( "None" );
        this->m_StatisticsUpdatePending = false;
        return;
      }
      // TODO: enable line profile widget
      m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 1 );
      m_Controls->m_LineProfileWidget->SetImage( this->m_CalculationThread->GetStatisticsImage() );
      m_Controls->m_LineProfileWidget->SetPlanarFigure( m_SelectedPlanarFigure );
      m_Controls->m_LineProfileWidget->UpdateItemModelFromPath();
    }
  }
  this->m_StatisticsUpdatePending = false;
}

void QmitkImageStatisticsView::ComputeIntensityProfile( mitk::PlanarLine* line )
{
  double sampling = 300;
  QmitkVtkHistogramWidget::HistogramType::Pointer histogram = QmitkVtkHistogramWidget::HistogramType::New();
  itk::Size<1> siz;
  siz[0] = sampling;
  itk::FixedArray<double,1> lower, higher;
  lower.Fill(0);
  mitk::Point3D begin = line->GetWorldControlPoint(0);
  mitk::Point3D end = line->GetWorldControlPoint(1);
  itk::Vector<double,3> direction = (end - begin);
  higher.Fill(direction.GetNorm());
  histogram->Initialize(siz, lower, higher);
  for(int i = 0; i < sampling; i++)
  {
    double d = m_SelectedImage->GetPixelValueByWorldCoordinate(begin + double(i)/sampling * direction);
    histogram->SetFrequency(i,d);
  }
  m_Controls->m_HistogramWidget->SetHistogramModeToDirectHistogram();
  m_Controls->m_HistogramWidget->SetHistogram( histogram );
  m_Controls->m_HistogramWidget->UpdateItemModelFromHistogram();
}

void QmitkImageStatisticsView::FillStatisticsTableView(
  const mitk::ImageStatisticsCalculator::Statistics &s,
  const mitk::Image *image )
{
  this->m_Controls->m_StatisticsTable->setItem( 0, 0, new QTableWidgetItem(
    QString("%1").arg(s.Mean, 0, 'f', 2) ) );
  this->m_Controls->m_StatisticsTable->setItem( 0, 1, new QTableWidgetItem(
    QString("%1").arg(s.Sigma, 0, 'f', 2) ) );

  this->m_Controls->m_StatisticsTable->setItem( 0, 2, new QTableWidgetItem(
    QString("%1").arg(s.RMS, 0, 'f', 2) ) );

  this->m_Controls->m_StatisticsTable->setItem( 0, 3, new QTableWidgetItem(
    QString("%1").arg(s.Max, 0, 'f', 2) ) );

  this->m_Controls->m_StatisticsTable->setItem( 0, 4, new QTableWidgetItem(
    QString("%1").arg(s.Min, 0, 'f', 2) ) );

  this->m_Controls->m_StatisticsTable->setItem( 0, 5, new QTableWidgetItem(
    QString("%1").arg(s.N) ) );

  const mitk::Geometry3D *geometry = image->GetGeometry();
  if ( geometry != NULL )
  {
    const mitk::Vector3D &spacing = image->GetGeometry()->GetSpacing();
    double volume = spacing[0] * spacing[1] * spacing[2] * (double) s.N;
    this->m_Controls->m_StatisticsTable->setItem( 0, 6, new QTableWidgetItem(
      QString("%1").arg(volume, 0, 'f', 2) ) );
  }
  else
  {
    this->m_Controls->m_StatisticsTable->setItem( 0, 6, new QTableWidgetItem(
      "NA" ) );
  }
}

void QmitkImageStatisticsView::InvalidateStatisticsTableView()
{
  for ( unsigned int i = 0; i < 7; ++i )
  {
    this->m_Controls->m_StatisticsTable->setItem( 0, i, new QTableWidgetItem( "NA" ) );
  }
}

void QmitkImageStatisticsView::Activated()
{
}

void QmitkImageStatisticsView::Deactivated()
{
}

void QmitkImageStatisticsView::Visible()
{
  m_Visible = true;
  this->OnSelectionChanged(this->GetSite()->GetPage()->FindView("org.mitk.views.datamanager"),
    this->GetDataManagerSelection());
}

void QmitkImageStatisticsView::Hidden()
{
  m_Visible = false;
}

void QmitkImageStatisticsView::SetFocus()
{
}
