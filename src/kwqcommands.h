/***************************************************************************
                          kwqcommands.h  -  description
                             -------------------
    begin          : Fri Jan 18 10:37:00 PST 2008
    copyright      : (C) 2002-2008 Peter Hedlund <peter.hedlund@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHeaderView>
#include <QUndoCommand>

#include <KLocale>

#include "kwqtableview.h"
#include "kwqsortfiltermodel.h"
#include "prefs.h"

void copyToClipboard(QTableView * view);

struct IndexAndData
{
  QModelIndex index;
  QVariant data;
};

typedef QList<IndexAndData> IndexAndDataList;

class KWQUndoCommand : public QUndoCommand
{
public:
  KWQUndoCommand(KWQTableView *view);

  virtual void undo();
  virtual void redo() {};

  KWQTableView * view() {return m_view;};
  QModelIndex oldCurrentIndex() const {return m_currentIndex;};
  QModelIndexList oldSelectedIndexes() const {return m_selectedIndexes;};
  IndexAndDataList oldData() {return m_indexAndData;};

private:
  KWQTableView *m_view;
  IndexAndDataList m_indexAndData;
  QModelIndexList m_selectedIndexes;
  QModelIndex m_currentIndex;
};


class KWQCommandEdit : public KWQUndoCommand
{
public:

  enum EditCommand {
    EditCut,
    EditClear,
  };

   KWQCommandEdit(KWQTableView *view, EditCommand cmd);
   virtual void redo();
private:
  EditCommand m_command;
};


class KWQCommandPaste : public KWQUndoCommand
{
public:
  KWQCommandPaste(KWQTableView *view);
  virtual void undo();
  virtual void redo();
private:
  int m_rowCount;
  IndexAndDataList m_pasteIndexAndData;
};


class KWQCommandFont : public KWQUndoCommand
{
public:
  KWQCommandFont(KWQTableView *view, const QFont oldFont, const QFont newFont) : KWQUndoCommand(view), m_oldFont(oldFont), m_newFont(newFont)
    { setText(i18n("Font")); }
  virtual void undo()
    { Prefs::setEditorFont(m_oldFont);
      view()->reset(); }
  virtual void redo()
    { Prefs::setEditorFont(m_newFont);
      view()->reset(); }
private:
  QFont m_oldFont;
  QFont m_newFont;
};


class KWQCommandEntry : public KWQUndoCommand
{
public:
  KWQCommandEntry(KWQTableView *view, const QString oldText, const QString newText)
    : KWQUndoCommand(view), m_oldText(oldText), m_newText(newText) 
      { setText(i18n("Entry")); }

  virtual void redo()
      { view()->model()->setData(oldCurrentIndex(), m_newText, Qt::EditRole);
        view()->setCurrentIndex(oldCurrentIndex()); }
private:
  QString m_oldText;
  QString m_newText;
};


class KWQCommandSort : public QUndoCommand
{
public:
  KWQCommandSort(QTableView *view, int column);
  virtual void undo();
  virtual void redo();
private:
  QTableView *m_view;
  int m_column;
};

class KWQCommandShuffle : public KWQCommandSort
{
public:
  KWQCommandShuffle(QTableView *view, int column);
  virtual void undo();
  virtual void redo();
private:
  QTableView *m_view;
  int m_column;
};
