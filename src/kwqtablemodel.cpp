/***************************************************************************
                              kwqtablemodel.cpp
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

#include "kwqtablemodel.h"
#include <QFont>
#include <QVector>
#include <QSize>
#include <QDebug>
//#include <klocale.h>

#include "prefs.h"
#include "documentsettings.h"
#include "keduvocexpression.h"

KWQTableModel::KWQTableModel(QObject * parent) : QAbstractTableModel(parent)
{
  m_doc = 0;
  m_decorationCache = new QPixmapCache(/*"kwordquiz"*/);
}

KWQTableModel::~KWQTableModel()
{
  delete m_decorationCache;
}

int KWQTableModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);
  return m_doc->lesson()->entryCount(KEduVocLesson::Recursive);
}

int KWQTableModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant KWQTableModel::data(const QModelIndex & index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QPixmap ip;
  QString image;
  QUrl tempUrl;

  switch (role) {
    case Qt::FontRole:
      return QVariant(Prefs::editorFont());

    case Qt::DisplayRole:
      return m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->text();

    case Qt::DecorationRole:
      tempUrl = m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->imageUrl();

      if (!tempUrl.isEmpty()) {
#ifdef Q_WS_WIN
          image = m_doc->url().resolved(tempUrl).toString();
#else
          image = m_doc->url().resolved(tempUrl).toLocalFile();
#endif
          if (!image.isEmpty()) {
            if (!m_decorationCache->find(image, &ip)) {
              ip = QPixmap(image).scaled(QSize(22, 22), Qt::KeepAspectRatio);
              m_decorationCache->insert(image, ip);
            }
          }
          return ip;
      }
      else
        return QVariant();

    case KWQTableModel::ImageRole:
      tempUrl = m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->imageUrl();
      if (!tempUrl.isEmpty())
#ifdef Q_WS_WIN
          return m_doc->url().resolved(tempUrl).toString();
#else
          return m_doc->url().resolved(tempUrl).toLocalFile();
#endif
      else
        return QVariant();

    case KWQTableModel::SoundRole:
      tempUrl = m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->soundUrl();
      if (!tempUrl.isEmpty())
#ifdef Q_WS_WIN
          return m_doc->url().resolved(tempUrl).toString();
#else
          return m_doc->url().resolved(tempUrl).toLocalFile();
#endif
      else
        return QVariant();

    default:
      return QVariant();
  }
}

QVariant KWQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
  {
    if (section < 0 || section > 1)
      return QVariant();

    if (role == Qt::DisplayRole) {
      return m_doc->identifier(section).name();
    }

    if (role == Qt::SizeHintRole) {
      DocumentSettings documentSettings(m_doc->url().toLocalFile());
      documentSettings.readConfig();
      return QSize(documentSettings.sizeHintColumn(section), 25);
    }

    if (role == KWQTableModel::KeyboardLayoutRole) {
      DocumentSettings documentSettings(m_doc->url().toLocalFile());
      documentSettings.readConfig();
      return documentSettings.keyboardLayoutColumn(section);
    }

    return QVariant();
  }
  else
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags KWQTableModel::flags(const QModelIndex & index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool KWQTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if (!index.isValid())
    return false;

  switch (role) {
    case Qt::EditRole:
      m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->setTranslation(index.column(), value.toString());
      break;

    case KWQTableModel::ImageRole:
      m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->setImageUrl(value.toUrl());
      break;

    case KWQTableModel::SoundRole:
      m_doc->lesson()->entries(KEduVocLesson::Recursive).value(index.row())->translation(index.column())->setSoundUrl(value.toUrl());
      break;
  }

  emit dataChanged(index, index);
  return true;
}

bool KWQTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role)
{
  if (orientation == Qt::Horizontal) {
    if (section < 0 || section > 1)
      return false;

    if (role == Qt::EditRole) {
      m_doc->identifier(section).setName(value.toString());
    }

    if (role == Qt::SizeHintRole) {
      DocumentSettings documentSettings(m_doc->url().toLocalFile());
      documentSettings.setSizeHintColumn(section, qvariant_cast<QSize>(value).width());
      documentSettings.writeConfig();
    }

    if (role == KWQTableModel::KeyboardLayoutRole) {
      DocumentSettings documentSettings(m_doc->url().toLocalFile());
      documentSettings.setKeyboardLayoutColumn(section, value.toString());
      documentSettings.writeConfig();
    }

    emit headerDataChanged(orientation, section, section);
    return true;
  }
  return false;
}


