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
#define SCIM_SKK_CONFIG_START_PREEDIT_KEY    "/IMEngine/SKK/StartPreedit"
#define SCIM_SKK_CONFIG_CANCEL_KEY           "/IMEngine/SKK/Cancel"
#define SCIM_SKK_CONFIG_ASCII_CONVERT_KEY    "/IMEngine/SKK/ASCIIConvert"
#define SCIM_SKK_CONFIG_PREVCAND_KEY         "/IMEngine/SKK/PrevCand"
#define SCIM_SKK_CONFIG_BACKSPACE_KEY        "/IMEngine/SKK/BackSpace"
#define SCIM_SKK_CONFIG_DELETE_KEY           "/IMEngine/SKK/Delete"
#define SCIM_SKK_CONFIG_FORWARD_KEY          "/IMEngine/SKK/Forward"
#define SCIM_SKK_CONFIG_BACKWARD_KEY         "/IMEngine/SKK/Backward"
#define SCIM_SKK_CONFIG_HOME_KEY             "/IMEngine/SKK/Home"
#define SCIM_SKK_CONFIG_END_KEY              "/IMEngine/SKK/End"
#define SCIM_SKK_CONFIG_UPCASE_KEY           "/IMEngine/SKK/UPCASE"
#define SCIM_SKK_CONFIG_COMPLETION_KEY       "/IMEngine/SKK/Completion"
#define SCIM_SKK_CONFIG_COMPLETION_BACK_KEY  "/IMEngine/SKK/CompletionBack"
#define SCIM_SKK_CONFIG_SELECTION_STYLE      "/IMEngine/SKK/SelectionStyle"

#define SCIM_SKK_CONFIG_SYSDICT          "/IMEngine/SKK/SysDict"
#define SCIM_SKK_CONFIG_USERDICT         "/IMEngine/SKK/UserDict"

#define SCIM_SKK_CONFIG_CANDVEC_SIZE      "/IMEngine/SKK/DictListSize"

#define SCIM_SKK_CONFIG_ANNOT_VIEW     "/IMEngine/SKK/AnnotView"
#define SCIM_SKK_CONFIG_ANNOT_POS      "/IMEngine/SKK/AnnotPos"
#define SCIM_SKK_CONFIG_ANNOT_TARGET   "/IMEngine/SKK/AnnotTarget"

#define SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT "/IMEngine/SKK/AnnotHighlight"
#define SCIM_SKK_CONFIG_ANNOT_BGCOLOR   "/IMEngine/SKK/AnnotBGColor"

#define SCIM_SKK_CONFIG_IGNORE_RETURN   "/IMEngine/SKK/IgnoreReturn"

#define SCIM_SKK_CONFIG_STYLE_FILENAME  "/IMEngine/SKK/StyleFileName"

/* default values */
#define SCIM_SKK_CONFIG_KAKUTEI_KEY_DEFAULT          "Control+j"
#define SCIM_SKK_CONFIG_KATAKANA_KEY_DEFAULT         "q"
#define SCIM_SKK_CONFIG_HALF_KATAKANA_KEY_DEFAULT    ""
#define SCIM_SKK_CONFIG_ASCII_KEY_DEFAULT            "l"
#define SCIM_SKK_CONFIG_WIDE_ASCII_KEY_DEFAULT       "Shift+L"
#define SCIM_SKK_CONFIG_CONVERT_KEY_DEFAULT          "space"
#define SCIM_SKK_CONFIG_START_PREEDIT_KEY_DEFAULT    "Shift+Q"
#define SCIM_SKK_CONFIG_CANCEL_KEY_DEFAULT           "Control+g,Escape"
#define SCIM_SKK_CONFIG_ASCII_CONVERT_KEY_DEFAULT    "slash"
#define SCIM_SKK_CONFIG_PREVCAND_KEY_DEFAULT         "x"
#define SCIM_SKK_CONFIG_BACKSPACE_KEY_DEFAULT        "BackSpace,Control+h"
#define SCIM_SKK_CONFIG_DELETE_KEY_DEFAULT           "Delete,Control+d"
#define SCIM_SKK_CONFIG_FORWARD_KEY_DEFAULT          "Right,Control+f,Down"
#define SCIM_SKK_CONFIG_BACKWARD_KEY_DEFAULT         "Left,Control+b,Up"
#define SCIM_SKK_CONFIG_HOME_KEY_DEFAULT             "Home,Control+a"
#define SCIM_SKK_CONFIG_END_KEY_DEFAULT              "End,Control+e"
#define SCIM_SKK_CONFIG_UPCASE_KEY_DEFAULT           "Control+u"
#define SCIM_SKK_CONFIG_COMPLETION_KEY_DEFAULT       "Tab"
#define SCIM_SKK_CONFIG_COMPLETION_BACK_KEY_DEFAULT  "period"
#define SCIM_SKK_CONFIG_SELECTION_STYLE_DEFAULT      "Qwerty"

#define SCIM_SKK_CONFIG_SYSDICT_DEFAULT          "DictFile:/usr/share/skk/SKK-JISYO.L"
#define SCIM_SKK_CONFIG_USERDICT_DEFAULT         ".skk-scim-jisyo"
#define SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT      4
#define SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT    true
#define SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT     "AuxWindow"
#define SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT  "all"

#define SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT true
#define SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT   "#a0ff80"

#define SCIM_SKK_CONFIG_IGNORE_RETURN_DEFAULT   false
#endif
