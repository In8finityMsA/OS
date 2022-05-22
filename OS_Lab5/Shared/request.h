#pragma once
#include "pch.h"

struct Request {
	enum RequestType {
		READ,
		WRITE,
		CLOSE
	} type;
	size_t index;
};