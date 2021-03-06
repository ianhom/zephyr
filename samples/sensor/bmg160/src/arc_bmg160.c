/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>

#include <misc/printk.h>
#include <sys_clock.h>
#include <stdio.h>

#include <device.h>
#include <sensor.h>
#include <i2c.h>

#define MAX_TEST_TIME	15000
#define SLEEPTIME	300

static void print_gyro_data(struct device *bmg160)
{
	struct sensor_value val[3];

	if (sensor_channel_get(bmg160, SENSOR_CHAN_GYRO_ANY, val) < 0) {
		printf("Cannot read bmg160 gyro channels.\n");
		return;
	}

	printf("Gyro (rad/s): X=%f, Y=%f, Z=%f\n",
	       val[0].val1 + val[0].val2 / 1000000.0,
	       val[1].val1 + val[1].val2 / 1000000.0,
	       val[2].val1 + val[2].val2 / 1000000.0);
}

static void print_temp_data(struct device *bmg160)
{
	struct sensor_value val;

	if (sensor_channel_get(bmg160, SENSOR_CHAN_TEMP, &val) < 0) {
		printf("Temperature channel read error.\n");
		return;
	}

	printf("Temperature (Celsius): %f\n",
	       val.val1 + val.val2 / 1000000.0);
}

static void test_polling_mode(struct device *bmg160)
{
	int32_t remaining_test_time = MAX_TEST_TIME;

	do {
		if (sensor_sample_fetch(bmg160) < 0) {
			printf("Gyro sample update error.\n");
		}

		print_gyro_data(bmg160);

		print_temp_data(bmg160);

		/* wait a while */
		k_sleep(SLEEPTIME);

		remaining_test_time -= SLEEPTIME;
	} while (remaining_test_time > 0);
}

static void trigger_handler(struct device *bmg160, struct sensor_trigger *trigger)
{
	if (trigger->type != SENSOR_TRIG_DATA_READY &&
	    trigger->type != SENSOR_TRIG_DELTA) {
		printf("Gyro: trigger handler: unknown trigger type.\n");
		return;
	}

	if (sensor_sample_fetch(bmg160) < 0) {
		printf("Gyro sample update error.\n");
	}

	print_gyro_data(bmg160);
}

static void test_trigger_mode(struct device *bmg160)
{
	int32_t remaining_test_time = MAX_TEST_TIME;
	struct sensor_trigger trig;
	struct sensor_value attr;

	trig.type = SENSOR_TRIG_DELTA;
	trig.chan = SENSOR_CHAN_GYRO_ANY;

	printf("Gyro: Testing anymotion trigger.\n");

	/* set up the trigger */

	/* set slope threshold to 10 dps */

	sensor_degrees_to_rad(10, &attr); /* convert to rad/s */

	if (sensor_attr_set(bmg160, SENSOR_CHAN_GYRO_ANY,
			    SENSOR_ATTR_SLOPE_TH, &attr) < 0) {
		printf("Gyro: cannot set slope threshold.\n");
		return;
	}

	/* set slope duration to 4 samples */
	attr.type = SENSOR_VALUE_TYPE_INT_PLUS_MICRO;
	attr.val1 = 4;
	attr.val2 = 0;

	if (sensor_attr_set(bmg160, SENSOR_CHAN_GYRO_ANY,
			    SENSOR_ATTR_SLOPE_DUR, &attr) < 0) {
		printf("Gyro: cannot set slope duration.\n");
		return;
	}

	if (sensor_trigger_set(bmg160, &trig, trigger_handler) < 0) {
		printf("Gyro: cannot set trigger.\n");
		return;
	}

	printf("Gyro: rotate the device and wait for events.\n");
	do {
		k_sleep(SLEEPTIME);
		remaining_test_time -= SLEEPTIME;
	} while (remaining_test_time > 0);

	if (sensor_trigger_set(bmg160, &trig, NULL) < 0) {
		printf("Gyro: cannot clear trigger.\n");
		return;
	}

	printf("Gyro: Anymotion trigger test finished.\n");

	printf("Gyro: Testing data ready trigger.\n");

	attr.type = SENSOR_VALUE_TYPE_INT_PLUS_MICRO;
	attr.val1 = 100;
	attr.val2 = 0;

	if (sensor_attr_set(bmg160, SENSOR_CHAN_GYRO_ANY,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &attr) < 0) {
		printf("Gyro: cannot set sampling frequency.\n");
		return;
	}

	trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_GYRO_ANY;

	if (sensor_trigger_set(bmg160, &trig, trigger_handler) < 0) {
		printf("Gyro: cannot set trigger.\n");
		return;
	}

	remaining_test_time = MAX_TEST_TIME;

	do {
		k_sleep(SLEEPTIME);
		remaining_test_time -= SLEEPTIME;
	} while (remaining_test_time > 0);

	if (sensor_trigger_set(bmg160, &trig, NULL) < 0) {
		printf("Gyro: cannot clear trigger.\n");
		return;
	}

	printf("Gyro: Data ready trigger test finished.\n");
}

void main(void)
{
	struct device *bmg160;
#if defined(CONFIG_BMG160_GYRO_RANGE_RUNTIME)
	struct sensor_value attr;
#endif

	bmg160 = device_get_binding("bmg160");
	if (!bmg160) {
		printf("Device not found.\n");
		return;
	}

#if defined(CONFIG_BMG160_GYRO_RANGE_RUNTIME)
	/*
	 * Set gyro range to +/- 250 degrees/s. Since the sensor API needs SI
	 * units, convert the range to rad/s.
	 */
	sensor_degrees_to_rad(250, &attr);

	if (sensor_attr_set(bmg160, SENSOR_CHAN_GYRO_ANY,
			    SENSOR_ATTR_FULL_SCALE, &attr) < 0) {
		printf("Cannot set gyro range.\n");
		return;
	}
#endif

	printf("Testing the polling mode.\n");
	test_polling_mode(bmg160);
	printf("Polling mode test finished.\n");

	printf("Testing the trigger mode.\n");
	test_trigger_mode(bmg160);
	printf("Trigger mode test finished.\n");
}
