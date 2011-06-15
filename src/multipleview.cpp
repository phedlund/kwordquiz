/*
    This file is part of KWordQuiz
    Copyright (C) 2003-2010 Peter Hedlund <peter.hedlund@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "multipleview.h"
#include <QtCore/QDir>

#include <QtGui/QButtonGroup>

//#include <KIconLoader>
//#include <KLocale>
#include <Phonon/MediaObject>
//#include <KActionCollection>

#include "kwqquizmodel.h"
#include "kwqscorewidget.h"
#include "prefs.h"

MultipleView::MultipleView(QWidget *parent/*, KActionCollection *actionCollection*/) : KWQQuizView(parent/*, actionCollection*/)
{
  setupUi(this);
  m_choicesButtons = new QButtonGroup(this);
  m_choicesButtons->addButton(opt1, 1);
  m_choicesButtons->addButton(opt2, 2);
  m_choicesButtons->addButton(opt3, 3);
  connect(m_choicesButtons, SIGNAL(buttonClicked(int )), this, SLOT(slotChoiceClicked(int )));
  m_choicesActions = new QActionGroup(this);
  connect(m_choicesActions, SIGNAL(triggered(QAction *)), this, SLOT(slotChoiceActionTriggered(QAction *)));
  foreach(QAction * a, actions()) {
    if (a->objectName() == "quiz_Opt1") {
      m_choicesActions->addAction(a);
      a->setData(1);
    }
    if (a->objectName() == "quiz_Opt2") {
      m_choicesActions->addAction(a);
      a->setData(2);
    }
    if (a->objectName() == "quiz_Opt3") {
      m_choicesActions->addAction(a);
      a->setData(3);
    }
  }
}

void MultipleView::init()
{
  m_choices.clear();

  score->clear();
  score->setQuestionCount(m_quiz->questionCount());
  score->setAsPercent(Prefs::percent());

  opt1->show();
  opt2->show();
  opt3->show();

  lblQuestion->setFont(Prefs::editorFont());
  lblPreviousQuestion->setFont(Prefs::editorFont());
  lblYourAnswer->setFont(Prefs::editorFont());
  lblCorrect->setFont(Prefs::editorFont());
  opt1->setFont(Prefs::editorFont());
  opt2->setFont(Prefs::editorFont());
  opt3->setFont(Prefs::editorFont());

  lblPreviousQuestionHeader->clear();
  lblPreviousQuestion->clear();
  lblYourAnswerHeader->clear();
  lblYourAnswer->clear();
  lblCorrectHeader->clear();
  lblCorrect->clear();

  picPrevious->clear();
  picYourAnswer->clear();
  picCorrectAnswer->clear();

  foreach(QAction * a, actions()) {
    if (a->objectName() == "quizCheck")
       a->setEnabled(true);
    if (a->objectName() == "quizRepeatErrors")
       a->setEnabled(false);
    if (a->objectName() == "quizExportErrors")
       a->setEnabled(false);
    if (a->objectName() == "quiz_Opt1")
       a->setEnabled(true);
    if (a->objectName() == "quiz_Opt2")
       a->setEnabled(true);
    if (a->objectName() == "quiz_Opt3")
       a->setEnabled(true);
    if (a->objectName() == "quizPlayAudio")
       a->setEnabled(false);
  }
  showQuestion();
}

