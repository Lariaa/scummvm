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

#ifndef GRAPHICS_MACGUI_MACFONTMANAGER_H
#define GRAPHICS_MACGUI_MACFONTMANAGER_H

#include "common/hash-ptr.h"
#include "common/language.h"
#include "common/util.h"
#include "graphics/fonts/bdf.h"
#include "graphics/fontman.h"

#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif

namespace Common {
	class SeekableReadStream;
	class MacResManager;
}

namespace Graphics {

struct TTFMap {
	const char *ttfName;
	uint16 slant;
};

class MacFONTFont;
class MacFontFamily;

enum {
	kMacFontNonStandard = -1,
	kMacFontSystem = 0,
	kMacFontApplication = 1,

	kMacFontNewYork = 2,
	kMacFontGeneva = 3,
	kMacFontMonaco = 4,
	kMacFontVenice = 5,
	kMacFontLondon = 6,
	kMacFontAthens = 7,
	kMacFontSanFrancisco = 8,
	kMacFontToronto = 9,
	kMacFontCairo = 11,
	kMacFontLosAngeles = 12,
	kMacFontZapfDingbats = 13,
	kMacFontBookman = 14,
	kMacFontHelveticaNarrow = 15,
	kMacFontPalatino = 16,
	kMacFontZapfChancery = 18,
	kMacFontTimes = 20,
	kMacFontHelvetica = 21,
	kMacFontCourier = 22,
	kMacFontSymbol = 23,
	kMacFontTaliesin = 24,
	kMacFontAvantGarde = 33,
	kMacFontNewCenturySchoolbook = 34,
	kMacFontChicago = 16383,

	kMacFontOsaka = 16384,
	kMacFontBookMinchoM = 16396,
	kMacFontMonoGothic = 16433,
	kMacFontMonoMing = 16435,
	kMacFontOsakaMono = 16436,
	kMacFontMediumGothic = 16640,
	kMacFontMing = 16641,
	kMacFontHeiseiMincho = 16700,
	kMacFontHeiseiKakuGothic = 16701
};

enum {
	kMacFontRegular,
	kMacFontBold = 1,
	kMacFontItalic = 2,
	kMacFontUnderline = 4,
	kMacFontOutline = 8,
	kMacFontShadow = 16,
	kMacFontCondense = 32,
	kMacFontExtend = 64
};

class Font;

struct FontInfo {
	Common::Language lang;
	Common::CodePage encoding;
	int aliasForId;
	Common::String name;

	FontInfo() : lang(Common::UNK_LANG), encoding(Common::kCodePageInvalid), aliasForId(-1) {}
};

class MacFont {
public:
	MacFont(int id = kMacFontSystem, int size = 12, int slant = kMacFontRegular) {
		_id = id;
		_size = size ? size : 12;
		_slant = slant;
		_fallback = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		_fallbackName = Common::String(((const BdfFont *)_fallback)->getFamilyName());
		_generated = false;
		_truetype = false;
		_font = NULL;
	}

	int getId() const { return _id; };
	void setId(int id) { _id = id; }
	int getSize() const { return _size; }
	int getSlant() const { return _slant; }
	Common::String getName() const { return _name; }
	void setName(Common::String &name) { setName(name.c_str()); }
	void setName(const char *name);
	const Graphics::Font *getFallback() const { return _fallback; }
	bool isGenerated() const { return _generated; }
	void setGenerated(bool gen) { _generated = gen; }
	bool isTrueType() const { return _truetype; }
	Font *getFont() const { return _font; }
	void setFont(Font *font, bool truetype) { _font = font; _truetype = truetype; }
	void setFallback(const Font *font, Common::String name = "");
	Common::String getFallbackName() const { return _fallbackName; }

private:
	int _id;
	int _size;
	int _slant;
	bool _truetype;
	Common::String _name;
	const Graphics::Font *_fallback;
	Common::String _fallbackName;

	bool _generated;
	Font *_font;
};

class MacFontManager {
public:
	MacFontManager(uint32 mode, Common::Language language);
	~MacFontManager();

	void setLocalizedFonts();

	/**
	 * Accessor method to check the presence of built-in fonts.
	 * @return True if there are bult-in fonts.
	 */
	bool hasBuiltInFonts() { return _builtInFonts; }
	/**
	 * Retrieve a font from the available ones.
	 * @param name Name of the desired font.
	 * @param fallback Fallback policy in case the desired font isn't there.
	 * @return The requested font or the fallback.
	 */
	const Font *getFont(MacFont *macFont);
	const Font *getFont(MacFont macFont);

	/**
	 * Return font name from standard ID
	 * @param id ID of the font
	 * @param size size of the font
	 * @return the font name or NULL if ID goes beyond the mapping
	 */
	const Common::String getFontName(uint16 id, int size, int slant = kMacFontRegular, bool tryGen = false);
	const Common::String getFontName(const MacFont &font);
	int getFontIdByName(Common::String name);

