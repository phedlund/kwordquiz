/***************************************************************************
                          kwordquiz.cpp  -  description
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
//
// include files for QT
#include <qpainter.h>
#include <qbitmap.h>

// include files for KDE
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kstandarddirs.h> //locate
#include <kfontdialog.h>
#include <kpopupmenu.h>
#include <knotifydialog.h>
//#include <keduvocdata.h>

// application specific includes
#include "kwordquiz.h"
#include "kwordquizdoc.h"
#include "dlglanguage.h"
#include "kwordquizprefs.h"
#include "qaview.h"
#include "flashview.h"
#include "multipleview.h"
#include "configuration.h"
#include "wqprintdialogpage.h"

#define ID_STATUS_MSG 1
#define ID_STATUS_MSG_MODE 2
#define ID_STATUS_MSG_SCORE 3

#define ID_MODE_1 1

#define ID_MENU_QUIZ 1001

KWordQuizApp::KWordQuizApp(QWidget* , const char* name):KMainWindow(0, name)
{
  
  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initDocument();
  
  readOptions();  
  
  initView();

  m_dirWatch = KDirWatch::self();
  m_quizType = WQQuiz::qtEditor;
  m_quiz = 0;
  m_flashView = 0;
  m_multipleView = 0;
  m_qaView = 0;
  
  slotQuizEditor();
  slotUndoChange(i18n("Cannot &Undo"), false);
  updateMode(Config().m_mode);

  m_prefDialog = 0;

  editMarkBlank->setEnabled(Config().m_enableBlanks);
  editUnmarkBlank->setEnabled(Config().m_enableBlanks); 
    
  if (Config().m_firstRun)
  {
    fileOpenRecent->addURL( locate("data", "kwordquiz/examples/example.kvtml"));
    fileOpenRecent->addURL( locate("data", "kwordquiz/examples/french_verbs.kvtml"));
    fileOpenRecent->addURL( locate("data", "kwordquiz/examples/fill_in_the_blank.kvtml"));
    fileOpenRecent->addURL( locate("data", "kwordquiz/examples/us_states_and_capitals.kvtml"));
  }
}

KWordQuizApp::~KWordQuizApp()
{

}

void KWordQuizApp::initActions()
{
  fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  fileNew->setWhatsThis(i18n("Creates a new blank vocabulary document"));
  fileNew->setToolTip(fileNew->whatsThis());
  
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpen->setWhatsThis(i18n("Opens an existing vocabulary document"));
  fileOpen->setToolTip(fileOpen->whatsThis());
  
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSave->setWhatsThis(i18n("Saves the active vocabulary document"));
  fileSave->setToolTip(fileSave->whatsThis());
  
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileSaveAs->setWhatsThis(i18n("Saves the active vocabulary document with a different name"));
  fileSaveAs->setToolTip(fileSaveAs->whatsThis());
  
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  fileClose->setWhatsThis(i18n("Closes the active vocabulary document"));
  fileClose->setToolTip(fileClose->whatsThis());
  
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
  filePrint->setWhatsThis(i18n("Prints the active vocabulary document"));
  filePrint->setToolTip(filePrint->whatsThis());
  
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  fileQuit->setWhatsThis(i18n("Quits KWordQuiz"));
  fileQuit->setToolTip(fileQuit->whatsThis());
  
  editUndo = KStdAction::undo(this, SLOT(slotEditUndo()), actionCollection());
  editUndo->setWhatsThis(i18n("Undoes the last command"));
  editUndo->setToolTip(editUndo->whatsThis());
  
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCut->setWhatsThis(i18n("Cuts the text from the selected cells and places it on the clipboard"));
  editCut->setToolTip(editCut->whatsThis());
  
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editCopy->setWhatsThis(i18n("Copies the text from the selected cells and places it on the clipboard"));
  editCopy->setToolTip(editCopy->whatsThis());
  
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  editPaste->setWhatsThis(i18n("Pastes previously cut or copied text from the clipboard into the selected cells"));
  editPaste->setToolTip(editPaste->whatsThis());
  
  editClear = KStdAction::clear(this, SLOT(slotEditClear()), actionCollection());  
  editClear->setWhatsThis(i18n("Clears the content of the selected cells"));
  editClear->setToolTip(editClear->whatsThis());
  
  editInsert = new KAction(i18n("&Insert Row"),/* "insrow"*/locate("data", "kwordquiz/pics/insertrow.png"), "CTRL+I", this, SLOT(slotEditInsert()), actionCollection(),"edit_insert");
  editInsert->setWhatsThis(i18n("Inserts a new row above the current row"));
  editInsert->setToolTip(editInsert->whatsThis());
  
  editDelete = new KAction(i18n("&Delete Row"), /*"remrow"*/ locate("data", "kwordquiz/pics/deleterow.png"), "CTRL+K", this, SLOT(slotEditDelete()), actionCollection(),"edit_delete");
  editDelete->setWhatsThis(i18n("Deletes the selected row(s)"));
  editDelete->setToolTip(editDelete->whatsThis());
  
  editMarkBlank = new KAction(i18n("&Mark As Blank"), locate("data", "kwordquiz/pics/markasblank.png"), "CTRL+M", this, SLOT(slotEditMarkBlank()), actionCollection(),"edit_mark_blank");
  editMarkBlank->setWhatsThis(i18n("Marks the current or selected word as a blank for Fill-in-the-blank"));
  editMarkBlank->setToolTip(editMarkBlank->whatsThis());
  
  editUnmarkBlank = new KAction(i18n("&Unmark Blanks"), locate("data", "kwordquiz/pics/unmarkasblank.png"), 0, this, SLOT(slotEditUnmarkBlank()), actionCollection(),"edit_unmark_blank");
  editUnmarkBlank->setWhatsThis(i18n("Removes blanks from the current or selected word"));
  editUnmarkBlank->setToolTip(editUnmarkBlank->whatsThis());
  
  //@todo implement editFind = KStdAction::find(this, SLOT(slotEditFind()), actionCollection());

  vocabLanguages = new KAction(i18n("&Column Titles..."), locate("data", "kwordquiz/pics/languages.png"), "CTRL+L", this, SLOT(slotVocabLanguages()), actionCollection(),"vocab_languages");
  vocabLanguages->setWhatsThis(i18n("Defines the column titles for the active vocabulary"));
  vocabLanguages->setToolTip(vocabLanguages->whatsThis());
  
  vocabFont = new KAction(i18n("&Font..."), "fonts", 0, this, SLOT(slotVocabFont()), actionCollection(),"vocab_font");
  vocabFont->setWhatsThis(i18n("Defines the font used by the editor"));
  vocabFont->setToolTip(vocabFont->whatsThis());
  
  //@todo implement vocabKeyboard = new KAction(i18n("&Keyboard..."), "kxkb", 0, this, SLOT(slotVocabKeyboard()), actionCollection(),"vocab_keyboard");
  
  vocabRC = new KAction(i18n("&Rows/Columns..."), 0, 0, this, SLOT(slotVocabRC()), actionCollection(),"vocab_rc");
  vocabRC->setWhatsThis(i18n("Defines the number of rows, row heights, and column widths for the active vocabulary"));
  vocabRC->setToolTip(vocabRC->whatsThis());
  
  vocabSort = new KAction(i18n("&Sort..."), locate("data", "kwordquiz/pics/sort.png"), 0, this, SLOT(slotVocabSort()), actionCollection(),"vocab_sort");
  vocabSort->setWhatsThis(i18n("Sorts the vocabulary in ascending or descending order based on the left or right column"));
  vocabSort->setToolTip(vocabSort->whatsThis());
  
  vocabShuffle = new KAction(i18n("Sh&uffle"), 0, 0, this, SLOT(slotVocabShuffle()), actionCollection(),"vocab_shuffle");
  vocabShuffle->setWhatsThis(i18n("Shuffles the entries of the active vocabulary"));
  vocabShuffle->setToolTip(vocabShuffle->whatsThis());

  mode = new KToolBarPopupAction(i18n("Change Mode"), locate("data", "kwordquiz/pics/mode1.png"), 0, this, SLOT(slotMode0()), actionCollection(),"mode_0");
  mode->setWhatsThis(i18n("Changes the mode used in quiz sessions"));
  mode->setToolTip(mode->whatsThis());
  
  KPopupMenu *popup = mode->popupMenu();
  popup->clear();
  popup->insertItem(QIconSet(QPixmap(locate("data", "kwordquiz/pics/mode1.png"))), "", this, SLOT(slotMode1()), 0, 0);
  popup->insertItem(QIconSet(QPixmap(locate("data", "kwordquiz/pics/mode2.png"))), "", this, SLOT(slotMode2()), 0, 1);
  popup->insertItem(QIconSet(QPixmap(locate("data", "kwordquiz/pics/mode3.png"))), "", this, SLOT(slotMode3()), 0, 2);
  popup->insertItem(QIconSet(QPixmap(locate("data", "kwordquiz/pics/mode4.png"))), "", this, SLOT(slotMode4()), 0, 3);
  popup->insertItem(QIconSet(QPixmap(locate("data", "kwordquiz/pics/mode5.png"))), "", this, SLOT(slotMode5()), 0, 4);

  mode1 = new KToggleAction("", locate("data", "kwordquiz/pics/mode1.png"), 0, this, SLOT(slotMode1()), actionCollection(),"mode_1");
  mode2 = new KToggleAction("", locate("data", "kwordquiz/pics/mode2.png"), 0, this, SLOT(slotMode2()), actionCollection(),"mode_2");
  mode3 = new KToggleAction("", locate("data", "kwordquiz/pics/mode3.png"), 0, this, SLOT(slotMode3()), actionCollection(),"mode_3");
  mode4 = new KToggleAction("", locate("data", "kwordquiz/pics/mode4.png"), 0, this, SLOT(slotMode4()), actionCollection(),"mode_4");
  mode5 = new KToggleAction("", locate("data", "kwordquiz/pics/mode5.png"), 0, this, SLOT(slotMode5()), actionCollection(),"mode_5");
  mode1->setWhatsThis(i18n("Selects this mode"));
  mode2->setWhatsThis(i18n("Selects this mode"));
  mode3->setWhatsThis(i18n("Selects this mode"));
  mode4->setWhatsThis(i18n("Selects this mode"));
  mode5->setWhatsThis(i18n("Selects this mode"));
  mode1->setToolTip(mode1->whatsThis());
  mode2->setToolTip(mode2->whatsThis());
  mode3->setToolTip(mode3->whatsThis());
  mode4->setToolTip(mode4->whatsThis());
  mode5->setToolTip(mode5->whatsThis());
          
  quizEditor = new KAction(i18n("&Editor"), locate("data", "kwordquiz/pics/tabedit.png"), "F6", this, SLOT(slotQuizEditor()), actionCollection(),"quiz_editor");
  quizEditor->setWhatsThis(i18n("Activates the vocabulary editor"));
  quizEditor->setToolTip(quizEditor->whatsThis());
  
  quizFlash = new KAction(i18n("&Flashcard"), locate("data", "kwordquiz/pics/tabflash.png"), "F7", this, SLOT(slotQuizFlash()), actionCollection(),"quiz_flash");
  quizFlash->setWhatsThis(i18n("Starts a flashcard session using the active vocabulary"));
  quizFlash->setToolTip(quizFlash->whatsThis());
  
  quizMultiple = new KAction(i18n("&Multiple Choice"), locate("data", "kwordquiz/pics/tabmultiple.png"), "F8", this, SLOT(slotQuizMultiple()), actionCollection(),"quiz_multiple");
  quizMultiple->setWhatsThis(i18n("Starts a multiple choice session using the active vocabulary"));
  quizMultiple->setToolTip(quizMultiple->whatsThis());
  
  quizQA = new KAction(i18n("&Question && Answer"), locate("data", "kwordquiz/pics/tabqa.png"), "F9", this, SLOT(slotQuizQA()), actionCollection(),"quiz_qa");
  quizQA->setWhatsThis(i18n("Starts a question and answer session using the active vocabulary"));
  quizQA->setToolTip(quizQA->whatsThis());
  
  quizCheck = new KAction(i18n("&Check"), locate("data", "kwordquiz/pics/check.png"), "Return", this, 0, actionCollection(),"quiz_check");
  quizCheck->setWhatsThis(i18n("Checks your answer to this question"));
  quizCheck->setToolTip(quizCheck->whatsThis());
  
  flashKnow = new KAction(i18n("I &Know"), locate("data", "kwordquiz/pics/know.png"), "K", this, 0, actionCollection(),"flash_know");
  flashKnow->setWhatsThis(i18n("Counts this card as correct and shows the next card"));
  flashKnow->setToolTip(flashKnow->whatsThis());
  
  flashDontKnow = new KAction(i18n("I &Do Not Know"), locate("data", "kwordquiz/pics/dontknow.png"), "D", this, 0, actionCollection(),"flash_dont_know");
  flashDontKnow->setWhatsThis(i18n("Counts this card as incorrect and shows the next card"));
  flashDontKnow->setToolTip(flashDontKnow->whatsThis());
  
  qaHint = new KAction(i18n("&Hint"), locate("data", "kwordquiz/pics/hint.png"), "CTRL+H", this, 0, actionCollection(),"qa_hint");
  qaHint->setWhatsThis(i18n("Gets the next correct letter of the answer"));
  qaHint->setToolTip(qaHint->whatsThis());
  
  quizRestart = new KAction(i18n("&Restart"), locate("data", "kwordquiz/pics/restart.png"), "CTRL+R", this, 0, actionCollection(), "quiz_restart");
  quizRestart->setWhatsThis(i18n("Restarts the quiz session from the beginning"));
  quizRestart->setToolTip(quizRestart->whatsThis());
  
  quizRepeatErrors = new KAction(i18n("Repeat &Errors"), locate("data", "kwordquiz/pics/repeat.png"), "CTRL+E", this, 0, actionCollection(),"quiz_repeat_errors");
  quizRepeatErrors->setWhatsThis(i18n("Repeats all incorrectly answered questions"));
  quizRepeatErrors->setToolTip(quizRepeatErrors->whatsThis());

  KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());  

  configToolbar = KStdAction::configureToolbars(this, SLOT( slotConfigureToolbar() ), actionCollection());
  configToolbar->setWhatsThis(i18n("Toggles display of the toolbars"));
  configToolbar->setToolTip(configToolbar->whatsThis());
  
  configNotifications = KStdAction::configureNotifications(this, SLOT(slotConfigureNotifications()), actionCollection());
  configNotifications->setWhatsThis(i18n("Configures sound and other notifications for certain events"));
  configNotifications->setToolTip(configNotifications->whatsThis());
  
  configApp = KStdAction::preferences(this, SLOT( slotConfigure()), actionCollection());
  configApp->setWhatsThis(i18n("Specifies preferences for the vocabulary editor and quiz sessions"));
  configApp->setToolTip(configApp->whatsThis());
  
  charMapper = new QSignalMapper(this);
  connect(charMapper, SIGNAL(mapped(int)), this, SLOT(slotInsertChar(int)));

  specialChar1 = new KAction(i18n("Special Character 1"), "", "CTRL+1", charMapper, SLOT(map()), actionCollection(), "char_1") ;    
  specialChar2 = new KAction(i18n("Special Character 2"), 0, "CTRL+2", charMapper, SLOT(map()), actionCollection(), "char_2") ; 
  specialChar3 = new KAction(i18n("Special Character 3"), 0, "CTRL+3", charMapper, SLOT(map()), actionCollection(), "char_3") ; 
  specialChar4 = new KAction(i18n("Special Character 4"), 0, "CTRL+4", charMapper, SLOT(map()), actionCollection(), "char_4") ; 
  specialChar5 = new KAction(i18n("Special Character 5"), 0, "CTRL+5", charMapper, SLOT(map()), actionCollection(), "char_5") ; 
  specialChar6 = new KAction(i18n("Special Character 6"), 0, "CTRL+6", charMapper, SLOT(map()), actionCollection(), "char_6") ; 
  specialChar7 = new KAction(i18n("Special Character 7"), 0, "CTRL+7", charMapper, SLOT(map()), actionCollection(), "char_7") ; 
  specialChar8 = new KAction(i18n("Special Character 8"), 0, "CTRL+8", charMapper, SLOT(map()), actionCollection(), "char_8") ; 
  specialChar9 = new KAction(i18n("Special Character 9"), 0, "CTRL+9", charMapper, SLOT(map()), actionCollection(), "char_9") ;     

  charMapper->setMapping(specialChar1, 1);
  charMapper->setMapping(specialChar2, 2);
  charMapper->setMapping(specialChar3, 3);
  charMapper->setMapping(specialChar4, 4);
  charMapper->setMapping(specialChar5, 5);
  charMapper->setMapping(specialChar6, 6);
  charMapper->setMapping(specialChar7, 7);
  charMapper->setMapping(specialChar8, 8);
  charMapper->setMapping(specialChar9, 9);

  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);
  
  actionCollection()->setHighlightingEnabled(true);
  connect(actionCollection(), SIGNAL(actionStatusText(const QString &)), this, SLOT(slotStatusMsg(const QString &)));  
  connect(actionCollection(), SIGNAL(actionHighlighted(KAction *, bool)), this, SLOT(slotActionHighlighted(KAction *, bool)));    
  updateSpecialCharIcons();
    
  // use the absolute path to your kwordquizui.rc file for testing purpose in createGUI();
  createGUI("kwordquizui.rc");
  setAutoSaveSettings();
}

