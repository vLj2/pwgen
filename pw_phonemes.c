/*
 * pw_phonemes.c --- generate secure passwords using phoneme rules
 *
 * Copyright (C) 2001 by Theodore Ts'o
 * 
 * This file may be distributed under the terms of the GNU Public
 * License.
 */

#include "pwgen.h"

struct pw_element elements[] = {
	{ "a",	VOWEL },
	{ "ae", VOWEL | DIPTHONG },
	{ "ah",	VOWEL | DIPTHONG },
	{ "ai", VOWEL | DIPTHONG },
	{ "b",  CONSONANT },
	{ "c",	CONSONANT },
	{ "ch", CONSONANT | DIPTHONG },
	{ "d",	CONSONANT },
	{ "e",	VOWEL },
	{ "ee", VOWEL | DIPTHONG },
	{ "ei",	VOWEL | DIPTHONG },
	{ "f",	CONSONANT },
	{ "g",	CONSONANT },
	{ "gh", CONSONANT | DIPTHONG | NOT_FIRST },
	{ "h",	CONSONANT },
	{ "i",	VOWEL },
	{ "ie", VOWEL | DIPTHONG },
	{ "j",	CONSONANT },
	{ "k",	CONSONANT },
	{ "l",	CONSONANT },
	{ "m",	CONSONANT },
	{ "n",	CONSONANT },
	{ "ng",	CONSONANT | DIPTHONG | NOT_FIRST },
	{ "o",	VOWEL },
	{ "oh",	VOWEL | DIPTHONG },
	{ "oo",	VOWEL | DIPTHONG},
	{ "p",	CONSONANT },
	{ "ph",	CONSONANT | DIPTHONG },
	{ "qu",	CONSONANT | DIPTHONG},
	{ "r",	CONSONANT },
	{ "s",	CONSONANT },
	{ "sh",	CONSONANT | DIPTHONG},
	{ "t",	CONSONANT },
	{ "th",	CONSONANT | DIPTHONG},
	{ "u",	VOWEL },
	{ "v",	CONSONANT },
	{ "w",	CONSONANT },
	{ "x",	CONSONANT },
	{ "y",	CONSONANT },
	{ "z",	CONSONANT }
};

#define NUM_ELEMENTS (sizeof(elements) / sizeof (struct pw_element))

void pw_phonemes(char *buf, int size, int pw_flags)
{
	int	c, i, len, flags, feature_flags;
	int	prev, should_be, first;
	char	*str;

try_again:
	feature_flags = pw_flags;
	c = 0;
	prev = 0;
	should_be = 0;
	first = 1;

	should_be = pw_random_number(1) ? VOWEL : CONSONANT;
	
	while (c < size) {
		i = pw_random_number(NUM_ELEMENTS);
		str = elements[i].str;
		len = strlen(str);
		flags = elements[i].flags;
		/* Filter on the basic type of the next element */
		if ((flags & should_be) == 0)
			continue;
		/* Handle the NOT_FIRST flag */
		if (first && (flags & NOT_FIRST))
			continue;
		/* Don't allow VOWEL followed a Vowel/Dipthong pair */
		if ((prev & VOWEL) && (flags & VOWEL) &&
		    (flags & DIPTHONG))
			continue;
		/* Don't allow us to overflow the buffer */
		if (len > size-c)
			continue;
		/*
		 * OK, we found an element which matches our criteria,
		 * let's do it!
		 */
		strcpy(buf+c, str);

		/* Handle PW_ONE_CASE */
		if (feature_flags & PW_ONE_CASE) {
			if ((first || flags & CONSONANT) &&
			    (pw_random_number(10) < 3)) {
				buf[c] = toupper(buf[c]);
				feature_flags &= ~PW_ONE_CASE;
			}
		}
		
		c += len;
		
		/* Time to stop? */
		if (c >= size)
			break;
		
		/*
		 * Handle PW_ONE_NUMBER
		 */
		if (feature_flags & PW_ONE_NUMBER) {
			if (!first && (pw_random_number(10) < 3)) {
				buf[c++] = pw_random_number(9)+'0';
				buf[c] = 0;
				feature_flags &= ~PW_ONE_NUMBER;
				
				first = 1;
				prev = 0;
				should_be = pw_random_number(1) ?
					VOWEL : CONSONANT;
				continue;
			}
		}
				
		/*
		 * OK, figure out what the next element should be
		 */
		if (should_be == CONSONANT) {
			should_be = VOWEL;
		} else { /* should_be == VOWEL */
			if ((prev & VOWEL) ||
			    (flags & DIPTHONG) ||
			    (pw_random_number(10) > 3))
				should_be = CONSONANT;
			else
				should_be = VOWEL;
		}
		prev = flags;
		first = 0;
	}
	if (feature_flags & (PW_ONE_CASE | PW_ONE_NUMBER))
		goto try_again;
}