# Vyasa

This project is intended to parse Sanskrit Texts with the purposes of discovering quantifiable linguistic patterns and paradigms to be evaluated and compared. All current iterations are proofs of concept, and Rig Veda Suktas are used as base cases for testing. There is a template attached in `/Hymns`.

Sanskritists and Academics alike take note of linguistic patterns within Indian Religious Poetry and other texts, and make claim to them corresponding to Theological thought - whether Theologians and Practictioners of the traditions accept such interpretations is up in the air, although I would argue that such analysis does not detract from the claims of Śruti and Apauruṣeya - arguably, it favors them. In turn, this project is intended to serve the interests of both Theologian-Practictioners and Scholars alike, providing the tools to reveal patterns in the manifested hymnology of the Veda and Āgama.  

## Examples

### [R.V. 10.125.4.d]

**अमन्तवो मां त उप कषियन्ति शरुधिश्रुत शरद्धिवं ते वदामि ||**

_amantavo māṃ ta upa kṣiyanti **śrudhiśruta śraddhivaṃ te vadāmi** ||_

Within the bolded hemistich is contained "the hidden message of the goddess Speech to the poet, the extreme phonetic figure, is an exhaustive classification of the speech sounds of the Vedic language, with one example of each class: the vowels _a_, _i_, _u_ and a single icon each of the oppositions of quantity (_a_ : _ā_) and nasalization (_a_ : _aṃ_); a single sibilant _ś_; a single liquid _r_; a single semi-vowel (glide) _v_; a single nasal _m_; and a single order of stops, _t_ _d_ _dh_ as tokens of the oppositions of voicing (_t_ : _d_) and aspiration or murmur (_d_ : _dh_)."[^1]

### [R.V. 10.129.7]

**इयं विस्र्ष्टिर्यत आबभूव [यदि वा दधे यदि वा न] |**

**यो अस्याध्यक्षः परमे वयोमन सो अङग वेद यदि वा नवेद ||**


_iyaṃ visṛṣṭiryata ābabhūva **[yadi vā dadhe yadi vā na]** |_

_yo asyādhyakṣaḥ parame vyoman **[so aṅgha veda yadi vā na veda]** ||_


"Line 7b _yadi vā dadhe yadi vā na_, has only nine syllables, two syllables shy of the normal eleven syllable line... The line stops short,as if the poet had suddenly stepped on his own metrical shoe-laces. The rhythmic incompleteness of the line stands out particularly strongly because it could so easily be corrected... Whether created by accident or intention, this metrically unresolved cadence is a verbal image of the unresolved cosmogony. Moreover, the metrically incomplete line anticipates the hymn's syntactically incomplete conclusion, 7d _so aṅgha veda yadi vā na veda_. This line ends with a subordinate clause, for which there is no main clause: "he surely knows. Or if he does not know ... ?" Thus, the metrical and then the syntactic incompleteness of the two lines act as metaphors for the unconcluded cosmogony."[^2]

## Current Functionalities
<sup><sub>Listed In No Particular Order</sub></sup>
+ JSON and CSV Exports
+ Devanagari and IAST Syllable Alignment
+ Swara Frequency and Weight Analysis
+ Letter/Varna Frequency
+ Sandhi Analysis (Known Bugs, Planned Improvements)
+ Phoneme Class Identification
+ Baseline Verse Meter and Irregularity Identification (Known Bugs)
+ Confidence Value
+ Dot Product & Vector Magnitude -> Cosine Similarity
+ Phoneme, Varna, Swara Searching
+ Cross Hymn/Verse Comparison

## In Progress Functionalities
- N-Gram Phonetic Analysis

## Planned Functionalities and Enhancements
- Character-Level Matrix Analysis
- Meter-Sandhi Proportionality Detection
- Metadata Improvements
- Light/Heavy Syllable Pattern Finding
- Levenshtein Distance

## Structure

`Hymn -> Verse -> Word -> [Syllable -> Onset/Nucleus/Coda] | [Letter -> Svara/Phoneme_Class]`

# Usage

## Initializatiion
Any `.txt` will be referred to as a _Hymn_. Sample Hymns are provided in `/Hymns`. Sample `Hymn.txt` is provided as a template for your inputs.

Running the program will need to be in a directory with `/Hymns` and `/HymnExports`.

The program will require an integer of Hymns to be inputted selection of each Hymn. Afterwards they will be parsed and exported into HymnExports with the following:

+ `Hymn.json` (JSON Export)
+ `Hymn.csv` (Hymn Metadata, Verses (Meter), Words, Syllables (Onset | Nucleus | Coda), Letters (Svara | Phoneme Class))
+ `Hymn_Analysis.csv` (Letter Frequency, Svara Frequency, Phoneme Class Frequency)
+ `Hymn_Sandhi.csv` (Potential Sandhi Instances)
+ `Hymn_Similarty.csv` (Hymn Phoneme, Svara, and Metrical similarities with other indexed Hymns)

## Search and Compare
Following will be a prompt with the following commands enumerated:

+ `search` 
+ `compare verse`
+ `compare hymns`
+ `compare query`
+ `list`
+ `help` 
+ `back`
+ `exit`


Type `help` anywhere for further instructions. Contact with any issues.

### Attributions:

+ Lohmann, N. (2025). JSON for Modern C++ (Version 3.12.0) [Computer software]. https://github.com/nlohmann

[^1]: Calvert Watkins, _How to Kill a Dragon: Aspects of Indo-European Poetics_. (New York, Oxford University Press, 1995), 111.
[^2]: Brereton, Joel P. "The Nasadiya Sukta: Edifying Puzzlement: Ṛgveda 10.129 and the Uses of Enigma." Journal of the American Oriental Society 119, no. 2 (1999): 248–60. https://doi.org/10.2307/606109.
