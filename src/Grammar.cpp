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

#include "Grammar.h"

using namespace CG3;

Grammar::Grammar() {
	has_dep = false;
	is_binary = false;
	last_modified = 0;
	name = 0;
	lines = 0;
	curline = 0;
	total_time = 0;
	delimiters = 0;
	soft_delimiters = 0;
	tag_any = 0;
	sets_any = 0;
	rules_any = 0;
	mapping_prefix = '@';
	srand((unsigned int)time(0));
}

Grammar::~Grammar() {
	if (name) {
		delete[] name;
	}

	foreach (std::vector<Set*>, sets_list, iter_set, iter_set_end) {
		destroySet(*iter_set);
	}

	foreach (std::set<Set*>, sets_all, rsets, rsets_end) {
		delete (*rsets);
	}

	std::map<uint32_t, Anchor*>::iterator iter_anc;
	for (iter_anc = anchor_by_line.begin() ; iter_anc != anchor_by_line.end() ; iter_anc++) {
		if (iter_anc->second) {
			delete iter_anc->second;
		}
	}
	
	stdext::hash_map<uint32_t, CompositeTag*>::iterator iter_ctag;
	for (iter_ctag = tags.begin() ; iter_ctag != tags.end() ; iter_ctag++) {
		if (iter_ctag->second) {
			delete iter_ctag->second;
		}
	}

	stdext::hash_map<uint32_t, Tag*>::iterator iter_stag;
	for (iter_stag = single_tags.begin() ; iter_stag != single_tags.end() ; iter_stag++) {
		if (iter_stag->second) {
			delete iter_stag->second;
		}
	}

	foreach (RuleVector, rules, iter_rules1, iter_rules1_end) {
		if (*iter_rules1) {
			delete *iter_rules1;
		}
	}

	foreach (RuleVector, before_sections, iter_rules2, iter_rules2_end) {
		if (*iter_rules2) {
			delete *iter_rules2;
		}
	}

	foreach (RuleVector, after_sections, iter_rules3, iter_rules3_end) {
		if (*iter_rules3) {
			delete *iter_rules3;
		}
	}

	foreach (RuleVector, null_section, iter_rules4, iter_rules4_end) {
		if (*iter_rules4) {
			delete *iter_rules4;
		}
	}

	uint32HashSetuint32HashMap::iterator xrule;
	for (xrule = rules_by_tag.begin() ; xrule != rules_by_tag.end() ; xrule++) {
		delete xrule->second;
		xrule->second = 0;
	}

	uint32HashSetuint32HashMap::iterator xset;
	for (xset = sets_by_tag.begin() ; xset != sets_by_tag.end() ; xset++) {
		delete xset->second;
		xset->second = 0;
	}

	foreach(uint32Setuint32HashMap, rules_by_set, irbs, irbs_end) {
		delete irbs->second;
		irbs->second = 0;
	}
}

void Grammar::addPreferredTarget(UChar *to) {
	Tag *tag = 0;
	uint32_t hash = hash_sdbm_uchar(to);
	if (single_tags.find(hash) != single_tags.end()) {
		tag = single_tags[hash];
	}
	else {
		tag = new Tag();
		tag->parseTag(to, ux_stderr);
		tag->rehash();
		tag = addTag(tag);
	}
	preferred_targets.push_back(tag->hash);
}
void Grammar::addSet(Set *to) {
	if (!delimiters && u_strcmp(to->name, stringbits[S_DELIMITSET]) == 0) {
		delimiters = to;
	}
	else if (!soft_delimiters && u_strcmp(to->name, stringbits[S_SOFTDELIMITSET]) == 0) {
		soft_delimiters = to;
	}
	uint32_t chash = to->rehash();
	if (to->name[0] != '_' || to->name[1] != 'G' || to->name[2] != '_') {
		uint32_t nhash = hash_sdbm_uchar(to->name, 0);
		if (sets_by_name.find(nhash) == sets_by_name.end()) {
			sets_by_name[nhash] = chash;
		}
		else if (chash != sets_by_contents.find(sets_by_name.find(nhash)->second)->second->hash) {
			Set *a = sets_by_contents.find(sets_by_name.find(nhash)->second)->second;
			u_fprintf(ux_stderr, "Error: Set %S already defined at line %u. Redefinition as set %S attempted at line %u!\n", a->name, a->line, to->name, to->line);
			CG3Quit(1);
		}
	}
	if (sets_by_contents.find(chash) == sets_by_contents.end()) {
		sets_by_contents[chash] = to;
	}
}
Set *Grammar::getSet(uint32_t which) {
	Set *retval = 0;
	if (sets_by_contents.find(which) != sets_by_contents.end()) {
		retval = sets_by_contents[which];
	}
	else if (sets_by_name.find(which) != sets_by_name.end()) {
		retval = getSet(sets_by_name[which]);
	}
	return retval;
}

