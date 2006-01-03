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
#ifndef SCIMSKKADDDIC_H
#define SCIMSKKADDDIC_H

#include <kdialogbase.h>
#include <klocale.h>

class ScimSKKAddDictDialog : public KDialogBase
{
Q_OBJECT
public:
    ScimSKKAddDictDialog  (QWidget *parent = 0,
                           const char *name = 0);
    ~ScimSKKAddDictDialog ();

    QString get_dict_type ();
    QString get_dict_name ();

public slots:
};

#endif // SCIMSKKADDDICT_H