	Common::Language getFontLanguage(uint16 id);
	Common::CodePage getFontEncoding(uint16 id);
	int getFontAliasForId(uint16 id);
	Common::String getFontName(uint16 id);

	void loadFonts(Common::SeekableReadStream *stream);
	void loadFonts(const Common::Path &fileName);
	void loadMacFont(Common::MacResManager *fontFile, const Common::String &family, Common::SeekableReadStream *fond);
	void loadFonts(Common::MacResManager *fontFile);
	void loadWindowsFont(const Common::Path &fileName);

	/**
	 * Register a font name if it doesn't already exist.
	 * @param name name of the font
	 * @return the font's ID
	 */
	int registerFontName(Common::String name, int preferredId = -1);

	/**
	 * Hand the manager the bytes of a font file for an already registered
	 * name, so that the next request for it is rendered from them instead of
	 * falling back on a substitute. The manager takes the stream over.
	 *
	 * This is the same shelf the Japanese TTFs are put on; it is public
	 * because a game can carry its own font inside its data. Director does
	 * exactly that, as a font cast member whose payload is a Bitstream
	 * Portable Font Resource.
	 *
	 * @param fontId the id registerFontName() gave the name
	 * @param stream the font file's bytes
	 */
	void registerFontData(int fontId, Common::SeekableReadStream *stream);

	/**
	 * Hand the manager the advance widths of a font it cannot otherwise read,
	 * so that whatever substitute draws the letters still measures the line
	 * the way the real font would.
	 *
	 * Director's embedded fonts are the case this exists for. They are
	 * Bitstream Portable Font Resources whose glyph programs no font library
	 * here can rasterise, but whose advance widths are plain numbers in the
	 * file. Drawing in a substitute and measuring with these keeps a line's
	 * length, its breaks and its centring where the author put them, which is
	 * what goes visibly wrong first -- a title set in a wider face overflows
	 * its box or breaks onto a line that was never meant to exist.
	 *
	 * The same goes for how tall a line of it stands. A substitute's ascent
	 * and descent are its own, and a line set in them is a different height
	 * from the one the author laid out -- which pushes every line after it
	 * down and the last one out of the bottom of the box.
	 *
	 * @param fontId the id registerFontName() gave the name
	 * @param widths advance width per character code, in font units
	 * @param unitsPerEm how many of those units make up the em square
	 * @param ascent how far the font rises above the baseline, in font units
	 * @param descent how far it falls below it, as a positive magnitude;
	 *                pass 0 for both when the file does not say
	 */
	void registerFontAdvanceWidths(int fontId, const Common::HashMap<uint32, int32> &widths, int unitsPerEm,
			int ascent = 0, int descent = 0);

	void forceBuiltinFonts() { _builtInFonts = true; }
	int parseSlantFromName(const Common::String &name);
	Common::String getNameFromSlant(int slantVal);

	const Common::Array<MacFontFamily *> &getFontFamilies() { return _fontFamilies; }

	void printFontRegistry(int debugLevel, uint32 channel);

	int registerTTFFont(const Graphics::TTFMap ttfList[]);

	int getFamilyId(int newId, int newSlant);

	/**
	 * Whether this font addresses its glyphs by Unicode code point rather than
	 * by a byte in some code page. A TTF substitute does; a Mac bitmap font
	 * does not.
	 */
	bool isUnicodeFont(const Graphics::Font *font) const { return font && _unicodeFonts.contains(font); }

#ifdef USE_FREETYPE2
	/**
	 * Set the TTF rendering mode used when loading TTF fonts.
	 * Defaults to kTTFRenderModeMonochrome for classic Mac look.
	 * Set to kTTFRenderModeLight for anti-aliased text.
	 */
	void setTTFRenderMode(TTFRenderMode mode) { _ttfRenderMode = mode; }
	TTFRenderMode getTTFRenderMode() const { return _ttfRenderMode; }
#endif

private:
	void loadFontsBDF();
	void loadFonts();
	void loadJapaneseFonts();

	const Font *resolveFont(MacFont *macFont);
	const Font *applyAdvanceWidths(int fontId, int size, const Font *font);

	void generateFontSubstitute(MacFont &macFont);
	void generateFONTFont(MacFont &toFont, MacFont &fromFont);

#ifdef USE_FREETYPE2
	void generateTTFFont(MacFont &toFront, Common::SeekableReadStream *stream);
	TTFRenderMode _ttfRenderMode = Graphics::kTTFRenderModeMonochrome;
#endif

private:
	bool _builtInFonts;
	bool _japaneseFontsLoaded;
	uint32 _mode;
	Common::Language _language;
	Common::HashMap<Common::String, Graphics::Font *> _winFontRegistry;
	Common::HashMap<Common::String, MacFont *> _fontRegistry;
	Common::Array<MacFontFamily *> _fontFamilies;

