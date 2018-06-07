/*!\file
 * This file is part of the CANopen library; it contains the implementation of
 * the object dictionary.
 *
 * \see lely/co/obj.h, src/obj.h
 *
 * \copyright 2018 Lely Industries N.V.
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

#include "co.h"
#include <lely/util/cmp.h>
#include <lely/util/errnum.h>
#include <lely/co/dev.h>
#include <lely/co/sdo.h>
#include "obj.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*!
 * Updates an object by allocating a new memory region containing the members
 * and moving the old values.
 */
static void co_obj_update(co_obj_t *obj);

//! Destroys all sub-objects.
static void co_obj_clear(co_obj_t *obj);

//! The default download indication function. \see co_sub_dn_ind_t
static co_unsigned32_t default_sub_dn_ind(co_sub_t *sub, struct co_sdo_req *req,
		void *data);

//! The default upload indication function. \see co_sub_up_ind_t
static co_unsigned32_t default_sub_up_ind(const co_sub_t *sub,
		struct co_sdo_req *req, void *data);

LELY_CO_EXPORT void *
__co_obj_alloc(void)
{
	void *ptr = malloc(sizeof(struct __co_obj));
	if (__unlikely(!ptr))
		set_errno(errno);
	return ptr;
}

LELY_CO_EXPORT void
__co_obj_free(void *ptr)
{
	free(ptr);
}

LELY_CO_EXPORT struct __co_obj *
__co_obj_init(struct __co_obj *obj, co_unsigned16_t idx)
{
	assert(obj);

	obj->node.key = &obj->idx;
	obj->dev = NULL;
	obj->idx = idx;

	rbtree_init(&obj->tree, &uint8_cmp);

#ifndef LELY_NO_CO_OBJ_NAME
	obj->name = NULL;
#endif

	obj->code = CO_OBJECT_VAR;

	obj->val = NULL;
	obj->size = 0;

	return obj;
}

LELY_CO_EXPORT void
__co_obj_fini(struct __co_obj *obj)
{
	assert(obj);

	if (obj->dev)
		co_dev_remove_obj(obj->dev, obj);

	co_obj_clear(obj);

#ifndef LELY_NO_CO_OBJ_NAME
	free(obj->name);
#endif
}

LELY_CO_EXPORT co_obj_t *
co_obj_create(co_unsigned16_t idx)
{
	trace("creating object %04X", idx);

	co_obj_t *obj = __co_obj_alloc();
	if (__unlikely(!obj))
		return NULL;

	return __co_obj_init(obj, idx);
}

LELY_CO_EXPORT void
co_obj_destroy(co_obj_t *obj)
{
	if (obj) {
		trace("destroying object %04X", obj->idx);
		__co_obj_fini(obj);
		__co_obj_free(obj);
	}
}

LELY_CO_EXPORT co_dev_t *
co_obj_get_dev(const co_obj_t *obj)
{
	assert(obj);

	return obj->dev;
}

LELY_CO_EXPORT co_unsigned16_t
co_obj_get_idx(const co_obj_t *obj)
{
	assert(obj);

	return obj->idx;
}

LELY_CO_EXPORT co_unsigned8_t
co_obj_get_subidx(const co_obj_t *obj, co_unsigned8_t maxidx,
		co_unsigned8_t *subidx)
{
	assert(obj);

	if (!subidx)
		maxidx = 0;

	if (maxidx) {
		struct rbnode *node = rbtree_first(&obj->tree);
		for (size_t i = 0; node && i < maxidx;
				node = rbnode_next(node), i++)
			subidx[i] = co_sub_get_subidx(structof(node, co_sub_t,
					node));
	}

	return (co_unsigned8_t)rbtree_size(&obj->tree);
}

LELY_CO_EXPORT int
co_obj_insert_sub(co_obj_t *obj, co_sub_t *sub)
{
	assert(obj);
	assert(sub);

	if (__unlikely(sub->obj && sub->obj != obj))
		return -1;

	if (__unlikely(sub->obj == obj))
		return 0;

	if (__unlikely(rbtree_find(&obj->tree, sub->node.key)))
		return -1;

	sub->obj = obj;
	rbtree_insert(&sub->obj->tree, &sub->node);

	co_obj_update(obj);

	return 0;
}

