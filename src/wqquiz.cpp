/* This file is part of KWordQuiz
  Copyright (C) 2003 Peter Hedlund <peter.hedlund@kdemail.net>

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

#include <krandomsequence.h>

#include <QRegExp>

#include "leitnersystem.h"
#include "kwordquiz.h"
#include "wqquiz.h"
#include "prefs.h"

WQQuiz::WQQuiz(QWidget * parent, KEduVocDocument * doc) : QObject(parent)
{
  m_app = parent;
  m_doc = doc;

  m_list.clear();
  m_errorList.clear();
  m_quizList.clear();
}

void WQQuiz::activateErrorList()
{
  m_list.clear();
  foreach(WQListItem l, m_errorList)
    m_list.append(l);

  m_errorList.clear();
  m_questionCount = m_list.count();
}

void WQQuiz::activateBaseList()
{
  m_list.clear();

  if (m_quizMode > 2)
  {
    KRandomSequence *rs = new KRandomSequence(0);
    rs->randomize(m_quizList);
  };

  foreach(WQListItem l, m_quizList)
    m_list.append(l);

  m_questionCount = m_list.count();
}

void WQQuiz::addToList(int aCol, int /*bCol*/)
{
  //build a list of row numbers containing text in both columns

  QList<int> tempList;
  for (int current = 0; current < m_doc->numEntries(); ++current)
  {
    if (!m_doc->entry(current)->original().isEmpty() && !m_doc->entry(current)->translation(1).isEmpty())
    {
      tempList.append(current);
    }
  }

  KRandomSequence rs(0);

  int count = tempList.count();

  foreach(int i, tempList)
  {
    WQListItem li;
    li.setQuestion(aCol);
    li.setCorrect(1);
    li.setOneOp(i);

    if (count > 2)
    {
      int a, b;
      do
        a = rs.getLong(count);
      while(a==i);

      li.setTwoOp(a);

      do
        b = rs.getLong(count);
      while(b == i || b == a);

      li.setThreeOp(b);
    }
    m_quizList.append(li);
  }
}

bool WQQuiz::init()
{

  bool result = false;
  if (Prefs::enableBlanks())
  {
    KWordQuizApp *theApp=(KWordQuizApp *) m_app;
    result = theApp->checkSyntax(true);
  }
  else
  {
    result = true;
  }

  if (!result)
  {
    return false;
  }

  int aCol;
  int bCol;

  switch (m_quizMode)
  {
  case 1:
    aCol = 0;
    bCol = 1;
    break;
  case 2:
    aCol = 1;
    bCol = 0;
    break;
  case 3:
    aCol = 0;
    bCol = 1;
    break;
  case 4:
    aCol = 1;
    bCol = 0;
    break;
  case 5:
  default:
    aCol = 0;
    bCol = 1;
    break;
  }

  addToList(aCol, bCol);

  //check if enough in list
  switch (m_quizType)
  {
  case qtEditor:
    //
    break;
  case qtFlash:
    result = (m_quizList.count() > 0);
    break;
  case qtQA:
    result = (m_quizList.count() > 0);
    break;
  case qtMultiple:
    result = (m_quizList.count() > 2);
    break;
  }

  if (!result)
  {
    return false;
  }

  if (m_quizMode == 5)
  {
    aCol = 1;
    bCol = 0;
    addToList(aCol, bCol);
  }

  //Prepare final lists
  activateBaseList();
  return true;
}

bool WQQuiz::checkAnswer(int i, const QString & a)
{
  bool result = false;
  WQListItem li = m_list.at(i);
  QString ans = a;
  QString tTemp;
  if (li.question() == 0)
  {
    tTemp = m_doc->entry(li.oneOp())->translation(1);
  }
  else
  {
    tTemp = m_doc->entry(li.oneOp())->original();
  }
  tTemp = tTemp.simplified();
  ans = ans.simplified();

  if (m_quizType == qtQA)
  {
    if (QString(m_correctBlank).length() > 0)
    {
      QStringList la, ls;
      if (ans.indexOf(";") > 0)
        ls = ans.split(";", QString::SkipEmptyParts);
      else
        ls.append(ans);

      if (m_correctBlank.indexOf(";") > 0)
        la = m_correctBlank.split(";", QString::SkipEmptyParts);
      else
        la.append(m_correctBlank);

      result = (ls.count() == la.count());
      if (result)
      {
        for (int counter = 0; counter < la.count(); counter++)
        {
          result = (ls[counter].simplified() == la[counter].simplified());
          if (!result)
            break;
        }
      }
    }
    else
    {
      result = (ans == tTemp);
    }
  }
  else
  {
    if (m_quizType == qtMultiple)
    {
      if (Prefs::enableBlanks())
      {
        tTemp.remove("[");
        tTemp.remove("]");
      }
      result = (ans == tTemp);

    }
    else
    {
      result = (ans == tTemp);
    }

  }
  ///@todo currently the Leitner stuff crashes KWQ
  //QString tmpLeitner( m_doc->entry(li.oneOp())->leitnerBox() );

  if (!result)
  {
    //m_doc->entry(li.oneOp())->setLeitnerBox(m_doc->leitnerSystem()->wrongBox( tmpLeitner ));
    m_errorList.append(li);
  }
  //else
    //m_doc->entry(li.oneOp())->setLeitnerBox(m_doc->leitnerSystem()->correctBox( tmpLeitner ));

  return result;
}