void KWordQuizApp::initStatusBar()
{
  statusBar()->insertFixedItem("", ID_STATUS_MSG_MODE, true);
  statusBar()->setItemFixed(ID_STATUS_MSG_MODE, 250);
  statusBar()->setItemAlignment(ID_STATUS_MSG_MODE, AlignLeft|AlignVCenter);
}

void KWordQuizApp::initDocument()
{
  doc = new KWordQuizDoc(this);
  doc->newDocument();
}

void KWordQuizApp::initView()
{
  ////////////////////////////////////////////////////////////////////
  // create the main widget here that is managed by KTMainWindow's view-region and
  // connect the widget to your document to display document contents.

  m_editView = new KWordQuizView(this);
  doc->addView(m_editView);
  setCentralWidget(m_editView);
  setCaption(doc->URL().fileName(),false);
  m_editView->setFont(Config().m_editorFont);
  connect(m_editView, SIGNAL(undoChange(const QString&, bool )), this, SLOT(slotUndoChange(const QString&, bool)));
  connect(m_editView, SIGNAL(contextMenuRequested(int, int, const QPoint &)), this, SLOT(slotContextMenuRequested(int, int, const QPoint& )));
}

void KWordQuizApp::openDocumentFile(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));
  if (url.path() != "") {
    doc->openDocument( url);
    m_dirWatch->addFile(url.path());
    setCaption(url.fileName(), false);
    fileOpenRecent->addURL( url );
    updateMode(Config().m_mode);    
  }
  slotStatusMsg(i18n("Ready"));
}


