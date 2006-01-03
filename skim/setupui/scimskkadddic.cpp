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
    QFrame        *m_dict_file_frame;
    QFrame        *m_skk_serv_frame;
    KComboBox     *m_dict_type_combo;
    KURLRequester *m_dict_file_path;
    KLineEdit     *m_server_name;
    KLineEdit     *m_port_number;
};

ScimSKKAddDictDialog::ScimSKKAddDictDialog (QWidget *parent, const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
                   i18n ("Add a new dictionary"),
                   KDialogBase::Ok | KDialogBase::Cancel),
      d (new ScimSKKAddDictDialogPrivate)
{
    setMinimumWidth (280);
    setMinimumHeight (100);

    QVBoxLayout *main_vbox   = new QVBoxLayout (plainPage (),6);
    QHBoxLayout *dict_type_hbox  = new QHBoxLayout (main_vbox, 6);

    // types combo box
    QStringList types;
    types << "DictFile";
    types << "SKKServ";
    types << "CDBFile";

    QLabel *label = new QLabel (i18n ("Dictionary Type:"), plainPage ());
    d->m_dict_type_combo = new KComboBox (plainPage ());
    d->m_dict_type_combo->insertStringList (types);
    dict_type_hbox->addWidget (label);
    dict_type_hbox->addWidget (d->m_dict_type_combo);
    dict_type_hbox->addStretch (20);

    // DictFile frame
    d->m_dict_file_frame = new QFrame (plainPage ());
    d->m_dict_file_frame->setFrameStyle (QFrame::NoFrame);
    main_vbox->addWidget (d->m_dict_file_frame);

    QHBoxLayout *hbox = new QHBoxLayout (d->m_dict_file_frame, 0);

    label = new QLabel (i18n ("Path:"), d->m_dict_file_frame);
    d->m_dict_file_path = new KURLRequester (d->m_dict_file_frame);
    d->m_dict_file_path->setMode (KFile::File | KFile::LocalOnly);
    hbox->addWidget (label);
    hbox->addWidget (d->m_dict_file_path);

    // SKKServ frame
    d->m_skk_serv_frame = new QFrame (plainPage ());
    d->m_skk_serv_frame->setFrameStyle (QFrame::Box);
    d->m_skk_serv_frame->hide ();
    main_vbox->addWidget (d->m_skk_serv_frame);

    QVBoxLayout *vbox = new QVBoxLayout (d->m_skk_serv_frame, 6);
    QHBoxLayout *hbox1 = new QHBoxLayout (vbox, 0);
    QHBoxLayout *hbox2 = new QHBoxLayout (vbox, 0);

    label = new QLabel (i18n ("Server Name:"), d->m_skk_serv_frame);
    hbox1->addWidget (label);
    d->m_server_name = new KLineEdit (d->m_skk_serv_frame);
    d->m_server_name->setText ("localhost");
    hbox1->addWidget (d->m_server_name);

    label = new QLabel (i18n ("Port Number:"), d->m_skk_serv_frame);
    hbox2->addWidget (label);
    d->m_port_number = new KLineEdit (d->m_skk_serv_frame);
    d->m_port_number->setText ("1178");
    hbox2->addWidget (d->m_port_number);

    // connect to signals
    connect (d->m_dict_type_combo, SIGNAL (activated (const QString &)),
             this , SLOT (set_dict_type (const QString &)));
}

ScimSKKAddDictDialog::~ScimSKKAddDictDialog ()
{
    delete d;
}

void ScimSKKAddDictDialog::set_dict (QString &type, QString &name)
{
    set_dict_type (type);

    if (type == "SKKServ") {
    } else {
        d->m_dict_file_path->lineEdit()->setText (name);
    }
}

QString ScimSKKAddDictDialog::get_dict_type ()
{
    return d->m_dict_type_combo->currentText ();
}

QString ScimSKKAddDictDialog::get_dict_name ()
{
    if (d->m_dict_type_combo->currentText () == "SKKServ") {
        return d->m_server_name->text () + ":" + d->m_port_number->text ();
    } else {
        return d->m_dict_file_path->url ();
    }
}

void ScimSKKAddDictDialog::set_dict_type (const QString & type)
{
    if (type == "SKKServ") {
        d->m_dict_file_frame->hide ();
        d->m_skk_serv_frame->show ();
    } else {
        d->m_dict_file_frame->show ();
        d->m_skk_serv_frame->hide ();
   }
}
