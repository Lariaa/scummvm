/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/str.h"
#include "common/system.h"
#include "common/util.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/x/xmlparser.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9
 *
 **************************************************/

/*
-- xtra XmlParser
new object me
-- XmlParser Xtra --
parseString object me, string data -- parses the buffer
parseURL object me, string url, * -- parses the specified url
doneParsing object me -- returns true if it's done parsing a URL
ignoreWhitespace object me, int ignore -- toggles whether to ignore whitespace
getError object me -- returns the error string (if any) generated when parsing
makeList object me -- make a lingo list based on the XML document
 */

namespace Director {

const char *XMLParserXtra::xlibName = "XMLParser";
const XlibFileDesc XMLParserXtra::fileNames[] = {
	{ "xmlparser",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				XMLParserXtra::m_new,		 0, 0,	500 },
	{ "parseString",				XMLParserXtra::m_parseString,		 1, 1,	500 },
	{ "parseURL",				XMLParserXtra::m_parseURL,		 -1, 0,	500 },
	{ "doneParsing",				XMLParserXtra::m_doneParsing,		 0, 0,	500 },
	{ "ignoreWhitespace",				XMLParserXtra::m_ignoreWhitespace,		 1, 1,	500 },
	{ "getError",				XMLParserXtra::m_getError,		 0, 0,	500 },
	{ "makeList",				XMLParserXtra::m_makeList,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

XMLParserXtraObject::XMLParserXtraObject(ObjectType ObjectType) :Object<XMLParserXtraObject>("XMLParser") {
	_objType = ObjectType;
}

static Datum nodeField(const Datum &node, const char *field);

bool XMLParserXtraObject::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("name")
		|| propName.equalsIgnoreCase("child")
		|| propName.equalsIgnoreCase("text")
		|| propName.equalsIgnoreCase("attributeName")
		|| propName.equalsIgnoreCase("attributeValue");
}

Datum XMLParserXtraObject::getProp(const Common::String &propName) {
	// The parser object doubles as the document node, so `parserObj.child`
	// walks straight into the tree. `name` keeps answering the Xtra's own name
	// rather than the document node's empty one, as it did before.
	if (propName.equalsIgnoreCase("name"))
		return Datum(XMLParserXtra::xlibName);

	if (hasProp(propName))
		return nodeField(_doc, propName.c_str());

	warning("XMLParserXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void XMLParserXtra::open(ObjectType type, const Common::Path &path) {
    XMLParserXtraObject::initMethods(xlibMethods);
    XMLParserXtraObject *xobj = new XMLParserXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void XMLParserXtra::close(ObjectType type) {
    XMLParserXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

// A small, tolerant XML reader. Common::XMLParser is schema-driven -- it wants
// a fixed key layout declared up front -- while an Xtra has to swallow whatever
// document a game ships, so this reads a generic tree instead.
//
// Nodes are handed to Lingo as property lists rather than as objects:
//   [#name: "SET", #text: "", #attributeName: ["NAME"],
//    #attributeValue: ["NAME": "Mensch"], #child: [ ... ]]
// A property list answers `node.name` through Lingo's own PARRAY lookup, so the
// node model needs no object type of its own and nothing has to own it.
// Text is a child node with an empty #name, which is how the games read it:
// `element.child[i].child[1].text`.

struct XMLAttr {
	Common::String name;
	Common::String value;
};

static bool isNameStart(char c) {
	return Common::isAlpha(c) || c == '_' || c == ':';
}

static bool isNameChar(char c) {
	return Common::isAlnum(c) || c == '_' || c == ':' || c == '-' || c == '.';
}

static Common::String decodeEntities(const Common::String &in) {
	Common::String out;

	for (uint i = 0; i < in.size(); i++) {
		if (in[i] != '&') {
			out += in[i];
			continue;
		}

		uint end = i + 1;
		while (end < in.size() && in[end] != ';' && end - i <= 10)
			end++;
		if (end >= in.size() || in[end] != ';') {
			out += in[i];
			continue;
		}

		Common::String ent(in.c_str() + i + 1, end - i - 1);
		if (ent.equals("amp")) {
			out += '&';
		} else if (ent.equals("lt")) {
			out += '<';
		} else if (ent.equals("gt")) {
			out += '>';
		} else if (ent.equals("quot")) {
			out += '"';
		} else if (ent.equals("apos")) {
			out += '\'';
		} else if (ent.size() > 1 && ent[0] == '#') {
			int code = (ent[1] == 'x' || ent[1] == 'X')
				? (int)strtol(ent.c_str() + 2, nullptr, 16)
				: atoi(ent.c_str() + 1);
			// Lingo strings are bytes here; anything above Latin-1 would have
			// to be guessed at, so drop it rather than invent an encoding.
			if (code > 0 && code < 256)
				out += (char)code;
		} else {
			// Unknown entity: leave it as written.
			out += '&';
			out += ent;
			out += ';';
		}
		i = end;
	}

	return out;
}

static Datum makeSymbol(const char *name) {
	// Not `Datum d(Common::String(name))`: that parses as a function
	// declaration rather than a variable.
	Common::String propName(name);
	Datum d(propName);
	d.type = SYMBOL;
	return d;
}

static Datum makeList() {
	Datum d;
	d.type = ARRAY;
	d.u.farr = new FArray;
	return d;
}

static Datum makeNode(const Common::String &name, const Common::String &text,
		const Common::Array<XMLAttr> &attrs, const Datum &children) {
	Datum attrNames = makeList();
	Datum attrValues;
	attrValues.type = PARRAY;
	attrValues.u.parr = new PArray;

	for (uint i = 0; i < attrs.size(); i++) {
		attrNames.u.farr->arr.push_back(Datum(attrs[i].name));
		attrValues.u.parr->arr.push_back(PCell(Datum(attrs[i].name), Datum(attrs[i].value)));
	}

	Datum node;
	node.type = PARRAY;
	node.u.parr = new PArray;
	node.u.parr->arr.push_back(PCell(makeSymbol("name"), Datum(name)));
	node.u.parr->arr.push_back(PCell(makeSymbol("text"), Datum(text)));
	node.u.parr->arr.push_back(PCell(makeSymbol("attributeName"), attrNames));
	node.u.parr->arr.push_back(PCell(makeSymbol("attributeValue"), attrValues));
	node.u.parr->arr.push_back(PCell(makeSymbol("child"), children));
	return node;
}

static void addText(Datum &parent, const Common::String &text, bool decode, bool ignoreWhitespace) {
	if (text.empty())
		return;

	if (ignoreWhitespace) {
		bool blank = true;
		for (uint i = 0; i < text.size(); i++) {
			if (!Common::isSpace(text[i])) {
				blank = false;
				break;
			}
		}
		if (blank)
			return;
	}

	Common::Array<XMLAttr> noAttrs;
	parent.u.farr->arr.push_back(makeNode("", decode ? decodeEntities(text) : text, noAttrs, makeList()));
}

// Returns the document node. Its #child holds the top-level content, which is
// what `parserObj.child` reads.
static Datum parseDocument(const Common::String &src, bool ignoreWhitespace, Common::String &error) {
	Common::Array<Common::String> openNames;
	Common::Array<Common::Array<XMLAttr> > openAttrs;
	Common::Array<Datum> openChildren;

	openChildren.push_back(makeList());

	const char *p = src.c_str();

	while (*p && error.empty()) {
		if (*p != '<') {
			const char *start = p;
			while (*p && *p != '<')
				p++;
			addText(openChildren.back(), Common::String(start, p - start), true, ignoreWhitespace);
			continue;
		}

		if (!strncmp(p, "<!--", 4)) {
			const char *end = strstr(p + 4, "-->");
			if (!end) {
				error = "Unterminated comment";
				break;
			}
			p = end + 3;
			continue;
		}

		if (!strncmp(p, "<![CDATA[", 9)) {
			const char *end = strstr(p + 9, "]]>");
			if (!end) {
				error = "Unterminated CDATA section";
				break;
			}
			// CDATA is literal: no entity decoding, and it is kept even when
			// it is all whitespace, because it was written out deliberately.
			addText(openChildren.back(), Common::String(p + 9, end - (p + 9)), false, false);
			p = end + 3;
			continue;
		}

		if (!strncmp(p, "<?", 2)) {
			const char *end = strstr(p + 2, "?>");
			if (!end) {
				error = "Unterminated processing instruction";
				break;
			}
			p = end + 2;
			continue;
		}

		if (!strncmp(p, "<!", 2)) {
			const char *end = strchr(p + 2, '>');
			if (!end) {
				error = "Unterminated declaration";
				break;
			}
			p = end + 1;
			continue;
		}

		if (p[1] == '/') {
			p += 2;
			const char *start = p;
			while (*p && isNameChar(*p))
				p++;
			Common::String name(start, p - start);
			while (*p && *p != '>')
				p++;
			if (*p)
				p++;

			if (openNames.empty()) {
				error = Common::String::format("Unexpected closing tag </%s>", name.c_str());
				break;
			}
			if (!openNames.back().equalsIgnoreCase(name)) {
				error = Common::String::format("Closing tag </%s> does not match <%s>",
						name.c_str(), openNames.back().c_str());
				break;
			}

			Datum kids = openChildren.back();
			openChildren.pop_back();
			openChildren.back().u.farr->arr.push_back(makeNode(openNames.back(), "", openAttrs.back(), kids));
			openNames.pop_back();
			openAttrs.pop_back();
			continue;
		}

		p++;
		if (!isNameStart(*p)) {
			error = "Malformed tag";
			break;
		}
		const char *start = p;
		while (*p && isNameChar(*p))
			p++;
		Common::String name(start, p - start);

		Common::Array<XMLAttr> attrs;
		bool selfClosing = false;
		bool closed = false;

		while (*p) {
			while (*p && Common::isSpace(*p))
				p++;

			if (*p == '/' && p[1] == '>') {
				selfClosing = true;
				closed = true;
				p += 2;
				break;
			}
			if (*p == '>') {
				closed = true;
				p++;
				break;
			}
			if (!isNameStart(*p)) {
				error = Common::String::format("Malformed attribute in <%s>", name.c_str());
				break;
			}

			const char *attrStart = p;
			while (*p && isNameChar(*p))
				p++;

			XMLAttr attr;
			attr.name = Common::String(attrStart, p - attrStart);

			while (*p && Common::isSpace(*p))
				p++;
			if (*p == '=') {
				p++;
				while (*p && Common::isSpace(*p))
					p++;

				char quote = *p;
				if (quote == '"' || quote == '\'') {
					p++;
					const char *valStart = p;
					while (*p && *p != quote)
						p++;
					attr.value = decodeEntities(Common::String(valStart, p - valStart));
					if (*p)
						p++;
				} else {
					const char *valStart = p;
					while (*p && !Common::isSpace(*p) && *p != '>' && *p != '/')
						p++;
					attr.value = decodeEntities(Common::String(valStart, p - valStart));
				}
			}

			attrs.push_back(attr);
		}

		if (!error.empty())
			break;
		if (!closed) {
			error = Common::String::format("Unterminated tag <%s>", name.c_str());
			break;
		}

		if (selfClosing) {
			openChildren.back().u.farr->arr.push_back(makeNode(name, "", attrs, makeList()));
		} else {
			openNames.push_back(name);
			openAttrs.push_back(attrs);
			openChildren.push_back(makeList());
		}
	}

	if (error.empty() && !openNames.empty())
		error = Common::String::format("Unclosed tag <%s>", openNames.back().c_str());

	Common::Array<XMLAttr> noAttrs;
	return makeNode("", "", noAttrs, openChildren[0]);
}

static XMLParserXtraObject *getInstance() {
	if (g_lingo->_state->me.type != OBJECT)
		return nullptr;
	return (XMLParserXtraObject *)g_lingo->_state->me.u.obj;
}

// Reads one field out of a node property list.
static Datum nodeField(const Datum &node, const char *field) {
	if (node.type != PARRAY)
		return Datum();

	for (uint i = 0; i < node.u.parr->arr.size(); i++) {
		if (node.u.parr->arr[i].p.asString().equalsIgnoreCase(field))
			return node.u.parr->arr[i].v;
	}
	return Datum();
}

void XMLParserXtra::m_new(int nargs) {
	g_lingo->dropStack(nargs);

	// open() exposes a single instance and every new() hands back that same
	// object, so a movie can only have one parser alive at a time. No game
	// seen so far keeps two.
	XMLParserXtraObject *me = getInstance();
	if (me) {
		me->_doc = Datum();
		me->_error.clear();
	}

	g_lingo->push(g_lingo->_state->me);
}

void XMLParserXtra::m_parseString(int nargs) {
	Datum data = g_lingo->pop();
	XMLParserXtraObject *me = getInstance();
	if (!me) {
		g_lingo->push(Datum(0));
		return;
	}

	me->_error.clear();
	me->_doc = parseDocument(data.asString(), me->_ignoreWhitespace, me->_error);

	if (me->_error.empty()) {
		Datum top = nodeField(me->_doc, "child");
		debugC(3, kDebugXObj, "XMLParserXtra::m_parseString(): parsed %u bytes into %u top-level nodes",
				(uint)data.asString().size(), top.type == ARRAY ? (uint)top.u.farr->arr.size() : 0);
	} else {
		warning("XMLParserXtra::m_parseString(): %s", me->_error.c_str());
	}

	g_lingo->push(Datum(0));
}

void XMLParserXtra::m_getError(int nargs) {
	g_lingo->dropStack(nargs);

	// The documented contract is "the error string (if any)", and games test it
	// with voidp() -- TKKG 7 aborts its whole photofit program when this
	// answers 0 on a good parse. No error must mean VOID.
	XMLParserXtraObject *me = getInstance();
	if (!me || me->_error.empty())
		g_lingo->pushVoid();
	else
		g_lingo->push(Datum(me->_error));
}

void XMLParserXtra::m_makeList(int nargs) {
	g_lingo->dropStack(nargs);

	XMLParserXtraObject *me = getInstance();
	g_lingo->push(me ? me->_doc : Datum());
}

void XMLParserXtra::m_ignoreWhitespace(int nargs) {
	Datum d = g_lingo->pop();

	XMLParserXtraObject *me = getInstance();
	if (me)
		me->_ignoreWhitespace = d.asInt() != 0;

	g_lingo->push(Datum(0));
}

void XMLParserXtra::m_doneParsing(int nargs) {
	g_lingo->dropStack(nargs);

	// parseString() is synchronous, so a parse is always finished by the time
	// anyone can ask. Only parseURL() could answer otherwise, and that is still
	// a stub.
	g_lingo->push(Datum(1));
}

XOBJSTUB(XMLParserXtra::m_parseURL, 0)

}
