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
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qsize.h>
#include <qdialog.h>
#include <qclipboard.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstandarddirs.h> //locate
#include <kfontdialog.h>
#include <kpopupmenu.h>
#include <knotifydialog.h>
#include <krandomsequence.h>
//#include <keduvocdata.h>

// application specific includes
#include "kwordquiz.h"
#include "kwordquizview.h"
#include "kwordquizdoc.h"
#include "dlglanguage.h"
#include "kwordquizprefs.h"
#include "qaview.h"
#include "flashview.h"
#include "multipleview.h"
#include "wqquiz.h"
#include "configuration.h"
#include "dlgsort.h"

#define ID_STATUS_MSG 1
#define ID_STATUS_MSG_MODE 2
#define ID_STATUS_MSG_SCORE 3

#define ID_MODE_1 1

#define ID_MENU_QUIZ 1001

KWordQuizApp::KWordQuizApp(QWidget* , const char* name):KMainWindow(0, name)
{
  config=kapp->config();

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initDocument();
  initView();

  readOptions();

  m_quizType = WQQuiz::qtEditor;
  m_quiz = 0;
  slotQuizEditor();
  slotUndoChange(i18n("Can't &Undo"), false);
  updateMode(m_mode);
  m_prefDialog = 0;
}

KWordQuizApp::~KWordQuizApp()
{

}

