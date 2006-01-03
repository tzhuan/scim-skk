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

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <klocale.h>

#include "scimskkadddic.h"

class ScimSKKAddDictDialog::ScimSKKAddDictDialogPrivate {
public:
    KComboBox     *m_dict_type_combo;
    KURLRequester *m_dict_file_path;
};

ScimSKKAddDictDialog::ScimSKKAddDictDialog (QWidget *parent, const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
                   i18n ("Add new dictionary"),
                   KDialogBase::Ok | KDialogBase::Cancel),
      d (new ScimSKKAddDictDialogPrivate)
{
    setMinimumWidth (280);
    setMinimumHeight (100);

    QVBoxLayout *main_vbox   = new QVBoxLayout (plainPage (),6);
    QHBoxLayout *dict_type_hbox  = new QHBoxLayout (main_vbox, 6);
    QHBoxLayout *editor_hbox = new QHBoxLayout (main_vbox, 6);

    QStringList types;
    types << "DictFile";
    types << "SKKServ";
    types << "CDBFile";

    QLabel *label = new QLabel (i18n ("Type:"), plainPage ());
    d->m_dict_type_combo = new KComboBox (plainPage ());
    d->m_dict_type_combo->insertStringList (types);
    dict_type_hbox->addWidget (label);
    dict_type_hbox->addWidget (d->m_dict_type_combo);
    dict_type_hbox->addStretch (20);

    label = new QLabel (i18n ("Path:"), plainPage ());
    d->m_dict_file_path = new KURLRequester (plainPage ());
    d->m_dict_file_path->setMode (KFile::File | KFile::LocalOnly);
    editor_hbox->addWidget (label);
    editor_hbox->addWidget (d->m_dict_file_path);
}

ScimSKKAddDictDialog::~ScimSKKAddDictDialog ()
{
    delete d;
}

void ScimSKKAddDictDialog::set_dict (QString &type, QString &name)
{
    if (type == "SKKServe") {
    } else if (type == "CDBFile") {
    } else {
    }
}

QString ScimSKKAddDictDialog::get_dict_type ()
{
    return d->m_dict_type_combo->currentText ();
}

QString ScimSKKAddDictDialog::get_dict_name ()
{
    return d->m_dict_file_path->url ();
}
