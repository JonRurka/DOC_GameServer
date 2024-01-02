#pragma once

#pragma execution_character_set("utf-8")

#define NOMINMAX

#include <cstdio>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <utility>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <array>
#include <unordered_map>
#include <optional>
#include <set>
#include <algorithm>
#include <iomanip>
#include <cstdarg>
#include <random>
#include <ctime>
#include <queue>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CL_TARGET_OPENCL_VERSION 210
#define CL_HPP_TARGET_OPENCL_VERSION 210

//#include <boost/stacktrace.hpp>
#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>


#if defined(WIN32) || defined(WIN64)
#define WINDOWS_PLATFROM
#include <windows.h>
#include <conio.h>
#pragma comment(lib, "dxgi")
#pragma comment(lib,"d3d11.lib")
#define WIN32_LEAN_AND_MEAN
#else
#define LINUX_PLATFORM
#include "Linux_conio.hpp"
#endif