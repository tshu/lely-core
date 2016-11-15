/*!\file
 * This header file is part of the CANopen library; it contains the ASCII
 * gateway declarations (see CiA 309-3 version 2.1).
 *
 * \copyright 2016 Lely Industries N.V.
 *
 * \author J. S. Seldenthuis <jseldenthuis@lely.com>
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

#ifndef LELY_CO_GW_TXT_H
#define LELY_CO_GW_TXT_H

#include <lely/co/gw.h>

struct __co_gw_txt;
#ifndef __cplusplus
//! An opaque CANopen ASCII gateway type.
typedef struct __co_gw_txt co_gw_txt_t;
#endif

// The file location struct from <lely/util/diag.h>.
struct floc;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The type of a CANopen ASCII gateway receive callback function, invoked when
 * when an indication of conformation is received from a gateway and needs to be
 * sent to the user.
 *
 * \param txt  a pointer to a human-readable string containing the indication or
 *             confirmation.
 * \param data a pointer to user-specified data.
 *
 * \returns 0 on success, or -1 on error. In the latter case, implementations
 * SHOULD set the error number with `set_errnum()`.
 */
typedef int co_gw_txt_recv_func_t(const char *txt, void *data);

/*!
 * The type of a CANopen ASCII gateway send callback function, invoked when a
 * request has been issued by the user and needs to be sent to a gateway.
 *
 * \param req  a pointer to the parameters of the request to be sent.
 * \param data a pointer to user-specified data.
 *
 * \returns 0 on success, or -1 on error. In the latter case, implementations
 * SHOULD set the error number with `set_errnum()`.
 */
typedef int co_gw_txt_send_func_t(const struct co_gw_req *req, void *data);

LELY_CO_EXTERN void *__co_gw_txt_alloc(void);
LELY_CO_EXTERN void __co_gw_txt_free(void *ptr);
LELY_CO_EXTERN struct __co_gw_txt *__co_gw_txt_init(struct __co_gw_txt *gw);
LELY_CO_EXTERN void __co_gw_txt_fini(struct __co_gw_txt *gw);

//! Creates a new CANopen ASCII gateway. \see co_gw_txt_destroy()
LELY_CO_EXTERN co_gw_txt_t *co_gw_txt_create(void);

//! Destroys a CANopen ASCII gateway. \see co_gw_txt_create()
LELY_CO_EXTERN void co_gw_txt_destroy(co_gw_txt_t *gw);

/*!
 * Receives and forwards an indication or confirmation from a CANopen gateway.
 *
 * \param gw  a pointer to a CANopen ASCII gateway.
 * \param srv a pointer to the service parameters.
 *
 * \returns 0 on success, or -1 on error. In the latter case, the error number
 * can be obtained with `get_errnum()`.
 */
LELY_CO_EXTERN int co_gw_txt_recv(co_gw_txt_t *gw, const struct co_gw_srv *srv);

/*!
 * Retrieves the callback function used to forward indications and confirmations
 * received by a CANopen gateway to the user.
 *
 * \param gw   a pointer to a CANopen ASCII gateway.
 * \param pfunc the address at which to store a pointer to the callback function
 *              (can be NULL).
 * \param pdata the address at which to store a pointer to user-specified data
 *              (can be NULL).
 *
 * \see co_gw_txt_set_recv_func()
 */
LELY_CO_EXTERN void co_gw_txt_get_recv_func(const co_gw_txt_t *gw,
		co_gw_txt_recv_func_t **pfunc, void **pdata);

/*!
 * Sets the callback function used to forward indications and confirmations
 * received by a CANopen gateway to the user.
 *
 * \param gw   a pointer to a CANopen ASCII gateway.
 * \param func a pointer to the function invoked by co_gw_txt_recv().
 * \param data a pointer to user-specified data (can be NULL). \a data is passed
 *             as the last parameter to \a func.
 *
 * \see co_gw_txt_get_recv_func()
 */
LELY_CO_EXTERN void co_gw_txt_set_recv_func(co_gw_txt_t *gw,
		co_gw_txt_recv_func_t *func, void *data);

/*!
 * Sends a user request to a CANopen gateway.
 *
 * \param gw    a pointer to a CANopen ASCII gateway.
 * \param begin a pointer to the start of the buffer containing the request.
 * \param end   a pointer to the end of the buffer (can be NULL if the buffer is
 *              null-terminated).
 * \param at    an optional pointer to the file location of \a begin (used for
 *              diagnostic purposes). On success, if `at != NULL`, *\a at points
 *              to one past the last character parsed. On error, *\a at is left
 *              untouched.
 *
 * \returns the number of characters read.
 */
LELY_CO_EXTERN size_t co_gw_txt_send(co_gw_txt_t *gw, const char *begin,
		const char *end, struct floc *at);

/*!
 * Retrieves the callback function used to send requests from the user to a
 * CANopen gateway.
 *
 * \param gw    a pointer to a CANopen ASCII gateway.
 * \param pfunc the address at which to store a pointer to the callback function
 *              (can be NULL).
 * \param pdata the address at which to store a pointer to user-specified data
 *              (can be NULL).
 *
 * \see co_gw_txt_set_send_func()
 */
LELY_CO_EXTERN void co_gw_txt_get_send_func(const co_gw_txt_t *gw,
		co_gw_txt_send_func_t **pfunc, void **pdata);

/*!
 * Sets the callback function used to send requests from the user to a CANopen
 * gateway.
 *
 * \param gw   a pointer to a CANopen ASCII gateway.
 * \param func a pointer to the function invoked by co_gw_txt_send().
 * \param data a pointer to user-specified data (can be NULL). \a data is passed
 *             as the last parameter to \a func.
 *
 * \see co_gw_txt_get_send_func()
 */
LELY_CO_EXTERN void co_gw_txt_set_send_func(co_gw_txt_t *gw,
		co_gw_txt_send_func_t *func, void *data);

#ifdef __cplusplus
}
#endif

#endif

