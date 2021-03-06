/*
 * Copyright 2013, epixoip & magnum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that redistribution of source
 * retains the above copyright.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "od6config.h"
#include "adl.h"
#include "functions.h"

void print_fanspeed (device_t *adapter)
{
	printf ("    Fan Speed           : %d%% (%d RPM)\n", adapter->fan_percent_current, adapter->fan_rpm_current);
}

void print_temp (device_t *adapter)
{
	printf ("    Current Temperature : %d C\n", adapter->temp);
	if (ADL_Overdrive6_TargetTemperatureData_Get)
		printf ("    Target  Temperature : %d C\n", adapter->target_temp_current);
}

void print_clocks (device_t *adapter)
{
	printf ("                         Core (Mhz)\tMemory (Mhz)\n");
	printf ("    Current Clocks      : %-d\t\t%-d\n", adapter->core_clock_current, adapter->mem_clock_current);
	printf ("    Min Clocks          : %-d\t\t%-d\n", adapter->core_clock_custom_range_min, adapter->mem_clock_custom_range_min);
	printf ("    Peak Clocks         : %-d\t\t%-d\n", adapter->core_clock_custom_range_max, adapter->mem_clock_custom_range_max);
	printf ("    PowerTune TDP       : %-d%%\n", adapter->pt_current);
	printf ("    GPU Load            : %-d%%\n", adapter->utilization);
}

void print_voltage (device_t *adapter)
{
	if (ADL_Overdrive6_VoltageControl_Get)
	{
		printf ("    Voltage offset      : %d mV ", adapter->voltage_current);
		if (adapter->voltage_current != adapter->voltage_default)
			printf ("(default: %d mV)", adapter->voltage_default);
		if (adapter->voltage_info.iStepValue)
			printf(" - range %d..%d in steps of %d", adapter->voltage_info.iMinValue, adapter->voltage_info.iMaxValue, adapter->voltage_info.iStepValue);
		puts("");
	} else
		printf ("    Failed to get voltage offset!\n");
}

void set_fanspeed (device_t *adapter, int speed)
{
	ADLOD6FanSpeedValue fan = {0};
	fan.iFanSpeed  = speed;
	fan.iSpeedType = ADL_OD6_FANSPEED_TYPE_PERCENT;

	if (speed < adapter->fan_min_percent || speed > adapter->fan_max_percent)
	{
		printf ("    Invalid fan speed: `%d'. Configurable range: %d-%d\n", speed, adapter->fan_min_percent, adapter->fan_max_percent);
		return;
	}

	if (ADL_Overdrive6_FanSpeed_Set (adapter->real_id, &fan) == ADL_OK)
		printf ("    New fan speed       : %d%%\n", speed);
	else
		printf ("    Failed to set fan speed!\n");
}

void set_clocks (device_t *adapter, int core, int mem)
{
	int bufsize = sizeof (ADLOD6StateInfo) + sizeof (ADLOD6PerformanceLevel);
	ADLOD6StateInfo *custom = (ADLOD6StateInfo*) calloc (1, bufsize);

	if (core < adapter->core_clock_min || core > adapter->core_clock_max)
	{
		printf ("    Invalid core clock: `%d'. Configurable range: %d-%d\n", core, adapter->core_clock_min, adapter->core_clock_max);
		return;
	}

	if (mem < adapter->mem_clock_min || mem > adapter->mem_clock_max)
	{
		printf ("    Invalid memory clock: `%d'. Configurable range: %d-%d\n", mem, adapter->mem_clock_min, adapter->mem_clock_max);
		return;
	}

	if (core % adapter->core_clock_step != 0)
	{
		printf ("    Invalid core clock: `%d'. Clock rate must be a multiple of %d\n", core, adapter->core_clock_step);
		return;
	}

	if (mem % adapter->mem_clock_step != 0)
	{
		printf ("    Invalid core clock: `%d'. Clock rate must be a multiple of %d\n", mem, adapter->mem_clock_step);
		return;
	}

	custom->iNumberOfPerformanceLevels = 2;
	custom->aLevels[0].iEngineClock = core * 100;
	custom->aLevels[1].iEngineClock = core * 100;
	custom->aLevels[0].iMemoryClock = mem  * 100;
	custom->aLevels[1].iMemoryClock = mem  * 100;

	if (ADL_Overdrive6_State_Set (adapter->real_id, ADL_OD6_SETSTATE_PERFORMANCE, custom) == ADL_OK)
		printf ("    New clocks          : %d Mhz core, %d Mhz memory\n", core, mem);
	else
		printf ("    Failed to set clocks!\n");
}

void set_voltage (device_t *adapter, int voltage)
{
	if (ADL_Overdrive6_VoltageControl_Set &&
	    ADL_Overdrive6_VoltageControl_Set (adapter->real_id, voltage) == ADL_OK)
		printf ("    New voltage offset  : %d mV\n", voltage);
	else
		printf ("    Failed to set voltage offset!\n");
}

void set_powertune (device_t *adapter, int threshold)
{
	if (threshold < adapter->pt_min || threshold > adapter->pt_max)
	{
		printf ("    Invalid powertune threshold: `%d'. Configurable range: %d-%d\n", threshold, adapter->pt_min, adapter->pt_max);
		return;
	}

	if (threshold % adapter->pt_step != 0)
	{
		printf ("    Invalid powertune threshold: `%d'. Threshold must be a multiple of %d\n", threshold, adapter->pt_step);
		return;
	}

	if (ADL_Overdrive6_PowerControl_Set (adapter->real_id, threshold) == ADL_OK)
		printf ("    New powertune value : %d\n", threshold);
	else
		printf ("    Failed to set powertune threshold!\n");
}

void set_targettemp (device_t *adapter, int target)
{
	if (ADL_Overdrive6_TargetTemperatureData_Set &&
	    ADL_Overdrive6_TargetTemperatureData_Set (adapter->real_id, target) == ADL_OK)
		printf ("    New target temp     : %d\n", target);
	else
		printf ("    Failed to set target temperature!\n");
}
