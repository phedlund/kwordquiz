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

#include "qaview.h"

#ifdef Q_WS_X11
#include <QtDBus/QDBusInterface>
#endif
#include <QtCore/QDir>

//#include <KIconLoader>
//#include <KLocale>
#include "wqnotification.h"
#include <QtCore/QUrl>

#include "prefs.h"
#include "kwqquizmodel.h"
#include "kwqscorewidget.h"
#include "wqmackeyboard.h"
#include "wqwinkeyboard.h"

QString highlightError(const QString & c, const QString & e)
{
  if (c == e)
    return c;

  QString s = c;
  if (s.left(4) == "<qt>" && e.left(4) != "<qt>")
      s = s.mid(4, s.length() - 9);

  if (s == e)
    return s;

  QString result = "<qt>";
  int i = 0;
  while (i < e.length() && s[i] == e[i])
    result.append(e[i++]);
  result.append("<b>");
  QString result2 = "</qt>";
  int j = s.length() - 1;
  int k = e.length() - 1;
  while (j >= 0 && k >= 0 && s[j] == e[k])
  {
    result2.prepend(e[k]);
    j--;
    k--;
  }
  result2.prepend("</b>");

  for (int m = i; m <= k; m++)
    result.append(e[m]);

  result.append(result2);
  return result;
}

QAView::QAView(QWidget *parent/*, KActionCollection * actionCollection*/) : KWQQuizView(parent/*, actionCollection*/)
{
  setupUi(this);

  connect(txtAnswer, SIGNAL(returnPressed()), this, SLOT(slotCheck()));
}

void QAView::init()
{
  score->clear();
  score->setQuestionCount(m_quiz->questionCount());
  score->setAsPercent(Prefs::percent());

  m_hintUsed = false;

  QFont f = Prefs::editorFont();
  f.setWeight(QFont::Normal);
  lblQuestion->setFont(f);
  lblAnswerBlank->setFont(f);
  txtAnswer->setFont(f);
  lblPreviousQuestion->setFont(f);
  lblYourAnswer->setFont(f);
  lblCorrect->setFont(f);

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
    if (a->objectName() == "qaHint")
       a->setEnabled(true);
    if (a->objectName() == "quizRepeatErrors")
       a->setEnabled(false);
    if (a->objectName() == "quizExportErrors")
       a->setEnabled(false);
    if (a->objectName() == "quizPlayAudio")
       a->setEnabled(false);
    if (a->objectName() == "qaMarkLastCorrect")
       a->setEnabled(false);
  }

  showQuestion();
  txtAnswer->show();
  txtAnswer->setFocus();
}

void QAView::slotCheck()
{
  if (actions()[0]->isEnabled()/*quizCheck*/)
  {
    bool fIsCorrect;

    if (m_hintUsed && Prefs::hintError())
    {
      //Force an Error
      fIsCorrect = m_quiz->checkAnswer("");
    }
    else
    {
      //Really check the answer
      fIsCorrect = m_quiz->checkAnswer(txtAnswer->text());
    }
    //Reset for next question
    m_hintUsed = false;

    if (fIsCorrect)
    {
      picYourAnswer->setPixmap(QPixmap(":/kwordquiz/src/pics/hi32-action-answer-correct.png"));
      lblYourAnswer->setText(m_quiz->yourAnswer(txtAnswer->text()));
      lblCorrectHeader->clear();
      picCorrectAnswer->clear();
      lblCorrect->clear();
      score->countIncrement(KWQScoreWidget::cdCorrect);
      WQNotification::event("QuizCorrect", tr("Your answer was correct!"));
      foreach(QAction * a, actions()) {
        if (a->objectName() == "qaMarkLastCorrect")
           a->setEnabled(false);
      }
    }
    else
    {
      picYourAnswer->setPixmap(QPixmap(":/kwordquiz/src/pics/hi32-action-error.png"));
      lblYourAnswer->setText(highlightError(m_quiz->answer(), m_quiz->yourAnswer(txtAnswer->text())));
      lblCorrect->setText(m_quiz->answer());
      picCorrectAnswer->setPixmap(QPixmap(":/kwordquiz/src/pics/hi32-action-answer-correct.png"));
      lblCorrectHeader->setText(tr("Correct Answer"));
      score->countIncrement(KWQScoreWidget::cdError);
      WQNotification::event("QuizError", tr("Your answer was incorrect."));
      foreach(QAction * a, actions()) {
        if (a->objectName() == "qaMarkLastCorrect")
           a->setEnabled(true);
      }
    }

    lblPreviousQuestionHeader->setText(tr("Previous Question"));
    audioPlayAnswer();

    lblPreviousQuestion->setText(m_quiz->question());
    //lblPreviousQuestion->setFont(m_quiz->fontQuestion());
    picPrevious->setPixmap(QPixmap(":/kwordquiz/src/pics/hi32-action-question.png"));

    lblYourAnswerHeader->setText(tr("Your Answer"));

    m_quiz->toNext();
    if (!m_quiz->atEnd())
    {
      showQuestion();
    }
    else
    {
      m_quiz->finish();
      //qtport m_actionCollection->action("qa_mark_last_correct")->setEnabled(false);
      
      foreach(QAction * a, actions()) {
        if (a->objectName() == "quizCheck")
          a->setEnabled(false);
        if (a->objectName() == "qaHint")
          a->setEnabled(false);
        if (a->objectName() == "quizRepeatErrors")
          a->setEnabled(m_quiz->hasErrors());
        if (a->objectName() == "quizExportErrors")
          a->setEnabled(m_quiz->hasErrors());
        if (a->objectName() == "quizPlayAudio")
           a->setEnabled(false);
      }

      lblQuestionLanguage->setText(tr("Summary"));
      lblQuestion->clear();
      lblAnswerLanguage->clear();
      lblAnswerBlank->hide();
      txtAnswer->hide();
      picQuestion->setPixmap(QPixmap(":/kwordquiz/src/icons/hi32-app-kwordquiz.png"));
      picAnswer->clear();
    }
  }
}

