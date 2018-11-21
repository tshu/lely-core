/**@file
 * This file is part of the CANopen library; it contains the implementation of
 * the time stamp (TIME) object functions.
 *
 * @see lely/co/time.h
 *
 * @copyright 2016-2018 Lely Industries N.V.
 *
 * @author J. S. Seldenthuis <jseldenthuis@lely.com>
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

#include "co.h"

#ifndef LELY_NO_CO_TIME

#include <lely/co/dev.h>
#include <lely/co/obj.h>
#include <lely/co/sdo.h>
#include <lely/co/time.h>
#include <lely/co/val.h>
#include <lely/util/endian.h>
#include <lely/util/errnum.h>
#include <lely/util/time.h>

#include <assert.h>
#include <stdlib.h>

/// A CANopen TIME producer/consumer service.
struct __co_time {
	/// A pointer to a CAN network interface.
	can_net_t *net;
	/// A pointer to a CANopen device.
	co_dev_t *dev;
	/// The TIME COB-ID.
	uint32_t cobid;
	/// A pointer to the high-resolution time stamp sub-object.
	co_sub_t *sub_1013_00;
	/// A pointer to the CAN frame receiver.
	can_recv_t *recv;
	/// A pointer to the CAN timer.
	can_timer_t *timer;
	/// The creation time of the service.
	struct timespec start;
	/// A pointer to the indication function.
	co_time_ind_t *ind;
	/// A pointer to user-specified data for #ind.
	void *data;
};

/**
 * Updates and (de)activates a TIME producer/consumer service. This function is
 * invoked by the download indication functions when the TIME COB-ID (object
 * 1012) is updated.
 *
 * @returns 0 on success, or -1 on error.
 */
static int co_time_update(co_time_t *time);

/**
 * The download indication function for (all sub-objects of) CANopen object 1012
 * (COB-ID time stamp object).
 *
 * @see co_sub_dn_ind_t
 */
static co_unsigned32_t co_1012_dn_ind(
		co_sub_t *sub, struct co_sdo_req *req, void *data);

/**
 * The CAN receive callback function for a TIME consumer service.
 *
 * @see can_recv_func_t
 */
static int co_time_recv(const struct can_msg *msg, void *data);

/**
 * The CAN timer callback function for a TIME producer service.
 *
 * @see can_timer_func_t
 */
static int co_time_timer(const struct timespec *tp, void *data);

void
co_time_of_day_get(const co_time_of_day_t *tod, struct timespec *tp)
{
	assert(tod);
	assert(tp);

	// Convert the CANopen time (seconds since January 1, 1984) to the Unix
	// epoch (seconds since January 1, 1970). This is a difference of 14
	// years and 3 leap days.
	co_time_diff_get((const co_time_diff_t *)tod, tp);
	tp->tv_sec += (14 * 365 + 3) * 24 * 60 * 60;
}

void
co_time_of_day_set(co_time_of_day_t *tod, const struct timespec *tp)
{
	assert(tod);
	assert(tp);

	// Convert the Unix epoch (seconds since January 1, 1970) to the CANopen
	// time (seconds since January 1, 1984). This is a difference of 14
	// years and 3 leap days.
	// clang-format off
	co_time_diff_set((co_time_diff_t *)tod, &(struct timespec){
			tp->tv_sec - (14 * 365 + 3) * 24 * 60 * 60,
			tp->tv_nsec });
	// clang-format on
}

void
co_time_diff_get(const co_time_diff_t *td, struct timespec *tp)
{
	assert(td);
	assert(tp);

	tp->tv_sec = (time_t)td->days * 24 * 60 * 60 + td->ms / 1000;
	tp->tv_nsec = (long)(td->ms % 1000) * 1000000;
}

void
co_time_diff_set(co_time_diff_t *td, const struct timespec *tp)
{
	assert(td);
	assert(tp);

	// Compute the number of milliseconds since midnight.
	td->ms = (tp->tv_sec % (24 * 60 * 60)) * 1000 + tp->tv_nsec / 1000000;
	// Compute the number of days.
	td->days = (co_unsigned16_t)(tp->tv_sec / (24 * 60 * 60));
}

void *
__co_time_alloc(void)
{
	void *ptr = malloc(sizeof(struct __co_time));
	if (__unlikely(!ptr))
		set_errno(errno);
	return ptr;
}

