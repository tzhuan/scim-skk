// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/

/*
 *  2005-10-21 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to SKK IMEngine.
 */

#ifndef SCIMSKKSETTINGPLUGIN_H
#define SCIMSKKSETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimSKKSettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimSKKSettingPlugin(QWidget *parent,
			 const char *name,
			 const QStringList &args);

    ~ScimSKKSettingPlugin();

    void load     ();
    void save     ();
    void defaults ();

protected slots:
    void slotWidgetModified  ();

private:
    class ScimSKKSettingPluginPrivate;
    ScimSKKSettingPluginPrivate * d;
};

#endif
