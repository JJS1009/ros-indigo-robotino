/*
 * RobotinoLeds.cpp
 *
 *  Created on: 15/07/2014
 *      Author: adrianohrl@unifei.edu.br
 */

#include "RobotinoLeds.h"

RobotinoLeds::RobotinoLeds()
{
	nh_.param<double>("frequency", frequency_, 1.0);
	digital_pub_ = nh_.advertise<robotino_msgs::DigitalReadings>("set_digital_values", 1, true);
	go_srv_ = nh_.advertiseService("go_from_to", &RobotinoLeds::goFromTo, this);
	stop_srv_ = nh_.advertiseService("stop_transportation", &RobotinoLeds::stopTransportation, this);
	transport_srv_ = nh_.advertiseService("transport_product", &RobotinoLeds::transportProduct, this);
	product_ = NONE;
	departure_place_ = ORIGIN;
	arrival_place_ = SETOR_DE_CONTROLE;
	size_ = 8;
	digital_msg_.values.resize(size_);
	resetLeds();
	publish();
}

RobotinoLeds::~RobotinoLeds()
{
	digital_pub_.shutdown();
	stop_srv_.shutdown();
	transport_srv_.shutdown();
	go_srv_.shutdown();	
}

bool RobotinoLeds::spin()
{
	ros::Rate loop_rate(2 * frequency_);
	ROS_INFO("Robotino LED node is up and running!!!");
	while(nh_.ok())
	{
		if (product_ != NONE || departure_place_ != ORIGIN || arrival_place_ != SETOR_DE_CONTROLE)
			sinalizeTransportation();
		ros::spinOnce();
		loop_rate.sleep();
	}
	return true;
}

void RobotinoLeds::publish()
{
	digital_msg_.stamp = ros::Time::now();
	digital_pub_.publish(digital_msg_);
}

bool RobotinoLeds::goFromTo(robotino_leds::GoFromTo::Request &req, robotino_leds::GoFromTo::Response &res)
{
	bool succeed = true;
	succeed = resetLeds();
	publish();
	switch (req.departure_place)
	{
		case 0:
			departure_place_ = ORIGIN;
			break;
		case 1:
			departure_place_ = SETOR_DE_CONTROLE;
			break;
		case 2:
			departure_place_ = EXAMES;
			break;
		case 3:
			departure_place_ = CENTRO_CIRURGICO;
			break;
		case 4:
			departure_place_ = SETOR_DE_RECUPERACAO;
			break;
		case 5:
			departure_place_ = SETOR_DE_SAIDA;
			break;
		default:
			ROS_ERROR("Invalid departure place code: %d", req.departure_place);
			succeed = false;
	}
	switch (req.arrival_place)
	{
		case 0:
			arrival_place_ = ORIGIN;
			break;
		case 1:
			arrival_place_ = SETOR_DE_CONTROLE;
			break;
		case 2:
			arrival_place_ = EXAMES;
			break;
		case 3:
			arrival_place_ = CENTRO_CIRURGICO;
			break;
		case 4:
			arrival_place_ = SETOR_DE_RECUPERACAO;
			break;
		case 5:
			arrival_place_ = SETOR_DE_SAIDA;
			break;
		default:
			ROS_ERROR("Invalid arrival place code: %d", req.arrival_place);
			succeed = false;
	}
	if (succeed)
		ROS_INFO("Going from %s to %s!!!", placeToString(departure_place_), placeToString(arrival_place_));
	res.succeed  = succeed;
	return succeed;
}

bool RobotinoLeds::stopTransportation(robotino_leds::StopTransportation::Request &req, robotino_leds::StopTransportation::Response &res)
{
	ROS_INFO("Stopping Transportation!!!");
	bool succeed = false;
	product_ = NONE;
	departure_place_ = ORIGIN;
	arrival_place_ = SETOR_DE_CONTROLE;
	succeed = sinalizeEnd();
	res.succeed = succeed;
	return succeed;
}

bool RobotinoLeds::transportProduct(robotino_leds::TransportProduct::Request &req, robotino_leds::TransportProduct::Response &res)
{
	bool succeed = true;
	succeed = resetLeds();
	publish();
	switch (req.product)
	{
		case 1:
			product_ = TV;
			ROS_INFO("Transporting a TV!!!");
			break;
		case 2:
			product_ = DVD;
			ROS_INFO("Transporting a DVD!!!");
			break;
		case 3:
			product_ = CELULAR;
			ROS_INFO("Transporting a CELULAR!!!");
			break;
		case 4:
			product_ = TABLET;
			ROS_INFO("Transporting a TABLET!!!");
			break;
		case 5:
			product_ = NOTEBOOK;
			ROS_INFO("Transporting a NOTEBOOK!!!");
			break;
		default:
			ROS_ERROR("Invalid requested product: %d!!!", req.product);
			succeed = false;
	}	
	res.succeed = succeed;
	return succeed;
}