void QAView::slotHint()
{
  QString answer = txtAnswer->text();
  QString correctAnswer = m_quiz->hint();
  if (correctAnswer.left(4) == "<qt>")
  {
    correctAnswer = correctAnswer.remove("<qt>");
    correctAnswer = correctAnswer.remove("</qt>");
  }
  int minLength = qMin(answer.length(), correctAnswer.length());

  int correctCharCount = 1;

  if (answer.length() > 0)
  {
    for(int i = 0; i < minLength; i++)
    {
    if (answer.at(i) == correctAnswer.at(i))
      correctCharCount++;
    }
  }

  txtAnswer->setText(correctAnswer.left(correctCharCount));
  txtAnswer->setFocus();
  txtAnswer->setCursorPosition(txtAnswer->text().length());

  m_hintUsed = true;
}

/*!
    \fn QAView::showQuestion()
 */
void QAView::showQuestion()
{
  lblQuestionLanguage->setText(m_quiz ->langQuestion());
  lblQuestion->setText(m_quiz ->question());
  //audioPlayQuestion();

  picQuestion->setPixmap(QPixmap(QString(":/kwordquiz/src/pics/hi32-action-%1.png").arg(m_quiz->quizIcon(KWQQuizModel::IconLeftCol))));

  lblAnswerLanguage->setText(m_quiz ->langAnswer());

  if (!QString(m_quiz->blankAnswer()).isEmpty())
  {
    lblAnswerBlank->show();
    lblAnswerBlank->setText(m_quiz->blankAnswer());
  }
  else
    lblAnswerBlank->hide();

  txtAnswer->setText("");

  picAnswer->setPixmap(QPixmap(QString(":/kwordquiz/src/pics/hi32-action-%1.png").arg(m_quiz->quizIcon(KWQQuizModel::IconRightCol))));

  QString layout = m_quiz->kbAnswer();
#ifdef Q_WS_X11
  if (!layout.isEmpty()) {
    QDBusInterface kxkb("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts");
    if (kxkb.isValid())
      kxkb.call("setLayout", layout);
  }
#endif
#ifdef Q_WS_MAC
  WQMacKeyboard::selectLayout(layout);
#endif
#ifdef Q_WS_WIN
  WQWinKeyboard::selectLayout(layout);
#endif
}

void QAView::slotApplySettings( )
{
  score->setAsPercent(Prefs::percent());
}

void QAView::slotSpecChar( const QChar & c)
{
  if (txtAnswer->hasFocus())
  {
    if (txtAnswer->hasSelectedText())
    {
      QString ls = txtAnswer->text();
      QString s = txtAnswer->selectedText();
      int len = s.length();
      int ss = txtAnswer->selectionStart();
      ls = ls.replace(ss, len, c);
      txtAnswer->setText(ls);
      txtAnswer->setSelection(ss, 1);
    }
    else
    {
      QString s = txtAnswer->text();
      int i = txtAnswer->cursorPosition();
      s.insert(i, c);
      txtAnswer->setText(s);
      txtAnswer->setCursorPosition(i + 1);
    }
  }
}

void QAView::slotMarkLastCorrect( )
{
  m_quiz->errorList().removeLast();
  score->swapCount();
  foreach(QAction * a, actions()) {
    if (a->objectName() == "qaMarkLastCorrect")
       a->setEnabled(false);
  }
}

//#include "qaview.moc"