Set *Grammar::allocateSet() {
	Set *ns = new Set;
	sets_all.insert(ns);
	return ns;
}
void Grammar::destroySet(Set *set) {
	sets_all.erase(set);
	delete set;
}
void Grammar::addSetToList(Set *s) {
	if (s->number == 0) {
		if (sets_list.empty() || sets_list.at(0) != s) {
			if (!s->sets.empty()) {
				foreach (uint32Vector, s->sets, sit, sit_end) {
					addSetToList(getSet(*sit));
				}
			}
			sets_list.push_back(s);
			s->number = (uint32_t)sets_list.size()-1;
		}
	}
}

CompositeTag *Grammar::addCompositeTag(CompositeTag *tag) {
	if (tag && tag->tags.size()) {
		tag->rehash();
		if (tags.find(tag->hash) != tags.end()) {
			uint32_t ct = tag->hash;
			delete tag;
			tag = tags[ct];
		}
		else {
			tags[tag->hash] = tag;
			tags_list.push_back(tag);
			tag->number = (uint32_t)tags_list.size()-1;
		}
	} else {
		u_fprintf(ux_stderr, "Error: Attempted to add empty composite tag to grammar on line %u!\n", lines);
		CG3Quit(1);
	}
	return tags[tag->hash];
}
CompositeTag *Grammar::addCompositeTagToSet(Set *set, CompositeTag *tag) {
	if (tag && tag->tags.size()) {
		if (tag->tags.size() == 1) {
			Tag *rtag = *(tag->tags.begin());
			set->tags_set.insert(rtag->hash);
			set->single_tags.insert(rtag);
			set->single_tags_hash.insert(rtag->hash);

			if (rtag->type & T_ANY) {
				set->match_any = true;
			}
			if (rtag->is_special) {
				set->is_special = true;
			}
			if (rtag->type & T_FAILFAST) {
				set->ff_tags.insert(rtag);
				set->ff_tags_hash.insert(rtag->hash);
			}
			delete tag;
			tag = 0;
		} else {
			tag = addCompositeTag(tag);
			set->tags_set.insert(tag->hash);
			set->tags.insert(tag);
			if (tag->is_special) {
				set->is_special = true;
			}
		}
	} else {
		u_fprintf(ux_stderr, "Error: Attempted to add empty composite tag to grammar and set on line %u!\n", lines);
		CG3Quit(1);
	}
	return tag;
}
CompositeTag *Grammar::allocateCompositeTag() {
	return new CompositeTag;
}
void Grammar::destroyCompositeTag(CompositeTag *tag) {
	delete tag;
}

Rule *Grammar::allocateRule() {
	return new Rule;
}
void Grammar::addRule(Rule *rule) {
	if (rule_by_line.find(rule->line) != rule_by_line.end()) {
		u_fprintf(ux_stderr, "Error: Multiple rules defined on line %u - cannot currently handle multiple rules per line!\n", rule->line);
		CG3Quit(1);
	}
	rule_by_line[rule->line] = rule;
}
void Grammar::destroyRule(Rule *rule) {
	delete rule;
}

