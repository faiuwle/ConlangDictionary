#ifndef CONST_H
#define CONST_H

// Version codes for obsolete savefiles
#define V0_1 0
#define V0_2 1

// Canonical database settings strings
#define VERSION_NUMBER "VersionNumber"
#define LANGUAGE_NAME "LanguageName"
#define USE_PHONOTACTICS "UsePhonotactics"
#define ONSET_REQUIRED "OnsetRequired"
#define IGNORED_CHARACTERS "IgnoredCharacters"
#define SQUARE_BRACKETS "SquareBrackets"
#define USE_UNICODE "UseUnicode"

// Codes for phonotactics information
#define ONSET 0
#define PEAK 1
#define CODA 2

// Codes for filtering feature models
#define UNIVALENT 0
#define BINARY 1
#define GROUPS 2
#define ALLSUBS -1

// Domain codes for multitudinous database feature functions
#define PHONEME 0
#define WORD 1

// Special code for initializing FB dialog as for natural classes
#define NATURAL_CLASS -1

// Feature display codes for strings stored in database
#define DISPLAY_COLON 0
#define DISPLAY_PREFIX 1
#define DISPLAY_SUFFIX 2
#define DISPLAY_BEFORE 3
#define DISPLAY_AFTER 4
#define DISPLAY_SOLO 5

// Suprasegmental codes stored in database
#define SUPRA_DOMAIN_PHON 0
#define SUPRA_DOMAIN_SYLL 1
#define TYPE_ACUTE 0
#define TYPE_GRAVE 1
#define TYPE_CIRCUMFLEX 2
#define TYPE_DIARESIS 3
#define TYPE_MACRON 4
#define TYPE_BEFORE 5
#define TYPE_AFTER 6
#define TYPE_DOUBLED 7

// Macros for making my life easier and involving less repetitive typing
#define UNICODE(n) QString::fromUtf8 (n)
#define MINUS_SIGN UNICODE("\u2212")

#include <QStringList>

// Strings for legal database values for feature display
// They correspond to the DISPLAY_* indices.
const QStringList displayType =
  QStringList () << "Colon" << "Prefix" << "Suffix" << "Before" << "After" << "Solo";
// Strings corresponding to the SUPRA_DOMAIN_* indices
const QStringList supraDomain = QStringList () << "Phonemes" << "Syllables";
// Strings corresponding to the TYPE_* indices
const QStringList supraType =
  QStringList () << "Acute Accent" << "Grave Accent" << "Circumflex" << "Diaresis/Umlaut"
                 << "Macron" << "Text Preceding" << "Text Following" << "Doubling";
  
// Fields for SIL Toolbox export
const QStringList wordFields =
  QStringList () << "Word" << "Type" << "Subtype" << "Definition";
  
// Table of characters with particular diacritics; used for phonotactics parsing
// Conviently, the rows of the table also correspond to the TYPE_* indices involving diacritics
const QStringList diacriticChars =
  QStringList () << UNICODE("Á") << UNICODE("É") << UNICODE("Í") << UNICODE("Ó")
                 << UNICODE("Ú") << UNICODE("Ý") << UNICODE("á") << UNICODE("é")
                 << UNICODE("í") << UNICODE("ó") << UNICODE("ú") << UNICODE("ý")
                 << UNICODE("À") << UNICODE("È") << UNICODE("Ì") << UNICODE("Ò")
                 << UNICODE("Ù") << UNICODE("Ỳ") << UNICODE("à") << UNICODE("è")
                 << UNICODE("ì") << UNICODE("ò") << UNICODE("ù") << UNICODE("ỳ")
                 << UNICODE("Â") << UNICODE("Ê") << UNICODE("Î") << UNICODE("Ô")
                 << UNICODE("Û") << UNICODE("Ŷ") << UNICODE("â") << UNICODE("ê")
                 << UNICODE("î") << UNICODE("ô") << UNICODE("û") << UNICODE("ŷ")
                 << UNICODE("Ä") << UNICODE("Ë") << UNICODE("Ï") << UNICODE("Ö")
                 << UNICODE("Ü") << UNICODE("Ÿ") << UNICODE("ä") << UNICODE("ë")
                 << UNICODE("ï") << UNICODE("ö") << UNICODE("ü") << UNICODE("ÿ")
                 << UNICODE("Ā") << UNICODE("Ē") << UNICODE("Ī") << UNICODE("Ō")
                 << UNICODE("Ū") << UNICODE("Ȳ") << UNICODE("ā") << UNICODE("ē")
                 << UNICODE("ī") << UNICODE("ō") << UNICODE("ū") << UNICODE("ȳ");
                 
// Characters without diacritics corresponding to above
const QStringList plainChars = 
  QStringList () << "A" << "E" << "I" << "O" << "U" << "Y" << "a" << "e" << "i"
                 << "o" << "u" << "y";

