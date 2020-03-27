#pragma once

#include <memory>

class command 
{
public:
	static const std::unique_ptr<command> create()
	{

	}

public:
	virtual void command_implementation() = 0;
};