Tag *Grammar::allocateTag() {
	return new Tag;
}
Tag *Grammar::allocateTag(const UChar *tag) {
	Tag *fresh = 0;
	uint32_t hash = hash_sdbm_uchar(tag);
	if (single_tags.find(hash) != single_tags.end()) {
		fresh = single_tags[hash];
		if (u_strcmp(single_tags[hash]->tag, tag) != 0) {
			u_fprintf(ux_stderr, "Error: Hash collision between %S and %S!\n", single_tags[hash]->tag, tag);
			CG3Quit(1);
		}
	}
	else {
		fresh = new Tag;
		fresh->parseTag(tag, ux_stderr);
	}
	return fresh;
}
Tag *Grammar::addTag(Tag *simpletag) {
	if (simpletag && simpletag->tag) {
		simpletag->rehash();
		if (single_tags.find(simpletag->hash) != single_tags.end()) {
			if (u_strcmp(single_tags[simpletag->hash]->tag, simpletag->tag) != 0) {
				u_fprintf(ux_stderr, "Error: Hash collision between %S and %S!\n", single_tags[simpletag->hash]->tag, simpletag->tag);
				CG3Quit(1);
			}
			Tag *t = single_tags[simpletag->hash];
			if (simpletag != t) {
				destroyTag(simpletag);
			}
			return t;
		}
		else {
			single_tags[simpletag->hash] = simpletag;
			single_tags_list.push_back(simpletag);
			simpletag->number = (uint32_t)single_tags_list.size()-1;
			return single_tags[simpletag->hash];
		}
	} else {
		u_fprintf(ux_stderr, "Error: Attempted to add empty tag to grammar on line %u!\n", lines);
		CG3Quit(1);
	}
	return 0;
}
void Grammar::addTagToCompositeTag(Tag *simpletag, CompositeTag *tag) {
	if (simpletag && simpletag->tag) {
		tag->addTag(simpletag);
	} else {
		u_fprintf(ux_stderr, "Error: Attempted to add empty tag to grammar and composite tag on line %u!\n", lines);
		CG3Quit(1);
	}
}
void Grammar::destroyTag(Tag *tag) {
	delete tag;
}

ContextualTest *Grammar::allocateContextualTest() {
	return new ContextualTest;
}
void Grammar::addContextualTest(ContextualTest *test, const UChar *name) {
	uint32_t cn = hash_sdbm_uchar(name);
	if (templates.find(cn) != templates.end()) {
		u_fprintf(ux_stderr, "Error: Redefinition attempt for template '%S' on line %u!\n", name, lines);
		CG3Quit(1);
	}
	templates[cn] = test;
	template_list.push_back(test);
}

void Grammar::addAnchor(const UChar *to, uint32_t line) {
	uint32_t ah = hash_sdbm_uchar(to);
	if (anchor_by_hash.find(ah) != anchor_by_hash.end()) {
		u_fprintf(ux_stderr, "Error: Redefinition attempt for anchor '%S' on line %u!\n", to, line);
		CG3Quit(1);
	}
	Anchor *anc = new Anchor;
	anc->setName(to);
	anc->line = line;
	anchor_by_hash[ah] = line;
	anchor_by_line[line] = anc;
}

void Grammar::setName(const char *to) {
	name = new UChar[strlen(to)+1];
	u_uastrcpy(name, to);
}

void Grammar::setName(const UChar *to) {
	name = new UChar[u_strlen(to)+1];
	u_strcpy(name, to);
}

void Grammar::resetStatistics() {
	total_time = 0;
	for (uint32_t j=0;j<rules.size();j++) {
		rules[j]->resetStatistics();
	}
}

void Grammar::renameAllRules() {
	foreach(RuleByLineHashMap, rule_by_line, iter_rule, iter_rule_end) {
		Rule *r = iter_rule->second;
		gbuffers[0][0] = 0;
		u_sprintf(gbuffers[0], "L%u", r->line);
		r->setName(gbuffers[0]);
	}
};