LELY_CO_EXPORT int
co_obj_remove_sub(co_obj_t *obj, co_sub_t *sub)
{
	assert(obj);
	assert(sub);

	if (__unlikely(sub->obj != obj))
		return -1;

	rbtree_remove(&sub->obj->tree, &sub->node);
	sub->obj = NULL;

	co_val_fini(co_sub_get_type(sub), sub->val);
	sub->val = NULL;

	co_obj_update(obj);

	return 0;
}

LELY_CO_EXPORT co_sub_t *
co_obj_find_sub(const co_obj_t *obj, co_unsigned8_t subidx)
{
	assert(obj);

	struct rbnode *node = rbtree_find(&obj->tree, &subidx);
	return __likely(node) ? structof(node, co_sub_t, node) : NULL;
}

#ifndef LELY_NO_CO_OBJ_NAME

LELY_CO_EXPORT const char *
co_obj_get_name(const co_obj_t *obj)
{
	assert(obj);

	return obj->name;
}

LELY_CO_EXPORT int
co_obj_set_name(co_obj_t *obj, const char *name)
{
	assert(obj);

	if (!name || !*name) {
		free(obj->name);
		obj->name = NULL;
		return 0;
	}

	void *ptr = realloc(obj->name, strlen(name) + 1);
	if (__unlikely(!ptr)) {
		set_errno(errno);
		return -1;
	}
	obj->name = ptr;
	strcpy(obj->name, name);

	return 0;
}

#endif // LELY_NO_CO_OBJ_NAME

LELY_CO_EXPORT co_unsigned8_t
co_obj_get_code(const co_obj_t *obj)
{
	assert(obj);

	return obj->code;
}

LELY_CO_EXPORT int
co_obj_set_code(co_obj_t *obj, co_unsigned8_t code)
{
	assert(obj);

	switch (code) {
	case CO_OBJECT_NULL:
	case CO_OBJECT_DOMAIN:
	case CO_OBJECT_DEFTYPE:
	case CO_OBJECT_DEFSTRUCT:
	case CO_OBJECT_VAR:
	case CO_OBJECT_ARRAY:
	case CO_OBJECT_RECORD:
		obj->code = code;
		return 0;
	default:
		set_errnum(ERRNUM_INVAL);
		return -1;
	}
}

LELY_CO_EXPORT void *
co_obj_addressof_val(const co_obj_t *obj)
{
	return __likely(obj) ? obj->val : NULL;
}

LELY_CO_EXPORT size_t
co_obj_sizeof_val(const co_obj_t *obj)
{
	return __likely(obj) ? obj->size : 0;
}

LELY_CO_EXPORT const void *
co_obj_get_val(const co_obj_t *obj, co_unsigned8_t subidx)
{
	co_sub_t *sub = __likely(obj) ? co_obj_find_sub(obj, subidx) : NULL;
	return co_sub_get_val(sub);
}

LELY_CO_EXPORT size_t
co_obj_set_val(co_obj_t *obj, co_unsigned8_t subidx, const void *ptr, size_t n)
{
	assert(obj);

	co_sub_t *sub = co_obj_find_sub(obj, subidx);
	if (__unlikely(!sub)) {
		set_errnum(ERRNUM_INVAL);
		return 0;
	}

	return co_sub_set_val(sub, ptr, n);
}

#define LELY_CO_DEFINE_TYPE(a, b, c, d) \
	LELY_CO_EXPORT co_##b##_t \
	co_obj_get_val_##c(const co_obj_t *obj, co_unsigned8_t subidx) \
	{ \
		co_sub_t *sub = __likely(obj) \
				? co_obj_find_sub(obj, subidx) \
				: NULL; \
		return co_sub_get_val_##c(sub); \
	} \
	\
	LELY_CO_EXPORT size_t \
	co_obj_set_val_##c(co_obj_t *obj, co_unsigned8_t subidx, co_##b##_t c) \
	{ \
		assert(obj); \
	\
		co_sub_t *sub = co_obj_find_sub(obj, subidx); \
		if (__unlikely(!sub)) { \
			set_errnum(ERRNUM_INVAL); \
			return 0; \
		} \
	\
		return co_sub_set_val_##c(sub, c); \
	}
#include <lely/co/def/basic.def>
#undef LELY_CO_DEFINE_TYPE

LELY_CO_EXPORT void
co_obj_set_dn_ind(co_obj_t *obj, co_sub_dn_ind_t *ind, void *data)
{
	assert(obj);

	rbtree_foreach(&obj->tree, node)
		co_sub_set_dn_ind(structof(node, co_sub_t, node), ind, data);
}

