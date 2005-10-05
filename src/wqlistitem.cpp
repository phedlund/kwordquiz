/* This file is part of KWordQuiz
  Copyright (C) 2003 Peter Hedlund <peter@peterandlinda.com>

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

#include "wqlistitem.h"

WQListItem::WQListItem()
{
}


WQListItem::~WQListItem()
{
}


void WQListItem::setQuestion(int i)
{
  m_question = i;
}

void WQListItem::setCorrect(int c)
{
  m_correct = c;
}

void WQListItem::setOneOp(int i)
{
  m_oneOp = i;
}

void WQListItem::setTwoOp(int i)
{
  m_twoOp = i;
}

void WQListItem::setThreeOp(int i)
{
  m_threeOp = i;
}
