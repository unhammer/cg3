/*
 * Copyright (C) 2007, GrammarSoft ApS
 * Developed by Tino Didriksen <tino@didriksen.cc>
 * Design by Eckhard Bick <eckhard.bick@mail.dk>, Tino Didriksen <tino@didriksen.cc>
 *
 * This file is part of VISL CG-3
 *
 * VISL CG-3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * VISL CG-3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with VISL CG-3.  If not, see <http://www.gnu.org/licenses/>.
 */

// MSVC 2005 (MSVC 8) fix.
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#include "stdafx.h"
#include "icu_uoptions.h"
#include "Recycler.h"
#include "Grammar.h"
#include "TextualParser.h"
#include "GrammarWriter.h"
#include "BinaryGrammar.h"
#include "GrammarApplicator.h"

#include <libgen.h>

#include "version.h"

using namespace std;
using CG3::CG3Quit;

void endProgram(char *name);

void 
endProgram(char *name)
{
	if(name != NULL) {
		fprintf(stdout, "VISL CG-3 Compiler version %u.%u.%u.%u\n",
			CG3_VERSION_MAJOR, CG3_VERSION_MINOR, CG3_VERSION_PATCH, CG3_REVISION);
		cout << basename(name) << ": compile a binary grammar from a text file" << endl;
		cout << "USAGE: " << basename(name) << " grammar_file output_file" << endl;
	}
	exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[])
{
	UFILE *ux_stderr = 0;
	UErrorCode status = U_ZERO_ERROR;

	U_MAIN_INIT_ARGS(argc, argv);

	if(argc != 3) {
		endProgram(argv[0]);
	}

	/* Initialize ICU */
	u_init(&status);
		if (U_FAILURE(status) && status != U_FILE_ACCESS_ERROR) {
		cerr << "Error: Cannot initialize ICU. Status = " << u_errorName(status) << std::endl;
		CG3Quit(1);
	}
	status = U_ZERO_ERROR;

	const char *locale_default = "en_US_POSIX"; //uloc_getDefault();
	ucnv_setDefaultName("UTF-8");
	const char *codepage_default = ucnv_getDefaultName();

	ux_stderr = u_finit(stderr, locale_default, codepage_default);

	CG3::Recycler::instance();
	init_gbuffers();
	init_strings();
	init_keywords();
	init_flags();
	CG3::Grammar grammar;

	CG3::IGrammarParser *parser = 0;
	FILE *input = fopen(argv[1], "rb");

	if (!input) {
		std::cerr << "Error: Error opening " << argv[1] << " for reading!" << std::endl;
		CG3Quit(1);
	}
	if (fread(cbuffers[0], 1, 4, input) != 4) {
		std::cerr << "Error: Error reading first 4 bytes from grammar!" << std::endl;
		CG3Quit(1);
	}
	fclose(input);

	if (cbuffers[0][0] == 'C' && cbuffers[0][1] == 'G' && cbuffers[0][2] == '3' && cbuffers[0][3] == 'B') {
		std::cerr << "Binary grammar detected. Cannot re-compile binary grammars." << std::endl;
		CG3Quit(1);
	} else {
		parser = new CG3::TextualParser(grammar, ux_stderr);
	}

	grammar.ux_stderr = ux_stderr;
	CG3::Tag *tag_any = grammar.allocateTag(stringbits[S_ASTERIK]);
	grammar.tag_any = tag_any->hash;

	if (parser->parse_grammar_from_file(argv[1], locale_default, codepage_default)) {
		std::cerr << "Error: Grammar could not be parsed - exiting!" << std::endl;
		CG3Quit(1);
	}

	grammar.reindex();

	std::cerr << "Sections: " << grammar.sections.size() << ", Rules: " << grammar.rule_by_line.size();
	std::cerr << ", Sets: " << grammar.sets_by_contents.size() << ", Tags: " << grammar.single_tags.size() << std::endl;

	if (grammar.rules_by_tag.find(tag_any->hash) != grammar.rules_by_tag.end()) {
		std::cerr << grammar.rules_by_tag.find(tag_any->hash)->second->size() << " rules cannot be skipped by index." << std::endl;
	}

	if (grammar.has_dep) {
		std::cerr << "Grammar has dependency rules." << std::endl;
	}

	FILE *gout = fopen(argv[2], "wb");

	if (gout) {
		CG3::BinaryGrammar *writer = new CG3::BinaryGrammar(grammar, ux_stderr);
		writer->writeBinaryGrammar(gout);
		delete writer;
		writer = 0;
	} else {
		std::cerr << "Could not write grammar to " << argv[2] << std::endl;
	}

	u_fclose(ux_stderr);

	free_strings();
	free_keywords();
	free_gbuffers();
	free_flags();

	CG3::Recycler::cleanup();

	u_cleanup();

	return status;
}