LELY_CO_EXPORT void
co_obj_set_up_ind(co_obj_t *obj, co_sub_up_ind_t *ind, void *data)
{
	assert(obj);

	rbtree_foreach(&obj->tree, node)
		co_sub_set_up_ind(structof(node, co_sub_t, node), ind, data);
}

LELY_CO_EXPORT void *
__co_sub_alloc(void)
{
	void *ptr = malloc(sizeof(struct __co_sub));
	if (__unlikely(!ptr))
		set_errno(errno);
	return ptr;
}

LELY_CO_EXPORT void
__co_sub_free(void *ptr)
{
	free(ptr);
}

LELY_CO_EXPORT struct __co_sub *
__co_sub_init(struct __co_sub *sub, co_unsigned8_t subidx, co_unsigned16_t type)
{
	assert(sub);

	sub->node.key = &sub->subidx;
	sub->obj = NULL;
	sub->subidx = subidx;

#ifndef LELY_NO_CO_OBJ_NAME
	sub->name = NULL;
#endif

	sub->type = type;
#ifndef LELY_NO_CO_OBJ_LIMITS
	if (__unlikely(co_val_init_min(sub->type, &sub->min) == -1))
		return NULL;
	if (__unlikely(co_val_init_max(sub->type, &sub->max) == -1))
		return NULL;
#endif
	if (__unlikely(co_val_init(sub->type, &sub->def) == -1))
		return NULL;
	sub->val = NULL;

	sub->access = CO_ACCESS_RW;
	sub->pdo_mapping = 0;
	sub->flags = 0;

	sub->dn_ind = &default_sub_dn_ind;
	sub->dn_data = NULL;
	sub->up_ind = &default_sub_up_ind;
	sub->up_data = NULL;

	return sub;
}

LELY_CO_EXPORT void
__co_sub_fini(struct __co_sub *sub)
{
	assert(sub);

	if (sub->obj)
		co_obj_remove_sub(sub->obj, sub);

	co_val_fini(sub->type, &sub->def);
#ifndef LELY_NO_CO_OBJ_LIMITS
	co_val_fini(sub->type, &sub->max);
	co_val_fini(sub->type, &sub->min);
#endif

#ifndef LELY_NO_CO_OBJ_NAME
	free(sub->name);
#endif
}

LELY_CO_EXPORT co_sub_t *
co_sub_create(co_unsigned8_t subidx, co_unsigned16_t type)
{
	errc_t errc = 0;

	co_sub_t *sub = __co_sub_alloc();
	if (__unlikely(!sub)) {
		errc = get_errc();
		goto error_alloc_sub;;
	}

	if (__unlikely(!__co_sub_init(sub, subidx, type))) {
		errc = get_errc();
		goto error_init_sub;
	}

	return sub;

error_init_sub:
	__co_sub_free(sub);
error_alloc_sub:
	set_errc(errc);
	return NULL;
}

LELY_CO_EXPORT void
co_sub_destroy(co_sub_t *sub)
{
	if (sub) {
		__co_sub_fini(sub);
		__co_sub_free(sub);
	}
}


LELY_CO_EXPORT co_obj_t *
co_sub_get_obj(const co_sub_t *sub)
{
	assert(sub);

	return sub->obj;
}

LELY_CO_EXPORT co_unsigned8_t
co_sub_get_subidx(const co_sub_t *sub)
{
	assert(sub);

	return sub->subidx;
}

#ifndef LELY_NO_CO_OBJ_NAME

LELY_CO_EXPORT const char *
co_sub_get_name(const co_sub_t *sub)
{
	assert(sub);

	return sub->name;
}

LELY_CO_EXPORT int
co_sub_set_name(co_sub_t *sub, const char *name)
{
	assert(sub);

	if (!name || !*name) {
		free(sub->name);
		sub->name = NULL;
		return 0;
	}

	void *ptr = realloc(sub->name, strlen(name) + 1);
	if (__unlikely(!ptr)) {
		set_errno(errno);
		return -1;
	}
	sub->name = ptr;
	strcpy(sub->name, name);

	return 0;
}

#endif // LELY_NO_CO_OBJ_NAME

LELY_CO_EXPORT co_unsigned16_t
co_sub_get_type(const co_sub_t *sub)
{
	assert(sub);

	return sub->type;
}

#ifndef LELY_NO_CO_OBJ_LIMITS

