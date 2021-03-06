/***************************************************************************
                               kwqtablemodel.h
                             -------------------

    begin                : Mon Feb 27 18:27:30 PST 2006
    copyright            : (C) 2006-2010 by Peter Hedlund
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

#ifndef KWQTABLEMODEL_H
#define KWQTABLEMODEL_H

#include <QAbstractTableModel>

#include <QPixmapCache>

#include <keduvocdocument.h>

/**
  @author Peter Hedlund <peter.hedlund@kdemail.net>
*/

const char delim_start = '[';
const char delim_end = ']';

class KWQTableModel : public QAbstractTableModel
{
Q_OBJECT
public:
  enum roles {
    KeyboardLayoutRole = Qt::UserRole,
    ImageRole,
    SoundRole
  };

  KWQTableModel(QObject * parent = 0);
  ~KWQTableModel();

  void setDocument(KEduVocDocument * doc);
  KEduVocDocument * document() { return m_doc;};
  bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());

  KEduVocLesson * currentLesson(int row);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags (const QModelIndex & index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  bool setHeaderData (int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole);

  bool isEmpty();
  bool checkSyntax() const;
  bool checkBlanksSyntax(const QString & text) const;

private:
  KEduVocDocument * m_doc;
  QPixmapCache *m_decorationCache;

  friend class WordQuizApp;
};

#endif
