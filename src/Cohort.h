/*
* Copyright (C) 2007, GrammarSoft Aps
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

#ifndef __COHORT_H
#define __COHORT_H

#include "stdafx.h"
#include "Window.h"
#include "Reading.h"
#include "Recycler.h"

namespace CG3 {

	class Cohort {
	public:
		bool is_disamb;
		uint32_t global_number;
		uint32_t local_number;
		uint32_t wordform;
		SingleWindow *parent;
		std::list<Reading*> readings;
		std::list<Reading*> deleted;
		UChar *text;

		bool dep_done;
		uint32_t dep_self;
		uint32_t dep_parent;
		uint32HashSet dep_children;
		uint32HashSet dep_siblings;

		bool is_related;
		std::multimap<uint32_t, uint32_t> relations;

		Cohort(SingleWindow *p);
		~Cohort();
		void clear(SingleWindow *p);

		void addSibling(uint32_t sibling);
		void remSibling(uint32_t sibling);
		void addChild(uint32_t child);
		void remChild(uint32_t child);
		void appendReading(Reading *read);
		Reading *allocateAppendReading();
	};

}

#endif