LELY_CO_EXPORT const void *
co_sub_addressof_min(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_addressof(sub->type, &sub->min) : NULL;
}

LELY_CO_EXPORT size_t
co_sub_sizeof_min(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_sizeof(sub->type, &sub->min) : 0;
}

LELY_CO_EXPORT const void *
co_sub_get_min(const co_sub_t *sub)
{
	return __likely(sub) ? &sub->min : NULL;
}

LELY_CO_EXPORT size_t
co_sub_set_min(co_sub_t *sub, const void *ptr, size_t n)
{
	assert(sub);

	co_val_fini(sub->type, &sub->min);
	return co_val_make(sub->type, &sub->min, ptr, n);
}

LELY_CO_EXPORT const void *
co_sub_addressof_max(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_addressof(sub->type, &sub->max) : NULL;
}

LELY_CO_EXPORT size_t
co_sub_sizeof_max(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_sizeof(sub->type, &sub->max) : 0;
}

LELY_CO_EXPORT const void *
co_sub_get_max(const co_sub_t *sub)
{
	return __likely(sub) ? &sub->max : NULL;
}

LELY_CO_EXPORT size_t
co_sub_set_max(co_sub_t *sub, const void *ptr, size_t n)
{
	assert(sub);

	co_val_fini(sub->type, &sub->max);
	return co_val_make(sub->type, &sub->max, ptr, n);
}

#endif // LELY_NO_CO_OBJ_LIMITS

LELY_CO_EXPORT const void *
co_sub_addressof_def(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_addressof(sub->type, &sub->def) : NULL;
}

LELY_CO_EXPORT size_t
co_sub_sizeof_def(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_sizeof(sub->type, &sub->def) : 0;
}

LELY_CO_EXPORT const void *
co_sub_get_def(const co_sub_t *sub)
{
	return __likely(sub) ? &sub->def : NULL;
}

LELY_CO_EXPORT size_t
co_sub_set_def(co_sub_t *sub, const void *ptr, size_t n)
{
	assert(sub);

	co_val_fini(sub->type, &sub->def);
	return co_val_make(sub->type, &sub->def, ptr, n);
}

LELY_CO_EXPORT const void *
co_sub_addressof_val(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_addressof(sub->type, sub->val) : NULL;
}

LELY_CO_EXPORT size_t
co_sub_sizeof_val(const co_sub_t *sub)
{
	return __likely(sub) ? co_val_sizeof(sub->type, sub->val) : 0;
}

LELY_CO_EXPORT const void *
co_sub_get_val(const co_sub_t *sub)
{
	return __likely(sub) ? sub->val : NULL;
}

LELY_CO_EXPORT size_t
co_sub_set_val(co_sub_t *sub, const void *ptr, size_t n)
{
	assert(sub);

	co_val_fini(sub->type, sub->val);
	return co_val_make(sub->type, sub->val, ptr, n);
}

