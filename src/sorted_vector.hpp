/*
* Copyright (C) 2007-2014, GrammarSoft ApS
* Developed by Tino Didriksen <mail@tinodidriksen.com>
* Design by Eckhard Bick <eckhard.bick@mail.dk>, Tino Didriksen <mail@tinodidriksen.com>
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

#pragma once
#ifndef c6d28b7452ec699b_SORTED_VECTOR_HPP
#define c6d28b7452ec699b_SORTED_VECTOR_HPP
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdint.h> // C99 or C++0x or C++ TR1 will have this header. ToDo: Change to <cstdint> when C++0x broader support gets under way.

namespace CG3 {

template<typename T, typename Comp = std::less<T> >
class sorted_vector {
public:
	typedef typename std::vector<T> container;
	typedef typename container::iterator iterator;
	typedef typename container::const_iterator const_iterator;
	typedef typename container::const_reverse_iterator const_reverse_iterator;
	typedef typename container::size_type size_type;
	typedef T value_type;
	typedef T key_type;

	#ifdef CG_TRACE_OBJECTS
	sorted_vector() {
		std::cerr << "OBJECT: " << __PRETTY_FUNCTION__ << std::endl;
	}

	~sorted_vector() {
		std::cerr << "OBJECT: " << __PRETTY_FUNCTION__ << ": " << elements.size() << std::endl;
	}
	#endif

	bool insert(T t) {
		if (elements.empty()) {
			elements.push_back(t);
			return true;
		}
		/*
		else if (comp(elements.back(), t)) {
			elements.push_back(t);
			return true;
		}
		else if (comp(t, elements.front())) {
			elements.insert(elements.begin(), t);
			return true;
		}
		//*/
		iterator it = std::lower_bound(elements.begin(), elements.end(), t, comp);
		if (it == elements.end() || comp(*it, t) || comp(t, *it)) {
			elements.insert(it, t);
			return true;
		}
		return false;
	}

	template<typename It>
	void insert(It b, It e) {
		for (; b != e ; ++b) {
			insert(*b);
		}
	}

	bool push_back(T t) {
		return insert(t);
	}

	bool erase(T t) {
		if (elements.empty()) {
			return false;
		}
		else if (comp(elements.back(), t)) {
			return false;
		}
		else if (comp(t, elements.front())) {
			return false;
		}
		iterator it = lower_bound(t);
		if (it != elements.end() && !comp(*it, t) && !comp(t, *it)) {
			elements.erase(it);
			return true;
		}
		return false;
	}

	const_iterator erase(const_iterator it) {
		size_type o = std::distance<const_iterator>(elements.begin(), it);
		return elements.erase(elements.begin() + o);
	}

	const_iterator find(T t) const {
		if (elements.empty()) {
			return elements.end();
		}
		else if (comp(elements.back(), t)) {
			return elements.end();
		}
		else if (comp(t, elements.front())) {
			return elements.end();
		}
		const_iterator it = lower_bound(t);
		if (it != elements.end() && (comp(*it, t) || comp(t, *it))) {
			return elements.end();
		}
		return it;
	}

	size_t count(T t) const {
		return (find(t) != end());
	}

	iterator begin() {
		return elements.begin();
	}

	iterator end() {
		return elements.end();
	}

	const_iterator begin() const {
		return elements.begin();
	}

	const_iterator end() const {
		return elements.end();
	}

	const_reverse_iterator rbegin() const {
		return elements.rbegin();
	}

	const_reverse_iterator rend() const {
		return elements.rend();
	}

	T front() const {
		return elements.front();
	}

	T back() const {
		return elements.back();
	}

	iterator lower_bound(T t) {
		return std::lower_bound(elements.begin(), elements.end(), t, comp);
	}

	const_iterator lower_bound(T t) const {
		return std::lower_bound(elements.begin(), elements.end(), t, comp);
	}

	const_iterator upper_bound(T t) const {
		return std::upper_bound(elements.begin(), elements.end(), t, comp);
	}

	size_type size() const {
		return elements.size();
	}

	bool empty() const {
		return elements.empty();
	}

	template<typename It>
	void assign(It b, It e) {
		clear();
		insert(b, e);
	}

	void assign(const_iterator b, const_iterator e) {
		elements.assign(b, e);
	}

	void swap(sorted_vector& other) {
		elements.swap(other.elements);
	}

	void clear() {
		elements.clear();
	}

	container& get() {
		return elements;
	}

private:
	container elements;
	Comp comp;
};

typedef sorted_vector<uint32_t> uint32SortedVector;

}

#endif
