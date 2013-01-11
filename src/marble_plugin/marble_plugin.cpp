// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

/* -- BEGIN LICENSE BLOCK ----------------------------------------------

Copyright (c) 2013, TB
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
advertising materials, and other materials related to such
distribution and use acknowledge that the software was developed
by TB. The name of the
TB may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

  -- END LICENSE BLOCK ----------------------------------------------*/

//----------------------------------------------------------------------
/*!\file
*
* \author  Tobias Bär <baer@fzi.de>
* \date    2013-01-11
*
*/
//----------------------------------------------------------------------


// Qt Includes
#include <QLineEdit>
#include <QFileInfo>
#include <QFileDialog>
#include <QStringList>
#include <QStandardItemModel>
#include <QModelIndex>

// Own Includes
#include "marble_plugin.h"

// ROS Plugin Includes
#include <pluginlib/class_list_macros.h>

// Marble Includes
#include <marble/MarbleWidget.h>
#include <marble/MarbleModel.h>
#include <marble/MapThemeManager.h>
#include <marble/GeoPainter.h>

// @TODO: setDistance does not work on reloading
// @TODO: ComboBox for the MarbleWidget projection method
// @TOOD: Draw icon on the current gps pos (MarbleWidget needs to be subclassed (custom paint))

namespace marble_plugin {

MarblePlugin::MarblePlugin()
  : rqt_gui_cpp::Plugin()
  , widget_(0)
{
  // give QObjects reasonable names
  setObjectName("MarbleWidgetPlugin");
}

void MarblePlugin::initPlugin(qt_gui_cpp::PluginContext& context)
{
  // access standalone command line arguments
  QStringList argv = context.argv();

  // create QWidget
  widget_ = new QWidget();

  // add widget to the user interface
  ui_.setupUi( widget_ );

  ui_.MarbleWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");

  ui_.MarbleWidget->setProjection( Marble::Mercator );

  ui_.MarbleWidget->centerOn( 115.87164 , -31.93452 , false );  // My Happy Place: The Scotto

  ui_.MarbleWidget->setDistance(0.05);

  context.addWidget(widget_);

  ui_.comboBox_theme->setModel( ui_.MarbleWidget->model()->mapThemeManager()->mapThemeModel() );


  // Connections

  connect( this , SIGNAL(NewGPSPosition(qreal,qreal)) , ui_.MarbleWidget , SLOT(centerOn(qreal,qreal)) );

  connect( ui_.lineEdit_topic , SIGNAL(editingFinished()) , this , SLOT( ChangeGPSTopic()) );

  connect( ui_.lineEdit_kml , SIGNAL(returnPressed()) , this, SLOT( SetKMLFile() ));

  connect( ui_.comboBox_theme , SIGNAL(currentIndexChanged(int)) , this , SLOT(ChangeMarbleModelTheme(int)));

  // AutoNavigation Connections ... soon
  /*
  m_autoNavigation = new Marble::AutoNavigation( ui_.MarbleWidget->model(), ui_.MarbleWidget->viewport(), this );

  connect( m_autoNavigation, SIGNAL( zoomIn( FlyToMode ) ),
                       ui_.MarbleWidget, SLOT( zoomIn() ) );
  connect( m_autoNavigation, SIGNAL( zoomOut( FlyToMode ) ),
                       ui_.MarbleWidget, SLOT( zoomOut() ) );
  connect( m_autoNavigation, SIGNAL( centerOn( const GeoDataCoordinates &, bool ) ),
                       ui_.MarbleWidget, SLOT( centerOn( const GeoDataCoordinates & ) ) );

  connect( ui_.MarbleWidget , SIGNAL( visibleLatLonAltBoxChanged() ),
                       m_autoNavigation, SLOT( inhibitAutoAdjustments() ) );
    */
}


void MarblePlugin::shutdownPlugin()
{
  // unregister all publishers here
  m_sat_nav_fix_subscriber.shutdown();
}

void MarblePlugin::ChangeMarbleModelTheme(int idx )
{
    QStandardItemModel* model = ui_.MarbleWidget->model()->mapThemeManager()->mapThemeModel();
    QModelIndex index = model->index( idx , 0 );
    QString theme = model->data( index , Qt::UserRole+1  ).toString();

    ui_.MarbleWidget->setMapThemeId( theme );
}

void MarblePlugin::ChangeGPSTopic()
{
    m_sat_nav_fix_subscriber.shutdown();

    m_sat_nav_fix_subscriber = getNodeHandle().subscribe< sensor_msgs::NavSatFix >(
                ui_.lineEdit_topic->text().toStdString().c_str() , 10 , &MarblePlugin::GpsCallback , this );
}

void MarblePlugin::SetKMLFile( bool envoke_file_dialog )
{
    QFileInfo fi( ui_.lineEdit_kml->text() );

    if( !fi.isFile() && envoke_file_dialog )
    {
        QString fn = QFileDialog::getOpenFileName( 0 ,
                                     tr("Open Geo Data File"), tr("") , tr("Geo Data Files (*.kml)"));
        fi.setFile( fn );
    }

    if( fi.isFile() )
    {
        ui_.MarbleWidget->model()->addGeoDataFile( fi.absoluteFilePath() );

        ui_.lineEdit_kml->setText( fi.absoluteFilePath() );
    }
    else
    {
        ui_.lineEdit_kml->setText( "" );
    }
}

void MarblePlugin::GpsCallback( const sensor_msgs::NavSatFixConstPtr& gpspt )
{
    // std::cout << "GPS Callback " << gpspt->longitude << " " << gpspt->latitude << std::endl;
    assert( widget_ );

    // Emit NewGPSPosition only, if it changes significantly. Has to be somehow related to the zoom
    static qreal _x = -1;
    static qreal _y = -1;

    qreal x;
    qreal y;

    // Recenter if lat long is not on screen
    bool recenter = !ui_.MarbleWidget->screenCoordinates(gpspt->longitude,gpspt->latitude , x , y );
    recenter |= ui_.checkBox_center->isChecked();

    // Recenter if lat long within <threshold> pixels away from center
    qreal threshold = 20;
    recenter |=  ((x - _x) * (x - _x) + (y - _y) * (y - _y)) > threshold;

    if( recenter )
    {
        emit NewGPSPosition( gpspt->longitude , gpspt->latitude );
        ui_.MarbleWidget->screenCoordinates(gpspt->longitude,gpspt->latitude , _x , _y );
    }
}

void MarblePlugin::saveSettings(qt_gui_cpp::Settings& plugin_settings, qt_gui_cpp::Settings& instance_settings) const
{
  // save intrinsic configuration, usually using:
  instance_settings.setValue( "marble_plugin_topic" , ui_.lineEdit_topic->text() );
  instance_settings.setValue( "marble_plugin_kml_file" , ui_.lineEdit_kml->text().replace("." , "___dot_replacement___" ) );
  instance_settings.setValue( "marble_plugin_zoom" , ui_.MarbleWidget->distance() );
  instance_settings.setValue( "marble_theme_index" , ui_.comboBox_theme->currentIndex() );
  instance_settings.setValue( "marble_center" , ui_.checkBox_center->isChecked() );
}

void MarblePlugin::restoreSettings(const qt_gui_cpp::Settings& plugin_settings, const qt_gui_cpp::Settings& instance_settings)
{
  // restore intrinsic configuration, usually using:
    ui_.lineEdit_topic->setText( instance_settings.value("marble_plugin_topic", "").toString() );
    ui_.lineEdit_kml->setText( instance_settings.value("marble_plugin_kml_file" , "" ).toString().replace("___dot_replacement___",".") );
  ui_.comboBox_theme->setCurrentIndex( instance_settings.value( "marble_theme_index" , 0 ).toInt() );
  ui_.checkBox_center->setChecked( instance_settings.value( "marble_center" , true ).toBool());


  // std::cout << "Set distance " << instance_settings.value( "marble_plugin_zoom" ).toReal() << std::endl;

  ChangeGPSTopic();
  SetKMLFile(false);

  // @TODO: Does not work since the KML loading changes the zoom
  ui_.MarbleWidget->setDistance( instance_settings.value( "marble_plugin_zoom" , 0.05 ).toReal() );
}

/*bool hasConfiguration() const
{
  return true;
}

void triggerConfiguration()
{
  // Usually used to open a dialog to offer the user a set of configuration
}*/

} // namespace
PLUGINLIB_DECLARE_CLASS(marble_plugin, MarblePlugin, marble_plugin::MarblePlugin, rqt_gui_cpp::Plugin)