void
__co_time_free(void *ptr)
{
	free(ptr);
}

struct __co_time *
__co_time_init(struct __co_time *time, can_net_t *net, co_dev_t *dev)
{
	assert(time);
	assert(net);
	assert(dev);

	errc_t errc = 0;

	time->net = net;
	time->dev = dev;

	// Retrieve the TIME COB-ID.
	co_obj_t *obj_1012 = co_dev_find_obj(time->dev, 0x1012);
	if (__unlikely(!obj_1012)) {
		errc = errnum2c(ERRNUM_NOSYS);
		goto error_obj_1012;
	}

	time->cobid = co_obj_get_val_u32(obj_1012, 0x00);
	time->sub_1013_00 = co_dev_find_sub(time->dev, 0x1013, 0x00);

	time->recv = NULL;
	time->timer = NULL;

	can_net_get_time(time->net, &time->start);

	time->ind = NULL;
	time->data = NULL;

	// Set the download indication function for the TIME COB-ID object.
	co_obj_set_dn_ind(obj_1012, &co_1012_dn_ind, time);

	if (__unlikely(co_time_update(time) == -1)) {
		errc = get_errc();
		goto error_update;
	}

	return time;

error_update:
	co_obj_set_dn_ind(obj_1012, NULL, NULL);
error_obj_1012:
	set_errc(errc);
	return NULL;
}

void
__co_time_fini(struct __co_time *time)
{
	assert(time);

	// Remove the download indication function for the TIME COB-ID object.
	co_obj_t *obj_1012 = co_dev_find_obj(time->dev, 0x1012);
	assert(obj_1012);
	co_obj_set_dn_ind(obj_1012, NULL, NULL);

	can_timer_destroy(time->timer);

	can_recv_destroy(time->recv);
}

co_time_t *
co_time_create(can_net_t *net, co_dev_t *dev)
{
	trace("creating TIME service");

	errc_t errc = 0;

	co_time_t *time = __co_time_alloc();
	if (__unlikely(!time)) {
		errc = get_errc();
		goto error_alloc_time;
	}

	if (__unlikely(!__co_time_init(time, net, dev))) {
		errc = get_errc();
		goto error_init_time;
	}

	return time;

error_init_time:
	__co_time_free(time);
error_alloc_time:
	set_errc(errc);
	return NULL;
}

void
co_time_destroy(co_time_t *time)
{
	if (time) {
		trace("destroying TIME service");
		__co_time_fini(time);
		__co_time_free(time);
	}
}

can_net_t *
co_time_get_net(const co_time_t *time)
{
	assert(time);

	return time->net;
}

co_dev_t *
co_time_get_dev(const co_time_t *time)
{
	assert(time);

	return time->dev;
}

void
co_time_get_ind(const co_time_t *time, co_time_ind_t **pind, void **pdata)
{
	assert(time);

	if (pind)
		*pind = time->ind;
	if (pdata)
		*pdata = time->data;
}

void
co_time_set_ind(co_time_t *time, co_time_ind_t *ind, void *data)
{
	assert(time);

	time->ind = ind;
	time->data = data;
}

void
co_time_start(co_time_t *time, const struct timespec *start,
		const struct timespec *interval)
{
	assert(time);

	if (__likely(time->timer))
		can_timer_start(time->timer, time->net, start, interval);
}

void
co_time_stop(co_time_t *time)
{
	assert(time);

	if (__likely(time->timer))
		can_timer_stop(time->timer);
}

static int
co_time_update(co_time_t *time)
{
	assert(time);

	if (time->cobid & CO_TIME_COBID_CONSUMER) {
		if (!time->recv) {
			time->recv = can_recv_create();
			if (__unlikely(!time->recv))
				return -1;
			can_recv_set_func(time->recv, &co_time_recv, time);
		}
		// Register the receiver under the specified CAN-ID.
		uint32_t id = time->cobid;
		uint8_t flags = 0;
		if (id & CO_TIME_COBID_FRAME) {
			id &= CAN_MASK_EID;
			flags |= CAN_FLAG_IDE;
		} else {
			id &= CAN_MASK_BID;
		}
		can_recv_start(time->recv, time->net, id, flags);
	} else if (time->recv) {
		can_recv_destroy(time->recv);
		time->recv = NULL;
	}

	if (time->cobid & CO_TIME_COBID_PRODUCER) {
		if (!time->timer) {
			time->timer = can_timer_create();
			if (__unlikely(!time->timer))
				return -1;
			can_timer_set_func(time->timer, &co_time_timer, time);
		}
	} else if (time->timer) {
		can_timer_destroy(time->timer);
		time->timer = NULL;
	}

	return 0;
}

