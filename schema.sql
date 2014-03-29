-- Special Note: Don't mess with this file; in particular, all the SQL statements
-- must have a blank line after them, or my rather stupid parsing loop will barf.

-- Statements for initializing a new dictionary:

-- Dictionary-specific settings, including database version
create table Settings
  (name text primary key not null,
   value text not null);
   
insert into Settings values ("VersionNumber", "0.4");

-- Phonemes
create table Phoneme
  (id integer primary key not null,
   alpha int unique not null,
   name text unique not null on conflict ignore,
   notes text not null default "",
   check (alpha >= -1));

-- Some triggers to keep the alphabet list consistent without my having to write
-- over 9000 queries in the code and dealing with all the accompanying exceptions
create trigger AlphaDelete 
  after delete on Phoneme for each row
  begin
    update Phoneme set alpha = alpha - 1 where alpha > old.alpha;
  end;

create trigger AlphaMoveBefore 
  before update of alpha on Phoneme for each row
  when old.alpha - new.alpha == 1 or new.alpha - old.alpha == 1
  begin
    update Phoneme set alpha = -1 where alpha == new.alpha;
  end;

create trigger AlphaMoveAfter 
  after update of alpha on Phoneme for each row
  when old.alpha - new.alpha == 1 or new.alpha - old.alpha == 1
  begin
    update Phoneme set alpha = old.alpha where alpha == -1;
  end;

