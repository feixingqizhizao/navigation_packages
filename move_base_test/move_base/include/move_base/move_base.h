/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Eitan Marder-Eppstein
*********************************************************************/
#ifndef NAV_MOVE_BASE_ACTION_H_
#define NAV_MOVE_BASE_ACTION_H_

#include <vector>
#include <string>

#include <ros/ros.h>

#include <actionlib/server/simple_action_server.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <nav_core/base_local_planner.h>
#include <nav_core/base_global_planner.h>
#include <nav_core/recovery_behavior.h>
#include <geometry_msgs/PoseStamped.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <costmap_2d/costmap_2d.h>
#include <nav_msgs/GetPlan.h>

#include <pluginlib/class_loader.hpp>
#include <std_srvs/Empty.h>

#include <dynamic_reconfigure/server.h>
#include "move_base/MoveBaseConfig.h"

namespace move_base {
  //typedefs to help us out with the action server so that we don't hace to type so much
  //??????server?????????????????????move_base_msgs::MoveBaseAction
  typedef actionlib::SimpleActionServer<move_base_msgs::MoveBaseAction> MoveBaseActionServer;
  //??????movebase????????????
  //?????????????????????????????????????????????goal??????PLANNING??????????????????????????????state_??????PLANNING??????CONTROLLING???
  //????????????????????????PLANNING??????CLEARING
  enum MoveBaseState {
    PLANNING,
    CONTROLLING,
    CLEARING
  };
  //???????????????????????????,?????????PLANNING ?????? ?????? Recovery ?????? PLANNING_R
  enum RecoveryTrigger
  {
    PLANNING_R,//??????????????????
    CONTROLLING_R,//??????????????????
    OSCILLATION_R//??????????????????????????????
  };

  /**
   * @class MoveBase
   * @brief MoveBase????????????actionlib??????ActionServer?????????????????????robot?????????????????????
   */
  class MoveBase {
    public:
      /**
       * @brief  ?????????????????????????????????tf
       * @param name The name of the action
       * @param tf A reference to a TransformListener
       */
      MoveBase(tf2_ros::Buffer& tf);

      /**
       * @brief  Destructor - Cleans up
       */
      virtual ~MoveBase();

      /**
       * @brief  ?????????????????????????????? ??????????????????true?????????????????????false
       * @param goal A reference to the goal to pursue
       * @return True if processing of the goal is done, false otherwise
       */
      bool executeCycle(geometry_msgs::PoseStamped& goal);

    private:
      /**
       * @brief ??????costmap???server???
       * @param req ??????server???request
       * @param resp ??????server???response
       * @return ??????server????????????????????????True?????????false
       */
      bool clearCostmapsService(std_srvs::Empty::Request &req, std_srvs::Empty::Response &resp);

      /**
       * @brief  ???action???????????????????????????????????????plan
       * @param  req ??????goal???request
       * @param  resp ??????plan???request
       * @return ??????????????????reue???????????????false
       */
      bool planService(nav_msgs::GetPlan::Request &req, nav_msgs::GetPlan::Response &resp);

      /**
       * @brief  ??????????????????
       * @param  goal ??????????????????
       * @param  plan ??????
       * @return  ?????????????????????True ??????false
       */
      bool makePlan(const geometry_msgs::PoseStamped& goal, std::vector<geometry_msgs::PoseStamped>& plan);

      /**
       * @brief  ????????????????????????????????????Load the recovery behaviors
       * @param node ?????? ros::NodeHandle ???????????????
       * @return ??????????????????True ?????? false
       */
      bool loadRecoveryBehaviors(ros::NodeHandle node);

      /**
       * @brief  ??????????????????????????????
       */
      void loadDefaultRecoveryBehaviors();


      /**
       * @brief  ??????????????????????????????????????????
       * @param size_x ?????????????????????x
       * @param size_y ?????????????????????y
       */
      void clearCostmapWindows(double size_x, double size_y);

      /**
       * @brief  ???????????????0?????????
       */
      void publishZeroVelocity();

      /**
       * @brief  ??????move_base action???????????????????????????0
       */
      void resetState();

      void goalCB(const geometry_msgs::PoseStamped::ConstPtr& goal);

      void planThread();

      /**
       * @brief  movebase??????actionlib?????????????????????,??????goal?????????
       * @param size_x ?????????????????????x
       * @param size_y ?????????????????????y
       */
      void executeCb(const move_base_msgs::MoveBaseGoalConstPtr& move_base_goal);

      bool isQuaternionValid(const geometry_msgs::Quaternion& q);

      bool getRobotPose(geometry_msgs::PoseStamped& global_pose, costmap_2d::Costmap2DROS* costmap);

