/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __SCIM_SKK_PREFS_H__
#define __SCIM_SKK_PREFS_H__

#define SCIM_SKK_CONFIG_KAKUTEI_KEY          "/IMEngine/SKK/Kakutei"
#define SCIM_SKK_CONFIG_KATAKANA_KEY         "/IMEngine/SKK/Katakana"
#define SCIM_SKK_CONFIG_HALF_KATAKANA_KEY    "/IMEngine/SKK/HalfKatakana"
#define SCIM_SKK_CONFIG_ASCII_KEY            "/IMEngine/SKK/ASCII"
#define SCIM_SKK_CONFIG_WIDE_ASCII_KEY       "/IMEngine/SKK/WideASCII"
#define SCIM_SKK_CONFIG_CONVERT_KEY          "/IMEngine/SKK/Convert"
#define SCIM_SKK_CONFIG_START_CONV_KEY       "/IMEngine/SKK/StartConv"
#define SCIM_SKK_CONFIG_CANCEL_KEY           "/IMEngine/SKK/Cancel"
#define SCIM_SKK_CONFIG_ASCII_CONVERT_KEY    "/IMEngine/SKK/ASCIIConvert"
#define SCIM_SKK_CONFIG_PREVCAND_KEY         "/IMEngine/SKK/PrevCand"
#define SCIM_SKK_CONFIG_BACKSPACE_KEY        "/IMEngine/SKK/BackSpace"
#define SCIM_SKK_CONFIG_DELETE_KEY           "/IMEngine/SKK/Delete"
#define SCIM_SKK_CONFIG_FORWARD_KEY          "/IMEngine/SKK/Forward"
#define SCIM_SKK_CONFIG_BACKWARD_KEY         "/IMEngine/SKK/Backward"

#define SCIM_SKK_CONFIG_SYSDICT          "/IMEngine/SKK/SysDict"
#define SCIM_SKK_CONFIG_USERDICT         "/IMEngine/SKK/UserDict"


/* default values */
#define SCIM_SKK_CONFIG_KAKUTEI_KEY_DEFAULT          "Control+j"
#define SCIM_SKK_CONFIG_KATAKANA_KEY_DEFAULT         "q"
#define SCIM_SKK_CONFIG_HALF_KATAKANA_KEY_DEFAULT    ""
#define SCIM_SKK_CONFIG_ASCII_KEY_DEFAULT            "l"
#define SCIM_SKK_CONFIG_WIDE_ASCII_KEY_DEFAULT       "Shift+L"
#define SCIM_SKK_CONFIG_CONVERT_KEY_DEFAULT          "space"
#define SCIM_SKK_CONFIG_START_CONV_KEY_DEFAULT       "Shift+Q"
#define SCIM_SKK_CONFIG_CANCEL_KEY_DEFAULT           "Control+g,Escape"
#define SCIM_SKK_CONFIG_ASCII_CONVERT_KEY_DEFAULT    "slash"
#define SCIM_SKK_CONFIG_PREVCAND_KEY_DEFAULT         "x"
#define SCIM_SKK_CONFIG_BACKSPACE_KEY_DEFAULT        "BackSpace,Control+h"
#define SCIM_SKK_CONFIG_DELETE_KEY_DEFAULT           "Delete,Control+d"
#define SCIM_SKK_CONFIG_FORWARD_KEY_DEFAULT          "Right,Control+f"
#define SCIM_SKK_CONFIG_BACKWARD_KEY_DEFAULT         "Left,Control+b"

#define SCIM_SKK_CONFIG_SYSDICT_DEFAULT          "/usr/share/skk/SKK-JISYO.L"
#define SCIM_SKK_CONFIG_USERDICT_DEFAULT         ".skk-scim-jisyo"
#endif
