/***************************************************************************
                          dlglanguage.h  -  description
                             -------------------
   copyright       : (C) 2004-2005 Peter Hedlund <peter.hedlund@kdemail.net>

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

#include "dlglanguagebase.h"

class KCompletion;

/**
@author Peter Hedlund
*/
class DlgLanguage : public KDialog {
  Q_OBJECT
public:
  DlgLanguage(QWidget *parent = 0);
  ~DlgLanguage();

  void setLanguage(int index, const QString &lang);
  QString Language(int index);

private:
  Ui::DlgLanguageBase *dlgBase;
  KCompletion * completion1;
  KCompletion * completion2;

};

#endif