      double distance(const geometry_msgs::PoseStamped& p1, const geometry_msgs::PoseStamped& p2);

      geometry_msgs::PoseStamped goalToGlobalFrame(const geometry_msgs::PoseStamped& goal_pose_msg);

      /**
       * @brief ???????????????????????????
       */
      void wakePlanner(const ros::TimerEvent& event);

      tf2_ros::Buffer& tf_;

      MoveBaseActionServer* as_;//??????actionlib???server???

      boost::shared_ptr<nav_core::BaseLocalPlanner> tc_;//???????????????????????????????????????????????????
      costmap_2d::Costmap2DROS* planner_costmap_ros_, *controller_costmap_ros_;//costmap??????????????????

      boost::shared_ptr<nav_core::BaseGlobalPlanner> planner_;//???????????????????????????????????????????????????
      std::string robot_base_frame_, global_frame_;

      std::vector<boost::shared_ptr<nav_core::RecoveryBehavior> > recovery_behaviors_;//???????????????????????????
      unsigned int recovery_index_;

      geometry_msgs::PoseStamped global_pose_;
      //controller_frequency_ ???????????????????????????cmd_vel?????????????????????.
      //planner_frequency_ ?????????????????????????????????.???????????????0.0,????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????.
      double planner_frequency_, controller_frequency_, inscribed_radius_, circumscribed_radius_;
      //controller_patience ??????????????????????????????,?????????????????????????????????????????????.
      //planner_patience_ ??????????????????????????????,??????????????????????????????????????????????????????.
      double planner_patience_, controller_patience_;
      int32_t max_planning_retries_;
      uint32_t planning_retries_;
      double conservative_reset_dist_, clearing_radius_;
      ros::Publisher current_goal_pub_, vel_pub_, action_goal_pub_;
      ros::Subscriber goal_sub_;
      ros::ServiceServer make_plan_srv_, clear_costmaps_srv_;
      bool shutdown_costmaps_, clearing_rotation_allowed_, recovery_behavior_enabled_;
      bool make_plan_clear_costmap_, make_plan_add_unreachable_goal_;
      //oscillation_timeout_?????????????????????,?????????????????????.
      //oscillation_distance_?????????????????????????????????????????????????????????.
      double oscillation_timeout_, oscillation_distance_;

      MoveBaseState state_;
      RecoveryTrigger recovery_trigger_;

      ros::Time last_valid_plan_, last_valid_control_, last_oscillation_reset_;
      geometry_msgs::PoseStamped oscillation_pose_;
      //?????????ROS?????????????????????????????????plugin
      //??????????????????, ?????????????????????ROS???polygon_interface_package, ??????????????????polygon??????,
      //??????, ???????????????????????????polygon, ?????????rectangle, ?????????rectangle_plugin???,
      //???????????????triangle, ?????????triangle_plugin???. ????????????rectangle???triangle??????????????????.

      //ClassLoader(std::string package, std::string base_class,
      //std::string attrib_name = std::string("plugin"),
      //std::vector<std::string> plugin_xml_paths = std::vector<std::string>());
      pluginlib::ClassLoader<nav_core::BaseGlobalPlanner> bgp_loader_;
      pluginlib::ClassLoader<nav_core::BaseLocalPlanner> blp_loader_;
      pluginlib::ClassLoader<nav_core::RecoveryBehavior> recovery_loader_;

      //?????????????????????
      std::vector<geometry_msgs::PoseStamped>* planner_plan_;//????????????????????????????????????latest_plan_
      std::vector<geometry_msgs::PoseStamped>* latest_plan_;//???executeCycle?????????controller_plan_
      std::vector<geometry_msgs::PoseStamped>* controller_plan_;//????????????????????????

       //???????????????
      bool runPlanner_;//?????????????????????????????????????????????
      boost::recursive_mutex planner_mutex_;//??????????????????????????????????????????
      //?????????wait???notify_one ???????????????????????????????????????
      //boost????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
      boost::condition_variable_any planner_cond_;
      //??????????????????,????????????????????????MoveBase::executeCycle????????????controller_plan_
      geometry_msgs::PoseStamped planner_goal_;
      boost::thread* planner_thread_;


      boost::recursive_mutex configuration_mutex_;
      dynamic_reconfigure::Server<move_base::MoveBaseConfig> *dsrv_;
      
      void reconfigureCB(move_base::MoveBaseConfig &config, uint32_t level);

      move_base::MoveBaseConfig last_config_;
      move_base::MoveBaseConfig default_config_;
      bool setup_, p_freq_change_, c_freq_change_;
      bool new_global_plan_;
  };
};
#endif