static co_unsigned32_t
co_1012_dn_ind(co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	assert(sub);
	assert(co_obj_get_idx(co_sub_get_obj(sub)) == 0x1012);
	assert(req);
	co_time_t *time = data;
	assert(time);

	co_unsigned32_t ac = 0;

	co_unsigned16_t type = co_sub_get_type(sub);
	union co_val val;
	if (__unlikely(co_sdo_req_dn_val(req, type, &val, &ac) == -1))
		return ac;

	if (__unlikely(co_sub_get_subidx(sub))) {
		ac = CO_SDO_AC_NO_SUB;
		goto error;
	}

	assert(type == CO_DEFTYPE_UNSIGNED32);
	co_unsigned32_t cobid = val.u32;
	co_unsigned32_t cobid_old = co_sub_get_val_u32(sub);
	if (cobid == cobid_old)
		goto error;

	// The CAN-ID cannot be changed while the producer or consumer is and
	// remains active.
	int active = (cobid & CO_TIME_COBID_PRODUCER)
			|| (cobid & CO_TIME_COBID_CONSUMER);
	int active_old = (cobid_old & CO_TIME_COBID_PRODUCER)
			|| (cobid_old & CO_TIME_COBID_CONSUMER);
	uint32_t canid = cobid & CAN_MASK_EID;
	uint32_t canid_old = cobid_old & CAN_MASK_EID;
	if (active && active_old && canid != canid_old) {
		ac = CO_SDO_AC_PARAM_VAL;
		goto error;
	}

	// A 29-bit CAN-ID is only valid if the frame bit is set.
	// clang-format off
	if (__unlikely(!(cobid & CO_TIME_COBID_FRAME)
			&& (cobid & (CAN_MASK_EID ^ CAN_MASK_BID)))) {
		// clang-format on
		ac = CO_SDO_AC_PARAM_VAL;
		goto error;
	}

	time->cobid = cobid;

	co_sub_dn(sub, &val);
	co_val_fini(type, &val);

	co_time_update(time);
	return 0;

error:
	co_val_fini(type, &val);
	return ac;
}

static int
co_time_recv(const struct can_msg *msg, void *data)
{
	assert(msg);
	co_time_t *time = data;
	assert(time);

	// Ignore remote frames.
	if (__unlikely(msg->flags & CAN_FLAG_RTR))
		return 0;

#ifndef LELY_NO_CANFD
	// Ignore CAN FD format frames.
	if (__unlikely(msg->flags & CAN_FLAG_EDL))
		return 0;
#endif

	if (__unlikely(msg->len < 6))
		return 0;

	co_time_of_day_t tod;
	tod.ms = ldle_u32(msg->data) & UINT32_C(0x0fffffff);
	tod.days = ldle_u16(msg->data + 4);

	struct timespec tv;
	co_time_of_day_get(&tod, &tv);

	if (time->ind)
		time->ind(time, &tv, time->data);

	return 0;
}

static int
co_time_timer(const struct timespec *tp, void *data)
{
	assert(tp);
	co_time_t *time = data;
	assert(time);

	// Update the high-resolution time stamp, if it exists.
	if (time->sub_1013_00)
		co_sub_set_val_u32(time->sub_1013_00,
				(co_unsigned32_t)timespec_diff_usec(
						tp, &time->start));

	// Convert the time to a TIME_OF_DAY value.
	co_time_of_day_t tod = { 0, 0 };
	co_time_of_day_set(&tod, tp);

	struct can_msg msg = CAN_MSG_INIT;
	msg.id = time->cobid;
	if (time->cobid & CO_TIME_COBID_FRAME) {
		msg.id &= CAN_MASK_EID;
		msg.flags |= CAN_FLAG_IDE;
	} else {
		msg.id &= CAN_MASK_BID;
	}
	msg.len = 6;
	stle_u32(msg.data, tod.ms & UINT32_C(0x0fffffff));
	stle_u16(msg.data + 4, tod.days);
	can_net_send(time->net, &msg);

	return 0;
}

#endif // !LELY_NO_CO_TIME