QStringList WQQuiz::multiOptions(int i)
{
  QString s;
  QStringList Result;
  QStringList ls;

  WQListItem li = m_list.at(i);

  int j;
  if (li.question() == 0)
  {
    j = 1;
  }
  else
  {
    j= 0;
  }

  s = (j ?  m_doc->entry(li.oneOp())->translation(1) :  m_doc->entry(li.oneOp())->original());
  if (Prefs::enableBlanks())
  {
    s.remove("[");
    s.remove("]");
  }
  ls.append(s);

  s =(j ? m_doc->entry(li.twoOp())->translation(1) : m_doc->entry(li.twoOp())->original());
  if (Prefs::enableBlanks())
  {
    s.remove("[");
    s.remove("]");
  }
  ls.append(s);

  s = (j ? m_doc->entry(li.threeOp())->translation(1) : m_doc->entry(li.threeOp())->original());
  if (Prefs::enableBlanks())
  {
    s.remove("[");
    s.remove("]");
  }
  ls.append(s);

  KRandomSequence rs(0);
  rs.randomize(ls);

  foreach(QString s, ls)
    Result.append(s);

  return Result;
}

QString WQQuiz::quizIcon(int i, QuizIcon ico)
{
  QString s;
  WQListItem li = m_list.at(i);
  if (ico == qiLeftCol)
  {
    if (li.question() == 0)
      s = "question";
    else
      s = "answer";
  }

  if (ico == qiRightCol)
  {
    if (li.question() == 0)
      s = "answer";
    else
      s = "question";
  }
  return s;
}

QString WQQuiz::yourAnswer(int /*i*/, const QString & s)
{
  QString result ="";

  if (QString(m_answerBlank).length() > 0)
  {
    QStringList ls;

    if (s.indexOf(";") > 0)
      ls = s.split(";");
    else
      ls.append(s);

    result = m_answerBlank.replace("..........", "<u></u>");

    int offset = 0, counter = 0;
    while (offset >= 0)
    {
      offset = result.indexOf("<u>", offset);
      if (offset >= 0)
      {
        result.insert(offset + 3, ls[counter]);
        offset++;
        counter++;
      }
    }
    result.append("</qt>");
    result.prepend("<qt>");
  }
  else
  {
    result = s;
  }
  return result;
}

QString WQQuiz::hint(int i)
{
  if (QString(m_correctBlank).length() > 0)
  {
    return m_correctBlank;
  }
  else
  {
    return answer(i);
  }
}

void WQQuiz::setQuizType(QuizType qt)
{
  m_quizType = qt;
}

void WQQuiz::setQuizMode(int qm)
{
  m_quizMode = qm;
}


QString WQQuiz::question(int i)
{
  WQListItem li = m_list.at(i);
  QString s = (li.question() ? m_doc->entry(li.oneOp())->translation(1) : m_doc->entry(li.oneOp())->original());
  if (Prefs::enableBlanks())
  {
    s.remove("[");
    s.remove("]");
  }
  if (m_quizType != qtFlash && i > 0)
  {
    WQListItem li2 = m_list.at(i - 1);
    emit checkingAnswer(li2.oneOp());
  }
  else
    emit checkingAnswer(li.oneOp());

  return s;
}

QString WQQuiz::blankAnswer(int i)
{

  QString r = "";
  m_correctBlank = "";
  m_answerBlank = "";
  QString tTemp;

  if (m_quizType == qtQA && Prefs::enableBlanks())
  {
    WQListItem li = m_list.at(i);
    tTemp = (li.question() ? m_doc->entry(li.oneOp())->original() : m_doc->entry(li.oneOp())->translation(1));
    r = tTemp;
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("\\[.*\\]");

    r.replace(rx, "..........");

    if (r != tTemp)
    {
      m_answerBlank = r;
      int offset = 0;
      while (offset >= 0)
      {
        offset = rx.indexIn(tTemp, offset);
        if (offset >= 0)
        {
          if (m_correctBlank.length() > 0)
            m_correctBlank = m_correctBlank + ";" + " " + tTemp.mid(offset + 1, tTemp.indexOf(']', offset) - offset - 1);
          else
            m_correctBlank = tTemp.mid(offset + 1, tTemp.indexOf(']', offset) - offset - 1);
          offset++;
        }
      }
    }
  }
  return m_answerBlank;
}

QString WQQuiz::answer(int i)
{
  QString s;
  WQListItem li = m_list.at(i);

  if (m_quizType == qtQA)
  {
    s = (li.question() ? m_doc->entry(li.oneOp())->original() : m_doc->entry(li.oneOp())->translation(1));
    if (Prefs::enableBlanks())
    {
      s.replace("[", "<u>");
      s.replace("]", "</u>");
      s.prepend("<qt>");
      s.append("</qt>");
    }
  }
  else
  {
    s = (li.question() ? m_doc->entry(li.oneOp())->original() : m_doc->entry(li.oneOp())->translation(1));
    if (Prefs::enableBlanks())
    {
      s.remove("[");
      s.remove("]");
    }
  }
  return s;
}

QString WQQuiz::langQuestion(int i)
{
  WQListItem li = m_list.at(i);
  if (li.question() == 0)
    return m_doc->originalIdentifier();
  else
    return m_doc->identifier(1);
}

QString WQQuiz::langAnswer(int i)
{

  WQListItem li = m_list.at(i);
  if (li.question() == 1)
    return m_doc->originalIdentifier();
  else
    return m_doc->identifier(1);
}

int WQQuiz::kbAnswer(int /*i*/)
{
/*  WQListItem *li = m_list->at(i);
  if (li->question() == 0)
  {
    //@todo return m_table ->layoutLeft();
  }
  else
  {
    //@todo return m_table -> layoutRight();
  }*/
  return 0;
}

int WQQuiz::questionCount()
{
  return m_questionCount;
}

void WQQuiz::finish()
{
  emit checkingAnswer(-1);
}

#include "wqquiz.moc"
