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

#include <QtDBus/QDBusInterface>

#include <KIconLoader>
#include <KLocale>
#include <KNotification>
#include <KUrl>

#include "prefs.h"
#include "kwqquizmodel.h"
#include "kwqscorewidget.h"

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

QAView::QAView(QWidget *parent, KActionCollection * actionCollection) : KWQQuizView(parent, actionCollection)
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

  m_actionCollection->action("quiz_check")->setEnabled(true);
  m_actionCollection->action("qa_mark_last_correct")->setEnabled(false);
  m_actionCollection->action("qa_hint")->setEnabled(true);
  m_actionCollection->action("quiz_repeat_errors")->setEnabled(false);
  m_actionCollection->action("quiz_export_errors")->setEnabled(false);
  m_actionCollection->action("quiz_audio_play")->setEnabled(false);

  // reset last file
  audioPlayFile(KUrl(), true);

  showQuestion();
  txtAnswer->show();
  txtAnswer->setFocus();
}

void QAView::slotCheck()
{
  if (m_actionCollection->action("quiz_check")->isEnabled())
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
      picYourAnswer->setPixmap(KIconLoader::global()->loadIcon("answer-correct", KIconLoader::Panel));
      lblYourAnswer->setText(m_quiz->yourAnswer(txtAnswer->text()));
      lblCorrectHeader->clear();
      picCorrectAnswer->clear();
      lblCorrect->clear();
      score->countIncrement(KWQScoreWidget::cdCorrect);
      KNotification::event("QuizCorrect", i18n("Your answer was correct!"));
      m_actionCollection->action("qa_mark_last_correct")->setEnabled(false);
    }
    else
    {
      picYourAnswer->setPixmap(KIconLoader::global()->loadIcon("error", KIconLoader::Panel));
      lblYourAnswer->setText(highlightError(m_quiz->answer(), m_quiz->yourAnswer(txtAnswer->text())));
      lblCorrect->setText(m_quiz->answer());
      picCorrectAnswer->setPixmap(KIconLoader::global()->loadIcon("answer-correct", KIconLoader::Panel));
      lblCorrectHeader->setText(i18n("Correct Answer"));
      score->countIncrement(KWQScoreWidget::cdError);
      KNotification::event("QuizError", i18n("Your answer was incorrect."));
      m_actionCollection->action("qa_mark_last_correct")->setEnabled(true);
    }

    audioPlayAnswer();

    lblPreviousQuestionHeader->setText(i18n("Previous Question"));
    lblPreviousQuestion->setText(m_quiz->question());
    //lblPreviousQuestion->setFont(m_quiz->fontQuestion(m_question));
    picPrevious->setPixmap(KIconLoader::global()->loadIcon("question", KIconLoader::Panel));

    lblYourAnswerHeader->setText(i18n("Your Answer"));

    m_quiz->toNext();
    if (!m_quiz->atEnd())
    {
      showQuestion();
    }
    else
    {
      m_quiz->finish();
      m_actionCollection->action("quiz_check")->setEnabled(false);
      m_actionCollection->action("qa_hint")->setEnabled(false);
      m_actionCollection->action("quiz_repeat_errors")->setEnabled(m_quiz->hasErrors());
      m_actionCollection->action("quiz_export_errors")->setEnabled(m_quiz->hasErrors());
      m_actionCollection->action("qa_mark_last_correct")->setEnabled(false);

      lblQuestionLanguage->setText(i18n("Summary"));
      lblQuestion->clear();
      lblAnswerLanguage->clear();
      lblAnswerBlank->hide();
      txtAnswer->hide();
      picQuestion->setPixmap(KIconLoader::global()->loadIcon("kwordquiz", KIconLoader::Panel));
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
    \fn QAView::showQuestion(int i)
 */
void QAView::showQuestion()
{
  lblQuestionLanguage->setText(m_quiz ->langQuestion());
  lblQuestion->setText(m_quiz ->question());
  //audioPlayQuestion();

  picQuestion->setPixmap(KIconLoader::global()->loadIcon(m_quiz->quizIcon(KWQQuizModel::IconLeftCol), KIconLoader::Panel));

  lblAnswerLanguage->setText(m_quiz ->langAnswer());

  if (!QString(m_quiz->blankAnswer()).isEmpty())
  {
    lblAnswerBlank->show();
    lblAnswerBlank->setText(m_quiz->blankAnswer());
  }
  else
    lblAnswerBlank->hide();

  txtAnswer->setText("");

  picAnswer->setPixmap(KIconLoader::global()->loadIcon(m_quiz->quizIcon(KWQQuizModel::IconRightCol), KIconLoader::Panel));

  QString layout = m_quiz->kbAnswer();
  if (!layout.isEmpty()) {
    QDBusInterface kxkb("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts");
    if (kxkb.isValid())
      kxkb.call("setLayout", layout);
  }
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
  m_actionCollection->action("qa_mark_last_correct")->setEnabled(false);
}

#include "qaview.moc"