void Grammar::reindex() {
	set_alias.clear();
	sets_by_name.clear();
	rules.clear();
	before_sections.clear();
	after_sections.clear();
	null_section.clear();
	sections.clear();
	sets_list.clear();
	sets_any = 0;
	rules_any = 0;

	foreach (Setuint32HashMap, sets_by_contents, dset, dset_end) {
		dset->second->is_used = false;
		dset->second->number = 0;
	}

	RuleByLineMap rule_by_line;
	rule_by_line.insert(this->rule_by_line.begin(), this->rule_by_line.end());
	foreach(RuleByLineMap, rule_by_line, iter_rule, iter_rule_end) {
		Set *s = 0;
		s = getSet(iter_rule->second->target);
		s->markUsed(this);
		if (iter_rule->second->dep_target) {
			iter_rule->second->dep_target->markUsed(this);
		}
		ContextualTest *test = iter_rule->second->dep_test_head;
		while (test) {
			test->markUsed(this);
			test = test->next;
		}
		test = iter_rule->second->test_head;
		while (test) {
			test->markUsed(this);
			test = test->next;
		}
	}
	if (delimiters) {
		delimiters->markUsed(this);
	}
	if (soft_delimiters) {
		soft_delimiters->markUsed(this);
	}

	/*
	// ToDo: Actually destroying the data still needs more work to determine what is safe to kill off.

	foreach (Setuint32HashMap, sets_by_contents, rset, rset_end) {
		if (!rset->second->is_used) {
			destroySet(rset->second);
			rset->second = 0;
		}
	}

	sets_by_contents.clear();
	foreach(std::set<Set*>, sets_all, sall, sall_end) {
		sets_by_contents[(*sall)->hash] = *sall;
	}

	std::vector<CompositeTag*> ctmp;
	foreach(std::vector<CompositeTag*>, tags_list, citer, citer_end) {
		if ((*citer)->is_used) {
			ctmp.push_back(*citer);
		}
		else {
			destroyCompositeTag(*citer);
			*citer = 0;
		}
	}
	tags_list.clear();
	tags.clear();
	foreach(std::vector<CompositeTag*>, ctmp, cniter, cniter_end) {
		addCompositeTag(*cniter);
	}

	std::vector<Tag*> stmp;
	foreach(std::vector<Tag*>, single_tags_list, siter, siter_end) {
		if ((*siter)->is_used) {
			stmp.push_back(*siter);
		}
		else {
			destroyTag(*siter);
			*siter = 0;
		}
	}
	single_tags_list.clear();
	single_tags.clear();
	foreach(std::vector<Tag*>, stmp, sniter, sniter_end) {
		addTag(*sniter);
	}

	//*/

	// Stuff below this line is not optional...

	foreach (Setuint32HashMap, sets_by_contents, tset, tset_end) {
		if (tset->second->is_used) {
			addSetToList(tset->second);
		}
	}

	stdext::hash_map<uint32_t, Tag*>::iterator iter_tags;
	for (iter_tags = single_tags.begin() ; iter_tags != single_tags.end() ; iter_tags++) {
		Tag *tag = iter_tags->second;
		if (tag->tag[0] == mapping_prefix) {
			tag->type |= T_MAPPING;
		}
		else {
			tag->type &= ~T_MAPPING;
		}
	}

	foreach (Setuint32HashMap, sets_by_contents, iter_sets, iter_sets_end) {
		if (iter_sets->second->is_used) {
			iter_sets->second->reindex(this);
			indexSets(iter_sets->first, iter_sets->second);
		}
	}

	uint32Set sects;

	for (iter_rule = rule_by_line.begin() ; iter_rule != rule_by_line.end() ; iter_rule++) {
		if (iter_rule->second->section == -1) {
			before_sections.push_back(iter_rule->second);
		}
		else if (iter_rule->second->section == -2) {
			after_sections.push_back(iter_rule->second);
		}
		else if (iter_rule->second->section == -3) {
			null_section.push_back(iter_rule->second);
		}
		else {
			sects.insert(iter_rule->second->section);
			rules.push_back(iter_rule->second);
		}
		if (iter_rule->second->target) {
			indexSetToRule(iter_rule->second->line, getSet(iter_rule->second->target));
			if (rules_by_set.find(iter_rule->second->target) == rules_by_set.end()) {
				rules_by_set[iter_rule->second->target] = new uint32Set;
			}
			rules_by_set[iter_rule->second->target]->insert(iter_rule->first);
		}
		else {
			u_fprintf(ux_stderr, "Warning: Rule on line %u had no target.\n", iter_rule->second->line);
		}
	}

	sections.insert(sections.end(), sects.begin(), sects.end());

	if (sets_by_tag.find(tag_any) != sets_by_tag.end()) {
		sets_any = sets_by_tag.find(tag_any)->second;
	}
	if (rules_by_tag.find(tag_any) != rules_by_tag.end()) {
		rules_any = rules_by_tag.find(tag_any)->second;
	}
}