// Corresponding XSAMPA characters for XSAMPA->IPA conversion
const QStringList xsampa =
  QStringList () << "d`" << "f\\" << "h\\" << "j\\" << "l`" << "l\\" << "n`"
                 <<  "p\\" << "r`" << "r\\" << "r\\`" << "s`" << "s\\" << "t`"
                 << "v\\" << "x\\" << "z`" << "z\\" << "A" << "B" << "C" << "D"
                 << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M"
                 << "N" << "O" << "P" << "Q" << "R" << "S" << "T" << "U" << "V"
                 << "W" << "X" << "Y" << "Z" << "B\\" << "G\\" << "H\\" << "I\\"
                 << "J\\" << "K\\" << "L\\" << "M\\" << "N\\" << "O\\" << "R\\"
                 << "U\\" << "X\\" << "@" << "@\\" << "1" << "2" << "3" << "3\\"
                 << "4" << "5" << "6" << "7" << "8" << "9" << "&" << "?" << "?\\"
                 << "^" << "!" << "!\\" << ")" << "\"" << "%" << "_a" << "_A"
                 << "_B" << "_c" << "_d" << "_e" << "_E" << "_F" << "_G"
                 << "_h" << "_H" << "_j" << "_k" << "_l" << "_L" << "_m" << "_M"
                 << "_n" << "_N" << "_o" << "_O" << "_q" << "_r" << "_R" << "_t"
                 << "_v" << "_w" << "_x" << "_X" << "_0" << "_\"" << "_+" << "_-"
                 << "_/" << "_;" << "_}" << "=" << "~" << ":" << ":\\" << "\'"
                 << "{" << "}" << "_>" << "b_<" << "d_<" << "J\\_<" << "g_<"
                 << "G\\_<";
 
// Corresponding IPA characters for XSAMPA->IPA conversion
const QStringList ipa = 
  QStringList () << UNICODE("ɖ") << UNICODE("\u02A9") << UNICODE("ɦ") << UNICODE("ʝ")
                 << UNICODE("ɭ") << UNICODE("ɺ") << UNICODE("ɳ") << UNICODE("ɸ")
                 << UNICODE("ɽ") << UNICODE("ɹ") << UNICODE("ɻ") << UNICODE("ʂ")
                 << UNICODE("ɕ") << UNICODE("ʈ") << UNICODE("ʋ") << UNICODE("ɧ")
                 << UNICODE("ʐ") << UNICODE("ʑ") << UNICODE("ɑ") << UNICODE("β")
                 << UNICODE("ç") << UNICODE("ð") << UNICODE("ɛ") << UNICODE("ɱ")
                 << UNICODE("ɣ") << UNICODE("ɥ") << UNICODE("ɪ") << UNICODE("ɲ")
                 << UNICODE("ɬ") << UNICODE("ʎ") << UNICODE("ɯ") << UNICODE("ŋ")
                 << UNICODE("ɔ") << UNICODE("ʋ") << UNICODE("ɒ") << UNICODE("ʁ")
                 << UNICODE("ʃ") << UNICODE("θ") << UNICODE("ʊ") << UNICODE("ʌ")
                 << UNICODE("ʍ") << UNICODE("χ") << UNICODE("ʏ") << UNICODE("ʒ")
                 << UNICODE("ʙ") << UNICODE("ɢ") << UNICODE("ʜ") << UNICODE("ᵻ")
                 << UNICODE("ɟ") << UNICODE("ɮ") << UNICODE("ʟ") << UNICODE("ɰ")
                 << UNICODE("ɴ") << UNICODE("ʘ") << UNICODE("ʀ") << UNICODE("ᵿ")
                 << UNICODE("ħ") << UNICODE("ə") << UNICODE("ɘ") << UNICODE("ɨ")
                 << UNICODE("ø") << UNICODE("ɜ") << UNICODE("ɞ") << UNICODE("ɾ")
                 << UNICODE("ɫ") << UNICODE("ɐ") << UNICODE("ɤ") << UNICODE("ɵ")
                 << UNICODE("œ") << UNICODE("ɶ") << UNICODE("ʔ") << UNICODE("ʕ")
                 << UNICODE("↑") << UNICODE("↓") << UNICODE("!") << UNICODE("\u0361")
                 << UNICODE("ˈ") << UNICODE("ˌ") << UNICODE("\u033A") << UNICODE("\u031F") 
                 << UNICODE("\u02E9") << UNICODE("\u031C") << UNICODE("\u032A") 
                 << UNICODE("\u02E4") << UNICODE("\u033C") << UNICODE("\u02E5") + UNICODE("\u02E9")
                 << UNICODE("\u02E0") << UNICODE("\u02B0") << UNICODE("\u02E6")
                 << UNICODE("\u02B2") << UNICODE("\u0330") << UNICODE("\u02E1")
                 << UNICODE("\u02E8") << UNICODE("\u033B") << UNICODE("\u02E7")
                 << UNICODE("\u207F") << UNICODE("\u033C") << UNICODE("\u031E")
                 << UNICODE("\u0339") << UNICODE("\u0319") << UNICODE("\u031D")
                 << UNICODE("\u02E9") + UNICODE("\u02E5") << UNICODE("\u0324")
                 << UNICODE("\u032C") << UNICODE("\u02B7") << UNICODE("\u033D")
                 << UNICODE("\u0306") << UNICODE("\u0325") << UNICODE("̈'")
                 << UNICODE("\u031F") << UNICODE("\u0319") << UNICODE("\u02E9") + UNICODE("\u02E5")
                 << UNICODE("\u207F") << UNICODE("\u031A") << UNICODE("\u0329")
                 << UNICODE("\u0303") << UNICODE("ː") << UNICODE("\u02D1")
                 << UNICODE("\u02B2") << UNICODE("æ") << UNICODE("ʉ") << UNICODE("ʼ")
                 << UNICODE("ɓ") << UNICODE("ɗ") << UNICODE("ʄ") << UNICODE("ɠ")
                 << UNICODE("ʛ");
                 
// typedefed structs for edit phonology dialog
typedef struct s_Phoneme  {
  QString name;
  QStringList supras;
} Phoneme;

typedef struct s_Syllable  {
  QList<Phoneme> onset;
  QList<Phoneme> peak;
  QList<Phoneme> coda;
  QStringList supras;
} Syllable;

typedef struct s_Suprasegmental  {
  QString name;
  int domain;
  QStringList applicablePhonemes;
  int type;
  QString text;
} Suprasegmental;

#endif