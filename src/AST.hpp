/*
* Copyright (C) 2007-2025, GrammarSoft ApS
* Developed by Tino Didriksen <mail@tinodidriksen.com>
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
#ifndef c6d28b7452ec699b_AST_HPP
#define c6d28b7452ec699b_AST_HPP

#include "stdafx.hpp"

enum ASTType {
	AST_Unknown,
	AST_AfterSections,
	AST_Anchor,
	AST_AnchorName,
	AST_Barrier,
	AST_BarrierSafe,
	AST_BeforeSections,
	AST_CmdArgs,
	AST_CompositeTag,
	AST_Context,
	AST_ContextMod,
	AST_ContextPos,
	AST_Contexts,
	AST_ContextsTarget,
	AST_Delimiters,
	AST_Grammar,
	AST_Include,
	AST_IncludeFilename,
	AST_List,
	AST_MappingPrefix,
	AST_NullSection,
	AST_Option,
	AST_Options,
	AST_Parentheses,
	AST_PreferredTargets,
	AST_ReopenMappings,
	AST_Rule,
	AST_RuleAddcohortWhere,
	AST_RuleDirection,
	AST_RuleExcept,
	AST_RuleExternalCmd,
	AST_RuleExternalType,
	AST_RuleFlag,
	AST_RuleMaplist,
	AST_RuleMoveType,
	AST_RuleName,
	AST_RuleSublist,
	AST_RuleSubrules,
	AST_RuleTarget,
	AST_RuleType,
	AST_RuleWithChildDepTarget,
	AST_RuleWithChildTarget,
	AST_RuleWordform,
	AST_Section,
	AST_Set,
	AST_SetInline,
	AST_SetName,
	AST_SetOp,
	AST_SoftDelimiters,
	AST_StaticSets,
	AST_UndefSets,
	AST_StrictTags,
	AST_ListTags,
	AST_SubReadings,
	AST_SubReadingsDirection,
	AST_Tag,
	AST_TagList,
	AST_Template,
	AST_TemplateInline,
	AST_TemplateName,
	AST_TemplateRef,
	AST_TemplateShorthand,
	AST_TextDelimiters,
	NUM_ASTTypes
};
const char* ASTType_str[NUM_ASTTypes] = {};

struct ASTHelper;
struct ASTNode {
	ASTType type;
	size_t line = 0;
	const UChar *b = nullptr, *e = nullptr;
	uint32_t u = 0;
	std::vector<ASTNode> cs;

	ASTNode(ASTType type = AST_Unknown, size_t line = 0, const UChar* b = nullptr, const UChar* e = nullptr)
	  : type(type)
	  , line(line)
	  , b(b)
	  , e(e)
	{}
};

bool parse_ast = false;
ASTNode ast;
ASTNode* cur_ast = &ast;
ASTHelper* cur_ast_help = nullptr;

const UChar* xml_encode(const UChar* b, const UChar* e) {
	static thread_local CG3::UString buf;
	buf.clear();
	buf.reserve(e - b);
	for (; b != e; ++b) {
		if (*b == '&') {
			buf.append(u"&amp;");
		}
		else if (*b == '"') {
			buf.append(u"&quot;");
		}
		else if (*b == '\'') {
			buf.append(u"&apos;");
		}
		else if (*b == '<') {
			buf.append(u"&lt;");
		}
		else if (*b == '>') {
			buf.append(u"&gt;");
		}
		else {
			buf += *b;
		}
	}
	return buf.data();
}

void print_ast(std::ostream& out, const UChar* b, size_t n, const ASTNode& node) {
	std::string indent(n, ' ');
	u_fprintf(out, "%s<%s l=\"%u\" b=\"%u\" e=\"%u\"", indent.data(), ASTType_str[node.type], node.line, UI32(node.b - b), UI32(node.e - b));
	if (node.type == AST_AnchorName || node.type == AST_ContextMod || node.type == AST_ContextPos || node.type == AST_IncludeFilename || node.type == AST_MappingPrefix || node.type == AST_Option || node.type == AST_RuleAddcohortWhere || node.type == AST_RuleDirection || node.type == AST_RuleExternalCmd || node.type == AST_RuleExternalType || node.type == AST_RuleFlag || node.type == AST_RuleMoveType || node.type == AST_RuleName || node.type == AST_RuleType || node.type == AST_RuleWordform || node.type == AST_SetName || node.type == AST_SetOp || node.type == AST_SubReadingsDirection || node.type == AST_Tag || node.type == AST_TemplateName || node.type == AST_TemplateRef) {
		u_fprintf(out, " t=\"%S\"", xml_encode(node.b, node.e));
	}
	if (node.u) {
		u_fprintf(out, " u=\"%u\"", node.u);
	}
	if (node.cs.empty()) {
		u_fprintf(out, "/>\n");
		return;
	}
	u_fprintf(out, ">\n");
	for (auto& it : node.cs) {
		if (it.type == AST_Grammar) {
			print_ast(out, it.b, n + 1, it);
		}
		else {
			print_ast(out, b, n + 1, it);
		}
	}
	u_fprintf(out, "%s</%s>\n", indent.data(), ASTType_str[node.type]);
}

struct ASTHelper {
	ASTNode* c;
	ASTHelper* h;

	ASTHelper(ASTType type = AST_Unknown, size_t line = 0, const UChar* b = nullptr, const UChar* e = nullptr)
	  : c(cur_ast)
	  , h(cur_ast_help)
	{
		cur_ast_help = this;
		if (!parse_ast) {
			c = nullptr;
			h = nullptr;
			return;
		}
		c->cs.push_back(ASTNode(type, line, b, e));
		cur_ast = &c->cs.back();
	}

	~ASTHelper() {
		if (c || h) {
			destroy();
		}
	}

	void destroy() {
		if (!parse_ast) {
			return;
		}
		cur_ast = c;
		cur_ast_help = h;
		c = nullptr;
		h = nullptr;
	}
};

#ifdef _MSC_VER
	// warning C4127: conditional expression is constant
	#pragma warning (disable: 4127)
#endif
#define _AST_CONCAT(x, y) x##y
#define _AST_CONCAT2(x, y) _AST_CONCAT(x, y)
#define AST_OPEN(type)               \
	ASTType_str[AST_##type] = #type; \
	ASTHelper _AST_CONCAT2(_ast_, __LINE__)(AST_##type, result->lines, p)
#define AST_CLOSE(p)             \
	do {                         \
		cur_ast->e = (p);        \
		cur_ast_help->destroy(); \
	} while (false)
#define AST_CLOSE_ID(p, n)       \
	do {                         \
		cur_ast->e = (p);        \
		cur_ast->u = (n);        \
		cur_ast_help->destroy(); \
	} while (false)

#endif