void Grammar::indexSetToRule(uint32_t r, Set *s) {
	if (s->is_special || s->is_unified) {
		indexTagToRule(tag_any, r);
		return;
	}
	if (s->sets.empty()) {
		TagHashSet::const_iterator tomp_iter;
		for (tomp_iter = s->single_tags.begin() ; tomp_iter != s->single_tags.end() ; tomp_iter++) {
			indexTagToRule((*tomp_iter)->hash, r);
		}
		CompositeTagHashSet::const_iterator comp_iter;
		for (comp_iter = s->tags.begin() ; comp_iter != s->tags.end() ; comp_iter++) {
			CompositeTag *curcomptag = *comp_iter;
			if (curcomptag->tags.size() == 1) {
				indexTagToRule((*(curcomptag->tags.begin()))->hash, r);
			} else {
				TagSet::const_iterator tag_iter;
				for (tag_iter = curcomptag->tags_set.begin() ; tag_iter != curcomptag->tags_set.end() ; tag_iter++) {
					indexTagToRule((*tag_iter)->hash, r);
				}
			}
		}
	} else if (!s->sets.empty()) {
		for (uint32_t i=0;i<s->sets.size();i++) {
			Set *set = sets_by_contents.find(s->sets.at(i))->second;
			indexSetToRule(r, set);
		}
	}
}

void Grammar::indexTagToRule(uint32_t t, uint32_t r) {
	if (rules_by_tag.find(t) == rules_by_tag.end()) {
		std::pair<uint32_t,uint32HashSet*> p;
		p.first = t;
		p.second = new uint32HashSet;
		rules_by_tag.insert(p);
	}
	rules_by_tag.find(t)->second->insert(r);
}

void Grammar::indexSets(uint32_t r, Set *s) {
	if (s->is_special || s->is_unified) {
		indexTagToSet(tag_any, r);
		return;
	}
	if (s->sets.empty()) {
		TagHashSet::const_iterator tomp_iter;
		for (tomp_iter = s->single_tags.begin() ; tomp_iter != s->single_tags.end() ; tomp_iter++) {
			indexTagToSet((*tomp_iter)->hash, r);
		}
		CompositeTagHashSet::const_iterator comp_iter;
		for (comp_iter = s->tags.begin() ; comp_iter != s->tags.end() ; comp_iter++) {
			CompositeTag *curcomptag = *comp_iter;
			if (curcomptag->tags.size() == 1) {
				indexTagToSet((*(curcomptag->tags.begin()))->hash, r);
			} else {
				TagSet::const_iterator tag_iter;
				for (tag_iter = curcomptag->tags_set.begin() ; tag_iter != curcomptag->tags_set.end() ; tag_iter++) {
					indexTagToSet((*tag_iter)->hash, r);
				}
			}
		}
	} else if (!s->sets.empty()) {
		for (uint32_t i=0;i<s->sets.size();i++) {
			Set *set = sets_by_contents.find(s->sets.at(i))->second;
			indexSets(r, set);
		}
	}
}

void Grammar::indexTagToSet(uint32_t t, uint32_t r) {
	if (sets_by_tag.find(t) == sets_by_tag.end()) {
		std::pair<uint32_t,uint32HashSet*> p;
		p.first = t;
		p.second = new uint32HashSet;
		sets_by_tag.insert(p);
	}
	sets_by_tag.find(t)->second->insert(r);
}