bool RobotinoLeds::sinalizeTransportation()
{
	bool succeed = true;
	switch (product_)
	{
		case NONE:
			succeed = false;
			break;
		case TV:
			succeed = toggleLed(YELLOW, BLUE);
			break;
		case DVD:
			succeed = toggleLed(BLUE, GREEN);
			break;
		case CELULAR:
			succeed = toggleLed(GREEN, YELLOW);
			break;
		case TABLET:
			succeed = toggleLed(RED, BLUE);
			break;
		case NOTEBOOK:
			succeed = toggleLed(GREEN, RED);
			break;
		default:
			succeed = false;
	}
	if (departure_place_ == EXAMES && arrival_place_ == SETOR_DE_SAIDA)
		succeed = toggleLed(GREEN);
	else if (departure_place_ == CENTRO_CIRURGICO && arrival_place_ == SETOR_DE_RECUPERACAO)
		succeed = toggleLed(RED);
	else if (departure_place_ == SETOR_DE_RECUPERACAO && arrival_place_ == SETOR_DE_SAIDA)
		succeed = toggleLed(YELLOW);
	else if (departure_place_ == EXAMES && arrival_place_ == CENTRO_CIRURGICO)
		succeed = toggleLed(BLUE);
	publish();
	return succeed;
}

bool RobotinoLeds::sinalizeEnd()
{
	ROS_INFO("Sinalizing The End!!!");
	ros::Duration d(.5 / frequency_);
	resetLeds();
	publish();
	for (int i = 0; i < 3; i++)
	{
		d.sleep();
		setLeds();
		publish();
		d.sleep();
		resetLeds();
		publish();
	}
	
	return true;
}

bool RobotinoLeds::toggleLed(int led)
{
	bool succeed = false;
	std::vector<bool> mask(size_, false);
	mask[led] = true;
	for (int i = 0; i < size_; i++)
		digital_msg_.values[i] = digital_msg_.values[i] ? !mask[i] : mask[i]; // xor boolean logic
	return succeed;
}

bool RobotinoLeds::toggleLed(int led1, int led2)
{
	bool succeed = false;
	if (!isLighting(led1) && !isLighting(led2))
		toggleLed(led1);
	else
	{
		toggleLed(led1);
		toggleLed(led2);
	}
	return succeed;
}

bool RobotinoLeds::setLeds()
{
	std::vector<bool> mask(size_, false);
	mask[RED] = true;
	mask[BLUE] = true;
	mask[YELLOW] = true;
	mask[GREEN] = true;
	return setLeds(mask);
}

bool RobotinoLeds::setLeds(std::vector<bool> mask)
{
	if (mask.size() != digital_msg_.values.size())
	{
		ROS_ERROR("mask size must be equals to digital_msg_.values size!!!");
		return false;
	}
	for (int i = 0; i < mask.size(); i++)
		digital_msg_.values[i] = true ? digital_msg_.values[i] || mask[i] : false; // or boolean logic
	return true;		
}

bool RobotinoLeds::resetLeds()
{
	std::vector<bool> mask(size_, false);
	mask[RED] = true;
	mask[BLUE] = true;
	mask[YELLOW] = true;
	mask[GREEN] = true;
	return resetLeds(mask);
}
	
bool RobotinoLeds::resetLeds(std::vector<bool> mask)
{
	if (mask.size() != digital_msg_.values.size())
	{
		ROS_ERROR("mask size must be equals to digital_msg_.values size!!!");
		return false;
	}
	for (int i = 0; i < mask.size(); i++)
		digital_msg_.values[i] = true ? !(!digital_msg_.values[i] || mask[i]) : false; // nonimplication boolean logic
	return true;	
}

bool RobotinoLeds::isLighting(int led)
{	
	return digital_msg_.values[led];
}

char* RobotinoLeds::placeToString(Place place)
{
	char* place_name;
	switch (place)
	{
		case ORIGIN:
			place_name = "ORIGIN";
			break;
		case SETOR_DE_CONTROLE:
			place_name = "SETOR_DE_CONTROLE";
			break;
		case EXAMES:
			place_name = "EXAMES";
			break;
		case CENTRO_CIRURGICO:
			place_name = "CENTRO_CIRURGICO";
			break;
		case SETOR_DE_RECUPERACAO:
			place_name = "SETOR_DE_RECUPERACAO";
			break;
		case SETOR_DE_SAIDA:
			place_name = "SETOR_DE_SAIDA";
			break;
	}
	return place_name;
}