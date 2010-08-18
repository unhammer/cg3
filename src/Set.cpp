/*
* Copyright (C) 2007-2010, GrammarSoft ApS
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

#include "Set.h"
#include "CompositeTag.h"
#include "Strings.h"
#include "Grammar.h"

namespace CG3 {

bool Set::dump_hashes = false;
UFILE* Set::dump_hashes_out = 0;

Set::Set() :
type(0),
line(0),
hash(0),
number(0),
num_fail(0),
num_match(0),
total_time(0)
{
	// Nothing in the actual body...
}

Set::Set(const Set& from) :
type(from.type),
line(from.line),
hash(0),
number(0),
num_fail(0),
num_match(0),
total_time(0),
tags_set(from.tags_set),
tags(from.tags),
single_tags(from.single_tags),
single_tags_hash(from.single_tags_hash),
ff_tags(from.ff_tags),
set_ops(from.set_ops),
sets(from.sets)
{
	// Nothing in the actual body...
}

void Set::setName(uint32_t to) {
	if (!to) {
		to = static_cast<uint32_t>(rand());
	}
	size_t n = sprintf(&cbuffers[0][0], "_G_%u_%u_", line, to);
	name.clear();
	name.reserve(n);
	for (size_t i=0 ; i<n ; ++i) {
		name.push_back(cbuffers[0][i]);
	}
}

void Set::setName(const UChar *to) {
	if (to) {
		name = to;
	}
	else {
		setName((uint32_t)rand());
	}
}

void Set::setName(const UString& to) {
	if (!to.empty()) {
		name = to;
	}
	else {
		setName((uint32_t)rand());
	}
}

uint32_t Set::rehash() {
	uint32_t retval = 0;
	if (sets.empty()) {
		retval = hash_sdbm_uint32_t(3499, retval); // Combat hash-collisions
		foreach (uint32Set, tags_set, iter, iter_end) {
			retval = hash_sdbm_uint32_t(*iter, retval);
		}
	}
	else {
		retval = hash_sdbm_uint32_t(2683, retval); // Combat hash-collisions
		for (uint32_t i=0;i<sets.size();i++) {
			retval = hash_sdbm_uint32_t(sets.at(i), retval);
		}
		for (uint32_t i=0;i<set_ops.size();i++) {
			retval = hash_sdbm_uint32_t(set_ops.at(i), retval);
		}
	}
	hash = retval;

	if (dump_hashes && dump_hashes_out) {
		if (sets.empty()) {
			u_fprintf(dump_hashes_out, "DEBUG: Hash %u for set %S (LIST)\n", hash, name.c_str());
		}
		else {
			u_fprintf(dump_hashes_out, "DEBUG: Hash %u for set %S (SET)\n", hash, name.c_str());
		}
	}

	return retval;
}

void Set::reindex(Grammar& grammar) {
	type &= ~ST_SPECIAL;
	type &= ~ST_CHILD_UNIFY;

	if (sets.empty()) {
		const_foreach (TagHashSet, single_tags, tomp_iter, tomp_iter_end) {
			if ((*tomp_iter)->type & T_SPECIAL) {
				type |= ST_SPECIAL;
			}
			if ((*tomp_iter)->type & T_MAPPING) {
				type |= ST_MAPPING;
			}
		}
		const_foreach (CompositeTagHashSet, tags, comp_iter, comp_iter_end) {
			const_foreach (TagSet, (*comp_iter)->tags_set, tag_iter, tag_iter_end) {
				if ((*tag_iter)->type & T_SPECIAL) {
					type |= ST_SPECIAL;
				}
				if ((*tag_iter)->type & T_MAPPING) {
					type |= ST_MAPPING;
				}
			}
		}
	}
	else {
		for (uint32_t i=0;i<sets.size();i++) {
			Set *set = grammar.sets_by_contents.find(sets.at(i))->second;
			set->reindex(grammar);
			if (set->type & ST_SPECIAL) {
				type |= ST_SPECIAL;
			}
			if (set->type & (ST_TAG_UNIFY|ST_SET_UNIFY|ST_CHILD_UNIFY)) {
				type |= ST_CHILD_UNIFY;
			}
			if (set->type & ST_MAPPING) {
				type |= ST_MAPPING;
			}
		}
	}

	if (type & (ST_TAG_UNIFY|ST_SET_UNIFY|ST_CHILD_UNIFY)) {
		type |= ST_SPECIAL;
		type |= ST_CHILD_UNIFY;
	}
}

void Set::markUsed(Grammar& grammar) {
	type |= ST_USED;

	if (sets.empty()) {
		TagHashSet::iterator tomp_iter;
		for (tomp_iter = single_tags.begin() ; tomp_iter != single_tags.end() ; tomp_iter++) {
			Tag *tag = *tomp_iter;
			tag->markUsed();
		}
		CompositeTagHashSet::iterator comp_iter;
		for (comp_iter = tags.begin() ; comp_iter != tags.end() ; comp_iter++) {
			CompositeTag *curcomptag = *comp_iter;
			curcomptag->markUsed();
		}
	}
	else {
		for (uint32_t i=0;i<sets.size();i++) {
			Set *set = grammar.sets_by_contents.find(sets.at(i))->second;
			set->markUsed(grammar);
		}
	}
}

void Set::resetStatistics() {
	num_fail = 0;
	num_match = 0;
	total_time = 0;
}

}