bool KWQTableModel::insertRows(int row, int count, const QModelIndex & parent)
{
  Q_UNUSED(parent);
  if (count < 1 || row < 0 || row > m_doc->lesson()->entryCount(KEduVocLesson::Recursive))
    return false;

  KEduVocLesson * cl = currentLesson(row);

  beginInsertRows(QModelIndex(), row, row + count - 1);

  for (int i = row; i < row + count; i++)
    cl->insertEntry(i, new KEduVocExpression); //m_doc->lesson()->insertEntry(i, new KEduVocExpression);

  endInsertRows();
  m_doc->setModified(true);
  return true;
}

bool KWQTableModel::removeRows(int row, int count, const QModelIndex & parent)
{
  Q_UNUSED(parent);
  if (count < 1 || row < 0 || row + count > m_doc->lesson()->entryCount(KEduVocLesson::Recursive) || count >= m_doc->lesson()->entryCount(KEduVocLesson::Recursive))
    return false;

  int bottomRow = row + count -1;
  beginRemoveRows(QModelIndex(), row, row + count - 1);

  for (int i = bottomRow; i >= row; i--) {
    KEduVocExpression* entry = m_doc->lesson()->entries(KEduVocLesson::Recursive).value(i);
    entry->lesson()->removeEntry(entry);
    delete entry;
  }

  endRemoveRows();
  m_doc->setModified(true);
  return true;
}

void KWQTableModel::setDocument(KEduVocDocument * doc)
{
  m_doc = doc;
  reset();
}

bool KWQTableModel::isEmpty()
{
  if (m_doc->url().toString() == tr("Untitled")){
    int rc = rowCount(QModelIndex());
    for (int i = 0; i < rc; i++)
      if (!data(index(i, 0), Qt::DisplayRole).toString().isEmpty() || !data(index(i, 1), Qt::DisplayRole).toString().isEmpty())
        return false;

    return true;
  }
  else
    return false;
}


bool KWQTableModel::checkBlanksSyntax(const QString & text) const
{
  if (!Prefs::enableBlanks())
    return true;

  bool result = false;
  int openCount = 0;
  int closeCount = 0;
  QVector<int> openPos(0);
  QVector<int> closePos(0);

  for (int i = 0; i< text.length(); ++i)
  {
    if (text[i] == delim_start)
    {
      openCount++;
      openPos.resize(openCount);
      openPos[openCount - 1] = i;
    }

    if (text[i] == delim_end)
    {
      closeCount++;
      closePos.resize(closeCount);
      closePos[closeCount - 1] = i;
    }
  }

  if (openCount == 0 && closeCount == 0)
    return true;

  if (openCount > 0 && closeCount > 0)
    if (openPos.size() == closePos.size())
      for (int i = 0; i < openPos.size(); ++i)
        result = (openPos[i] < closePos[i]);

  return result;
}

bool KWQTableModel::checkSyntax() const
{
  int errorCount = 0;

  for (int r = 0; r < rowCount(QModelIndex()); ++r)
  {
    QString s = data(index(r, 0, QModelIndex()), Qt::DisplayRole).toString();
    if (s.length() > 0)
      for (int i = 0; i <= s.length(); ++i)
        if (s[i] == delim_start || s[i] == delim_end)
          if (!checkBlanksSyntax(s))
            errorCount++;
    s = data(index(r, 1, QModelIndex()), Qt::DisplayRole).toString();
    if (s.length() > 0)
      for (int i = 0; i <= s.length(); ++i)
        if (s[i] == delim_start || s[i] == delim_end)
          if (!checkBlanksSyntax(s))
            errorCount++;
  }

  return (errorCount == 0);
}

KEduVocLesson * KWQTableModel::currentLesson(int row)
{
  int i = 0;
  QList<KEduVocContainer *>  lessons = m_doc->lesson()->childContainers();
  foreach(KEduVocContainer * c, lessons) {
    if (c->containerType() == KEduVocLesson::Lesson) {
      i += c->entryCount();
      if (i >= row)
        return static_cast<KEduVocLesson *>(c);
    }
  }
  return m_doc->lesson();
}

//#include "kwqtablemodel.moc"