void KWordQuizApp::initActions()
{
  fileNewWindow = new KAction(i18n("New &Window"), 0, 0, this, SLOT(slotFileNewWindow()), actionCollection(),"file_new_window");
  fileNew = KStdAction::openNew(this, SLOT(slotFileNewWindow()), actionCollection());
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  editUndo = KStdAction::undo(this, SLOT(slotEditUndo()), actionCollection());
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  editClear = new KAction(i18n("C&lear"), "editclear", 0, this, SLOT(slotEditClear()), actionCollection(),"edit_clear");

  //How can I use just the file name for the icons like other programs?
  editInsert = new KAction(i18n("&Insert"),/* "insrow"*/locate("data", "kwordquiz/pics/insertrow.png"), "CTRL+I", this, SLOT(slotEditInsert()), actionCollection(),"edit_insert");
  editDelete = new KAction(i18n("&Delete"), /*"remrow"*/ locate("data", "kwordquiz/pics/deleterow.png"), "CTRL+K", this, SLOT(slotEditDelete()), actionCollection(),"edit_delete");
  editMarkBlank = new KAction(i18n("&Mark As Blank"), locate("data", "kwordquiz/pics/markasblank.png"), 0, this, SLOT(slotEditMarkBlank()), actionCollection(),"edit_mark_blank");
  editUnmarkBlank = new KAction(i18n("&Unmark Blanks"), locate("data", "kwordquiz/pics/unmarkasblank.png"), 0, this, SLOT(slotEditUnmarkBlank()), actionCollection(),"edit_unmark_blank");
  editFind = KStdAction::find(this, SLOT(slotEditFind()), actionCollection());
  //editFind = new KAction(i18n("&Find/Replace"), 0, 0, this, SLOT(slotEditFind()), actionCollection(),"edit_find");

  vocabLanguages = new KAction(i18n("&Languages..."), locate("data", "kwordquiz/pics/languages.png"), "CTRL+L", this, SLOT(slotVocabLanguages()), actionCollection(),"vocab_languages");
  vocabFont = new KAction(i18n("&Font..."), "fonts", 0, this, SLOT(slotVocabFont()), actionCollection(),"vocab_font");
  vocabKeyboard = new KAction(i18n("&Keyboard..."), "kxkb", 0, this, SLOT(slotVocabKeyboard()), actionCollection(),"vocab_keyboard");
  vocabChar = new KAction(i18n("Special &Characters..."), "kcharselect", 0, this, SLOT(slotVocabChar()), actionCollection(),"vocab_char");
  vocabRC = new KAction(i18n("&Rows/Columns..."), 0, 0, this, SLOT(slotVocabRC()), actionCollection(),"vocab_rc");
  vocabSort = new KAction(i18n("&Sort"), locate("data", "kwordquiz/pics/sort.png"), 0, this, SLOT(slotVocabSort()), actionCollection(),"vocab_sort");
  vocabShuffle = new KAction(i18n("Sh&uffle"), 0, 0, this, SLOT(slotVocabShuffle()), actionCollection(),"vocab_shuffle");

  mode = new KToolBarPopupAction(i18n("Change Mode"), locate("data", "kwordquiz/pics/mode1.png"), 0, this, SLOT(slotMode0()), actionCollection(),"mode_0");

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

  quizEditor = new KAction(i18n("&Editor"), locate("data", "kwordquiz/pics/tabedit.png"), "F6", this, SLOT(slotQuizEditor()), actionCollection(),"quiz_editor");
  quizFlash = new KAction(i18n("&Flashcard"), locate("data", "kwordquiz/pics/tabflash.png"), "F7", this, SLOT(slotQuizFlash()), actionCollection(),"quiz_flash");
  quizMultiple = new KAction(i18n("&Multiple Choice"), locate("data", "kwordquiz/pics/tabmultiple.png"), "F8", this, SLOT(slotQuizMultiple()), actionCollection(),"quiz_multiple");
  quizQA = new KAction(i18n("&Question && Answer"), locate("data", "kwordquiz/pics/tabqa.png"), "F9", this, SLOT(slotQuizQA()), actionCollection(),"quiz_qa");

  quizCheck = new KAction(i18n("&Check"), locate("data", "kwordquiz/pics/check.png"), "Return", this, 0, actionCollection(),"quiz_check");

  flashKnow = new KAction(i18n("I &Know"), locate("data", "kwordquiz/pics/know.png"), "K", this, 0, actionCollection(),"flash_know");
  flashDontKnow = new KAction(i18n("I &Don't Know"), locate("data", "kwordquiz/pics/dontknow.png"), "D", this, 0, actionCollection(),"flash_dont_know");

  qaHint = new KAction(i18n("&Hint"), locate("data", "kwordquiz/pics/hint.png"), "CTRL+H", this, 0, actionCollection(),"qa_hint");

  quizRestart = new KAction(i18n("&Restart"), locate("data", "kwordquiz/pics/restart.png"), "CTRL+R", this, 0, actionCollection(), "quiz_restart");
  quizRepeatErrors = new KAction(i18n("Repeat &Errors"), locate("data", "kwordquiz/pics/repeat.png"), "CTRL+T", this, 0, actionCollection(),"quiz_repeat_errors");

  //viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());

  configKeys = KStdAction::keyBindings(this, SLOT( slotConfigureKeys() ), actionCollection());
  configToolbar = KStdAction::configureToolbars(this, SLOT( slotConfigureToolbar() ), actionCollection());
  configNotifications = KStdAction::configureNotifications(this, SLOT(slotConfigureNotifications()), actionCollection());
  configApp = KStdAction::preferences(this, SLOT( slotConfigure()), actionCollection());


  //Question: isn't this supposed to show up in the statusbar?
  /*TODO*/
  fileNewWindow->setStatusText(i18n("Opens a new application window"));
  fileNew->setStatusText(i18n("Creates a new document"));
  fileOpen->setStatusText(i18n("Opens an existing document"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used file"));
  fileSave->setStatusText(i18n("Saves the actual document"));
  fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
  fileClose->setStatusText(i18n("Closes the actual document"));
  filePrint ->setStatusText(i18n("Prints out the actual document"));
  fileQuit->setStatusText(i18n("Quits the application"));
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
  //viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  setStandardToolBarMenuEnabled(true);
  // use the absolute path to your kwordquizui.rc file for testing purpose in createGUI();
  createGUI("kwordquizui.rc");

}


void KWordQuizApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR
  // TODO: add your own items you need for displaying current application status.
  statusBar()->insertFixedItem(i18n("Ready."), ID_STATUS_MSG);
  statusBar()->setItemFixed(ID_STATUS_MSG, 300);
  statusBar()->setItemAlignment(ID_STATUS_MSG, AlignLeft|AlignVCenter);

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

  connect(m_editView, SIGNAL(undoChange(const QString&, bool )), this, SLOT(slotUndoChange(const QString&, bool)));
}

void KWordQuizApp::openDocumentFile(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));
  if (url.path() != "") {
    doc->openDocument( url);
    fileOpenRecent->addURL( url );
  }
  slotStatusMsg(i18n("Ready."));
}


KWordQuizDoc *KWordQuizApp::getDocument() const
{
  return doc;
}

void KWordQuizApp::saveOptions()
{	
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());
  config->writeEntry("Show Statusbar",viewStatusBar->isChecked());

  toolBar("mainToolBar")->saveSettings(config, "MainToolBar");
  toolBar("sessionToolBar")->saveSettings(config, "SessionToolBar");
  toolBar("quizToolBar")->saveSettings(config, "QuizToolBar");

  fileOpenRecent->saveEntries(config,"Recent Files");

  config->setGroup("Quiz Options");
  config->writeEntry("Mode", m_mode);
}


