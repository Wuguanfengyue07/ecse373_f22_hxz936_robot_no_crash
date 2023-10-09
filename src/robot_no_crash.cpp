#include <ros/ros.h>
#include <sstream>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/LaserScan.h>
#include "robot_no_crash/topic_name.h"

ros::Publisher* p_pub;
sensor_msgs::LaserScan* lidar_out;
geometry_msgs::Twist desired_velocity;
double wall_dist;

void lidarCallback(const sensor_msgs::LaserScan::ConstPtr& msg2)
{
    ROS_INFO("Laser?: [%d]", msg2->header.seq);

    bool obstacle = false;
    for (float range : msg2->ranges)
    {
        if (range < wall_dist)
        {
            obstacle = true;
            break;
        }
    }
    if (obstacle)
    {
        desired_velocity.linear.x = 0.0;
        desired_velocity.angular.z = 1.0;
    }
    else
    {
        desired_velocity.linear.x = 0.5;
        desired_velocity.angular.z = 0.0;
    }
}

void desvelCallback(const geometry_msgs::Twist::ConstPtr& msg)
{
    p_pub->publish(*msg);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "combined_node");
    ros::NodeHandle nh;

    // Publisher for cmd_vel
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10);

    // Subscriber for laser data
    ros::Subscriber lidar_sub = nh.subscribe<sensor_msgs::LaserScan>("laser_1", 10, lidarCallback);

    // Publisher for des_vel
    ros::Publisher publisher_handle = nh.advertise<geometry_msgs::Twist>("des_vel", 10);
    p_pub = &publisher_handle;

    // Parameter for wall_dist
    ros::param::param("wall_dist", wall_dist, 0.1);

    ros::Rate loop_rate(10);

    while (ros::ok())
    {
        // Publish desired_velocity on cmd_vel topic
        cmd_vel_pub.publish(desired_velocity);

        // Process subscribers and callbacks
        ros::spinOnce();

        // Publish des_vel on des_vel topic
        geometry_msgs::Twist des_vel_msg;
        des_vel_msg.linear.x = desired_velocity.linear.x;
        des_vel_msg.angular.z = desired_velocity.angular.z;
        publisher_handle.publish(des_vel_msg);

        loop_rate.sleep();
    }

    return 0;
}
