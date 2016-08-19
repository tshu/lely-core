/*!\file
 * This is the public header file of the CAN library.
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

#ifndef LELY_CAN_CAN_H
#define LELY_CAN_CAN_H

#include <lely/libc/libc.h>
#include <lely/util/util.h>

#ifndef LELY_CAN_EXTERN
#ifdef LELY_CAN_INTERN
#define LELY_CAN_EXTERN	extern LELY_DLL_EXPORT
#else
#define LELY_CAN_EXTERN	extern LELY_DLL_IMPORT
#endif
#endif

enum {
	//! The error active state (TX/RX error count < 128).
	CAN_STATE_ACTIVE,
	//! The error passive state (TX/RX error count < 256).
	CAN_STATE_PASSIVE,
	//! The bus off state (TX/RX error count >= 256).
	CAN_STATE_BUSOFF
};

enum {
	//! A single bit error.
	CAN_ERROR_BIT = 1 << 0,
	//! A bit stuffing error.
	CAN_ERROR_STUFF = 1 << 1,
	//! A CRC sequence error.
	CAN_ERROR_CRC = 1 << 2,
	//! A form error.
	CAN_ERROR_FORM = 1 << 3,
	//! An acknowledgment error.
	CAN_ERROR_ACK = 1 << 4
};

#endif

