/***************************************************************************
                          kwordquizview.h  -  description
                             -------------------
    begin                : Wed Jul 24 20:12:30 PDT 2002
    copyright            : (C) 2002 by Peter Hedlund
    email                : peter@peterandlinda.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWORDQUIZVIEW_H
#define KWORDQUIZVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for Qt
#include <qwidget.h>
#include <qtable.h>
#include <qtextstream.h>

//include <wqviewbase.h>

//class QTable;
class KWordQuizDoc;
class DlgSpecChar;

class WQUndo {
public:
  QString text() const {return m_text;};
  void setText(const QString & s) {m_text = s;};
  QString * data() const {return m_data;};
  void setData(QString * ts) {m_data = ts;};
private:
  QString m_text;
  QString * m_data;
};

/** The KWordQuizView class provides the view widget for the KWordQuizApp instance.	
 * The View instance inherits QWidget as a base class and represents the view object of a KTMainWindow. As KWordQuizView is part of the
 * docuement-view model, it needs a reference to the document object connected with it by the KWordQuizApp class to manipulate and display
 * the document structure provided by the KWordQuizDoc class.
 *
 * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
 * @version KDevelop version 0.4 code generation
 */
class KWordQuizView : public QTable
{
  Q_OBJECT
  public:
    /** Constructor for the main view */
    KWordQuizView(QWidget *parent = 0, const char *name=0);
    /** Destructor for the main view */
    ~KWordQuizView();

    /** returns a pointer to the document connected to the view instance. Mind that this method requires a KWordQuizApp instance as a parent
     * widget to get to the window document pointer by calling the KWordQuizApp::getDocument() method.
     *
     * @see KWordQuizApp#getDocument
     */
    KWordQuizDoc *getDocument() const;

    /** contains the implementation for printing functionality */
    void print(QPrinter *pPrinter);
    bool gridIsEmpty();
    QFont colFont(int i);

    void addUndo(const QString & caption);
    void doEditUndo();
    void doEditCut();
    void doEditCopy();
    void doEditPaste();
    void doEditClear();
    void doEditInsert();
    void doEditDelete();
    void doVocabSort();
    void doVocabShuffle();
    void doVocabRC();
    void doVocabSpecChar();
    void fromStream(QTextStream* ts);
    void toStream(QTextStream* ts);
  protected:
    QWidget * beginEdit(int row, int col, bool replace);
    void endEdit ( int row, int col, bool accept, bool replace );
    void activateNextCell();
  public slots:
    void adjustRow(int row);

  private slots:
    void slotDlgSpecCharClosed();
    void slotSpecChar(QChar);

  signals:
    void undoChange(const QString & text, bool enabled);    

  private:
    int m_currentRow;
    int m_currentCol;
    QString m_currentText;
    QTableSelection m_currentSel;
    DlgSpecChar* dlgSpecChar;
    /** the list of the undo objects */
    static QValueList<WQUndo> *m_undoList;
    void wqCurrentSelection(bool clear);
};

#endif // KWORDQUIZVIEW_H
