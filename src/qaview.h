/* This file is part of KWordQuiz
  Copyright (C) 2003-2009 Peter Hedlund <peter.hedlund@kdemail.net>

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

#ifndef QAVIEW_H
#define QAVIEW_H

#include "ui_qaviewbase.h"

#include <KActionCollection>

class KWQQuiz;

/**
@author Peter Hedlund
*/
class QAView : public QWidget, public Ui::QAViewBase
{
Q_OBJECT
public:
    QAView(QWidget *parent, KActionCollection * actionCollection);

    void setQuiz(KWQQuiz *quiz);
    void init();

public slots:
    void slotCheck();
    void slotHint();
    void slotRepeat();
    void slotRestart();
    void slotApplySettings();
    void slotSpecChar(const QChar &);

private:
    KWQQuiz *m_quiz;
    KActionCollection *m_actionCollection;

    int m_question;
    int m_error;
    bool m_hintUsed;

    void showQuestion(int i);
};

#endif
