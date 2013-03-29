// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

/* -- BEGIN LICENSE BLOCK ----------------------------------------------

 Copyright (c) 2013, TB
 All rights reserved.

 Redistribution and use in source and binary forms are permitted
 provided that the above copyright notice and this paragraph are
 duplicated in all such forms and that any documentation,
 advertising materials, and other materials related to such
 distribution and use acknowledge that the software was developed
 by TB.  The name of the
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
#ifndef _MARBLE_PLUGIN_H
#define _MARBLE_PLUGIN_H

#include <QtCore/QObject>
#include <QMutex>
#include <marble/RouteRequest.h>
#include <marble/RoutingManager.h>

#include <ros/ros.h>
#include <rqt_gui_cpp/plugin.h>
#include <sensor_msgs/NavSatFix.h>

#include <rqt_marble/bridge_ros_marble.h>
#include <ui_marble_plugin.h> // Generated into ./build/rqt_robot_plugins/rqt_marble

namespace rqt_marble
{

class MarblePlugin : public rqt_gui_cpp::Plugin
{
  Q_OBJECT
public:
  MarblePlugin();
  virtual void initPlugin(qt_gui_cpp::PluginContext& context);
  virtual void shutdownPlugin();
  virtual void saveSettings(qt_gui_cpp::Settings& plugin_settings, qt_gui_cpp::Settings& instance_settings) const;
  virtual void restoreSettings(const qt_gui_cpp::Settings& plugin_settings,
                               const qt_gui_cpp::Settings& instance_settings);

  // Comment in to signal that the plugin has a way to configure it
  //bool hasConfiguration() const;
  //void triggerConfiguration();

  void GpsCallback(const sensor_msgs::NavSatFixConstPtr& gpspt);

private:

  Q_SIGNALS:

  void NewGPSPosition(qreal, qreal);

private Q_SLOTS:

  void ChangeGPSTopic(const QString &topic_name);
  void SetKMLFile(bool envoke_file_dialog = true);
  void ChangeMarbleModelTheme(int idx);
  void FindGPSTopics();
  void enableNavigation(bool checked);
  void routeChanged();

private:

  Ui_Form ui_;
  QWidget* widget_;

  ros::Subscriber m_sat_nav_fix_subscriber;

  Marble::RoutingManager* manager;
  Marble::RouteRequest* request;
  rqt_marble::BridgeRosMarble* ros_navigation;
  Marble::RoutingModel* routeModel;
};
} // namespace
#endif // _MARBLE_PLUGIN_H