	Common::HashMap<int, FontInfo *> _fontInfo;
	Common::HashMap<Common::String, int> _fontIds;

	int parseFontSlant(Common::String slant);

	/* Unicode font */
	Common::HashMap<Common::String, const Graphics::Font *> _uniFontRegistry;

	// The fonts in _uniFontRegistry again, by pointer, so that a caller holding
	// only the resolved font can ask whether it takes code points.
	Common::HashMap<const Graphics::Font *, bool> _unicodeFonts;

	Common::HashMap<Common::String, Common::SeekableReadStream *> _ttfData;

	// The advance widths a font carried with it, by font id, waiting for a
	// substitute to lend its letters to. See registerFontAdvanceWidths().
	struct AdvanceWidths {
		Common::HashMap<uint32, int32> widths;
		int unitsPerEm = 0;
		int ascent = 0;
		int descent = 0;
	};
	Common::HashMap<int, AdvanceWidths *> _advanceWidths;

	// The wrappers handed out so far, by font name, so that the same request
	// keeps answering with the same pointer -- isUnicodeFont() and the text
	// machinery key on it.
	Common::HashMap<Common::String, const Font *> _measuredFonts;
};

/**
 * A font that draws with one font's glyphs and measures with another's advance
 * widths.
 *
 * It exists for a font a game carries that nothing here can rasterise: the
 * widths are read out of the file and a substitute supplies the shapes. The
 * letters then look wrong but sit in the right places, which is the lesser of
 * the two errors -- a line measured in the wrong widths overflows its box or
 * breaks where it should not, and that is what a reader notices.
 *
 * It answers about its height the same way, when the file said how tall the
 * font stands. That is what a text layout asks for the height of a line, and
 * a substitute's own ascent and descent make the line a different height from
 * the one the author laid out. Director's text Xtra writes the authored
 * numbers into the document beside every style run, which is how these can be
 * checked: TKKG 10's title card stores 139 and 33 for its 160 pt line and 84
 * and 20 for its 96 pt one, and the font's 870 and 208 over 1000 units per em
 * give exactly those four numbers back.
 *
 * Everything else is the inner font's answer. Kerning is dropped on purpose:
 * the inner font's pair adjustments belong to its own widths, not to these.
 */
class MeasuredFont : public Font {
public:
	MeasuredFont(const Font *glyphs, const Common::HashMap<uint32, int32> *widths, int unitsPerEm, int size,
			int ascent = 0, int descent = 0)
			: _glyphs(glyphs), _widths(widths), _unitsPerEm(unitsPerEm), _size(size),
			  _ascent(ascent), _descent(descent) {}

	// Rounded apart and then added, the way the authored numbers come out.
	int getFontHeight() const override {
		return hasMetrics() ? scale(_ascent) + scale(_descent) : _glyphs->getFontHeight();
	}
	int getFontAscent() const override {
		return hasMetrics() ? scale(_ascent) : _glyphs->getFontAscent();
	}
	int getFontDescent() const override {
		return hasMetrics() ? scale(_descent) : _glyphs->getFontDescent();
	}
	int getFontLeading() const override { return _glyphs->getFontLeading(); }
	Common::String getFontName() const override { return _glyphs->getFontName(); }

	int getCharWidth(uint32 chr) const override {
		int w = scaled(chr);
		return w >= 0 ? w : _glyphs->getCharWidth(chr);
	}

	int getMaxCharWidth() const override {
		int maxWidth = 0;
		for (auto &it : *_widths)
			maxWidth = MAX(maxWidth, scale(it._value));
		return MAX(maxWidth, _glyphs->getMaxCharWidth());
	}

	// The inner font's kerning was measured against its own advances.
	int getKerningOffset(uint32 left, uint32 right) const override { return 0; }

	Common::Rect getBoundingBox(uint32 chr) const override { return _glyphs->getBoundingBox(chr); }

	void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const override {
		_glyphs->drawChar(dst, chr, x, y, color);
	}
	void drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override {
		_glyphs->drawChar(dst, chr, x, y, color);
	}

private:
	// A font may bring widths without the auxiliary records that carry the
	// ascent and the descent, and then the inner font keeps answering.
	bool hasMetrics() const { return _ascent > 0 || _descent > 0; }

	int scale(int32 units) const {
		// Round to nearest: a title at 170 pt loses a visible amount over a
		// line of text if every character is truncated downwards.
		return (int)((units * (int32)_size + _unitsPerEm / 2) / _unitsPerEm);
	}

	int scaled(uint32 chr) const {
		if (!_widths->contains(chr))
			return -1;
		return scale(_widths->getVal(chr));
	}

	const Font *_glyphs;
	const Common::HashMap<uint32, int32> *_widths;
	int _unitsPerEm;
	int _size;
	int _ascent;
	int _descent;
};

} // End of namespace Graphics

#endif