void KWordQuizApp::readOptions()
{

  config->setGroup("General Options");

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();

  toolBar("mainToolBar")->applySettings(config, "MainToolBar");
  toolBar("sessionToolBar")->applySettings(config, "SessionToolBar");
  //toolBar("quizToolBar")->applySettings(config, "QuizToolBar");

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty())
  {
    resize(size);
  }

  config->setGroup("Quiz Options");
  m_mode = config->readNumEntry("Mode", 1);
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
  return doc->saveModified();
}

bool KWordQuizApp::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KWordQuizApp::slotFileNewWindow()
{
  slotStatusMsg(i18n("Opening a new application window..."));
	if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
    // neither saved nor has content, as good as new
  }
  else
  {
    KWordQuizApp *new_window= new KWordQuizApp();
    new_window->show();

  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileNew()
{
  slotStatusMsg(i18n("Creating new document..."));

  if(!doc->saveModified())
  {
     // here saving wasn't successful

  }
  else
  {	
    doc->newDocument();		
    setCaption(doc->URL().fileName(), false);
  }

  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KURL url=KFileDialog::getOpenURL(QString::null, i18n("*.kvtml *.wql|All Supported Documents\n*.kvtml|KDE Vocabulary Document\n*.wql|KWordQuiz Document"), this,
      i18n("Open Vocabulary Document..."));

  if(!url.isEmpty()) {
  	if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
      // neither saved nor has content, as good as new
      doc->openDocument(url);
      setCaption(url.fileName(), false);
      fileOpenRecent->addURL( url );
      updateMode(m_mode);
    }
    else
    {
      KWordQuizApp *new_window= new KWordQuizApp();
      new_window->show();
      new_window->doc->openDocument(url);
      new_window->setCaption(url.fileName(), false);
      new_window->fileOpenRecent->addURL( url );
      new_window->updateMode(m_mode);
    }
  }

  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  if (doc->URL().fileName() == i18n("Untitled")  && m_editView->gridIsEmpty()){
    // neither saved nor has content, as good as new
    doc->openDocument(url);
    setCaption(url.fileName(), false);
    updateMode(m_mode);
  }
  else
  {
    KWordQuizApp *new_window= new KWordQuizApp();
    new_window->show();
    new_window->doc->openDocument(url);
    new_window->setCaption(url.fileName(), false);
    new_window->updateMode(m_mode);
  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileSave()
{
  slotStatusMsg(i18n("Saving file..."));
	if (doc->URL().fileName() == i18n("Untitled") ){
    slotFileSaveAs();
  }
  else
  {
    doc->saveDocument(doc->URL());
  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving file with a new filename..."));

  KFileDialog *fd = new KFileDialog(QDir::currentDirPath(), QString::null, this, QString::null, true);
  fd->setOperationMode(KFileDialog::Saving);
  fd -> setCaption(i18n("Save vocabulary document as..."));
  fd->setFilter(i18n("*.kvtml|KDE Vocabulary Document\n*.wql|KWordQuiz Document"));

  if (fd->exec() == QDialog::Accepted)
  {
    KURL url= fd->selectedURL();
    //KURL url=KFileDialog::getSaveURL(QDir::currentDirPath(),
    //      i18n("*.kvtml|KDE Vocabulary Document\n*.wql|KWordQuiz Document"), this, i18n("Save vocabulary document as..."));
    if(!url.isEmpty()){

      //TODO check that a valid extension was really given
      if (!url.fileName().contains('.'))
      {
        if  (fd->currentFilter() == "*.wql")
          url = KURL(url.path() + ".wql");
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
        doc->saveDocument(url);
        fileOpenRecent->addURL(url);
        setCaption(url.fileName(),doc->isModified());
      }
    }
  }
  delete(fd);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));
	
  close();

  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  /*QPrinter printer;
  if (printer.setup(this))
  {
    m_editView->print(&printer);
  }
*/
  slotStatusMsg(i18n("Ready."));
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
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));
  m_editView->doEditCut();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));
  m_editView->doEditCopy();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));
  m_editView->doEditPaste();
  slotStatusMsg(i18n("Ready."));
}