KWordQuizDoc *KWordQuizApp::getDocument() const
{
  return doc;
}

void KWordQuizApp::saveOptions()
{
  fileOpenRecent->saveEntries(kapp->config(), "Recent Files");
  Config().write();
}


void KWordQuizApp::readOptions()
{
  Config().read();
  fileOpenRecent->loadEntries(kapp->config(), "Recent Files");
}

void KWordQuizApp::saveProperties(KConfig *_cfg)
{
  if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified())
  {
    // saving to tempfile not necessary

  }
  else
  {
    KURL url=doc->URL();
    _cfg->writeEntry("filename", url.url());
    _cfg->writeEntry("modified", doc->isModified());
    QString tempname = kapp->tempSaveName(url.url());
    QString tempurl= KURL::encode_string(tempname);
    KURL _url(tempurl);
    doc->saveDocument(_url);
  }
}


void KWordQuizApp::readProperties(KConfig* _cfg)
{
  QString filename = _cfg->readEntry("filename", "");
  KURL url(filename);
  bool modified = _cfg->readBoolEntry("modified", false);
  if(modified)
  {
    bool canRecover;
    QString tempname = kapp->checkRecoverFile(filename, canRecover);
    KURL _url(tempname);
  	
    if(canRecover)
    {
      doc->openDocument(_url);
      doc->setModified();
      setCaption(_url.fileName(),true);
      QFile::remove(tempname);
    }
  }
  else
  {
    if(!filename.isEmpty())
    {
      doc->openDocument(url);
      setCaption(url.fileName(),false);
    }
  }
}		