void MultipleView::slotCheck()
{
  if (actions()[0]->isEnabled()/*quizCheck*/)
  {
    if (m_choicesButtons->checkedId() == -1)
        return;

    QString ans = m_choices[m_choicesButtons->checkedId() - 1];

    bool fIsCorrect = m_quiz->checkAnswer(ans);

    if (fIsCorrect)
    {
      picYourAnswer->setPixmap(QPixmap(":/kwordquiz/pics/ox32-action-answer-correct.png"));
      lblCorrectHeader->clear();
      picCorrectAnswer->clear();
      lblCorrect->clear();
      score->countIncrement(KWQScoreWidget::cdCorrect);
      //qtport KNotification::event("QuizCorrect", i18n("Your answer was correct!"));
      Phonon::MediaObject *notification = Phonon::createPlayer(Phonon::NotificationCategory,
        Phonon::MediaSource(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("scrbar.wav")));
      notification->play();
    }
    else
    {
      picYourAnswer->setPixmap(QPixmap(":/kwordquiz/pics/ox32-action-error.png"));
      lblCorrect->setText(m_quiz->answer());
      picCorrectAnswer->setPixmap(QPixmap(":/kwordquiz/pics/ox32-action-answer-correct.png"));
      lblCorrectHeader->setText(tr("Correct Answer"));
      score->countIncrement(KWQScoreWidget::cdError);
      //qtport KNotification::event("QuizError", i18n("Your answer was incorrect."));
      Phonon::MediaObject *notification = Phonon::createPlayer(Phonon::NotificationCategory,
        Phonon::MediaSource(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("cancel.wav")));
      notification->play();
    }

    lblPreviousQuestionHeader->setText(tr("Previous Question"));
    lblPreviousQuestion->setText(m_quiz->question());
    picPrevious->setPixmap(QPixmap(":/kwordquiz/pics/ox32-action-question.png"));

    lblYourAnswerHeader->setText(tr("Your Answer"));
    lblYourAnswer->setText(m_quiz->yourAnswer(ans));

    m_quiz->toNext();
    if (!m_quiz->atEnd())
    {
      showQuestion();
    }
    else
    {
      m_quiz->finish();
      foreach(QAction * a, actions()) {
        if (a->objectName() == "quizCheck")
          a->setEnabled(false);
        if (a->objectName() == "quizRepeatErrors")
          a->setEnabled(m_quiz->hasErrors());
        if (a->objectName() == "quizExportErrors")
          a->setEnabled(m_quiz->hasErrors());
        if (a->objectName() == "quiz_Opt1")
           a->setEnabled(false);
        if (a->objectName() == "quiz_Opt2")
           a->setEnabled(false);
        if (a->objectName() == "quiz_Opt3")
           a->setEnabled(false);
        if (a->objectName() == "quizPlayAudio")
           a->setEnabled(false);
      }

      lblQuestionLanguage->setText(tr("Summary"));
      lblQuestion->clear();
      lblAnswerLanguage->clear();
      opt1->hide();
      opt2->hide();
      opt3->hide();
      picQuestion->setPixmap(QPixmap(":/kwordquiz/icons/hi32-app-kwordquiz.png"));
      picAnswer->clear();
    }
  }
}

void MultipleView::slotChoiceActionTriggered(QAction *choice)
{
  slotChoiceClicked(choice->data().toInt());
}

void MultipleView::slotChoiceClicked(int choice)
{
  m_choicesButtons->button(choice)->setChecked(true);
  if (Prefs::autoCheck())
    slotCheck();
}

/*!
    \fn MultipleView::showQuestion()
 */
void MultipleView::showQuestion()
{
  lblQuestionLanguage->setText(m_quiz->langQuestion());
  lblQuestion->setText(m_quiz->question());

  picQuestion->setPixmap(QPixmap(QString(":/kwordquiz/pics/ox32-action-%1.png").arg(m_quiz->quizIcon(KWQQuizModel::IconLeftCol))));

  lblAnswerLanguage->setText(m_quiz->langAnswer());

  m_choices = m_quiz->multiOptions();

#ifdef Q_WS_MAC //mnemonics are not used on the mac
  opt1->setText(m_choices[0]);
  opt2->setText(m_choices[1]);
  opt3->setText(m_choices[2]);
#else
  opt1->setText("&1 " + m_choices[0]);
  opt2->setText("&2 " + m_choices[1]);
  opt3->setText("&3 " + m_choices[2]);
#endif

  m_choicesButtons->setExclusive(false);
  opt1->setChecked(false);
  opt2->setChecked(false);
  opt3->setChecked(false);
  m_choicesButtons->setExclusive(true);
  setFocus();
  picAnswer->setPixmap(QPixmap(QString(":/kwordquiz/pics/ox32-action-%1.png").arg(m_quiz->quizIcon(KWQQuizModel::IconRightCol))));
}

void MultipleView::slotApplySettings()
{
  score->setAsPercent(Prefs::percent());
}

//#include "multipleview.moc"