void KWordQuizApp::slotEditClear()
{
  slotStatusMsg(i18n("Clearing the selected cells..."));
  m_editView->doEditClear();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditInsert()
{
  slotStatusMsg(i18n("Inserting rows..."));
  m_editView->doEditInsert();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditDelete()
{
  slotStatusMsg(i18n("Deleting selected rows..."));
  m_editView->doEditDelete();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditMarkBlank()
{
  slotStatusMsg(i18n("Undoing previous command..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotEditUnmarkBlank()
{
  slotStatusMsg(i18n("Undoing previous command..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready."));
}


void KWordQuizApp::slotEditFind()
{
  slotStatusMsg(i18n("Undoing previous command..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabLanguages()
{
  slotStatusMsg(i18n("Setting the languages of the vocabulary..."));
  DlgLanguage* dlg;
  dlg = new DlgLanguage(this, "dlg_lang", true);
  dlg->setInitialSize(QSize(310, 180), true);
  dlg->setLanguage(1, m_editView -> horizontalHeader()->label(0));
  dlg->setLanguage(2, m_editView -> horizontalHeader()->label(1));
  if (dlg->exec() == KDialogBase::Accepted)
  {
    m_editView -> horizontalHeader()->setLabel(0, dlg->Language(1));
    m_editView -> horizontalHeader()->setLabel(1, dlg->Language(2));
    updateMode(m_mode);
  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabFont()
{
  slotStatusMsg(i18n("Setting the font of the vocabulary..."));
  KFontDialog* dlg;
  dlg = new KFontDialog(this, "dlg_font", false, true);
  dlg->setFont(m_editView -> font());
  if ( dlg->exec() == KFontDialog::Accepted )
  {
    m_editView ->setFont(dlg->font());
    m_editView ->horizontalHeader()->setFont(QFont());
    m_editView ->verticalHeader()->setFont(QFont());
    doc->setModified(true);
  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabKeyboard()
{
  slotStatusMsg(i18n("Changing the keyboard layout..."));
  KMessageBox::sorry(0, i18n("Not implemented yet"));
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabChar()
{
  slotStatusMsg(i18n("Inserting special character..."));
  m_editView->doVocabSpecChar();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabRC()
{
  slotStatusMsg(i18n("Changing row and column properties..."));
  m_editView->doVocabRC();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabSort()
{
  slotStatusMsg(i18n("Sorting the vocabulary..."));
  m_editView->doVocabSort();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotVocabShuffle()
{
  slotStatusMsg(i18n("Randomizing the vocabulary..."));
  m_editView->doVocabShuffle();
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode0()
{
  slotStatusMsg(i18n("Updating mode..."));
  if (m_mode < 5) {
    updateMode(m_mode + 1);
  }
  else
  {
  updateMode(1);
  }
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode1()
{
  slotStatusMsg(i18n("Updating mode..."));
  //m_mode = 1;
  updateMode(1);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode2()
{
  slotStatusMsg(i18n("Updating mode..."));
  //m_mode = 2;
  updateMode(2);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode3()
{
  slotStatusMsg(i18n("Updating mode..."));
  //m_mode = 3;
  updateMode(3);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode4()
{
  slotStatusMsg(i18n("Updating mode..."));
  //m_mode = 4;
  updateMode(4);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotMode5()
{
  slotStatusMsg(i18n("Updating mode..."));
  //m_mode = 5;
  updateMode(5);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotQuizEditor()
{
  slotStatusMsg(i18n("Starting editor session..."));
  updateSession(WQQuiz::qtEditor);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotQuizFlash()
{
  slotStatusMsg(i18n("Starting flashcard session..."));
  updateSession(WQQuiz::qtFlash);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotQuizMultiple()
{
  slotStatusMsg(i18n("Starting multiple choice session..."));
  updateSession(WQQuiz::qtMultiple);
  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotQuizQA()
{
  slotStatusMsg(i18n("Starting question & answer session..."));
  updateSession(WQQuiz::qtQA);
  slotStatusMsg(i18n("Ready."));
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
      delete(m_flashView);
      break;
    case WQQuiz::qtMultiple:
      delete(m_multipleView);
      break;
    case WQQuiz::qtQA:
      delete(m_qaView);
      break;
  }

  m_quizType = qt;

  if (m_quizType == WQQuiz::qtEditor)
    stateChanged("switchEditQuiz");
  else
    stateChanged("switchEditQuiz", StateReverse );

  switch( m_quizType ){
    case WQQuiz::qtEditor:
      m_editView->show();
      setCentralWidget(m_editView);
      stateChanged("showingEdit");

      toolBar("quizToolBar")->hide();
      break;
    case WQQuiz::qtFlash:
      m_quiz = new WQQuiz(m_editView);

      m_quiz ->setQuizType(WQQuiz::qtFlash);
      m_quiz->setQuizMode(m_mode);
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
        m_flashView->show();
        setCentralWidget(m_flashView);

        m_flashView -> setQuiz(m_quiz);
        m_flashView ->init();
        stateChanged("showingFlash");
        toolBar("quizToolBar")->show();
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
      m_quiz->setQuizMode(m_mode);
      m_quiz-> setEnableBlanks(Config().m_enableBlanks);
      if (m_quiz -> init())
      {
        m_editView->hide();
        m_multipleView = new MultipleView(this);
        connect(quizCheck, SIGNAL(activated()), m_multipleView, SLOT(slotCheck()));
        connect(quizRestart, SIGNAL(activated()), m_multipleView, SLOT(slotRestart()));
        connect(quizRepeatErrors, SIGNAL(activated()), m_multipleView, SLOT(slotRepeat()));
        connect(this, SIGNAL(settingsChanged()), m_multipleView, SLOT(slotApplySettings()));
        m_multipleView->show();
        setCentralWidget(m_multipleView);

        m_multipleView -> setQuiz(m_quiz);
        m_multipleView ->init();
        stateChanged("showingMultiple");
        toolBar("quizToolBar")->show();
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
      m_quiz->setQuizMode(m_mode);
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
        m_qaView->show();
        setCentralWidget(m_qaView);

        m_qaView -> setQuiz(m_quiz);
        m_qaView ->init();
        stateChanged("showingQA");
        toolBar("quizToolBar")->show();
      }
      else
      {
        delete(m_quiz);
        m_quiz = 0;
      }
      break;

  }
}

void KWordQuizApp::slotViewToolBar()
{
  slotStatusMsg(i18n("Toggling toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
//   if(!viewToolBar->isChecked())
//   {
//     toolBar("mainToolBar")->hide();
//   }
//   else
//   {
//     toolBar("mainToolBar")->show();
//   }

  slotStatusMsg(i18n("Ready."));
}

void KWordQuizApp::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  if(!viewStatusBar->isChecked())
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  slotStatusMsg(i18n("Ready."));
}


/** Configure keys */
void KWordQuizApp::slotConfigureKeys()
{
    KKeyDialog::configureKeys( actionCollection(), xmlFile() );
}

/** Configure toolbar */
void KWordQuizApp::slotConfigureToolbar()
{
    KEditToolbar edit( guiFactory(), this );
    if( edit.exec() )
        createGUI( "kwordquizui.rc" );
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
  emit settingsChanged();
}

void KWordQuizApp::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

/*!
    \fn KWordQuizApp::updateMode(int m)
 */
void KWordQuizApp::updateMode(int m)
{
  if (m_quiz != 0)
    if (KMessageBox::warningContinueCancel(this, i18n("This will restart your quiz. Do you wish to continue?"), QString::null, KStdGuiItem::cont(), "askModeQuiz") != KMessageBox::Continue)
    {
      mode1->setChecked(m_mode == 1);
      mode2->setChecked(m_mode == 2);
      mode3->setChecked(m_mode == 3);
      mode4->setChecked(m_mode == 4);
      mode5->setChecked(m_mode == 5);
      return;
    }

  m_mode = m;
  QString s1 = m_editView -> horizontalHeader()->label(0);
  QString s2 = m_editView -> horizontalHeader()->label(1);

  mode1->setText(i18n("&1 %1 -> %2 In Order").arg(s1).arg(s2));
  mode2->setText(i18n("&2 %1 -> %2 In Order").arg(s2).arg(s1));
  mode3->setText(i18n("&3 %1 -> %2 Randomly").arg(s1).arg(s2));
  mode4->setText(i18n("&4 %1 -> %2 Randomly").arg(s2).arg(s1));
  mode5->setText(i18n("&5 %1 <-> %2 Randomly").arg(s1).arg(s2));

  mode1->setChecked(m_mode == 1);
  mode2->setChecked(m_mode == 2);
  mode3->setChecked(m_mode == 3);
  mode4->setChecked(m_mode == 4);
  mode5->setChecked(m_mode == 5);

  KPopupMenu *popup = mode->popupMenu();
  popup->setItemChecked(0, m_mode == 1);
  popup->setItemChecked(1, m_mode == 2);
  popup->setItemChecked(2, m_mode == 3);
  popup->setItemChecked(3, m_mode == 4);
  popup->setItemChecked(4, m_mode == 5);

  popup->changeItem(0, i18n("&1 %1 -> %2 In Order").arg(s1).arg(s2));
  popup->changeItem(1, i18n("&2 %1 -> %2 In Order").arg(s2).arg(s1));
  popup->changeItem(2, i18n("&3 %1 -> %2 Randomly").arg(s1).arg(s2));
  popup->changeItem(3, i18n("&4 %1 -> %2 Randomly").arg(s2).arg(s1));
  popup->changeItem(4, i18n("&5 %1 <-> %2 Randomly").arg(s1).arg(s2));

  QString s;
  mode->setIcon(locate("data", "kwordquiz/pics/mode" + s.setNum(m_mode) + ".png"));

  switch( m_mode ){
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






