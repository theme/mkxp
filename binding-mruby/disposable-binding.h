/*
** disposable-binding.h
**
** This file is part of mkxp.
**
** Copyright (C) 2013 Jonas Kulla <Nyocurio@gmail.com>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISPOSABLEBINDING_H
#define DISPOSABLEBINDING_H

#include "disposable.h"
#include "binding-util.h"

#include "mruby/array.h"

#include <string.h>

/* 'Children' are disposables that are disposed together
 * with their parent. Currently this is only used by Viewport
 * in RGSS1.
 * FIXME: Disable this behavior when RGSS2 or 3 */
inline void
disposableAddChild(mrb_state *mrb, mrb_value disp, mrb_value child)
{
	mrb_sym sym = getMrbData(mrb)->symbols[CSchildren];
	mrb_value children = mrb_iv_get(mrb, disp, sym);

	if (mrb_nil_p(children))
	{
		children = mrb_ary_new(mrb);
		mrb_iv_set(mrb, disp, sym, children);
	}

	/* Assumes children are never removed until destruction */
	mrb_ary_push(mrb, children, child);
}

inline void
disposableDisposeChildren(mrb_state *mrb, mrb_value disp)
{
	MrbData *mrbData = getMrbData(mrb);
	mrb_value children = mrb_iv_get(mrb, disp, mrbData->symbols[CSchildren]);

	if (mrb_nil_p(children))
		return;

	for (mrb_int i = 0; i < RARRAY_LEN(children); ++i)
		mrb_funcall_argv(mrb, mrb_ary_entry(children, i),
						 mrbData->symbols[CSdispose], 0, 0);
}

template<class C>
MRB_METHOD(disposableDispose)
{
	C *c = static_cast<C*>(DATA_PTR(self));

	/* Nothing to do if already disposed */
	if (!c)
		return mrb_nil_value();

	/* Inform core */
	c->wasDisposed();

	disposableDisposeChildren(mrb, self);

	delete c;
	DATA_PTR(self) = 0;

	return mrb_nil_value();
}

template<class C>
MRB_METHOD(disposableDisposed)
{
	MRB_UNUSED_PARAM;

	return mrb_bool_value(DATA_PTR(self) == 0);
}

template<class C>
static void disposableBindingInit(mrb_state *mrb, RClass *klass)
{
	mrb_define_method(mrb, klass, "dispose", disposableDispose<C>, MRB_ARGS_NONE());
	mrb_define_method(mrb, klass, "disposed?", disposableDisposed<C>, MRB_ARGS_NONE());
}

#endif // DISPOSABLEBINDING_H
