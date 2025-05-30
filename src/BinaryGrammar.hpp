/*
* Copyright (C) 2007-2025, GrammarSoft ApS
* Developed by Tino Didriksen <mail@tinodidriksen.com>
* Design by Eckhard Bick <eckhard.bick@mail.dk>, Tino Didriksen <mail@tinodidriksen.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this progam.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef c6d28b7452ec699b_BINARYGRAMMAR_H
#define c6d28b7452ec699b_BINARYGRAMMAR_H

#include "IGrammarParser.hpp"

namespace CG3 {
class ContextualTest;

enum : uint32_t {
	BINF_DEP          = (1 <<  0),
	BINF_PREFIX       = (1 <<  1),
	BINF_SUB_LTR      = (1 <<  2),
	BINF_TAGS         = (1 <<  3),
	BINF_REOPEN_MAP   = (1 <<  4),
	BINF_PREF_TARGETS = (1 <<  5),
	BINF_ENCLS        = (1 <<  6),
	BINF_ANCHORS      = (1 <<  7),
	BINF_SETS         = (1 <<  8),
	BINF_DELIMS       = (1 <<  9),
	BINF_SOFT_DELIMS  = (1 << 10),
	BINF_CONTEXTS     = (1 << 11),
	BINF_RULES        = (1 << 12),
	BINF_RELATIONS    = (1 << 13),
	BINF_BAG          = (1 << 14),
	BINF_ORDERED      = (1 << 15),
	BINF_TEXT_DELIMS  = (1 << 16),
	BINF_ADDCOHORT_ATTACH = (1 << 17),
};

constexpr uint32_t BIN_REV_ANCIENT = 10297;
constexpr uint32_t BIN_REV_CMDARGS = 13898;

class BinaryGrammar : public IGrammarParser {
public:
	BinaryGrammar(Grammar& result, std::ostream& ux_err);

	int writeBinaryGrammar(std::ostream& output);

	void setCompatible(bool compat) override;
	void setVerbosity(uint32_t level) override;
	int parse_grammar(std::istream& input);
	int parse_grammar(const char* buffer, size_t length) override;
	int parse_grammar(const UChar* buffer, size_t length) override;
	int parse_grammar(const std::string& buffer) override;
	int parse_grammar(const char* filename) override;

private:
	int parse_grammar(UString& buffer) override;

	Grammar* grammar = nullptr;
	void writeContextualTest(ContextualTest* t, std::ostream& output);
	ContextualTest* readContextualTest(std::istream& input);

	typedef std::unordered_map<ContextualTest*, uint32_t> deferred_t;
	deferred_t deferred_tmpls;
	typedef std::unordered_map<ContextualTest*, std::vector<uint32_t>> deferred_ors_t;
	deferred_ors_t deferred_ors;

	uint32FlatHashSet seen_uint32;

	int readBinaryGrammar_10043(std::istream& input);
	ContextualTest* readContextualTest_10043(std::istream& input);
};
}

#endif
