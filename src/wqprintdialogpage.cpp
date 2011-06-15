/* This file is part of KWordQuiz
  Copyright (C) 2004-2008 Peter Hedlund <peter.hedlund@kdemail.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "wqprintdialogpage.h"

#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>

//#include <KLocale>
//#include <KDialog>

WQPrintDialogPage::WQPrintDialogPage(QWidget *parent) : QWidget(parent)
{
  setWindowTitle(tr("Vocabulary Options"));

  QGridLayout * l = new QGridLayout(this);
  l->setSpacing(6);
  l->setMargin(6);
  QSpacerItem * s = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  l->addItem(s, 1, 0, 1, 1);

  g = new QGroupBox(tr("Select Type of Printout"), this );
  QVBoxLayout * vboxLayout = new QVBoxLayout(g);
  vboxLayout->setSpacing(6);
  vboxLayout->setMargin(6);

  bg = new QButtonGroup(this);

  QRadioButton *rb = new QRadioButton(tr("Vocabulary &list"),g);
  rb->setWhatsThis(tr("Select to print the vocabulary as displayed in the editor"));
  vboxLayout->addWidget(rb);
  bg->addButton(rb, Prefs::EnumPrintStyle::List);

  rb = new QRadioButton(tr("Vocabulary e&xam"),g);
  rb->setWhatsThis(tr("Select to print the vocabulary as a vocabulary exam"));
  vboxLayout->addWidget(rb);
  bg->addButton(rb, Prefs::EnumPrintStyle::Exam);

  rb = new QRadioButton(tr("&Flashcards"),g);
  rb->setWhatsThis(tr("Select to print flashcards"));
  vboxLayout->addWidget(rb);
  bg->addButton(rb, Prefs::EnumPrintStyle::Flashcard);

  l->addWidget(g, 0, 0, 1, 1);
}

int WQPrintDialogPage::printStyle()
{
  return bg->checkedId();
}

void WQPrintDialogPage::setPrintStyle(int style)
{
  bg->button(style)->setChecked(true);
}

//#include "wqprintdialogpage.moc"
