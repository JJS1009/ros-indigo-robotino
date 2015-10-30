#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <robotino_motion/ReadPuckAction.h>

int main (int argc, char **argv)
{
	ros::init(argc, argv, "read_puck_client_node");

	// create the action client
	// true causes the client to spin its own thread
	actionlib::SimpleActionClient<robotino_motion::ReadPuckAction> ac("read_puck", true);
	ROS_INFO("Waiting for action server to start.");
	// wait for the action server to start
	ac.waitForServer(); //will wait for infinite time

	ROS_INFO("Action server started, sending goal.");
	// send a goal to the action
	robotino_motion::ReadPuckGoal goal;
	goal.valid_colors.push_back(1); // YELLOW
	goal.valid_colors.push_back(2); // GREEN
	goal.valid_colors.push_back(3); // READ
	goal.verify_markers = false;
	ac.sendGoal(goal);

	//wait for the action to return
	bool finished_before_timeout = ac.waitForResult(ros::Duration(50.0));
	if (finished_before_timeout)
	{
		actionlib::SimpleClientGoalState state = ac.getState();
		ROS_INFO("Action finished: %s",state.toString().c_str());
	}
	else
	{
		ROS_INFO("Action did not finish before the time out.");
	}

	//exit
	return 0;
}