create table PhonemeSpelling
  (phonemeID int not null,
   spelling text not null,
   primary key (phonemeID, spelling) on conflict ignore,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

-- Phoneme Features
create table PhonemeFeatureDef
  (name text not null primary key on conflict ignore,
   parentName text,
   parentValue text,
   displayType text not null,
   check (parentName != name),
   check (displayType in ("Colon", "Prefix", "Suffix", "Before", "After", "Solo")),
   foreign key (parentName, parentValue) references PhonemeSubfeature(name, value)
     on update cascade on delete set null);

-- All possible feature/value pairs
create table PhonemeSubfeature
  (name text not null,
   value text not null,
   primary key (name, value) on conflict ignore,
   foreign key (name) references PhonemeFeatureDef(name)
     on update cascade on delete cascade);

-- Feature/value pairs associated with particular phonemes
create table PhonemeFeatureSet
  (phonemeID int not null,
   feature text not null,
   value text not null,
   primary key (phonemeID, feature) on conflict replace,
   foreign key (phonemeID) references Phoneme(id) on delete cascade,
   foreign key (feature, value) references PhonemeSubfeature(name, value)
     on update cascade on delete cascade);

-- Auto-delete feature from phoneme if parent feature isn't present
create trigger PhonemeFeatureCheck
  after insert on PhonemeFeatureSet for each row
  when (exists (select name from PhonemeFeatureDef 
                where name == new.feature and parentName not null
                  and parentName != "") and
        exists (select parentName, parentValue from PhonemeFeatureDef 
                where name == new.feature
                except
                select feature, value from PhonemeFeatureSet 
                where phonemeID == new.phonemeID))
  begin
    delete from PhonemeFeatureSet 
    where phonemeID == new.phonemeID and feature == new.feature;
  end;

-- Delete child features from phoneme when parent feature is deleted
create trigger PhonemeFeatureDelCheck
  after delete on PhonemeFeatureSet for each row
  begin
    delete from PhonemeFeatureSet
    where phonemeID == old.phonemeID and 
      exists (select name from PhonemeFeatureDef
              where name == feature and parentName == old.feature and parentValue == old.value);
  end;

create table NaturalClassPhon
  (bundleID integer primary key not null,
   name text unique not null);

-- Feature/value pairs associated with phoneme natural classes
create table FeatureBundlePhon
  (id int not null,
   feature text not null,
   value text not null,
   primary key (id, feature) on conflict replace,
   foreign key (id) references NaturalClassPhon(bundleID) on delete cascade,
   foreign key (feature, value) references PhonemeSubfeature(name, value)
     on update cascade on delete cascade);

-- Goddammit SQLite, why do you suck so much
create trigger DeleteObsoleteFeatureBundlesGodFuckingDammit
  before delete on NaturalClassPhon for each row
  begin
    delete from FeatureBundlePhon where id == old.bundleID;
  end;

-- A view showing which phonemes are of which classes
create view PhonClassList as
  select id, NaturalClassPhon.name as class
  from Phoneme, NaturalClassPhon
  where not exists (select feature, value from FeatureBundlePhon 
                    where FeatureBundlePhon.id == bundleID
                    except
                    select feature, value from PhonemeFeatureSet 
                    where phonemeID == Phoneme.id);

-- Suprasegmentals
create table Suprasegmental
  (id integer primary key not null,
   name text unique not null on conflict ignore,
   domain int not null default 0,
   repType int not null default 0,
   repText text not null default "",
   spellType int not null default 0,
   spellText text not null default "",
   notes text not null default "",
   check (domain in (0, 1)),
   check (repType >= 0 and repType < 8),
   check (spellType >= 0 and spellType < 8));

create table SupraApplies
  (supraID int not null,
   phonemeID int not null,
   primary key (supraID, phonemeID) on conflict ignore,
   foreign key (supraID) references Suprasegmental(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

-- Phonotactics
-- For defining phonotactics of a form similar to:
-- Plosive/Fricative + Liquid/Nasal/Rhotic + Glide
-- Each line like the above has a unique id; each segment joined with + has a unique ind
-- Multiple classes at the same id + ind will be ORed (not ANDed).
create table LegalOnset
  (id int not null,
   ind int not null,
   class int not null,
   primary key (id, ind, class) on conflict ignore,
   foreign key (class) references NaturalClassPhon(bundleID) on delete cascade);

create trigger OnsetDelete
  after delete on LegalOnset for each row
  when not exists (select id from LegalOnset where id == old.id)
  begin
    update LegalOnset set id = id - 1 where id > old.id;
  end;

create table LegalPeak
  (id int not null,
   ind int not null,
   class int not null,
   primary key (id, ind, class) on conflict ignore,
   foreign key (class) references NaturalClassPhon(bundleID) on delete cascade);

create trigger PeakDelete
  after delete on LegalPeak for each row
  when not exists (select id from LegalPeak where id == old.id)
  begin
    update LegalPeak set id = id - 1 where id > old.id;
  end;

create table LegalCoda
  (id int not null,
   ind int not null,
   class int not null,
   primary key (id, ind, class) on conflict ignore,
   foreign key (class) references NaturalClassPhon(bundleID) on delete cascade);

create trigger CodaDelete
  after delete on LegalCoda for each row
  when not exists (select id from LegalCoda where id == old.id)
  begin
    update LegalCoda set id = id - 1 where id > old.id;
  end;

-- Words
create table Word
  (id integer primary key not null,
   name text not null,
   definition text not null);

-- Every syllable has a unique syllNum.  
-- There is no syllable table; they are just the collection of stuff with the same syllNum.
create table SyllableSupra
  (wordID int not null,
   syllNum int not null,
   supraID int not null,
   primary key (wordID, syllNum, supraID),
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- Ind specifies the order of the phonemes in the onset.
create table Onset
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (wordID, syllNum, ind),
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

-- Supras associated with inds defined in the above table
create table OnsetSupra
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (wordID, syllNum, ind, supraID),
   foreign key (wordID, syllNum, ind) references Onset on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- These all work the same way.
create table Peak
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (wordID, syllNum, ind),
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table PeakSupra
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (wordID, syllNum, ind, supraID),
   foreign key (wordID, syllNum, ind) references Peak on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

create table Coda
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (wordID, syllNum, ind),
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table CodaSupra
  (wordID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (wordID, syllNum, ind, supraID),
   foreign key (wordID, syllNum, ind) references Coda on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- Word Features
-- These all work the same way as the phoneme features tables/triggers/views.
create table WordFeatureDef
  (name text not null primary key on conflict ignore,
   parentName text,
   parentValue text,
   displayType text not null,
   check (parentName != name),
   check (displayType in ("Colon", "Prefix", "Suffix", "Before", "After", "Solo")),
   foreign key (parentName, parentValue) references WordSubfeature(name, value)
     on update cascade on delete set null);

create table WordSubfeature
  (name text not null,
   value text not null,
   primary key (name, value) on conflict ignore,
   foreign key (name) references WordFeatureDef(name)
     on update cascade on delete cascade);

create table WordFeatureSet
  (wordID int not null,
   feature text not null,
   value text not null,
   primary key (wordID, feature) on conflict replace,
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (feature, value) references WordSubfeature(name, value)
     on update cascade on delete cascade);

create trigger WordFeatureCheck
  after insert on WordFeatureSet for each row
  when (exists (select name from WordFeatureDef 
                where name == new.feature and parentName not null
                  and parentName != "") and
        exists (select parentName, parentValue from WordFeatureDef 
                where name == new.feature
                except
                select feature, value from WordFeatureSet 
                where wordID == new.wordID))
  begin
    delete from WordFeatureSet 
    where wordID == new.wordID and feature == new.feature;
  end;

create trigger WordFeatureDelCheck
  after delete on WordFeatureSet for each row
  begin
    delete from WordFeatureSet
    where wordID == old.wordID and 
      exists (select name from WordFeatureDef
              where name == feature and parentName == old.feature and parentValue == old.value);
  end;

create table NaturalClassWord
  (bundleID integer primary key not null,
   name text unique not null);

create table FeatureBundleWord
  (id int not null,
   feature text not null,
   value text not null,
   primary key (id, feature) on conflict replace,
   foreign key (id) references NaturalClassWord(bundleID) on delete cascade,
   foreign key (feature, value) references WordSubfeature(name, value)
     on update cascade on delete cascade);

create view WordClassList as
  select id, NaturalClassWord.name as class
  from Word, NaturalClassWord
  where not exists (select feature, value from FeatureBundleWord 
                    where FeatureBundleWord.id == bundleID
                    except
                    select feature, value from WordFeatureSet 
                    where wordID == Word.id);

-- So that the model for the word page can be populated using "select * from WordPageTable".
-- Five lines of SQL to save me a headache later, when I inevitably forget how 
-- group_concat and the group by clause work.
create view ClassConcatView as
  select Word.id as id, Word.name as name, group_concat(class) as classlist
  from Word, WordClassList
  where Word.id == WordClassList.id
  group by WordClassList.id;

create view WordPageTable as
  select Word.id as id, Word.name as name, classlist
  from Word left outer join ClassConcatView on Word.id == ClassConcatView.id;
  
-- Morphemes
create table Morpheme
  (id integer primary key not null,
   name text not null,
   notes text not null);
  
-- Morpheme phonemic representations
create table MorphemeSyllableSupra
  (morphID int not null,
   syllNum int not null,
   supraID int not null,
   primary key (morphID, syllNum, supraID),
   foreign key (morphID) references Morpheme(id) on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- Ind specifies the order of the phonemes in the onset.
create table MorphemeOnset
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (morphID, syllNum, ind),
   foreign key (morphID) references Morpheme(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

-- Supras associated with inds defined in the above table
create table MorphemeOnsetSupra
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (morphID, syllNum, ind, supraID),
   foreign key (morphID, syllNum, ind) references MorphemeOnset on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- These all work the same way.
create table MorphemePeak
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (morphID, syllNum, ind),
   foreign key (morphID) references Morpheme(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table MorphemePeakSupra
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (morphID, syllNum, ind, supraID),
   foreign key (morphID, syllNum, ind) references MorphemePeak on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

create table MorphemeCoda
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (morphID, syllNum, ind),
   foreign key (morphID) references Morpheme(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table MorphemeCodaSupra
  (morphID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (morphID, syllNum, ind, supraID),
   foreign key (morphID, syllNum, ind) references MorphemeCoda on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);
  
-- Morpheme features
create table MorphemeFeatureDef
  (name text not null primary key on conflict ignore,
   parentName text,
   parentValue text,
   displayType text not null,
   check (parentName != name),
   check (displayType in ("Colon", "Prefix", "Suffix", "Before", "After", "Solo")),
   foreign key (parentName, parentValue) references MorphemeSubfeature(name, value)
     on update cascade on delete set null);

create table MorphemeSubfeature
  (name text not null,
   value text not null,
   primary key (name, value) on conflict ignore,
   foreign key (name) references MorphemeFeatureDef(name)
     on update cascade on delete cascade);

create table MorphemeFeatureSet
  (morphID int not null,
   feature text not null,
   value text not null,
   primary key (morphID, feature) on conflict replace,
   foreign key (morphID) references Morpheme(id) on delete cascade,
   foreign key (feature, value) references MorphemeSubfeature(name, value)
     on update cascade on delete cascade);

create trigger MorphemeFeatureCheck
  after insert on MorphemeFeatureSet for each row
  when (exists (select name from MorphemeFeatureDef 
                where name == new.feature and parentName not null
                  and parentName != "") and
        exists (select parentName, parentValue from MorphemeFeatureDef 
                where name == new.feature
                except
                select feature, value from MorphemeFeatureSet 
                where morphID == new.morphID))
  begin
    delete from MorphemeFeatureSet 
    where morphID == new.morphID and feature == new.feature;
  end;

create trigger MorphemeFeatureDelCheck
  after delete on MorphemeFeatureSet for each row
  begin
    delete from MorphemeFeatureSet
    where morphID == old.morphID and 
      exists (select name from MorphemeFeatureDef
              where name == feature and parentName == old.feature and parentValue == old.value);
  end;

create table NaturalClassMorpheme
  (bundleID integer primary key not null,
   name text unique not null);

create table FeatureBundleMorpheme
  (id int not null,
   feature text not null,
   value text not null,
   primary key (id, feature) on conflict replace,
   foreign key (id) references NaturalClassMorpheme(bundleID) on delete cascade,
   foreign key (feature, value) references MorphemeSubfeature(name, value)
     on update cascade on delete cascade);

create view MorphemeClassList as
  select id, NaturalClassMorpheme.name as class
  from Morpheme, NaturalClassMorpheme
  where not exists (select feature, value from FeatureBundleMorpheme 
                    where FeatureBundleMorpheme.id == bundleID
                    except
                    select feature, value from MorphemeFeatureSet 
                    where morphID == Morpheme.id);

-- So that the model for the morpheme page can be populated using "select * from MorphemePageTable".
-- Five lines of SQL to save me a headache later, when I inevitably forget how 
-- group_concat and the group by clause work.
create view MorphemeClassConcatView as
  select Morpheme.id as id, Morpheme.name as name, group_concat(class) as classlist
  from Morpheme, MorphemeClassList
  where Morpheme.id == MorphemeClassList.id
  group by MorphemeClassList.id;

create view MorphemePageTable as
  select Morpheme.id as id, Morpheme.name as name, classlist
  from Morpheme left outer join MorphemeClassConcatView on Morpheme.id == MorphemeClassConcatView.id;
  
-- Hooking morphemes up to words
create table HasMorpheme
  (wordID int not null,
   ind int not null,
   morphID int not null,
   head int not null,
   primary key (wordID, ind) on conflict replace,
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (morphID) references Morpheme(id) on delete cascade,
   check (head == 0 or head == 1));
  
create trigger MorphemeDelete
  after delete on HasMorpheme for each row
  begin
    update HasMorpheme set ind = ind - 1
    where wordID == old.wordID and ind > old.ind;
  end;
  
-- Derivations and inflections
create table Paradigm
  (id integer primary key not null,
   name text unique not null);
  
create table Form
  (id integer primary key not null,
   paradigmID int,
   input int,
   name text not null,
   foreign key (paradigmID) references Paradigm(id) on delete cascade,
   foreign key (input) references InputForm(id) on delete cascade);
  
create table WordIsForm
  (wordID int not null,
   formID int not null,
   inputWord int,
   inputMorpheme int,
   primary key (wordID, formID) on conflict ignore,
   foreign key (wordID) references Word(id) on delete cascade,
   foreign key (formID) references Form(id) on delete cascade,
   foreign key (inputWord) references Word(id) on delete cascade,
   foreign key (inputMorpheme) references Morpheme(id) on delete cascade);
  
create table InflectionalRule
  (id integer primary key not null,
   formID int not null,
   wordInput int,
   morphemeInput int,
   formInput int,
   foreign key (formID) references Form(id) on delete cascade,
   foreign key (wordInput) references NaturalClassWord(id) on delete cascade,
   foreign key (morphemeInput) references NaturalClassMorpheme(id) on delete cascade,
   foreign key (formInput) references Form(id) on delete cascade);
  
-- InflectionalRules contain a matching sequence for matching input to. A null
-- classID indicates that the position can have any number of phonemes.
create table RuleMatch
  (ruleID int not null,
   ind int not null,
   classID int,
   primary key (ruleID, ind) on conflict replace,
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   foreign key (classID) references NaturalClassPhon(bundleID) on delete cascade);
  
create trigger RuleMatchDelete
  after delete on RuleMatch for each row
  begin
    update RuleMatch set ind = ind - 1 
    where ruleID == old.ruleID and ind > old.ind;
  end;
  
-- RuleMatches have selections deliniated by parentheses.
create table RuleMatchSelection
  (ruleID int not null,
   ind int not null,
   starting int not null,
   ending int not null,
   primary key (ruleID, ind) on conflict replace,
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   check (starting <= ending));
  
-- InflectionalRules also have phonemic sequences for the output.
create table RuleSyllableSupra
  (ruleID int not null,
   syllNum int not null,
   supraID int not null,
   primary key (ruleID, syllNum, supraID),
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- Ind specifies the order of the phonemes in the onset.
create table RuleOnset
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (ruleID, syllNum, ind),
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

-- Supras associated with inds defined in the above table
create table RuleOnsetSupra
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (ruleID, syllNum, ind, supraID),
   foreign key (ruleID, syllNum, ind) references RuleOnset on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

-- These all work the same way.
create table RulePeak
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (ruleID, syllNum, ind),
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table RulePeakSupra
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (ruleID, syllNum, ind, supraID),
   foreign key (ruleID, syllNum, ind) references RulePeak on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);

create table RuleCoda
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   phonemeID int not null,
   primary key (ruleID, syllNum, ind),
   foreign key (ruleID) references InflectionalRule(id) on delete cascade,
   foreign key (phonemeID) references Phoneme(id) on delete cascade);

create table RuleCodaSupra
  (ruleID int not null,
   syllNum int not null,
   ind int not null,
   supraID int not null,
   primary key (ruleID, syllNum, ind, supraID),
   foreign key (ruleID, syllNum, ind) references RuleCoda on delete cascade,
   foreign key (supraID) references Suprasegmental(id) on delete cascade);
  
-- Table to represent the locations of the back references in the above.
create table RuleReference
  (ruleID int not null,
   refNum int not null,
   location int not null,
   primary key (ruleID, refNum) on conflict replace,
   foreign key (ruleID) references InflectionalRule(id) on delete cascade);