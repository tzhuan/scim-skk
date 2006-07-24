/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4  -*- */
/*
 *  Copyright (C) 2004 Jun Mukai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __SCIM_SKK_CONFIG_H__
#define __SCIM_SKK_CONFIG_H__

#include <scim.h>
#include "scim_skk_automaton.h"

namespace scim_skk {
extern bool annot_view;   /* view annotation if true */
extern bool annot_pos;    /* inline if true, auxwindow if otherwise */
extern bool annot_target; /* all if true, caret position otherwise */
extern int candvec_size;

extern bool annot_highlight;
extern int annot_bgcolor;

extern bool ignore_return;

extern SKKAutomaton key2kana;
};

#endif /*  __SCIM_SKK_CONFIG_H__ */
