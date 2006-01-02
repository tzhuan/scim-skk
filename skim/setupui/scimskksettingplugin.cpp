// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#include <iostream>

#include <qlistview.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimSKKSettingPlugin> ScimSKKSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY(kcm_skimplugin_scim_skk, 
			   ScimSKKSettingLoaderFactory("kcm_skimplugin_scim_skk"))

class ScimSKKSettingPlugin::ScimSKKSettingPluginPrivate {
public:
    SKKSettingUI * ui;
public:
    void setup_sysdict_view ()
    {
        ui->SystemDictionaryListView->clear ();
        ui->SystemDictionaryListView->setSorting (-1);

        QStringList dict_list = QStringList::split (",", SKKConfig::_IMEngine_SKK_SysDict());
        QStringList::iterator it;
        QListViewItem *item = NULL;

        for (it = dict_list.begin (); it != dict_list.end (); it++) {
            int p = (*it).find(':');
            QString type = p <= 0 ?
                "DictFile" : (*it).left (p);
            QString dict = p < 0 ?
                *it : (*it).right ((*it).length () - p - 1);

            item = new QListViewItem (
                ui->SystemDictionaryListView, item, type, dict);
        }
    }

    QString sysdict_list ()
    {
        QString str;
        QListViewItem *item = ui->SystemDictionaryListView->firstChild ();

        for (; item; item = item->nextSibling ()) {
            if (item != ui->SystemDictionaryListView->firstChild ())
                str += ",";
            str += item->text (0);
            str += ":";
            str += item->text (1);
        }

        return str;
    }

    bool is_changed ()
    {
        if (sysdict_list() == SKKConfig::_IMEngine_SKK_SysDict())
            return false;
        else
            return true;
    }
};

ScimSKKSettingPlugin::ScimSKKSettingPlugin(QWidget *parent, 
					   const char */*name*/,
					   const QStringList &args)
 : KAutoCModule (ScimSKKSettingLoaderFactory::instance(), 
		 parent, args, SKKConfig::self()),
   d (new ScimSKKSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-skk");
    d->ui = new SKKSettingUI(this);
    setMainWidget(d->ui);
    d->setup_sysdict_view ();
}

ScimSKKSettingPlugin::~ScimSKKSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-skk");
    delete d;
}

void ScimSKKSettingPlugin::load ()
{
    KAutoCModule::load ();

    d->setup_sysdict_view ();
}

void ScimSKKSettingPlugin::save ()
{
    KConfigSkeletonItem *tmp_item;
    tmp_item = SKKConfig::self()->findItem("_IMEngine_SKK_SysDict");
    if (tmp_item) {
        KConfigSkeletonGenericItem<QString> *item;
        item = dynamic_cast<KConfigSkeletonGenericItem<QString>*> (tmp_item);
        if (item)
            item->setValue (d->sysdict_list ());
    }

    KAutoCModule::save ();
}

void ScimSKKSettingPlugin::defaults ()
{
    KAutoCModule::defaults ();
}

void ScimSKKSettingPlugin::slotWidgetModified ()
{
    if (d->is_changed ())
        emit changed (true);
    else
        KAutoCModule::slotWidgetModified();
}

#include "scimskksettingplugin.moc"
