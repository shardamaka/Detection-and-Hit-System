#include <functional>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <ignition/math/Vector3.hh>
#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "ros/subscribe_options.h"
#include "std_msgs/Float32.h"
#include "geometry_msgs/Point.h"

namespace gazebo
{
  class LaunchProjectile : public ModelPlugin
  {
    public: LaunchProjectile() : ModelPlugin()
    {
      printf("Loaded launch projectile plugin. \n");
      this->topicName = "/launcher_gazebo/set_projectile_vel";
    }
    public: void Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf)
    {
      double velocity = 0;
      // Store the pointer to the model
      this->model = _parent;
      // Listen to the update event. This event is broadcast every
      // simulation iteration.

      // check if tag <velocity> exists in sdf file
      if(_sdf->HasElement("velocity"))
        velocity = _sdf->Get<double>("velocity");

      this->link = this->model->GetLink("link");
      //this->updateConnection = event::Events::ConnectWorldUpdateBegin(
        //  std::bind(&LaunchProjectile::OnUpdate, this));

      // create ROS node
      if(!ros::isInitialized())
      {
        int argc    = 0;
        char **argv = NULL;
        ros::init(argc, argv, "launchProjectile");
      }
      this->rosNode.reset(new ros::NodeHandle("launchProjectile"));
      // Create a named topic, and subscribe to it.
      ros::SubscribeOptions so = ros::SubscribeOptions::create<geometry_msgs::Point>(
            this->topicName,
            1,
            boost::bind(&LaunchProjectile::OnMsg, this, _1),
            ros::VoidPtr(),
            &this->rosQueue);
      this->rosSub = this->rosNode->subscribe(so);
      // Spin up the queue helper thread. equivalent to rospy.spin()
      this->rosQueueThread = boost::thread(std::bind(&LaunchProjectile::QueueThread, this));

    }

    // Called by the ros msg event
    public: void OnMsg(const geometry_msgs::PointConstPtr &_vel_cmd)
    {

      this->link->SetLinearVel(ignition::math::Vector3d(0,0,0));
      if(!this->isShot)
      {
        this->isShot = true;
        this->link->SetLinearVel(ignition::math::Vector3d(_vel_cmd->x,_vel_cmd->y,_vel_cmd->z));
      }
      printf("Subscriber works. \n");
    }

    private: void QueueThread()
    {
      static const double timeout = 0.01;
      while (this->rosNode->ok())
      {
        this->rosQueue.callAvailable(ros::WallDuration(timeout));
      }
    }
    // variable
    private: bool isShot = false;
    // Pointer to the model
    private: physics::ModelPtr model;

    // Pointer to the link
    private: physics::LinkPtr link;

    // ROS node
    private: std::unique_ptr<ros::NodeHandle> rosNode;
    private: ros::Subscriber rosSub;
    private: ros::CallbackQueue rosQueue;
    /// \brief A thread the keeps running the rosQueue
    private: boost::thread rosQueueThread;

    private: std::string topicName;
  };
  // Register this plugin with the simulator
  GZ_REGISTER_MODEL_PLUGIN(LaunchProjectile)

}
