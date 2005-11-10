/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/*
 *  2005-10-21 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to SKK IMEngine.
 */

#include "scimskksettingplugin.h"

#include "skk.h"
#include "skkui.h"

#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimSKKSettingPlugin> ScimSKKSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_skk, 
    ScimSKKSettingLoaderFactory( "kcm_skimplugin_scim_skk" ) )

class ScimSKKSettingPlugin::ScimSKKSettingPluginPrivate {
public:
    SKKSettingUI * ui;
};

ScimSKKSettingPlugin::ScimSKKSettingPlugin(QWidget *parent, 
					   const char */*name*/,
					   const QStringList &args)
 : KAutoCModule( ScimSKKSettingLoaderFactory::instance(), 
     parent, args, SKKConfig::self() ),
   d(new ScimSKKSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-skk");
    d->ui = new SKKSettingUI(this);
    setMainWidget(d->ui);
}

ScimSKKSettingPlugin::~ScimSKKSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-skk");
}


#include "scimskksettingplugin.moc"