#define LELY_CO_DEFINE_TYPE(a, b, c, d) \
	LELY_CO_EXPORT co_##b##_t \
	co_sub_get_val_##c(const co_sub_t *sub) \
	{ \
		static const co_##b##_t val; \
	\
		if (__unlikely(!sub || sub->type != CO_DEFTYPE_##a \
				|| !sub->val)) \
			return val; \
		return ((union co_val *)sub->val)->c; \
	} \
	\
	LELY_CO_EXPORT size_t \
	co_sub_set_val_##c(co_sub_t *sub, co_##b##_t c) \
	{ \
		assert(sub); \
	\
		if (__unlikely(sub->type != CO_DEFTYPE_##a)) { \
			set_errnum(ERRNUM_INVAL); \
			return 0; \
		} \
	\
		return co_sub_set_val(sub, &c, sizeof(c)); \
	}
#include <lely/co/def/basic.def>
#undef LELY_CO_DEFINE_TYPE

#ifndef LELY_NO_CO_OBJ_LIMITS
LELY_CO_EXPORT co_unsigned32_t
co_sub_chk_val(const co_sub_t *sub, co_unsigned16_t type, const void *val) {
	assert(sub);

	// Arrays do not have a minimum or maximum value.
	if (!co_type_is_basic(sub->type))
		return 0;

	if (__unlikely(sub->type != type))
		return CO_SDO_AC_TYPE_LEN;

	assert(val);

	// Check whether the value is within bounds.
	if (__unlikely(co_val_cmp(sub->type, &sub->min, &sub->max) > 0))
		return CO_SDO_AC_PARAM_RANGE;
	if (__unlikely(co_val_cmp(sub->type, val, &sub->max) > 0))
		return CO_SDO_AC_PARAM_HI;
	if (__unlikely(co_val_cmp(sub->type, val, &sub->min) < 0))
		return CO_SDO_AC_PARAM_LO;

	return 0;
}
#endif

LELY_CO_EXPORT unsigned int
co_sub_get_access(const co_sub_t *sub)
{
	assert(sub);

	return sub->access;
}

LELY_CO_EXPORT int
co_sub_set_access(co_sub_t *sub, unsigned int access)
{
	assert(sub);

	switch (access) {
	case CO_ACCESS_RO:
	case CO_ACCESS_WO:
	case CO_ACCESS_RW:
	case CO_ACCESS_RWR:
	case CO_ACCESS_RWW:
	case CO_ACCESS_CONST:
		sub->access = access;
		return 0;
	default:
		set_errnum(ERRNUM_INVAL);
		return -1;
	}
}

LELY_CO_EXPORT int
co_sub_get_pdo_mapping(const co_sub_t *sub)
{
	assert(sub);

	return sub->pdo_mapping;
}

LELY_CO_EXPORT void
co_sub_set_pdo_mapping(co_sub_t *sub, int pdo_mapping)
{
	assert(sub);

	sub->pdo_mapping = !!pdo_mapping;
}

LELY_CO_EXPORT unsigned int
co_sub_get_flags(const co_sub_t *sub)
{
	assert(sub);

	return sub->flags;
}

LELY_CO_EXPORT void
co_sub_set_flags(co_sub_t *sub, unsigned int flags)
{
	assert(sub);

	sub->flags = flags;
}

LELY_CO_EXPORT void
co_sub_get_dn_ind(const co_sub_t *sub, co_sub_dn_ind_t **pind, void **pdata)
{
	assert(sub);

	if (pind)
		*pind = sub->dn_ind;
	if (pdata)
		*pdata = sub->dn_data;
}

LELY_CO_EXPORT void
co_sub_set_dn_ind(co_sub_t *sub, co_sub_dn_ind_t *ind, void *data)
{
	assert(sub);

	sub->dn_ind = ind ? ind : &default_sub_dn_ind;
	sub->dn_data = ind ? data : NULL;
}

LELY_CO_EXPORT co_unsigned32_t
co_sub_on_dn(co_sub_t *sub, struct co_sdo_req *req)
{
	assert(sub);
	assert(req);

	co_unsigned32_t ac = 0;

#ifndef LELY_NO_CO_OBJ_FILE
	if (co_sub_get_type(sub) == CO_DEFTYPE_DOMAIN && (co_sub_get_flags(sub)
			& CO_OBJ_FLAGS_DOWNLOAD_FILE)) {
		co_sdo_req_dn_file(req, co_sub_addressof_val(sub), &ac);
		return ac;
	}
#endif

	// Read the value.
	co_unsigned16_t type = co_sub_get_type(sub);
	union co_val val;
	if (__unlikely(co_sdo_req_dn_val(req, type, &val, &ac) == -1))
		goto error_req;

#ifndef LELY_NO_CO_OBJ_LIMITS
	// Accept the value if it is within bounds.
	ac = co_sub_chk_val(sub, type, &val);
#endif
	if (__likely(!ac))
		co_sub_dn(sub, &val);

	co_val_fini(type, &val);
error_req:
	return ac;
}

LELY_CO_EXPORT co_unsigned32_t
co_sub_dn_ind(co_sub_t *sub, struct co_sdo_req *req)
{
	if (__unlikely(!sub))
		return CO_SDO_AC_NO_SUB;

	if (__unlikely(!(sub->access & CO_ACCESS_WRITE)))
		return CO_SDO_AC_NO_WRITE;

	if (__unlikely(!req))
		return CO_SDO_AC_ERROR;

	assert(sub->dn_ind);
	return sub->dn_ind(sub, req, sub->dn_data);
}

LELY_CO_EXPORT co_unsigned32_t
co_sub_dn_ind_val(co_sub_t *sub, co_unsigned16_t type, const void *val)
{
	if (__unlikely(co_sub_get_type(sub) != type))
		return CO_SDO_AC_TYPE_LEN;

	struct co_sdo_req req = CO_SDO_REQ_INIT;
	co_unsigned32_t ac = 0;

	errc_t errc = get_errc();

	if (__unlikely(co_sdo_req_up_val(&req, type, val, &ac) == -1))
		goto error;

	ac = co_sub_dn_ind(sub, &req);

error:
	co_sdo_req_fini(&req);
	set_errc(errc);
	return ac;
}

LELY_CO_EXPORT int
co_sub_dn(co_sub_t *sub, void *val)
{
	assert(sub);

	if (!(sub->flags & CO_OBJ_FLAGS_WRITE)) {
		co_val_fini(sub->type, sub->val);
		if (__unlikely(!co_val_move(sub->type, sub->val, val)))
			return -1;
	}

	return 0;
}

LELY_CO_EXPORT void
co_sub_get_up_ind(const co_sub_t *sub, co_sub_up_ind_t **pind, void **pdata)
{
	assert(sub);

	if (pind)
		*pind = sub->up_ind;
	if (pdata)
		*pdata = sub->up_data;
}

LELY_CO_EXPORT void
co_sub_set_up_ind(co_sub_t *sub, co_sub_up_ind_t *ind, void *data)
{
	assert(sub);

	sub->up_ind = ind ? ind : &default_sub_up_ind;
	sub->up_data = ind ? data : NULL;
}

LELY_CO_EXPORT co_unsigned32_t
co_sub_on_up(const co_sub_t *sub, struct co_sdo_req *req)
{
	assert(sub);
	assert(req);

	co_unsigned32_t ac = 0;

#ifndef LELY_NO_CO_OBJ_FILE
	if (co_sub_get_type(sub) == CO_DEFTYPE_DOMAIN
			&& (co_sub_get_flags(sub) & CO_OBJ_FLAGS_UPLOAD_FILE)) {
		co_sdo_req_up_file(req, co_sub_addressof_val(sub), &ac);
		return ac;
	}
#endif

	const void *val = co_sub_get_val(sub);
	if (__unlikely(!val))
		return CO_SDO_AC_NO_DATA;

	co_sdo_req_up_val(req, co_sub_get_type(sub), val, &ac);
	return ac;
}

LELY_CO_EXPORT co_unsigned32_t
co_sub_up_ind(const co_sub_t *sub, struct co_sdo_req *req)
{
	if (__unlikely(!sub))
		return CO_SDO_AC_NO_SUB;

	if (__unlikely(!(sub->access & CO_ACCESS_READ)))
		return CO_SDO_AC_NO_READ;

	if (__unlikely(!req))
		return CO_SDO_AC_ERROR;

	assert(sub->up_ind);
	return sub->up_ind(sub, req, sub->up_data);
}

static void
co_obj_update(co_obj_t *obj)
{
	assert(obj);

	// Compute the total size (in bytes) of the object.
	size_t size = 0;
	rbtree_foreach(&obj->tree, node) {
		co_sub_t *sub = structof(node, co_sub_t, node);
		co_unsigned16_t type = co_sub_get_type(sub);
		size = ALIGN(size, co_type_alignof(type));
		size += co_type_sizeof(type);
	}

	void *val = NULL;
	if (size) {
		val = calloc(1, size);
		if (__unlikely(!val)) {
			set_errno(errno);
			return;
		}
	}

	// Initialize the values of the sub-objects.
	size = 0;
	rbtree_foreach(&obj->tree, node) {
		co_sub_t *sub = structof(node, co_sub_t, node);
		co_unsigned16_t type = co_sub_get_type(sub);
		// Compute the offset of the sub-object.
		size = ALIGN(size, co_type_alignof(type));
		// Move the old value, if it exists.
		void *src = sub->val;
		sub->val = (char *)val + size;
		if (src)
			co_val_move(type, sub->val, src);
		size += co_type_sizeof(type);
	}

	free(obj->val);
	obj->val = val;
	obj->size = size;
}

static void
co_obj_clear(co_obj_t *obj)
{
	assert(obj);

	rbtree_foreach(&obj->tree, node)
		co_sub_destroy(structof(node, co_sub_t, node));

	free(obj->val);
	obj->val = NULL;
}

static co_unsigned32_t
default_sub_dn_ind(co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	__unused_var(data);

	return co_sub_on_dn(sub, req);
}

static co_unsigned32_t
default_sub_up_ind(const co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	__unused_var(data);

	return co_sub_on_up(sub, req);
}

