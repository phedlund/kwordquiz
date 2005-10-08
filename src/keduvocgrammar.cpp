/***************************************************************************

             manage grammar parts (articles, conjugation)

    -----------------------------------------------------------------------

    begin         : Sat Nov 27 09:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2004-2005 Peter Hedlund <peter.hedlund@kdemail.net>

    -----------------------------------------------------------------------

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "keduvocgrammar.h"

#include <klocale.h>

KEduVocConjugation::conjug_name_t
KEduVocConjugation::names [] =
{
  { CONJ_SIMPLE_PRESENT,    I18N_NOOP("Simple Present") },
  { CONJ_PRESENT_PROGR,     I18N_NOOP("Present Progressive") },
  { CONJ_PRESENT_PERFECT,   I18N_NOOP("Present Perfect") },

  { CONJ_SIMPLE_PAST,       I18N_NOOP("Simple Past") },
  { CONJ_PAST_PROGR,        I18N_NOOP("Past Progressive") },
  { CONJ_PAST_PARTICIPLE,   I18N_NOOP("Past Participle") },

  { CONJ_FUTURE,            I18N_NOOP("Future") }
};


QStringList KEduVocConjugation::userTenses;


//================================================================

KEduVocComparison::KEduVocComparison (
           const QString &l1,
           const QString &l2,
           const QString &l3
          )
{
  setL1 (l1);
  setL2 (l2);
  setL3 (l3);
}


bool KEduVocComparison::isEmpty() const
{
  return   ls1.stripWhiteSpace().isEmpty()
        && ls2.stripWhiteSpace().isEmpty()
        && ls3.stripWhiteSpace().isEmpty();
}


void KEduVocComparison::clear()
{
  ls1 = "";
  ls2 = "";
  ls3 = "";
}


//=================================================================


KEduVocArticle::KEduVocArticle
          (const QString &fem_def, const QString &fem_indef,
           const QString &mal_def, const QString &mal_indef,
           const QString &nat_def, const QString &nat_indef
          )
{
  setFemale  (fem_def, fem_indef);
  setMale    (mal_def, mal_indef);
  setNatural (nat_def, nat_indef);
}


void KEduVocArticle::setFemale(const QString &def, const QString &indef)
{
  fem_def = def;
  fem_indef = indef;
}


void KEduVocArticle::setMale(const QString &def, const QString &indef)
{
  mal_def = def;
  mal_indef = indef;
}


void KEduVocArticle::setNatural(const QString &def, const QString &indef)
{
  nat_def = def;
  nat_indef = indef;
}


void KEduVocArticle::female(QString &def, QString &indef) const
{
  def = fem_def;
  indef = fem_indef;
}


void KEduVocArticle::male(QString &def, QString &indef) const
{
  def = mal_def;
  indef = mal_indef;
}


void KEduVocArticle::natural(QString &def, QString &indef) const
{
  def = nat_def;
  indef = nat_indef;
}



//==============================================================


int KEduVocConjugation::numEntries() const
{
  return conjugations.size();
}


QValueList<KEduVocTenseRelation> KEduVocConjugation::getRelation ()
{
  QValueList<KEduVocTenseRelation> vec;

  for (int i = 0; i < numInternalNames(); i++) {
    vec.append(KEduVocTenseRelation(names[i].abbrev, i18n(names[i].name)));
  }

  for (int i = 0; i < (int) userTenses.size(); i++) {
    QString s;
    s.setNum(i+1);
    s.insert(0, UL_USER_TENSE);
    vec.append(KEduVocTenseRelation(s, userTenses[i]));
  }

  return vec;
}


void KEduVocConjugation::setTenseNames (QStringList names)
{
  userTenses = names;
}


QString KEduVocConjugation::getName (const QString &abbrev)
{
  if (abbrev.length() >= 2 && abbrev[0] == QString(UL_USER_TENSE)) {
    QString s = abbrev;
    s.remove(0, 1);
    int i = s.toInt() - 1;

    if (i < (int) userTenses.size() )
      return userTenses[i];
    else
      return "";
  }
  else {
    for (int i = 0; i < (int) numInternalNames(); i++)
      if (names[i].abbrev == abbrev) {
        return i18n(names[i].name);
      }
  }

  return "";
}


QString KEduVocConjugation::getName (int idx)
{
  if (idx < numInternalNames() )
    return i18n(names[idx].name);

  else if (idx < numTenses() )
    return userTenses[idx-numInternalNames()];

  else
    return "";
}


QString KEduVocConjugation::getAbbrev (const QString &name)
{
  for (int i = 0; i < (int) userTenses.size(); i++)
    if (userTenses[i] == name) {
      QString s;
      s.setNum(i+1);
      s.insert(0, UL_USER_TENSE);
      return s;
    }

  for (int i = 0; i < (int) numInternalNames(); i++)
    if (names[i].name == name)
      return names[i].abbrev;

  return "";
}


QString KEduVocConjugation::getAbbrev (int idx)
{
  if (idx < numInternalNames() )
    return names[idx].abbrev;

  else if (idx < numTenses() ) {
    QString s;
    s.setNum(idx-numInternalNames()+1);
    s.insert(0, UL_USER_TENSE);
    return s;
  }

  else
    return "";
}


int KEduVocConjugation::numInternalNames()
{
  return sizeof(names) / sizeof(names[0]);
}


int KEduVocConjugation::numTenses()
{
  return numInternalNames()+userTenses.size();
}


QString KEduVocConjugation::getType (int idx)
{
  if (idx >= (int) conjugations.size() )
    return "";

  return conjugations[idx].type;
}


void KEduVocConjugation::setType (int idx, const QString & type)
{
  if (idx >= (int) conjugations.size() )
    return;

  conjugations[idx].type = type;
}


void KEduVocConjugation::cleanUp ()
{
  for (int i = (int)conjugations.size()-1; i >= 0; i--) {
    const conjug_t *ctp = &conjugations[i];
    if (   ctp->pers1_sing.stripWhiteSpace().isEmpty()
        && ctp->pers2_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_m_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_f_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_n_sing.stripWhiteSpace().isEmpty()
        && ctp->pers1_plur.stripWhiteSpace().isEmpty()
        && ctp->pers2_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_m_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_f_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_n_plur.stripWhiteSpace().isEmpty()
       )
     conjugations.erase(conjugations.at(i));
  }
}


bool KEduVocConjugation::isEmpty (int idx)
{
  if (idx < (int) conjugations.size()) {
    const conjug_t *ctp = &conjugations[idx];
    return ctp->pers1_sing.stripWhiteSpace().isEmpty()
        && ctp->pers2_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_m_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_f_sing.stripWhiteSpace().isEmpty()
        && ctp->pers3_n_sing.stripWhiteSpace().isEmpty()
        && ctp->pers1_plur.stripWhiteSpace().isEmpty()
        && ctp->pers2_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_m_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_f_plur.stripWhiteSpace().isEmpty()
        && ctp->pers3_n_plur.stripWhiteSpace().isEmpty();
  }
  return true;
}


#define _GET_CON_(elem, type, default) \
   for (int i = 0; i < (int) conjugations.size(); i++) \
     if (conjugations[i].type == type) \
        return conjugations[i].elem; \
   return default;


bool KEduVocConjugation::pers3SingularCommon(const QString &type) const
{
  _GET_CON_(s3common, type, false);
}


bool KEduVocConjugation::pers3PluralCommon(const QString &type) const
{
  _GET_CON_(p3common, type, false);
}


QString KEduVocConjugation::pers1Singular
  (const QString &type) const
{
  _GET_CON_(pers1_sing, type, "");
}


QString KEduVocConjugation::pers2Singular
  (const QString &type) const
{
  _GET_CON_(pers2_sing, type, "");
}


QString KEduVocConjugation::pers3FemaleSingular
  (const QString &type) const
{
  _GET_CON_(pers3_f_sing, type, "");
}


QString KEduVocConjugation::pers3MaleSingular
  (const QString &type) const
{
  _GET_CON_(pers3_m_sing, type, "");
}


QString KEduVocConjugation::pers3NaturalSingular
  (const QString &type) const
{
  _GET_CON_(pers3_n_sing, type, "");
}


QString KEduVocConjugation::pers1Plural
  (const QString &type) const
{
  _GET_CON_(pers1_plur, type, "");
}


QString KEduVocConjugation::pers2Plural
  (const QString &type) const
{
  _GET_CON_(pers2_plur, type, "");
}


QString KEduVocConjugation::pers3FemalePlural
  (const QString &type) const
{
  _GET_CON_(pers3_f_plur, type, "");
}


QString KEduVocConjugation::pers3MalePlural
  (const QString &type) const
{
  _GET_CON_(pers3_m_plur, type, "");
}


QString KEduVocConjugation::pers3NaturalPlural
  (const QString &type) const
{
  _GET_CON_(pers3_n_plur, type, "");
}


#undef _GET_CON_


#define _SET_CON_(elem, type, str) \
   for (int i = 0; i < (int) conjugations.size(); i++) \
     if (conjugations[i].type == type) { \
       conjugations[i].elem = str; \
       return; \
     } \
   conjug_t ct; \
   ct.type = type; \
   ct.elem = str; \
   conjugations.append(ct);


void KEduVocConjugation::setPers3PluralCommon(const QString &type, bool f)
{
  _SET_CON_(p3common, type, f);
}


void KEduVocConjugation::setPers3SingularCommon(const QString &type, bool f)
{
  _SET_CON_(s3common, type, f);
}


void KEduVocConjugation::setPers1Singular
  (const QString &type, const QString &str)
{
  _SET_CON_(pers1_sing, type, str);
}


void KEduVocConjugation::setPers2Singular
  (const QString &type, const QString &str)
{
  _SET_CON_(pers2_sing, type, str);
}


void KEduVocConjugation::setPers3FemaleSingular
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_f_sing, type, str);
}


void KEduVocConjugation::setPers3MaleSingular
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_m_sing, type, str);
}


void KEduVocConjugation::setPers3NaturalSingular
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_n_sing, type, str);
}


void KEduVocConjugation::setPers1Plural
  (const QString &type, const QString &str)
{
  _SET_CON_(pers1_plur, type, str);
}


void KEduVocConjugation::setPers2Plural
  (const QString &type, const QString &str)
{
  _SET_CON_(pers2_plur, type, str);
}


void KEduVocConjugation::setPers3FemalePlural
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_f_plur, type, str);
}


void KEduVocConjugation::setPers3MalePlural
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_m_plur, type, str);
}


void KEduVocConjugation::setPers3NaturalPlural
  (const QString &type, const QString &str)
{
  _SET_CON_(pers3_n_plur, type, str);
}

#undef _SET_CON_

