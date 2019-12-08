#include "ros/ros.h"
#include <std_msgs/String.h>
//#include "template_library.hpp"
#include <sstream>
#include <iostream>
#include <chrono>
#include <math.h>
#include <iterator>
#include <chrono>
#include <thread>
//#include "controllers/DroneControllerBase.hpp"
//#include "common/Common.hpp"
#include <fstream>
#include <cstdlib>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <trajectory_msgs/MultiDOFJointTrajectoryPoint.h>
#include <trajectory_msgs/MultiDOFJointTrajectory.h>
#include <stdio.h>
#include <time.h>
#include "std_msgs/Bool.h"
#include <signal.h>
#include <cstring>
#include <string>
#include <geometry_msgs/Vector3.h>
#include <mavbench_msgs/multiDOFpoint.h>
#include <mavbench_msgs/multiDOFtrajectory.h>
#include "TimeBudgetter.h"

using namespace std;
std::string ip_addr__global;
double g_sensor_max_range, g_sampling_interval, g_v_max;
std::deque<double> MacroBudgets;

typedef struct node_budget_t {
	string node_type;
	double budget;
} NodeBudget;


typedef struct param_val_t {
	string param_name;
	double param_val;
} ParamVal;


typedef struct node_param_t {
	string node_type;
	vector<ParamVal>  param_val_vec;
} NodeParams;

vector<string> node_types;

vector<NodeBudget> calc_micro_budget(double macro_time_budget){
	vector<NodeBudget> node_budget_vec;
	double node_coeff;
	for (auto &node_type: node_types){

		if (node_type == "point_cloud"){
			node_coeff = .1;
		} else if (node_type == "octomap"){
			node_coeff = .55;
		} else if (node_type == "planning"){
			node_coeff = .35;
		}else{
			ROS_ERROR_STREAM("node:" << node_type << " is not defined for runtime");
			exit(0);
		}
		node_budget_vec.push_back(NodeBudget{node_type, node_coeff*macro_time_budget});
	}

	return node_budget_vec;
}

int ctr = 0;

vector<ParamVal> perf_model(NodeBudget node_budget) {
	vector<ParamVal> results;
	if (node_budget.node_type == "point_cloud"){
		if (node_budget.budget > .2) {
			ROS_ERROR_STREAM("widening point c loud");
			results.push_back(ParamVal{"point_cloud_height", 25});
			results.push_back(ParamVal{"point_cloud_width", 25});
			results.push_back(ParamVal{"point_cloud_resolution", .3});
		}else{
			//ROS_ERROR_STREAM("shrinking point c loud");
			results.push_back(ParamVal{"point_cloud_height", 5});
			results.push_back(ParamVal{"point_cloud_width", 5});
			results.push_back(ParamVal{"point_cloud_resolution", .8});
		}
	} else if (node_budget.node_type == "octomap"){
		if (node_budget.budget > .5) {
			results.push_back(ParamVal{"perception_lower_resolution", .8});
		}else{
			results.push_back(ParamVal{"perception_lower_resolution", .8});
		}
	}else if (node_budget.node_type == "planning"){
		results.push_back(ParamVal{"piecewise_planning_budget", node_budget.budget/2});
		results.push_back(ParamVal{"smoothening_budget", node_budget.budget/2});
	}else{
		ROS_ERROR_STREAM("node:" << node_budget.node_type << " is not defined for runtime");
		exit(0);
	}
	return results;
}


vector<ParamVal> calc_params(NodeBudget node_budget) {
	return perf_model(node_budget);
}


void set_param(ParamVal param) {
	ros::param::set(param.param_name, param.param_val);
}


void convertMavBenchMultiDOFtoMultiDOF(mavbench_msgs::multiDOFpoint copy_from, multiDOFpoint &copy_to){
    	copy_to.vx = copy_from.vx;
    	copy_to.vy = copy_from.vy;
    	copy_to.vz = copy_from.vz;
    	copy_to.ax = copy_from.ax;
    	copy_to.ay = copy_from.ay;
    	copy_to.az = copy_from.az;
    	copy_to.x = copy_from.x;
    	copy_to.y = copy_from.y;
    	copy_to.z = copy_from.z;
}

void next_steps_callback(const mavbench_msgs::multiDOFtrajectory::ConstPtr& msg){
    std::deque<multiDOFpoint> traj;
    for (auto point_: msg->points){
    	multiDOFpoint point__;
    	convertMavBenchMultiDOFtoMultiDOF(point_, point__);
    	traj.push_back(point__);
    }

	vector<double> accelerationCoeffs = {.1439,.8016};
	double maxSensorRange = g_sensor_max_range;
	double TimeIncr = g_sampling_interval;
	double maxVelocity = g_v_max;

	double latency = 1.55; //TODO: get this from follow trajectory
	TimeBudgetter MacrotimeBudgetter(maxSensorRange, maxVelocity, accelerationCoeffs, TimeIncr);

	auto macro_time_budgets = MacrotimeBudgetter.calcSamplingTime(traj, latency);
	auto node_budget_vec = calc_micro_budget(macro_time_budgets[0]);

	// calculate each node's params
	vector<vector<ParamVal>> node_params_vec;
	for (auto &node_budget: node_budget_vec) {
		node_params_vec.push_back(calc_params(node_budget));
	}

	//set each node's params
	for (auto &node_params: node_params_vec) {
		for (auto param : node_params)
			;
			//set_param(param);
	}

	MacroBudgets.pop_front();


	return;
}


void initialize_global_params() {
	if(!ros::param::get("/sampling_interval", g_sampling_interval)){
		ROS_FATAL_STREAM("Could not start run_time_node sampling_interval not provided");
        exit(-1);
	}

	if(!ros::param::get("/v_max", g_v_max)) {
		ROS_FATAL_STREAM("Could not start run_time_node v_max not provided");
        exit(-1);
	}

	if(!ros::param::get("/sensor_max_range", g_sensor_max_range)) {
		ROS_FATAL_STREAM("Could not start run_time_node sensor_max_range not provided");
        exit(-1);
	}

	if(!ros::param::get("/ip_addr",ip_addr__global)){
		ROS_FATAL_STREAM("Could not start run_time_node ip_addr not provided");
        exit(-1);
	}
}

// *** F:DN main function
int main(int argc, char **argv)
{
    // ROS node initialization
    ros::init(argc, argv, "run_time_node", ros::init_options::NoSigintHandler);
    ros::NodeHandle n;
    //signal(SIGINT, sigIntHandler);

    node_types.push_back("point_cloud");
    node_types.push_back("octomap");
    node_types.push_back("planning");


    std::string ns = ros::this_node::getName();
    ros::Subscriber next_steps_sub = n.subscribe<mavbench_msgs::multiDOFtrajectory>("/next_steps", 1, next_steps_callback);
    initialize_global_params();

    uint16_t port = 41451;

    ROS_INFO_STREAM("ip to contact to now"<<ip_addr__global);
    //Drone drone(ip_addr__global.c_str(), port);
	ros::Rate pub_rate(5);


    while (ros::ok())
	{
        ros::spinOnce();
        pub_rate.sleep();
    }

}


