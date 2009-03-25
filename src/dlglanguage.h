/***************************************************************************
                          dlglanguage.h  -  description
                             -------------------
   copyright       : (C) 2004-2009 Peter Hedlund <peter.hedlund@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGLANGUAGE_H
#define DLGLANGUAGE_H

#include <kdialog.h>

#include "ui_columndialogbase.h"
#include "kwqcommands.h"

/**
@author Peter Hedlund
*/

class KWQTableModel;

class DlgLanguage : public KDialog {
  Q_OBJECT
public:
  DlgLanguage(KWQTableModel *model, QWidget *parent = 0);

  ColumnDataList columnData();

private:
  Ui::ColumnDialogBase *dlgBase;
};

#endif
