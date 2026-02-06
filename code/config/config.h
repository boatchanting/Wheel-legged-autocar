#ifndef __CONFIG_H__
#define __CONFIG_H__
//如果我们创建的文件中涉及了系统配置的，我们需要在对应的.h中包含sys_options.h，如果涉及到小车选择的配置，则需要包含car_select.h，如果均涉及到，则两个都要包含...以此类推

#include "car_select.h"//选择哪个小车

#include "sys_options.h"//系统配置

#endif // __CONFIG_H__