bool KWordQuizApp::queryClose()
{
  bool f = doc->saveModified();
  if (f)
    if (m_dirWatch->contains(doc->URL().path()))
      m_dirWatch->removeFile(doc->URL().path());
  return f;
}

bool KWordQuizApp::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KWordQuizApp::slotFileNew()
{
  slotStatusMsg(i18n("Opening a new document window..."));
  if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
    // neither saved nor has content, as good as new
  }
  else
  {
    KWordQuizApp *new_window= new KWordQuizApp();
    new_window->show();
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KURL url=KFileDialog::getOpenURL(QString::null, i18n("*.kvtml *.wql *.xml.gz *.csv|All Supported Documents\n*.kvtml|KDE Vocabulary Document\n*.wql|KWordQuiz Document\n*.xml.gz|Pauker Lesson\n*.csv|Comma-Separated Values"), this,
      i18n("Open Vocabulary Document..."));

  if(!url.isEmpty()) {
    if (m_dirWatch->contains(url.path()))
    {
      KMainWindow* w;
      if(memberList)
      {
        for(w=memberList->first(); w!=0; w=memberList->next())
        {
          KWordQuizApp *a =(KWordQuizApp *) w;
          if(a->doc ->URL().path() == url.path())
          {
            if (w->isMinimized())
              w->showNormal();
            w->setActiveWindow();
            w->raise();
            break;
          }
        }
      }    
    }
    else
    {
      if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
        // neither saved nor has content, as good as new
        doc->openDocument(url);
        m_dirWatch->addFile(url.path());
        setCaption(doc->URL().fileName(), false);
        fileOpenRecent->addURL( url );
        updateMode(Config().m_mode);
      }
      else
      {
        KWordQuizApp *new_window= new KWordQuizApp();
        new_window->show();
        new_window->doc->openDocument(url);
        m_dirWatch->addFile(url.path());
        new_window->setCaption(new_window->doc->URL().fileName(), false);
        new_window->fileOpenRecent->addURL( url );
        new_window->updateMode(Config().m_mode);
      }
    }
  }

  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));
  fileOpenRecent->setCurrentItem(-1);
  if (m_dirWatch->contains(url.path()))
  {
    KMainWindow* w;
    if(memberList)
    {
      for(w=memberList->first(); w!=0; w=memberList->next())
      {
        KWordQuizApp *a =(KWordQuizApp *) w;
        if(a->doc ->URL().path() == url.path())
        {
          if (w->isMinimized())
            w->showNormal();
          w->setActiveWindow();
          w->raise();
          break;
        }
      }
    }    
  }
  else
  {
    if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
      // neither saved nor has content, as good as new
      doc->openDocument(url);
      m_dirWatch->addFile(url.path());
      setCaption(doc->URL().fileName(), false);
      updateMode(Config().m_mode);
    }
    else
    {
      KWordQuizApp *new_window= new KWordQuizApp();
      new_window->show();
      new_window->doc->openDocument(url);
      m_dirWatch->addFile(url.path());
      new_window->setCaption(new_window->doc->URL().fileName(), false);
      new_window->updateMode(Config().m_mode);
    }
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFileSave()
{
  slotStatusMsg(i18n("Saving file..."));
  if (doc->URL().fileName() == i18n("Untitled") )
  {
    slotFileSaveAs();
  }
  else
  {
    doc->saveDocument(doc->URL());
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving file with a new filename..."));
  saveAsFileName();
  slotStatusMsg(i18n("Ready"));
}

bool KWordQuizApp::saveAsFileName( )
{
  
  bool success = false;
  
  KFileDialog *fd = new KFileDialog(QDir::currentDirPath(), QString::null, this, 0, true);
  fd -> setOperationMode(KFileDialog::Saving);
  fd -> setCaption(i18n("Save vocabulary document as..."));
  fd -> setFilter(i18n("*.kvtml|KDE Vocabulary Document\n*.wql|KWordQuiz Document\n*.csv|Comma-Separated Values\n*.html|Hypertext Markup Language"));

  if (fd->exec() == QDialog::Accepted)
  {
    KURL url = fd -> selectedURL();
    if(!url.isEmpty()){

      //@todo check that a valid extension was really given
      if (!url.fileName().contains('.'))
      {
        if  (fd->currentFilter() == "*.wql")
          url = KURL(url.path() + ".wql");
        else if (fd->currentFilter() == "*.csv")
          url = KURL(url.path() + ".csv");
        else if (fd->currentFilter() == "*.html")
          url = KURL(url.path() + ".html");          
        else
          url = KURL(url.path() + ".kvtml");
      }

      QFileInfo fileinfo(url.path());
      if (fileinfo.exists() && KMessageBox::questionYesNo(0,
          i18n("<qt>The file<br><b>%1</b><br>already exists. Do you want to overwrite it?</qt>")
              .arg(url.path())) == KMessageBox::No)
      {
      // do nothing
      }
      else
      {
        if (m_dirWatch ->contains(doc->URL().path()))
          m_dirWatch ->removeFile(doc->URL().path());
        doc->saveDocument(url);
        m_dirWatch->addFile(url.path());
        fileOpenRecent->addURL(url);
        setCaption(doc->URL().fileName(), doc->isModified());
        success = true;
      }
    }
  }
  delete(fd);
  return success;
}

void KWordQuizApp::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));
  
  if (memberList->count() > 1)
    close();
  else
    if (queryClose())
    {
      doc->newDocument();
      setCaption(doc->URL().fileName(), doc->isModified());      
      delete (m_editView);
      initView();
      slotQuizEditor();
      slotUndoChange(i18n("Cannot &Undo"), false);
      updateMode(Config().m_mode); 
      m_editView ->setFocus();     
    }    

  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));
  WQPrintDialogPage * p = new WQPrintDialogPage(this);
  KPrinter printer;
  printer.addDialogPage(p);
  printer.setFullPage(true);
  if (printer.setup(this))
  {
    m_editView->print(&printer);
  }

  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  // close the first window, the list makes the next one the first again.
  // This ensures that queryClose() is called on each window to ask for closing
  KMainWindow* w;
  if(memberList)
  {
    for(w=memberList->first(); w!=0; w=memberList->next())
    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if(!w->close())	break;
    }
  }
}

