-- Statements to update the database from version 0.3 to version 0.4.

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
   name text not null,
   foreign key (paradigmID) references Paradigm(id) on delete cascade);
  
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
   morphemeID int,
   foreign key (formID) references Form(id) on delete cascade,
   foreign key (wordInput) references NaturalClassWord(id) on delete cascade,
   foreign key (morphemeInput) references NaturalClassMorpheme(id) on delete cascade,
   foreign key (formInput) references Form(id) on delete cascade,
   foreign key (morphemeID) references Morpheme(id) on delete set null);
  
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
   starting int not null,
   ending int not null,
   primary key (ruleID, starting, ending),
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
  
update Settings set value = "0.4" where name == "VersionNumber";

insert into Settings values ("NeedsMorphemeUpdateDialog", "true");