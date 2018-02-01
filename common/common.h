#ifndef COMMON_H
#define COMMON_H

#include <limits>
#include <trajectory_msgs/MultiDOFJointTrajectoryPoint.h>
#include "Drone.h"

void update_stats_file(std::string  stats_file__addr, std::string content);
void sigIntHandler(int sig);

// Recovery methods
enum slam_recovery_method { spin, backtrack, reset };

void action_upon_panic(Drone& drone);
void action_upon_future_col(Drone& drone);
bool action_upon_slam_loss(Drone& drone, slam_recovery_method slm...);

// Functions to manipulate and follow trajectories
typedef trajectory_msgs::MultiDOFJointTrajectoryPoint multiDOFpoint;
typedef std::deque<multiDOFpoint> trajectory_t;
enum yaw_strategy_t { ignore_yaw, face_forward, face_backward, follow_yaw };

void follow_trajectory(Drone& drone, trajectory_t& traj,
        trajectory_t& reverse_traj,
        yaw_strategy_t yaw_strategy = ignore_yaw,
        float max_speed = std::numeric_limits<double>::infinity(),
        bool check_position = true,
        float time = 0.5);

// Spinning commands
void spin_around(Drone &drone);
void scan_around(Drone &drone, int angle);
// void spin(Drone &drone, int n_pies=20);

// Utility functions
float distance(float x, float y, float z);

#endif

