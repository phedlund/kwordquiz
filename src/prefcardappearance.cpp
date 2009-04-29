/***************************************************************************
                          prefcardappearance.cpp
                             -------------------

    copyright            : (C) 2004-2009 by Peter Hedlund
    email                : peter.hedlund@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "prefcardappearance.h"

#include <KFontRequester>
#include <KColorButton>
#include <KLocale>

PrefCardAppearance::PrefCardAppearance(QWidget *parent) : QWidget(parent)
{
  setupUi(this);

  connect(flipButton, SIGNAL(clicked()), this, SLOT(slotFlipButtonClicked()));
  connect(kcfg_FrontFont, SIGNAL(fontSelected(const QFont&)), this, SLOT(slotFontChanged(const QFont&)));
  connect(kcfg_FrontTextColor, SIGNAL(changed(const QColor&)), this, SLOT(slotTextColorChanged(const QColor&)));
  connect(kcfg_FrontCardColor, SIGNAL(changed(const QColor&)), this, SLOT(slotCardColorChanged(const QColor&)));
  connect(kcfg_FrontFrameColor, SIGNAL(changed(const QColor&)), this, SLOT(slotFrameColorChanged(const QColor&)));
  connect(kcfg_BackFont, SIGNAL(fontSelected(const QFont&)), this, SLOT(slotFontChanged(const QFont&)));
  connect(kcfg_BackTextColor, SIGNAL(changed(const QColor&)), this, SLOT(slotTextColorChanged(const QColor&)));
  connect(kcfg_BackCardColor, SIGNAL(changed(const QColor&)), this, SLOT(slotCardColorChanged(const QColor&)));
  connect(kcfg_BackFrameColor, SIGNAL(changed(const QColor&)), this, SLOT(slotFrameColorChanged(const QColor&)));
  connect(widgetStack, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));

  widgetStack->setCurrentWidget(frontStackPage);
  // initialize the buddies, etc.
  slotCurrentChanged(widgetStack->indexOf(frontStackPage));
}

void PrefCardAppearance::slotFlipButtonClicked( )
{
  if (widgetStack->currentWidget() == frontStackPage)
  {
    widgetStack->setCurrentWidget(backStackPage);
  }
  else
  {
    widgetStack->setCurrentWidget(frontStackPage);
  }
}

void PrefCardAppearance::slotFontChanged(const QFont & font)
{
  flashcard->setTextFont(font);
}

void PrefCardAppearance::slotTextColorChanged( const QColor & color)
{
  flashcard->setTextColor(color);
}

void PrefCardAppearance::slotCardColorChanged( const QColor & color)
{
  flashcard->setCardColor(color);
}

void PrefCardAppearance::slotFrameColorChanged( const QColor & color)
{
  flashcard->setFrameColor(color);
}

void PrefCardAppearance::slotCurrentChanged(int index)
{
  if (index == widgetStack->indexOf(backStackPage))
  {
    flashcard->setIdentifier(i18nc("Back of the flashcard", "Back"));
    flashcard->setText(i18n("Answer"));
    flashcard->setTextFont(kcfg_BackFont->font());
    flashcard->setTextColor(kcfg_BackTextColor->color());
    flashcard->setCardColor(kcfg_BackCardColor->color());
    flashcard->setFrameColor(kcfg_BackFrameColor->color());

    fontLabel->setBuddy(kcfg_BackFont);
    textColorLabel->setBuddy(kcfg_BackTextColor);
    frameColorLabel->setBuddy(kcfg_BackFrameColor);
    cardColorLabel->setBuddy(kcfg_BackCardColor);
  }
  else
  {
    flashcard->setIdentifier(i18n("Front"));
    flashcard->setText(i18n("Question"));
    flashcard->setTextFont(kcfg_FrontFont->font());
    flashcard->setTextColor(kcfg_FrontTextColor->color());
    flashcard->setCardColor(kcfg_FrontCardColor->color());
    flashcard->setFrameColor(kcfg_FrontFrameColor->color());

    fontLabel->setBuddy(kcfg_FrontFont);
    textColorLabel->setBuddy(kcfg_FrontTextColor);
    frameColorLabel->setBuddy(kcfg_FrontFrameColor);
    cardColorLabel->setBuddy(kcfg_FrontCardColor);
  }
}

#include "prefcardappearance.moc"
