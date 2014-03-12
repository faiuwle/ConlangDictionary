-- Schema for .features files.

create table FeatureDef
  (name text not null primary key on conflict ignore,
   parentName text,
   parentValue text,
   displayType text not null,
   check (parentName != name),
   check (displayType in ("Colon", "Prefix", "Suffix", "Before", "After", "Solo")));

-- All possible feature/value pairs
create table Subfeature
  (name text not null,
   value text not null,
   primary key (name, value) on conflict ignore,
   foreign key (name) references FeatureDef(name)
     on update cascade on delete cascade);
   
create table NaturalClass
  (bundleID integer primary key not null,
   name text unique not null);

-- Feature/value pairs associated with natural classes
create table FeatureBundle
  (id int not null,
   feature text not null,
   value text not null,
   primary key (id, feature) on conflict replace,
   foreign key (id) references NaturalClass(bundleID) on delete cascade,
   foreign key (feature, value) references Subfeature(name, value)
     on update cascade on delete cascade);