void KWordQuizApp::slotUndoChange( const QString & text, bool enabled )
{
  editUndo->setText(text);
  editUndo->setEnabled(enabled);
}

void KWordQuizApp::slotEditUndo()
{
  slotStatusMsg(i18n("Undoing previous command..."));
  m_editView->doEditUndo();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));
  m_editView->doEditCut();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));
  m_editView->doEditCopy();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));
  m_editView->doEditPaste();
  slotStatusMsg(i18n("Ready"));
}


void KWordQuizApp::slotEditClear()
{
  slotStatusMsg(i18n("Clearing the selected cells..."));
  m_editView->doEditClear();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditInsert()
{
  slotStatusMsg(i18n("Inserting rows..."));
  m_editView->doEditInsert();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditDelete()
{
  slotStatusMsg(i18n("Deleting selected rows..."));
  m_editView->doEditDelete();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditMarkBlank()
{
  slotStatusMsg(i18n("Marking selected text as a blank..."));
  m_editView->doEditMarkBlank();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotEditUnmarkBlank()
{
  slotStatusMsg(i18n("Removing blank markings..."));
  m_editView->doEditUnmarkBlank();
  slotStatusMsg(i18n("Ready"));
}


void KWordQuizApp::slotEditFind()
{
  slotStatusMsg(i18n("Searching for indicated text..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabLanguages()
{
  slotStatusMsg(i18n("Setting the column titles of the vocabulary..."));
  DlgLanguage* dlg;
  dlg = new DlgLanguage(this, "dlg_lang", true);
  dlg->setLanguage(1, m_editView -> horizontalHeader()->label(0));
  dlg->setLanguage(2, m_editView -> horizontalHeader()->label(1));
  dlg->disableResize();
  if (dlg->exec() == KDialogBase::Accepted)
  {
    m_editView -> horizontalHeader()->setLabel(0, dlg->Language(1));
    m_editView -> horizontalHeader()->setLabel(1, dlg->Language(2));
    updateMode(Config().m_mode);
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabFont()
{
  slotStatusMsg(i18n("Setting the font of the vocabulary..."));
  KFontDialog* dlg;
  dlg = new KFontDialog(this, "dlg_font", false, true);
  if (m_flashView != 0)
  {
    dlg->setFont(Config().m_flashFont);  
    if ( dlg->exec() == KFontDialog::Accepted )
    {
      Config().m_flashFont = dlg->font();
      emit settingsChanged();
    } 
  }
  else
  {
    dlg->setFont(m_editView -> font());
    if ( dlg->exec() == KFontDialog::Accepted )
    {
      m_editView ->setFont(dlg->font());
      Config().m_editorFont = dlg->font();
      doc->setModified(true);
    }
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabKeyboard()
{
  slotStatusMsg(i18n("Changing the keyboard layout..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabChar()
{
  slotStatusMsg(i18n("Inserting special character..."));
  m_editView->doVocabSpecChar();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabRC()
{
  slotStatusMsg(i18n("Changing row and column properties..."));
  m_editView->doVocabRC();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabSort()
{
  slotStatusMsg(i18n("Sorting the vocabulary..."));
  m_editView->doVocabSort();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotVocabShuffle()
{
  slotStatusMsg(i18n("Randomizing the vocabulary..."));
  m_editView->doVocabShuffle();
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode0()
{
  slotStatusMsg(i18n("Updating mode..."));
  if (Config().m_mode < 5) {
    updateMode(Config().m_mode + 1);
  }
  else
  {
  updateMode(1);
  }
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode1()
{
  slotStatusMsg(i18n("Updating mode..."));
  updateMode(1);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode2()
{
  slotStatusMsg(i18n("Updating mode..."));
  updateMode(2);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode3()
{
  slotStatusMsg(i18n("Updating mode..."));
  updateMode(3);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode4()
{
  slotStatusMsg(i18n("Updating mode..."));
  updateMode(4);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotMode5()
{
  slotStatusMsg(i18n("Updating mode..."));
  updateMode(5);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotQuizEditor()
{
  slotStatusMsg(i18n("Starting editor session..."));
  updateSession(WQQuiz::qtEditor);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotQuizFlash()
{
  slotStatusMsg(i18n("Starting flashcard session..."));
  updateSession(WQQuiz::qtFlash);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotQuizMultiple()
{
  slotStatusMsg(i18n("Starting multiple choice session..."));
  updateSession(WQQuiz::qtMultiple);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotQuizQA()
{
  slotStatusMsg(i18n("Starting question & answer session..."));
  updateSession(WQQuiz::qtQA);
  slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::updateSession(WQQuiz::QuizType qt)
{
  if (m_quiz != 0)
  {
    delete(m_quiz);
    m_quiz = 0;
  }

  switch( m_quizType ){
    case WQQuiz::qtEditor:
      //
      break;
    case WQQuiz::qtFlash:
      if (m_flashView != 0)
      {
        delete(m_flashView);
        m_flashView = 0;
      }
      break;
    case WQQuiz::qtMultiple:
      if (m_multipleView != 0)
      {
        delete(m_multipleView);
        m_multipleView = 0;
      }
      break;
    case WQQuiz::qtQA:
      if (m_qaView != 0)
      {
        delete(m_qaView);
        m_qaView = 0;
      }
      break;
  }

  m_quizType = qt;

  switch( m_quizType ){
    case WQQuiz::qtEditor:
      m_editView->show();
      setCentralWidget(m_editView);
      stateChanged("switchEditQuiz");
      editMarkBlank->setEnabled(Config().m_enableBlanks);
      editUnmarkBlank->setEnabled(Config().m_enableBlanks);
      stateChanged("showingEdit");

      toolBar("quizToolBar")->hide();
      break;
    case WQQuiz::qtFlash:
      m_quiz = new WQQuiz(m_editView);
      m_quiz ->setQuizType(WQQuiz::qtFlash);
      m_quiz->setQuizMode(Config().m_mode);
      m_quiz-> setEnableBlanks(Config().m_enableBlanks);
      if (m_quiz -> init())
      {
        m_editView->hide();
        m_flashView = new FlashView(this);
        connect(quizCheck, SIGNAL(activated()), m_flashView, SLOT(slotFlip()));
        connect(flashKnow, SIGNAL(activated()), m_flashView, SLOT(slotKnow()));
        connect(flashDontKnow, SIGNAL(activated()), m_flashView, SLOT(slotDontKnow()));
        connect(quizRestart, SIGNAL(activated()), m_flashView, SLOT(slotRestart()));
        connect(quizRepeatErrors, SIGNAL(activated()), m_flashView, SLOT(slotRepeat()));
        connect(this, SIGNAL(settingsChanged()), m_flashView, SLOT(slotApplySettings()));
        
        stateChanged("switchEditQuiz", StateReverse );
        editMarkBlank->setEnabled(false);
        editUnmarkBlank->setEnabled(false);
        stateChanged("showingFlash");
        toolBar("quizToolBar")->show();

        setCentralWidget(m_flashView);
        m_flashView -> setQuiz(m_quiz);
        m_flashView ->init();
        m_flashView->show();
      }
      else
      {
        delete(m_quiz);
        m_quiz = 0;
      }
      break;
    case WQQuiz::qtMultiple:
      m_quiz = new WQQuiz(m_editView);
      m_quiz ->setQuizType(WQQuiz::qtMultiple);
      m_quiz->setQuizMode(Config().m_mode);
      m_quiz-> setEnableBlanks(Config().m_enableBlanks);
      if (m_quiz -> init())
      {
        m_editView->hide();
        m_multipleView = new MultipleView(this);
        connect(quizCheck, SIGNAL(activated()), m_multipleView, SLOT(slotCheck()));
        connect(quizRestart, SIGNAL(activated()), m_multipleView, SLOT(slotRestart()));
        connect(quizRepeatErrors, SIGNAL(activated()), m_multipleView, SLOT(slotRepeat()));
        connect(this, SIGNAL(settingsChanged()), m_multipleView, SLOT(slotApplySettings()));
        
        stateChanged("switchEditQuiz", StateReverse ); 
        editMarkBlank->setEnabled(false);
        editUnmarkBlank->setEnabled(false);               
        stateChanged("showingMultiple");
        toolBar("quizToolBar")->show();        
        
        setCentralWidget(m_multipleView);

        m_multipleView -> setQuiz(m_quiz);
        m_multipleView ->init();
        m_multipleView->show();
      }
      else
      {
        delete(m_quiz);
        m_quiz = 0;
      }
      break;
    case WQQuiz::qtQA:
      m_quiz = new WQQuiz(m_editView);
      m_quiz ->setQuizType(WQQuiz::qtQA);
      m_quiz->setQuizMode(Config().m_mode);
      m_quiz-> setEnableBlanks(Config().m_enableBlanks);
      if (m_quiz -> init())
      {
        m_editView->hide();
        m_qaView = new QAView(this);
        connect(quizCheck, SIGNAL(activated()), m_qaView, SLOT(slotCheck()));
        connect(qaHint, SIGNAL(activated()), m_qaView, SLOT(slotHint()));
        connect(quizRestart, SIGNAL(activated()), m_qaView, SLOT(slotRestart()));
        connect(quizRepeatErrors, SIGNAL(activated()), m_qaView, SLOT(slotRepeat()));
        connect(this, SIGNAL(settingsChanged()), m_qaView, SLOT(slotApplySettings()));
        
        stateChanged("switchEditQuiz", StateReverse );
        editMarkBlank->setEnabled(false);
        editUnmarkBlank->setEnabled(false);        
        stateChanged("showingQA");
        toolBar("quizToolBar")->show();
                
        setCentralWidget(m_qaView);

        m_qaView -> setQuiz(m_quiz);
        m_qaView ->init();
        m_qaView->show();
      }
      else
      {
        delete(m_quiz);
        m_quiz = 0;
      }
      break;

  }
}

/** Configure toolbar */
void KWordQuizApp::slotConfigureToolbar()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );
  KEditToolbar dlg(actionCollection());
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();        
}

void KWordQuizApp::slotNewToolbarConfig( )
{
  createGUI("kwordquizui.rc");
  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

/** Configure notifications */
void KWordQuizApp::slotConfigureNotifications( )
{
  KNotifyDialog::configure(this, "Notification Configuration Dialog");
}

/** Configure kwordquiz */
void KWordQuizApp::slotConfigure()
{
  // create dialog on demand
  if (m_prefDialog==0)
  {
    m_prefDialog=new KWordQuizPrefs(this);
    connect(m_prefDialog, SIGNAL(settingsChanged()), this, SLOT(slotApplyPreferences()));
  }
  m_prefDialog->updateDialog();
  if (m_prefDialog->exec()==QDialog::Accepted)
  {
    m_prefDialog->updateConfiguration();
    slotApplyPreferences();
  }
}

void KWordQuizApp::slotApplyPreferences()
{
  Config().write();
  editMarkBlank->setEnabled(Config().m_enableBlanks);
  editUnmarkBlank->setEnabled(Config().m_enableBlanks);
  m_editView->viewport()->repaint(true);
  updateSpecialCharIcons();
  emit settingsChanged();
}

void KWordQuizApp::updateSpecialCharIcons( )
{
  for (int i = 0; i < 9; i++){
    KAction * act = actionCollection()->action(QString("char_" + QString::number(i + 1)).latin1());
    act->setIcon(charIcon(Config().m_specialCharacters[i]));
    act->setToolTip(i18n("Inserts the character %1").arg(Config().m_specialCharacters[i]));
  }
}

QString KWordQuizApp::charIcon(const QChar & c)
{
  ///Create a name and path for the icon
  QString s = locateLocal("icon", "char" + QString::number(c.unicode()) + ".png");
  
  ///No need to redraw if it already exists
  if (KStandardDirs::exists(s))
    return s;
  
  QRect r(4, 4, 120, 120);

  ///A font to draw the character with
  QFont font("sans");
  font.setPixelSize(100);
  font.setWeight(QFont::Bold);
  
  ///Create the pixmap        
  QPixmap pm(128, 128);
  pm.fill(Qt::white);
  QPainter p(&pm);
  p.setFont(font);
  p.setPen(Qt::blue);
  p.drawText(r, Qt::AlignCenter, (QString) c);
  
  ///Create transparency mask
  QBitmap bm(128, 128);
  bm.fill(Qt::color0);
  QPainter b(&bm);
  b.setFont(font);
  b.setPen(Qt::color1);
  b.drawText(r, Qt::AlignCenter, (QString) c);
  
  ///Mask the pixmap
  pm.setMask(bm); 
  
  ///Save the icon to disk
  pm.save(s, "PNG");
  
  return s;
}

void KWordQuizApp::slotStatusMsg(const QString &text)
{
  statusBar()->clear();
  statusBar()->message(text);
}

/*!
    \fn KWordQuizApp::updateMode(int m)
 */
void KWordQuizApp::updateMode(int m)
{
  if (m_quiz != 0)
    if (KMessageBox::warningContinueCancel(this, i18n("This will restart your quiz. Do you wish to continue?"), QString::null, KStdGuiItem::cont(), "askModeQuiz") != KMessageBox::Continue)
    {
      mode1->setChecked(Config().m_mode == 1);
      mode2->setChecked(Config().m_mode == 2);
      mode3->setChecked(Config().m_mode == 3);
      mode4->setChecked(Config().m_mode == 4);
      mode5->setChecked(Config().m_mode == 5);
      return;
    }

  Config().m_mode = m;
  QString s1 = m_editView -> horizontalHeader()->label(0);
  QString s2 = m_editView -> horizontalHeader()->label(1);

  mode1->setText(i18n("&1 %1 -> %2 In Order").arg(s1).arg(s2));
  mode2->setText(i18n("&2 %1 -> %2 In Order").arg(s2).arg(s1));
  mode3->setText(i18n("&3 %1 -> %2 Randomly").arg(s1).arg(s2));
  mode4->setText(i18n("&4 %1 -> %2 Randomly").arg(s2).arg(s1));
  mode5->setText(i18n("&5 %1 <-> %2 Randomly").arg(s1).arg(s2));

  mode1->setChecked(Config().m_mode == 1);
  mode2->setChecked(Config().m_mode == 2);
  mode3->setChecked(Config().m_mode == 3);
  mode4->setChecked(Config().m_mode == 4);
  mode5->setChecked(Config().m_mode == 5);

  KPopupMenu *popup = mode->popupMenu();
  popup->setItemChecked(0, Config().m_mode == 1);
  popup->setItemChecked(1, Config().m_mode == 2);
  popup->setItemChecked(2, Config().m_mode == 3);
  popup->setItemChecked(3, Config().m_mode == 4);
  popup->setItemChecked(4, Config().m_mode == 5);

  popup->changeItem(0, i18n("&1 %1 -> %2 In Order").arg(s1).arg(s2));
  popup->changeItem(1, i18n("&2 %1 -> %2 In Order").arg(s2).arg(s1));
  popup->changeItem(2, i18n("&3 %1 -> %2 Randomly").arg(s1).arg(s2));
  popup->changeItem(3, i18n("&4 %1 -> %2 Randomly").arg(s2).arg(s1));
  popup->changeItem(4, i18n("&5 %1 <-> %2 Randomly").arg(s1).arg(s2));

  QString s;
  mode->setIcon(locate("data", "kwordquiz/pics/mode" + s.setNum(Config().m_mode) + ".png"));

  switch( Config().m_mode ){
  case 1:
    statusBar()->changeItem(i18n("%1 -> %2 In Order").arg(s1).arg(s2), ID_STATUS_MSG_MODE);
    break;
  case 2:
    statusBar()->changeItem(i18n("%1 -> %2 In Order").arg(s2).arg(s1), ID_STATUS_MSG_MODE);
    break;
  case 3:
    statusBar()->changeItem(i18n("%1 -> %2 Randomly").arg(s1).arg(s2), ID_STATUS_MSG_MODE);
    break;
  case 4:
    statusBar()->changeItem(i18n("%1 -> %2 Randomly").arg(s2).arg(s1), ID_STATUS_MSG_MODE);
    break;
  case 5:
    statusBar()->changeItem(i18n("%1 <-> %2 Randomly").arg(s1).arg(s2), ID_STATUS_MSG_MODE);
    break;
  }

  if (m_quiz !=0)
    updateSession(m_quizType);
}

void KWordQuizApp::slotInsertChar( int i )
{
  if (m_qaView != 0)
    m_qaView->slotSpecChar(Config().m_specialCharacters[i - 1]);
  else
    if (centralWidget() == m_editView)
      m_editView->slotSpecChar(Config().m_specialCharacters[i - 1]);
}

void KWordQuizApp::slotActionHighlighted( KAction * action, bool hl)
{
  if (!hl)
    slotStatusMsg(i18n("Ready"));
}

void KWordQuizApp::slotContextMenuRequested(int row, int col, const QPoint & pos)
{
  QWidget *w = factory()->container("editor_popup", this);
  QPopupMenu *popup = static_cast<QPopupMenu *>(w);
  popup->exec(pos);
}






#include "kwordquiz.moc"
