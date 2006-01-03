// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
/***************************************************************************
 *   Copyright (C) 2006 Takuro Ashie                                       *
 *   ashie@homa.ne.jp                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>
#include <qlayout.h>
#include "scimskkadddic.h"

ScimSKKAddDictDialog::ScimSKKAddDictDialog (QWidget *parent, const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
		   i18n ("Add new dictionary"),
		   KDialogBase::Ok | KDialogBase::Cancel)
{
}

ScimSKKAddDictDialog::~ScimSKKAddDictDialog ()
{
}

QString ScimSKKAddDictDialog::get_dict_type ()
{
  return "DictFile";
}

QString ScimSKKAddDictDialog::get_dict_name ()
{
  return "/hoge/huga";
}
