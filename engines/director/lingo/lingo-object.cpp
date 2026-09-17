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

#include "common/endian.h"

#include "graphics/macgui/mactext.h"
#include "graphics/managed_surface.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-the.h"

#include "director/lingo/xlibs/a/aiff.h"
#include "director/lingo/xlibs/a/applecdxobj.h"
#include "director/lingo/xlibs/a/askuser.h"
#include "director/lingo/xlibs/b/backdrop.h"
#include "director/lingo/xlibs/b/barakeobj.h"
#include "director/lingo/xlibs/b/batqt.h"
#include "director/lingo/xlibs/b/bimxobj.h"
#include "director/lingo/xlibs/b/blitpict.h"
#include "director/lingo/xlibs/b/blockthedrawingxobj.h"
#include "director/lingo/xlibs/c/cdromxobj.h"
#include "director/lingo/xlibs/c/closebleedwindowxcmd.h"
#include "director/lingo/xlibs/c/colorxobj.h"
#include "director/lingo/xlibs/c/colorcursorxobj.h"
#include "director/lingo/xlibs/c/consumer.h"
#include "director/lingo/xlibs/c/cursorxobj.h"
#include "director/lingo/xlibs/d/darkenscreen.h"
#include "director/lingo/xlibs/d/dateutil.h"
#include "director/lingo/xlibs/d/developerStack.h"
#include "director/lingo/xlibs/d/dialogsxobj.h"
#include "director/lingo/xlibs/d/dirutil.h"
#include "director/lingo/xlibs/d/dllglue.h"
#include "director/lingo/xlibs/d/dpwavi.h"
#include "director/lingo/xlibs/d/dpwqtw.h"
#include "director/lingo/xlibs/d/draw.h"
#include "director/lingo/xlibs/e/ecdctrl.h"
#include "director/lingo/xlibs/e/ednox.h"
#include "director/lingo/xlibs/e/eventq.h"
#include "director/lingo/xlibs/f/fadegammadownxcmd.h"
#include "director/lingo/xlibs/f/fadegammaupxcmd.h"
#include "director/lingo/xlibs/f/fadegammaxcmd.h"
#include "director/lingo/xlibs/f/fedracul.h"
#include "director/lingo/xlibs/f/feimasks.h"
#include "director/lingo/xlibs/f/feiprefs.h"
#include "director/lingo/xlibs/f/fileexists.h"
#include "director/lingo/xlibs/f/fileio.h"
#include "director/lingo/xlibs/f/findereventsxcmd.h"
#include "director/lingo/xlibs/f/findfolder.h"
#include "director/lingo/xlibs/f/findsys.h"
#include "director/lingo/xlibs/f/findwin.h"
#include "director/lingo/xlibs/f/flushxobj.h"
#include "director/lingo/xlibs/f/fplayxobj.h"
#include "director/lingo/xlibs/f/fsutil.h"
#include "director/lingo/xlibs/f/flushmousexfcn.h"
#include "director/lingo/xlibs/g/genutils.h"
#include "director/lingo/xlibs/g/getpath.h"
#include "director/lingo/xlibs/g/getscreenrectsxfcn.h"
#include "director/lingo/xlibs/g/getscreensizexfcn.h"
#include "director/lingo/xlibs/g/getsoundinlevel.h"
#include "director/lingo/xlibs/g/gpid.h"
#include "director/lingo/xlibs/g/getuinfo.h"
#include "director/lingo/xlibs/g/getsoundxfcn.h"
#include "director/lingo/xlibs/h/henry.h"
#include "director/lingo/xlibs/h/hitmap.h"
#include "director/lingo/xlibs/i/inixobj.h"
#include "director/lingo/xlibs/i/instobj.h"
#include "director/lingo/xlibs/j/jwxini.h"
#include "director/lingo/xlibs/i/iscd.h"
#include "director/lingo/xlibs/i/ispippin.h"
#include "director/lingo/xlibs/j/jitdraw3.h"
#include "director/lingo/xlibs/l/labeldrvxobj.h"
#include "director/lingo/xlibs/l/listdev.h"
#include "director/lingo/xlibs/m/maniacbg.h"
#include "director/lingo/xlibs/m/mapnavigatorxobj.h"
#include "director/lingo/xlibs/m/memcheckxobj.h"
#include "director/lingo/xlibs/m/memoryxobj.h"
#include "director/lingo/xlibs/m/misc.h"
#include "director/lingo/xlibs/m/miscx.h"
#include "director/lingo/xlibs/m/mmaskxobj.h"
#include "director/lingo/xlibs/m/mmovie.h"
#include "director/lingo/xlibs/m/moovxobj.h"
#include "director/lingo/xlibs/m/movemousejp.h"
#include "director/lingo/xlibs/m/movemousexobj.h"
#include "director/lingo/xlibs/m/movieidxxobj.h"
#include "director/lingo/xlibs/m/movutils.h"
#include "director/lingo/xlibs/m/msfile.h"
#include "director/lingo/xlibs/m/mystisle.h"
#include "director/lingo/xlibs/m/mazexobj.h"
#include "director/lingo/xlibs/m/myfolder.h"
#include "director/lingo/xlibs/o/openbleedwindowxcmd.h"
#include "director/lingo/xlibs/o/orthoplayxobj.h"
#include "director/lingo/xlibs/p/paco.h"
#include "director/lingo/xlibs/p/palxobj.h"
#include "director/lingo/xlibs/p/panel.h"
#include "director/lingo/xlibs/p/pharaohs.h"
#include "director/lingo/xlibs/p/popupmenuxobj.h"
#include "director/lingo/xlibs/p/porta.h"
#include "director/lingo/xlibs/p/prefpath.h"
#include "director/lingo/xlibs/p/printomatic.h"
#include "director/lingo/xlibs/p/processxobj.h"
#include "director/lingo/xlibs/p/putcurs.h"
#include "director/lingo/xlibs/p/playsoundmoviexobj.h"
#include "director/lingo/xlibs/q/qtcatmovieplayerxobj.h"
#include "director/lingo/xlibs/q/qtsupport.h"
#include "director/lingo/xlibs/q/qtmovie.h"
#include "director/lingo/xlibs/q/qtvr.h"
#include "director/lingo/xlibs/q/quicktime.h"
#include "director/lingo/xlibs/r/registercomponent.h"
#include "director/lingo/xlibs/r/remixxcmd.h"
#include "director/lingo/xlibs/s/serialportxobj.h"
#include "director/lingo/xlibs/s/smallutil.h"
#include "director/lingo/xlibs/s/soundjam.h"
#include "director/lingo/xlibs/s/spacemgr.h"
#include "director/lingo/xlibs/s/stagetc.h"
#include "director/lingo/xlibs/s/syscolor.h"
#include "director/lingo/xlibs/s/savenrestorexobj.h"
#include "director/lingo/xlibs/s/stagectl.h"
#include "director/lingo/xlibs/t/tengu.h"
#include "director/lingo/xlibs/t/temnotaxobj.h"
#include "director/lingo/xlibs/u/unittest.h"
#include "director/lingo/xlibs/v/valkyrie.h"
#include "director/lingo/xlibs/v/versions.h"
#include "director/lingo/xlibs/v/videodiscxobj.h"
#include "director/lingo/xlibs/v/vmisonxfcn.h"
#include "director/lingo/xlibs/v/vmpresent.h"
#include "director/lingo/xlibs/v/volumelist.h"
#include "director/lingo/xlibs/v/voyagerxsound.h"
#include "director/lingo/xlibs/w/widget.h"
#include "director/lingo/xlibs/w/window.h"
#include "director/lingo/xlibs/w/wininfo.h"
#include "director/lingo/xlibs/w/winxobj.h"
#include "director/lingo/xlibs/x/xcmdglue.h"
#include "director/lingo/xlibs/x/xio.h"
#include "director/lingo/xlibs/x/xplayanim.h"
#include "director/lingo/xlibs/x/xplaypacoxfcn.h"
#include "director/lingo/xlibs/x/xsoundxfcn.h"
#include "director/lingo/xlibs/x/xwin.h"
#include "director/lingo/xlibs/y/yasix.h"
#include "director/lingo/xtras/a/audio.h"
#include "director/lingo/xtras/b/border.h"
#include "director/lingo/xtras/a/amplitude.h"
#include "director/lingo/xtras/b/budapi.h"
#include "director/lingo/xtras/b/bamac.h"
#include "director/lingo/xtras/d/directos.h"
#include "director/lingo/xtras/d/directsound.h"
#include "director/lingo/xtras/d/displayres.h"
#include "director/lingo/xtras/d/datetime.h"
#include "director/lingo/xtras/d/directemail.h"
#include "director/lingo/xtras/d/directmedia.h"
#include "director/lingo/xtras/d/directxport.h"
#include "director/lingo/xtras/d/dirmmx.h"
#include "director/lingo/xtras/d/dmfade.h"
#include "director/lingo/xtras/d/dmxtremepack.h"
#include "director/lingo/xtras/f/fileflex.h"
#include "director/lingo/xtras/f/fileutil.h"
#include "director/lingo/xtras/f/filextra.h"
#include "director/lingo/xtras/f/filextra4.h"
#include "director/lingo/xtras/g/getdir.h"
#include "director/lingo/xtras/g/glu32.h"
#include "director/lingo/xtras/j/javaconvert.h"
#include "director/lingo/xtras/k/keypoll.h"
#include "director/lingo/xtras/m/masterapp.h"
#include "director/lingo/xtras/m/mbox.h"
#include "director/lingo/xtras/m/mui.h"
#include "director/lingo/xtras/n/netlingo.h"
#include "director/lingo/xtras/n/netfile.h"
#include "director/lingo/xtras/o/openurl.h"
#include "director/lingo/xtras/o/oscheck.h"
#include "director/lingo/xtras/p/paintx.h"
#include "director/lingo/xtras/q/qtvrxtra.h"
#include "director/lingo/xtras/r/registryreader.h"
#include "director/lingo/xtras/r/rtk.h"
#include "director/lingo/xtras/s/scrnutil.h"
#include "director/lingo/xtras/s/smacker.h"
#include "director/lingo/xtras/s/staytoonedhall.h"
#include "director/lingo/xtras/s/staytoonedball.h"
#include "director/lingo/xtras/s/staytoonedglop.h"
#include "director/lingo/xtras/s/staytoonedhigh.h"
#include "director/lingo/xtras/s/staytoonedober.h"
#include "director/lingo/xtras/s/staytoonedtoon.h"
#include "director/lingo/xtras/s/setmouse.h"
#include "director/lingo/xtras/s/scrnxtra.h"
#include "director/lingo/xtras/s/sharpexport.h"
#include "director/lingo/xtras/s/soundchannel.h"
#include "director/lingo/xtras/t/timextra.h"
#include "director/lingo/xtras/u/uihelper.h"
#include "director/lingo/xtras/v/vlist.h"
#include "director/lingo/xtras/w/wingroup.h"
#include "director/lingo/xtras/x/xsound.h"
#include "director/lingo/xtras/x/xmlparser.h"
#include "director/lingo/xtras/z/zipxtra.h"

namespace Director {

static const struct PredefinedProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	// all except window
	{ "new",					LM::m_new,					-1, 0,	kAllObj,				200 },	// D2

	// factory and XObject
	{ "describe",				LM::m_describe,				 0, 0,	kXObj,					200 },	// D2
	{ "dispose",				LM::m_dispose,				 0, 0,	kFactoryObj | kXObj,	200 },	// D2
	{ "get",					LM::m_get,					 1, 1,	kFactoryObj,			200 },	// D2
	{ "instanceRespondsTo",		LM::m_instanceRespondsTo,	 1, 1,	kXObj,					300 },	// D3
	{ "messageList",			LM::m_messageList,			 0, 0,	kXObj,					300 },	// D3
	{ "name",					LM::m_name,					 0, 0,	kXObj,					300 },	// D3
	{ "perform",				LM::m_perform,				-1, 0,	kFactoryObj | kXObj,	300 },	// D3
	{ "put",					LM::m_put,					 2, 2,	kFactoryObj,			200 },	// D2
	{ "respondsTo",				LM::m_respondsTo,			 1, 1,	kXObj,					200 },	// D2

	// script object and Xtra
	{ "birth",					LM::m_new,					-1, 0,	kScriptObj | kXtraObj,	400 },	// D4

	{ nullptr, nullptr, 0, 0, 0, 0 }
};

static const MethodProto timeoutMethods[] = {
	// timeout object -- D8
	{ "new",					LM::m_timeoutNew,			-1, 0,	800 },			// D8
	{ "forget",					LM::m_timeoutForget,		 0, 0,	800 },			// D8
	{ nullptr, nullptr, 0, 0, 0 }
};

static const MethodProto imageMethods[] = {
	// image object -- D8
	{ "copyPixels",				LM::m_imageCopyPixels,		 3, 4,	800 },			// D8
	{ "createMask",				LM::m_imageCreateMask,		 0, 0,	800 },			// D8
	{ "createMatte",			LM::m_imageCreateMatte,		 0, 1,	800 },			// D8
	{ "crop",					LM::m_imageCrop,			 1, 1,	800 },			// D8
	{ "draw",					LM::m_imageDraw,			 2, 5,	800 },			// D8
	{ "duplicate",				LM::m_imageDuplicate,		 0, 0,	800 },			// D8
	{ "extractAlpha",			LM::m_imageExtractAlpha,	 0, 0,	800 },			// D8
	{ "fill",					LM::m_imageFill,			 1, 5,	800 },			// D8
	{ "getPixel",				LM::m_imageGetPixel,		 1, 3,	800 },			// D8
	{ "setAlpha",				LM::m_imageSetAlpha,		 1, 1,	800 },			// D8
	{ "setPixel",				LM::m_imageSetPixel,		 2, 3,	800 },			// D8
	{ "trimWhiteSpace",			LM::m_imageTrimWhiteSpace,	 0, 0,	800 },			// D8
	{ nullptr, nullptr, 0, 0, 0 }
};

static const MethodProto windowMethods[] = {
	// window / stage
	{ "close",					LM::m_close,				 0, 0,	400 },			// D4
	{ "forget",					LM::m_forget,				 0, 0,	400 },			// D4
	{ "open",					LM::m_open,					 0, 0,	400 },			// D4
	{ "moveToBack",				LM::m_moveToBack,			 0, 0,	400 },			// D4
	{ "moveToFront",			LM::m_moveToFront,			 0, 0,	400 },			// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void Lingo::initMethods() {
	for (const PredefinedProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = HBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.targetType = mtd->type;
		sym.u.bltin = mtd->func;
		_methods[mtd->name] = sym;
	}
	Window::initMethods(windowMethods);
	TimeoutObject::initMethods(timeoutMethods);
	ImageObject::initMethods(imageMethods);
}

void Lingo::cleanupMethods() {
	_methods.clear();
	Window::cleanupMethods();
	TimeoutObject::cleanupMethods();
	ImageObject::cleanupMethods();
}

#define XLIBDEF(class, flags, version) \
	{ #class, class::fileNames, class::open, class::close, flags, version }

static const struct XLibProto {
	const char *className;
	const XlibFileDesc *names;
	XLibOpenerFunc opener;
	XLibCloserFunc closer;
	int type;
	int version;
} xlibs[] = {
	XLIBDEF(AiffXObj,			kXObj,			400),	// D4
	XLIBDEF(AmplitudeXtra,			kXtraObj,					500),	// D5
	XLIBDEF(AppleCDXObj,		kXObj,			300),	// D3
	XLIBDEF(AskUser,			kXObj,			400),	// D4
	XLIBDEF(AudioXtra,			kXtraObj,					500),	// D5
	XLIBDEF(BackdropXObj,		kXObj,			400),	// D4
	XLIBDEF(BarakeObj,			kXObj,			400),	// D4
	XLIBDEF(BatQT,				kXObj,			400),	// D4
	XLIBDEF(BIMXObj,			kXObj,			400),	// D4
	XLIBDEF(BlitPictXObj,		kXObj,			400),	// D4
	XLIBDEF(BlockTheDrawingXObj,			kXObj,					400),	// D4
	XLIBDEF(BorderXtra,			kXtraObj,		500),	// D5
	XLIBDEF(BudAPIMacXtra,			kXtraObj,					500),	// D5
	XLIBDEF(BudAPIXtra,			kXtraObj,					500),	// D5
	XLIBDEF(CDROMXObj,			kXObj,			200),	// D2
	XLIBDEF(CloseBleedWindowXCMD,kXObj,			300),	// D3
	XLIBDEF(ColorXObj,			kXObj,			400),	// D4
	XLIBDEF(ColorCursorXObj,	kXObj,			400),	// D4
	XLIBDEF(ConsumerXObj,		kXObj,			400),	// D4
	XLIBDEF(CursorXObj,			kXObj,			400),	// D4
	XLIBDEF(DLLGlueXObj,		kXObj,			400),	// D4
	XLIBDEF(DMXtremePackXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DPWAVIXObj,			kXObj,			300),	// D3
	XLIBDEF(DPWQTWXObj,			kXObj,			300),	// D3
	XLIBDEF(DarkenScreen,		kXObj,			300),	// D3
	XLIBDEF(DateTimeXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DateUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(DeveloperStack,		kXObj,			300),	// D3
	XLIBDEF(DialogsXObj,		kXObj,			400),	// D4
	XLIBDEF(DirMMXXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DirUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(DirectEmailXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DirectMediaXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DirectXportXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DirectOSXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DirectsoundXtra,	kXtraObj,		500),	// D5
	XLIBDEF(DisplayResXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DmFadeXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DrawXObj,			kXObj,			400),	// D4
	XLIBDEF(ECDCtrlXObj,			kXObj,					400),	// D4
	XLIBDEF(Ednox,				kXObj,			300),	// D3
	XLIBDEF(EventQXObj,			kXObj,			400),	// D4
	XLIBDEF(FEDraculXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIMasksXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIPrefsXObj,		kXObj,			400),	// D4
	XLIBDEF(FSUtilXObj,			kXObj,			400),	// D4
	XLIBDEF(FadeGammaDownXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaUpXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaXCMD,		kXObj,			400),	// D4
	XLIBDEF(FileExists,			kXObj,			300),	// D3
	XLIBDEF(FileFlexXtra,			kXtraObj,					500),	// D5
	XLIBDEF(FileIO,				kXObj | kXtraObj,200),	// D2
	XLIBDEF(FileUtilXtra,			kXtraObj,					500),	// D5
	XLIBDEF(FileXtra,			kXtraObj,		500),	// D5
	XLIBDEF(FileXtra4Xtra,			kXtraObj,					500),	// D5
	XLIBDEF(FindFolder,			kXObj,			300),	// D3
	XLIBDEF(FindSys,			kXObj,			400),	// D4
	XLIBDEF(FindWin,			kXObj,			400),	// D4
	XLIBDEF(FinderEventsXCMD,	kXObj,			400),	// D4
	XLIBDEF(FlushMouseXFCN,			kXObj,					300),	// D3
	XLIBDEF(FlushXObj,			kXObj,			300),	// D3
	XLIBDEF(FPlayXObj,			kXObj,			200),	// D2
	XLIBDEF(GLU32Xtra,			kXtraObj,					500),	// D5
	XLIBDEF(GenUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(GetDirXtra,			kXtraObj,					500),	// D5
	XLIBDEF(GetPathXObj,			kXObj,					400),	// D4
	XLIBDEF(GetScreenRectsXFCN,	kXObj,			300),	// D3
	XLIBDEF(GetScreenSizeXFCN,	kXObj,			300),	// D3
	XLIBDEF(GetSoundInLevelXObj,kXObj,			400),	// D4
	XLIBDEF(GetSoundXFCN,			kXObj,					300),	// D3
	XLIBDEF(GetUInfoXObj,			kXObj,					400),	// D4
	XLIBDEF(GpidXObj,			kXObj,			400),	// D4
	XLIBDEF(HenryXObj,			kXObj,			400),	// D4
	XLIBDEF(HitMapXObj,			kXObj,			400),	// D4
	XLIBDEF(IniXObj,			kXObj,			400),	// D4
	XLIBDEF(InstObjXObj,		kXObj,			400),	// D4
	XLIBDEF(IsCD,				kXObj,			300),	// D3
	XLIBDEF(IsPippin,			kXObj,			400),	// D4
	XLIBDEF(JITDraw3XObj,		kXObj,			400),	// D4
	XLIBDEF(JavaConvertXtra,			kXtraObj,					500),	// D5
	XLIBDEF(JourneyWareXINIXObj,kXObj,			400),	// D4
	XLIBDEF(KeypollXtra,		kXtraObj,		500),	// D5
	XLIBDEF(LabelDrvXObj,		kXObj,			400),	// D4
	XLIBDEF(ListDevXObj,		kXObj,			500),	// D5
	XLIBDEF(MBoxXtra,			kXtraObj,					500),	// D5
	XLIBDEF(MMovieXObj,			kXObj,			400),	// D4
	XLIBDEF(ManiacBgXObj,		kXObj,			300),	// D3
	XLIBDEF(MapNavigatorXObj,	kXObj,			400),	// D4
	XLIBDEF(MasterAppXtra,		kXtraObj,		500),	// D5
	XLIBDEF(MazeXObj,			kXObj,					400),	// D4
	XLIBDEF(MemCheckXObj,		kXObj,			400),	// D4
	XLIBDEF(MemoryXObj,			kXObj,			300),	// D3
	XLIBDEF(Misc,				kXObj,			400),	// D4
	XLIBDEF(MiscX,				kXObj,			400),	// D4
	XLIBDEF(MMaskXObj,			kXObj,			400),	// D4
	XLIBDEF(MoovXObj,			kXObj,			300),	// D3
	XLIBDEF(MoveMouseJPXObj,	kXObj,			400),	// D4
	XLIBDEF(MoveMouseXObj,		kXObj,			400),	// D4
	XLIBDEF(MovieIdxXObj,		kXObj,			400),	// D4
	XLIBDEF(MovUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(MSFile,             kXObj,          400),   // D4
	XLIBDEF(MuiXtra,			kXtraObj,					500),	// D5
	XLIBDEF(MyFolderXObj,			kXObj,					400),	// D4
	XLIBDEF(MystIsleXObj,		kXObj,			400),	// D4
	XLIBDEF(NetLingoXtra,			kXtraObj,					500),	// D5
	XLIBDEF(NetFileXtra,		kXtraObj,		500),	// D5
	XLIBDEF(OSCheckXtra,		kXtraObj,		400),	// D4
	XLIBDEF(OpenBleedWindowXCMD,kXObj,			300),	// D3
	XLIBDEF(OpenURLXtra,		kXtraObj,		500),	// D5
	XLIBDEF(OrthoPlayXObj,		kXObj,			400),	// D4
	XLIBDEF(PACoXObj,			kXObj,			300),	// D3
	XLIBDEF(PaintXXtra,			kXtraObj,					500),	// D5
	XLIBDEF(PalXObj,			kXObj,			400),	// D4
	XLIBDEF(PanelXObj,			kXObj,			200),	// D2
	XLIBDEF(PharaohsXObj,		kXObj,			400),	// D4
	XLIBDEF(PlaySoundMovieXObj,			kXObj,					400),	// D4
	XLIBDEF(PopUpMenuXObj,		kXObj,			200),	// D2
	XLIBDEF(Porta,				kXObj,			300),	// D3
	XLIBDEF(PrefPath,			kXObj,			400),	// D4
	XLIBDEF(PrintOMaticXObj,	kXObj | kXtraObj,400),	// D4
	XLIBDEF(ProcessXObj,		kXObj,			400),	// D4
	XLIBDEF(PutcursXObj,		kXObj,			400),	// D4
	XLIBDEF(QTCatMoviePlayerXObj,kXObj,			400),	// D4
	XLIBDEF(QTMovie,			kXObj,			400),	// D4
	XLIBDEF(QTSupport,			kXObj,			600),	// D4
	XLIBDEF(QTVR,				kXObj,			400),	// D4
	XLIBDEF(QtvrxtraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(Quicktime,			kXObj,			300),	// D3
	XLIBDEF(RearWindowXObj,		kXObj,			300),	// D3
	XLIBDEF(RegisterComponent,	kXObj,			400),	// D4
	XLIBDEF(RegistryReaderXtra,			kXtraObj,					500),	// D5
	XLIBDEF(RemixXCMD,			kXObj,			300),	// D3
	XLIBDEF(RolloverToolkitXtra,kXtraObj,		500),	// D5
	XLIBDEF(SaveNRestoreXObj,			kXObj,					400),	// D4
	XLIBDEF(ScrnUtilXtra,		kXtraObj,		500),	// D5
	XLIBDEF(ScrnXtraXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SerialPortXObj,		kXObj,			200),	// D2
	XLIBDEF(SetMouseXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SharpExportXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SmackerXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SmallUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(SoundChannelXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SoundJam,			kXObj,			400),	// D4
	XLIBDEF(SpaceMgr,			kXObj,			400),	// D4
	XLIBDEF(StageControlXObj,			kXObj,					300),	// D3
	XLIBDEF(StageTCXObj,		kXObj,			400),	// D4
	XLIBDEF(StayToonedBallXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedGlopXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedHallXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedHighXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedOberXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedToonXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SysColorXObj,		kXObj,			400),	// D4
	XLIBDEF(TemnotaXObj,			kXObj,					400),	// D4
	XLIBDEF(TenguXObj,			kXObj,			400),	// D4
	XLIBDEF(TimextraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(UiHelperXtra,			kXtraObj,					500),	// D5
	XLIBDEF(UnitTestXObj,		kXObj,			400),	// D4
	XLIBDEF(VListXtra,			kXtraObj,					500),	// D5
	XLIBDEF(VMPresentXObj,		kXObj,			400),	// D4
	XLIBDEF(VMisOnXFCN,			kXObj,			400),	// D4
	XLIBDEF(ValkyrieXObj,		kXObj,			400),	// D4
	XLIBDEF(VersionsXObj,		kXObj,			400),	// D4
	XLIBDEF(VideodiscXObj,		kXObj,			200),	// D2
	XLIBDEF(VolumeList,			kXObj,			300),	// D3
	XLIBDEF(VoyagerXSoundXObj,	kXObj,			400),	// D4
	XLIBDEF(WidgetXObj,			kXObj,					300),	// D3
	XLIBDEF(WinGroupXtra,			kXtraObj,					500),	// D5
	XLIBDEF(WinInfoXObj,		kXObj,			400),	// D4
	XLIBDEF(WindowXObj,			kXObj,			200),	// D2
	XLIBDEF(XCMDGlueXObj,		kXObj,			200),	// D2
	XLIBDEF(XMLParserXtra,			kXtraObj,					500),	// D5
	XLIBDEF(XPlayPACoXFCN,		kXObj,			300),	// D3
	XLIBDEF(XSoundXFCN,			kXObj,			400),	// D4
	XLIBDEF(XWINXObj,			kXObj,			300),	// D3
	XLIBDEF(XioXObj,			kXObj,			400),	// D3
	XLIBDEF(XPlayAnim,			kXObj,			300),	// D3
	XLIBDEF(XsoundXtra,			kXtraObj,		500),	// D5
	XLIBDEF(Yasix,				kXObj,			300),	// D3
	XLIBDEF(ZipXtraXtra,			kXtraObj,					500),	// D5
	{ nullptr, nullptr, nullptr, nullptr, 0, 0 }
};

void Lingo::initXLibs() {
	Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> quirks;
	for (const XLibProto *lib = xlibs; lib->names; lib++) {
		if (lib->version > _vm->getVersion())
			continue;

		for (uint i = 0; lib->names[i].name; i++) {
			bool isQuirk = false;
			if (lib->names[i].gameId) {
				isQuirk = strcmp(lib->names[i].gameId, g_director->getGameId()) == 0;
				// If this entry belongs to a specific game, skip it unless matched
				if (!isQuirk)
					continue;
			}

			if (isQuirk) {
				quirks[lib->names[i].name] = i;
			} else if (quirks.contains(lib->names[i].name)) {
				// Ignore new entries that conflict with per-game quirks
				continue;
			}

			if (!isQuirk && _xlibOpeners.contains(lib->names[i].name))
				warning("Lingo::initXLibs(): Duplicate entry for %s", lib->names[i].name);

			debugC(5, kDebugLingoExec, "Lingo::initXLibs(): %s -> %s", lib->names[i].name, lib->className);

			_xlibOpeners[lib->names[i].name] = lib->opener;
			_xlibClosers[lib->names[i].name] = lib->closer;
			_xlibTypes[lib->names[i].name] = lib->type;
		}
	}
}

void Lingo::cleanupXLibs() {
	_xlibOpeners.clear();
	_xlibClosers.clear();
}

Common::String Lingo::normalizeXLibName(Common::String name) {
	// Normalize to remove machintosh path delimiters (':', '@:')
	name = convertPath(name);

	size_t pos = name.findLastOf(g_director->_dirSeparator);
	if (pos != Common::String::npos)
		name = name.substr(pos + 1, name.size());

	Common::Platform platform = _vm->getPlatform();
	if (platform == Common::kPlatformMacintosh || platform == Common::kPlatformMacintoshII) {
		if (name.hasSuffixIgnoreCase(".xlib"))
			name = name.substr(0, name.size() - 5);
	} else if (platform == Common::kPlatformWindows) {
		if (name.hasSuffixIgnoreCase(".dll"))
			name = name.substr(0, name.size() - 4);
		if (name.hasSuffixIgnoreCase(".x16"))
			name = name.substr(0, name.size() - 4);
		if (name.hasSuffixIgnoreCase(".x32"))
			name = name.substr(0, name.size() - 4);
	}

	name.trim();

	return name;
}

Common::String Lingo::xlibFileName(Common::String name) {
	// Same as normalizeXLibName(), but keeps the file extension, as `the xtraList`
	// reports the Xtras by their on-disk file name (e.g. "DIRECTME.X32").
	name = convertPath(name);

	size_t pos = name.findLastOf(g_director->_dirSeparator);
	if (pos != Common::String::npos)
		name = name.substr(pos + 1, name.size());

	name.trim();

	return name;
}

void Lingo::openXLib(Common::String name, ObjectType type, const Common::Path &path) {
	Common::String fileName = xlibFileName(name);

	name = normalizeXLibName(name);

	if (_openXLibs.contains(name))
		return;

	if (type == 0 && _xlibTypes.contains(name)) {
		type = (_xlibTypes[name] & kXtraObj) ? kXtraObj : kXObj;
	}

	// manual override for game quirks
	if (name.equalsIgnoreCase("fileio")) {
		if (g_director->_fileIOType == kXtraObj && g_director->getVersion() >= 500) {
			type = kXtraObj;
		} else if (g_director->_fileIOType == kXObj) {
			type = kXObj;
		}
	}

	_openXLibs[name] = type;

	// Director keeps every Xtra it finds registered with the application, whether or
	// not the movie uses it, and `the xtraList` reports them by file name. Remember
	// the name we were opened with, including any extension; the ones we have no
	// implementation for still have to show up there, as games test the list to
	// detect Xtras. reloadOpenXLibs() reopens us under the normalized name, so keep
	// the first name seen for an Xtra.
	if (type == kXtraObj) {
		bool known = false;
		for (auto &it : _openXtraFiles) {
			if (normalizeXLibName(it).equalsIgnoreCase(name)) {
				known = true;
				break;
			}
		}

		if (!known)
			_openXtraFiles.push_back(fileName);
	}

	if (_xlibOpeners.contains(name)) {
		(*_xlibOpeners[name])(type, path);
	} else {
		warning("Lingo::openXLib: Unimplemented xlib: '%s'", name.c_str());
	}
}

void Lingo::closeXLib(Common::String name) {
	name = normalizeXLibName(name);

	if (!_openXLibs.contains(name)) {
		warning("Lingo::closeXLib: xlib %s is not open", name.c_str());
		return;
	}

	ObjectType type = _openXLibs[name];
	_openXLibs.erase(name);

	if (_xlibClosers.contains(name)) {
		(*_xlibClosers[name])(type);
	} else {
		warning("Lingo::closeXLib: Unimplemented xlib: '%s'", name.c_str());
	}
}

void Lingo::closeOpenXLibs() {
	for (auto &it : _openXLibs) {
		// does not affect Xtras
		if (it._value == kXObj) {
			closeXLib(it._key);
		}
	}
}

void Lingo::reloadOpenXLibs() {
	OpenXLibsHash openXLibsCopy = _openXLibs;
	for (auto &it : openXLibsCopy) {
		closeXLib(it._key);
		// FIXME: keep track of where the xlib path is
		openXLib(it._key, it._value, Common::Path());
	}
}

// Initialization/disposal

void LM::m_new(int nargs) {
	// This is usually overridden by a user-defined mNew
	//
	// However, in behaviors it is often absent, and it is
	// in essence our default constructor.
	g_lingo->push(g_lingo->_state->me);
}

void LM::m_dispose(int nargs) {
	g_lingo->_state->me.u.obj->dispose();
}

/* ScriptContext */

ScriptContext::ScriptContext(Common::String name, ScriptType type, int id, uint16 castLibHint, uint16 parentNumber, int scriptId)
	: Object<ScriptContext>(name), _scriptType(type), _id(id), _castLibHint(castLibHint), _parentNumber(parentNumber), _scriptId(scriptId) {
	_objType = kScriptObj;
}

ScriptContext::ScriptContext(const ScriptContext &sc) : Object<ScriptContext>(sc) {
	_scriptType = sc._scriptType;
	_functionNames = sc._functionNames;
	for (auto &it : sc._functionHandlers) {
		_functionHandlers[it._key] = it._value;
		_functionHandlers[it._key].ctx = this;
	}
	for (auto &it : sc._eventHandlers) {
		_eventHandlers[it._key] = it._value;
		_eventHandlers[it._key].ctx = this;
	}
	_constants = sc._constants;
	_properties = sc._properties;
	_propertyNames = sc._propertyNames;
	_parentNumber = sc._parentNumber;
	_scriptId = sc._scriptId;

	_id = sc._id;
	_castLibHint = sc._castLibHint;
	setCast(sc._cast);
}

ScriptContext::~ScriptContext() {
	if (_cast) {
		_cast->unregisterScriptContext(this);
	}
}

void ScriptContext::setCast(Cast *cast) {
	if (_cast == cast) {
		return;
	}
	if (_cast) {
		_cast->unregisterScriptContext(this);
	}
	_cast = cast;
	if (_cast) {
		_cast->registerScriptContext(this);
	}
}

Common::String ScriptContext::asString() {
	return Common::String::format("script: %d \"%s\" %d %p", _id, _name.c_str(), _inheritanceLevel, (void *)this);
}

Symbol ScriptContext::define(const Common::String &name, ScriptData *code, Common::Array<Common::String> *argNames, Common::Array<Common::String> *varNames) {
	Symbol sym;
	sym.name = new Common::String(name);
	sym.type = HANDLER;
	sym.u.defn = code;
	sym.nargs = argNames->size();
	sym.maxArgs = argNames->size();
	sym.argNames = argNames;
	sym.varNames = varNames;
	sym.ctx = this;

	if (debugChannelSet(1, kDebugCompile)) {
		debugC(1, kDebugCompile, "%s", g_lingo->formatFunctionBody(sym).c_str());
		debugC(1, kDebugCompile, "<end define code>");
	}

	_functionHandlers[name] = sym;
	if (g_lingo->_eventHandlerTypeIds.contains(name)) {
		_eventHandlers[g_lingo->_eventHandlerTypeIds[name]] = sym;
	}

	return sym;
}

Symbol ScriptContext::getMethod(const Common::String &methodName) {
	Symbol sym;

	if (_functionHandlers.contains(methodName)) {
		sym = _functionHandlers[methodName];
		sym.target = this;
		return sym;
	}

	sym = Object<ScriptContext>::getMethod(methodName);
	if (sym.type != VOIDSYM)
		return sym;

	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			// ancestor method
			sym = _properties["ancestor"].u.obj->getMethod(methodName);
			if (sym.type != VOIDSYM)
				debugC(3, kDebugLingoExec, "Calling method '%s' on ancestor: <%s>", methodName.c_str(), _properties["ancestor"].asString(true).c_str());
		}
	}

	return sym;
}

bool ScriptContext::hasProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return true;
	}
	if (_objType == kScriptObj) {
		// How many properties the instance holds. b_count() already answers the
		// call form, count(me); this is the same value reached as a property, which
		// is how `repeat with i = 1 to me.count` walks an instance. Tested before
		// the ancestor below because it describes this instance, not an inherited
		// one -- TKKG 13 and 14 both stall in media/intro.dir without it.
		if (propName.equalsIgnoreCase("count")) {
			return true;
		}

		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			return _properties["ancestor"].u.obj->hasProp(propName);
		}

		// This is used by behaviors
		if (propName.equalsIgnoreCase("spriteNum")) {
			return true;
		}
	}

	return false;
}

Datum ScriptContext::getProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return _properties[propName];
	}
	if (_objType == kScriptObj) {
		// Kept in step with hasProp(): without it the fallthrough at the end of this
		// function would quietly mint a new "count" property holding VOID.
		if (propName.equalsIgnoreCase("count")) {
			return Datum((int)getPropCount());
		}

		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->getProp(propName);
		}

		// This is used by behaviors
		if (propName.equalsIgnoreCase("spriteNum")) {
			return Datum((int)g_director->getCurrentMovie()->_currentSpriteNum);
		}
	}
	_propertyNames.push_back(propName);
	return _properties[propName]; // return new property
}

Common::String ScriptContext::getPropAt(uint32 index) {
	uint32 target = 1;
	for (auto &it : _propertyNames) {
		if (target == index) {
			return it;
		}
		target += 1;
	}
	return Common::String();
}

uint32 ScriptContext::getPropCount() {
	return _propertyNames.size();
}

void ScriptContext::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		_properties[propName] = value;
		return;
	}
	if (force) {
		// used by e.g. the script compiler to add properties
		_propertyNames.push_back(propName);
		_properties[propName] = value;
	} else if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			_properties["ancestor"].u.obj->setProp(propName, value, force);
		}
	} else if (_objType == kFactoryObj) {
		// D3 style anonymous objects/factories, set whatever properties you like
		_propertyNames.push_back(propName);
		_properties[propName] = value;
	}
}

Common::String ScriptContext::formatFunctionList(const char *prefix) {
	Common::String result;
	for (auto it = _functionHandlers.begin(); it != _functionHandlers.end(); ++it) {
		result += Common::String::format("%s%s\n", prefix, g_lingo->formatFunctionName(it->_value).c_str());
	}
	return result;
}


/* ColorObject */

ColorObject::ColorObject(uint8 r, uint8 g, uint8 b) : Object<ColorObject>("color") {
	_objType = kColorObj;
	_isRGB = true;
	_r = r;
	_g = g;
	_b = b;
}

ColorObject::ColorObject(int paletteIndex) : Object<ColorObject>("color") {
	_objType = kColorObj;
	_isRGB = false;
	_paletteIndex = paletteIndex;
}

// How Director prints one: `rgb( 120, 10, 35 )` or `paletteIndex( 142 )`.
Common::String ColorObject::asString() {
	if (_isRGB)
		return Common::String::format("rgb( %d, %d, %d )", _r, _g, _b);

	return Common::String::format("paletteIndex( %d )", _paletteIndex);
}

int ColorObject::toPaletteIndex() const {
	if (!_isRGB)
		return _paletteIndex;

	return (int)g_director->_wm->findBestColor(_r, _g, _b);
}

uint32 ColorObject::toPackedRGB() const {
	if (_isRGB)
		return (_r << 16) | (_g << 8) | _b;

	const byte *palette = g_director->getPalette();
	if (!palette || _paletteIndex < 0 || _paletteIndex >= (int)g_director->getPaletteColorCount())
		return 0;

	return (palette[_paletteIndex * 3] << 16) | (palette[_paletteIndex * 3 + 1] << 8) | palette[_paletteIndex * 3 + 2];
}

bool ColorObject::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("red")
		|| propName.equalsIgnoreCase("green")
		|| propName.equalsIgnoreCase("blue")
		|| propName.equalsIgnoreCase("colorType")
		|| propName.equalsIgnoreCase("paletteIndex")
		|| propName.equalsIgnoreCase("hexString")
		|| propName.equalsIgnoreCase("ilk");
}

Datum ColorObject::getProp(const Common::String &propName) {
	// The components read back whichever way the colour is stored: a
	// paletteIndex colour still answers red/green/blue, resolved through the
	// current palette, which is the whole point of the type.
	if (propName.equalsIgnoreCase("red") || propName.equalsIgnoreCase("green") || propName.equalsIgnoreCase("blue")) {
		uint32 rgb = toPackedRGB();

		if (propName.equalsIgnoreCase("red"))
			return Datum((int)((rgb >> 16) & 0xff));
		if (propName.equalsIgnoreCase("green"))
			return Datum((int)((rgb >> 8) & 0xff));
		return Datum((int)(rgb & 0xff));
	}

	if (propName.equalsIgnoreCase("colorType") || propName.equalsIgnoreCase("ilk")) {
		Datum d(Common::String(_isRGB ? "rgb" : "paletteIndex"));
		d.type = SYMBOL;
		return d;
	}

	if (propName.equalsIgnoreCase("paletteIndex"))
		return Datum(toPaletteIndex());

	if (propName.equalsIgnoreCase("hexString")) {
		uint32 rgb = toPackedRGB();
		return Datum(Common::String::format("#%02X%02X%02X", (rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff));
	}

	warning("ColorObject::getProp(): unknown property '%s'", propName.c_str());
	return Datum();
}

Common::String ColorObject::getPropAt(uint32 index) {
	static const char *rgbProps[] = { "red", "green", "blue", "colorType" };
	static const char *palProps[] = { "paletteIndex", "colorType" };

	if (_isRGB)
		return (index >= 1 && index <= 4) ? rgbProps[index - 1] : Common::String();

	return (index >= 1 && index <= 2) ? palProps[index - 1] : Common::String();
}

uint32 ColorObject::getPropCount() {
	return _isRGB ? 4 : 2;
}

void ColorObject::setProp(const Common::String &propName, const Datum &value, bool force) {
	// Assigning colorType converts in place, so `col.colorType = #paletteIndex`
	// turns an rgb() into the nearest paletteIndex() and back again.
	if (propName.equalsIgnoreCase("colorType")) {
		bool wantRGB = value.asString().equalsIgnoreCase("rgb");
		if (wantRGB == _isRGB)
			return;

		if (wantRGB) {
			uint32 rgb = toPackedRGB();
			_r = (rgb >> 16) & 0xff;
			_g = (rgb >> 8) & 0xff;
			_b = rgb & 0xff;
		} else {
			_paletteIndex = toPaletteIndex();
		}

		_isRGB = wantRGB;
		return;
	}

	if (propName.equalsIgnoreCase("paletteIndex")) {
		_paletteIndex = value.asInt();
		_isRGB = false;
		return;
	}

	if (propName.equalsIgnoreCase("red") || propName.equalsIgnoreCase("green") || propName.equalsIgnoreCase("blue")) {
		// Setting a component on a paletteIndex colour makes it an rgb one:
		// there is no other way to hold the result.
		if (!_isRGB) {
			uint32 rgb = toPackedRGB();
			_r = (rgb >> 16) & 0xff;
			_g = (rgb >> 8) & 0xff;
			_b = rgb & 0xff;
			_isRGB = true;
		}

		int v = CLIP<int>(value.asInt(), 0, 255);

		if (propName.equalsIgnoreCase("red"))
			_r = v;
		else if (propName.equalsIgnoreCase("green"))
			_g = v;
		else
			_b = v;

		return;
	}

	warning("ColorObject::setProp(): unknown property '%s'", propName.c_str());
}


// Object array

void LM::m_get(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_state->me.u.obj);
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	if (me->_objArray.contains(index)) {
		g_lingo->push(me->_objArray[index]);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_put(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_state->me.u.obj);
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	me->_objArray[index] = value;
}

// Other

void LM::m_perform(int nargs) {
	bool allowRetVal = g_lingo->pop().asInt() != 0; // Pop allowRetVal that should be used for the LC::Call

	// Lingo doesn't seem to bother cloning the object when
	// mNew is called with mPerform
	Datum d(g_lingo->_state->me);
	AbstractObject *me = d.u.obj;
	Datum methodName = g_lingo->_state->stack.remove_at(g_lingo->_state->stack.size() - nargs); // Take method name out of stack
	Symbol funcSym = me->getMethod(*methodName.u.s);
	// Object methods expect the first argument to be the object
	g_lingo->_state->stack.insert_at(g_lingo->_state->stack.size() - nargs + 1, d);
	LC::call(funcSym, nargs, allowRetVal);

	if (allowRetVal) {
		// If the method expects a return value, push dummy on stack
		g_lingo->pushVoid();
	}
}

// XObject

void LM::m_describe(int nargs) {
	warning("STUB: m_describe");
}

void LM::m_instanceRespondsTo(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_messageList(int nargs) {
	warning("STUB: m_messageList");
	g_lingo->push(Datum(""));
}

void LM::m_name(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	g_lingo->push(me->getName());
}

void LM::m_respondsTo(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	// TODO: Check inheritance level
	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

/* ImageObject */

// Lingo's image object, new in D8. TKKG 13 and 14 cannot get past their login
// without it: mainScript's goToScene() grabs the stage with
// member("stageBitmap").image = (the stage).image, and the dialogs paint
// themselves into (the stage).image with copyPixels().
//
// Every image is held in the screen's pixel format. The games shuffle pixels
// between the stage, cast members and scratch images constantly -- TKKG 14 alone
// makes 205 copyPixels calls, 91 duplicates, 75 draws and 61 fills -- and one
// common format keeps all of that to plain blits.

ImageObject::ImageObject(Graphics::ManagedSurface *surface, bool owned) : Object<ImageObject>("image") {
	_objType = kImageObj;
	_surface = surface;
	_owned = owned;
}

ImageObject::ImageObject(const ImageObject &obj) : Object<ImageObject>(obj) {
	_surface = new Graphics::ManagedSurface();
	if (obj._surface && obj._surface->w > 0 && obj._surface->h > 0)
		_surface->copyFrom(*obj._surface);
	_owned = true;
	_alphaThreshold = obj._alphaThreshold;
	_useAlpha = obj._useAlpha;
}

ImageObject::~ImageObject() {
	if (_owned && _surface) {
		_surface->free();
		delete _surface;
	}
	_surface = nullptr;
}

Common::String ImageObject::asString() {
	return Common::String::format("(image %d x %d)", _surface ? _surface->w : 0, _surface ? _surface->h : 0);
}

ImageObject *ImageObject::duplicate() const {
	Graphics::ManagedSurface *copy = new Graphics::ManagedSurface();
	if (_surface && _surface->w > 0 && _surface->h > 0)
		copy->copyFrom(*_surface);

	ImageObject *result = new ImageObject(copy, true);
	result->_alphaThreshold = _alphaThreshold;
	result->_useAlpha = _useAlpha;
	return result;
}

AbstractObject *ImageObject::clone() {
	return duplicate();
}

void ImageObject::flush() {
	if (_member.isNull())
		return;

	// A member's image is handed out as a copy, so painting into it only changes
	// the member once it is written back. Director hands out a live handle; this
	// is the same thing, one step later.
	Movie *movie = g_director->getCurrentMovie();
	CastMember *member = movie ? movie->getCastMember(_member) : nullptr;
	if (member && member->_type == kCastBitmap) {
		Datum self(this);
		member->setField(kTheImage, self);
	}
}

bool ImageObject::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("width")
		|| propName.equalsIgnoreCase("height")
		|| propName.equalsIgnoreCase("rect")
		|| propName.equalsIgnoreCase("depth")
		|| propName.equalsIgnoreCase("useAlpha")
		|| propName.equalsIgnoreCase("alphaThreshold")
		|| propName.equalsIgnoreCase("ilk");
}

Datum ImageObject::getProp(const Common::String &propName) {
	int w = _surface ? _surface->w : 0;
	int h = _surface ? _surface->h : 0;

	if (propName.equalsIgnoreCase("width"))
		return Datum(w);

	if (propName.equalsIgnoreCase("height"))
		return Datum(h);

	if (propName.equalsIgnoreCase("rect")) {
		Datum d;
		d.type = RECT;
		d.u.farr = new FArray;
		d.u.farr->arr.push_back(Datum(0));
		d.u.farr->arr.push_back(Datum(0));
		d.u.farr->arr.push_back(Datum(w));
		d.u.farr->arr.push_back(Datum(h));
		return d;
	}

	if (propName.equalsIgnoreCase("depth"))
		return Datum(_surface ? (int)(_surface->format.bytesPerPixel * 8) : 0);

	if (propName.equalsIgnoreCase("useAlpha"))
		return Datum(_useAlpha ? 1 : 0);

	if (propName.equalsIgnoreCase("alphaThreshold"))
		return Datum(_alphaThreshold);

	if (propName.equalsIgnoreCase("ilk")) {
		Datum d(Common::String("image"));
		d.type = SYMBOL;
		return d;
	}

	warning("ImageObject::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

Common::String ImageObject::getPropAt(uint32 index) {
	static const char *props[] = { "width", "height", "rect", "depth", "useAlpha", "alphaThreshold" };

	if (index < ARRAYSIZE(props))
		return props[index];

	return Common::String();
}

uint32 ImageObject::getPropCount() {
	return 6;
}

void ImageObject::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (propName.equalsIgnoreCase("useAlpha")) {
		_useAlpha = value.asInt() != 0;
		return;
	}

	if (propName.equalsIgnoreCase("alphaThreshold")) {
		_alphaThreshold = CLIP<int>(value.asInt(), 0, 255);
		return;
	}

	debugC(3, kDebugLingoExec, "ImageObject::setProp: ignoring '%s'", propName.c_str());
}

// The image a method was called on.
static ImageObject *imageMe(const char *method) {
	Datum me = g_lingo->_state->me;
	if (me.type != OBJECT || !me.u.obj || me.u.obj->getObjType() != kImageObj) {
		warning("%s: not called on an image", method);
		return nullptr;
	}

	return static_cast<ImageObject *>(me.u.obj);
}

static ImageObject *imageArg(const Datum &d) {
	if (d.type == OBJECT && d.u.obj && d.u.obj->getObjType() == kImageObj)
		return static_cast<ImageObject *>(d.u.obj);

	return nullptr;
}

// A method's arguments, in the order they were written. The image methods take
// several argument counts each, so it is easier to read them off a list than to
// pop against a guess.
static void imageArgs(int nargs, Common::Array<Datum> &args) {
	args.resize(nargs);
	for (int i = nargs - 1; i >= 0; i--)
		args[i] = g_lingo->pop();
}

// Lingo hands out rectangles with their corners in any order -- the Lexicon's
// own draw() example draws an "x" with rect(140, 30, 20, 150) -- so they have to
// be sorted before Common::Rect sees them.
static Common::Rect imageMakeRect(int x1, int y1, int x2, int y2) {
	return Common::Rect(MIN(x1, x2), MIN(y1, y2), MAX(x1, x2), MAX(y1, y2));
}

static uint32 imageWhite(const Graphics::PixelFormat &format) {
	// Index 0 is white in both system palettes.
	return format.bytesPerPixel == 1 ? 0 : format.ARGBToColor(255, 255, 255, 255);
}

static void imagePoint(const Datum &d, int &x, int &y) {
	if ((d.type == POINT || d.type == ARRAY) && d.u.farr->arr.size() >= 2) {
		x = d.u.farr->arr[0].asInt();
		y = d.u.farr->arr[1].asInt();
		return;
	}

	warning("image: expected a point, got %s", d.type2str());
}

// Lingo passes rectangles as RECT, or as a list of four numbers. copyPixels also
// takes a quad -- a list of four points for a free transform -- and its bounding
// box is as close as this gets.
static Common::Rect imageRect(const Datum &d) {
	if ((d.type == RECT || d.type == ARRAY) && d.u.farr->arr.size() >= 4) {
		const Datum &first = d.u.farr->arr[0];

		if (first.type == POINT) {
			Common::Rect r;
			bool started = false;
			for (uint i = 0; i < d.u.farr->arr.size(); i++) {
				const Datum &p = d.u.farr->arr[i];
				if (p.type != POINT || p.u.farr->arr.size() < 2)
					continue;

				int x = p.u.farr->arr[0].asInt();
				int y = p.u.farr->arr[1].asInt();
				if (!started) {
					r = Common::Rect(x, y, x, y);
					started = true;
				} else {
					r.extend(Common::Rect(x, y, x, y));
				}
			}
			return r;
		}

		return imageMakeRect(d.u.farr->arr[0].asInt(), d.u.farr->arr[1].asInt(),
				d.u.farr->arr[2].asInt(), d.u.farr->arr[3].asInt());
	}

	warning("image: expected a rect, got %s", d.type2str());
	return Common::Rect();
}

// A colour object, or a number -- which is a palette index even at higher colour
// depths (Lingo in a Nutshell, "the backColor of sprite").
static uint32 imageColor(const Datum &d, const Graphics::PixelFormat &format) {
	if (d.type == OBJECT && d.u.obj && d.u.obj->getObjType() == kColorObj) {
		ColorObject *color = static_cast<ColorObject *>(d.u.obj);
		if (format.bytesPerPixel == 1)
			return (uint32)color->toPaletteIndex();

		uint32 rgb = color->toPackedRGB();
		return format.ARGBToColor(255, (rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
	}

	int index = d.asInt();
	if (format.bytesPerPixel == 1)
		return (uint32)index;

	return g_director->transformColor(index);
}

// How bright a pixel is, 0 to 255. A mask image says with it how much of the
// source is copied -- extractAlpha() hands out exactly such a picture, and so do
// createMask() and createMatte() here.
static int imageLuminance(Graphics::ManagedSurface *surface, int x, int y) {
	uint32 pixel = surface->getPixel(x, y);
	byte r, g, b, a;

	if (surface->format.bytesPerPixel == 1) {
		const byte *palette = g_director->getPalette();
		if (!palette || (int)pixel >= g_director->getPaletteColorCount())
			return pixel ? 255 : 0;

		r = palette[pixel * 3];
		g = palette[pixel * 3 + 1];
		b = palette[pixel * 3 + 2];
	} else {
		surface->format.colorToARGB(pixel, a, r, g, b);
	}

	return (r * 77 + g * 151 + b * 28) >> 8;
}

// What copyPixels() was asked to do beyond the plain copy.
struct ImageBlitOptions {
	bool useAlpha = true;
	// 0 transparent .. 255 opaque, as #blendLevel counts.
	int blend = 255;
	Graphics::ManagedSurface *mask = nullptr;
	Common::Point maskOffset;
};

// Copy src into dst, scaling into the destination rectangle and compositing the
// source's alpha where it has one. One loop covers every combination the games
// ask for: stage to member, member to scratch image, and scaled panels.
static void imageBlit(Graphics::ManagedSurface *dst, const Common::Rect &destRect,
		Graphics::ManagedSurface *src, const Common::Rect &srcRect, const ImageBlitOptions &opts) {
	if (!dst || !src || destRect.isEmpty() || srcRect.isEmpty())
		return;

	Common::Rect clipped = destRect;
	clipped.clip(Common::Rect(dst->w, dst->h));
	if (clipped.isEmpty())
		return;

	const Graphics::PixelFormat &dstFormat = dst->format;
	const Graphics::PixelFormat &srcFormat = src->format;
	bool trueColor = dstFormat.bytesPerPixel == 4;
	bool srcAlpha = opts.useAlpha && srcFormat.bytesPerPixel == 4 && srcFormat.aBits() > 0;

	for (int y = clipped.top; y < clipped.bottom; y++) {
		int sy = srcRect.top + (y - destRect.top) * srcRect.height() / destRect.height();
		sy = CLIP<int>(sy, 0, src->h - 1);

		for (int x = clipped.left; x < clipped.right; x++) {
			int sx = srcRect.left + (x - destRect.left) * srcRect.width() / destRect.width();
			sx = CLIP<int>(sx, 0, src->w - 1);

			int coverage = opts.blend;
			if (opts.mask) {
				int mx = sx + opts.maskOffset.x;
				int my = sy + opts.maskOffset.y;
				// Beyond the mask nothing is copied.
				if (mx < 0 || my < 0 || mx >= opts.mask->w || my >= opts.mask->h)
					continue;

				coverage = coverage * imageLuminance(opts.mask, mx, my) / 255;
			}

			uint32 pixel = src->getPixel(sx, sy);
			if (srcFormat != dstFormat) {
				byte a, r, g, b;
				srcFormat.colorToARGB(pixel, a, r, g, b);
				pixel = dstFormat.ARGBToColor(a, r, g, b);
			}

			byte sa = 255, sr = 0, sg = 0, sb = 0;
			if (trueColor) {
				dstFormat.colorToARGB(pixel, sa, sr, sg, sb);
				if (!srcAlpha)
					sa = 255;
			}

			int alpha = sa * coverage / 255;
			if (alpha <= 0)
				continue;

			if (!trueColor) {
				// Nothing to blend with on a palette surface: a pixel is copied
				// whole or not at all.
				if (alpha < 128)
					continue;
			} else if (alpha < 255) {
				byte da, dr, dg, db;
				dstFormat.colorToARGB(dst->getPixel(x, y), da, dr, dg, db);
				pixel = dstFormat.ARGBToColor((byte)MAX<int>(alpha, da),
						(sr * alpha + dr * (255 - alpha)) / 255,
						(sg * alpha + dg * (255 - alpha)) / 255,
						(sb * alpha + db * (255 - alpha)) / 255);
			}

			dst->setPixel(x, y, pixel);
		}
	}
}

void LM::m_imageDuplicate(int nargs) {
	g_lingo->dropStack(nargs);

	ImageObject *me = imageMe("image.duplicate()");
	if (!me) {
		g_lingo->pushVoid();
		return;
	}

	g_lingo->push(Datum(me->duplicate()));
}

void LM::m_imageCrop(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Datum rectD = nargs > 0 ? g_lingo->pop() : Datum();

	ImageObject *me = imageMe("image.crop()");
	if (!me || !me->_surface) {
		g_lingo->pushVoid();
		return;
	}

	Common::Rect rect = imageRect(rectD);
	rect.clip(Common::Rect(me->_surface->w, me->_surface->h));

	Graphics::ManagedSurface *copy = new Graphics::ManagedSurface();
	if (!rect.isEmpty()) {
		copy->create(rect.width(), rect.height(), me->_surface->format);
		copy->blitFrom(*me->_surface, rect, Common::Point(0, 0));
	}

	g_lingo->push(Datum(new ImageObject(copy, true)));
}

void LM::m_imageCopyPixels(int nargs) {
	// copyPixels(source, destRect or destQuad, sourceRect [, paramList])
	if (nargs > 4)
		g_lingo->dropStack(nargs - 4);

	Datum params;
	if (nargs >= 4)
		params = g_lingo->pop();
	Datum srcRectD = nargs >= 3 ? g_lingo->pop() : Datum();
	Datum destD = nargs >= 2 ? g_lingo->pop() : Datum();
	Datum srcD = nargs >= 1 ? g_lingo->pop() : Datum();

	ImageObject *me = imageMe("image.copyPixels()");
	ImageObject *src = imageArg(srcD);
	if (!me || !me->_surface || !src || !src->_surface)
		return;

	ImageBlitOptions opts;
	opts.useAlpha = src->_useAlpha;

	// #blendLevel (0..255), #blend (0..100) and #maskImage are what the games
	// use; #ink, #color, #dither and #useFastQuads are taken and dropped.
	if (params.type == PARRAY) {
		for (uint i = 0; i < params.u.parr->arr.size(); i++) {
			const PCell &cell = params.u.parr->arr[i];
			Common::String key = cell.p.asString();

			if (key.equalsIgnoreCase("blendLevel")) {
				opts.blend = CLIP<int>(cell.v.asInt(), 0, 255);
			} else if (key.equalsIgnoreCase("blend")) {
				opts.blend = CLIP<int>(cell.v.asInt(), 0, 100) * 255 / 100;
			} else if (key.equalsIgnoreCase("maskImage")) {
				ImageObject *mask = imageArg(cell.v);
				if (mask)
					opts.mask = mask->_surface;
			} else if (key.equalsIgnoreCase("maskOffset")) {
				int mx = 0, my = 0;
				imagePoint(cell.v, mx, my);
				opts.maskOffset = Common::Point(mx, my);
			} else {
				debugC(5, kDebugLingoExec, "LM::m_imageCopyPixels(): ignoring #%s", key.c_str());
			}
		}
	}

	Common::Rect srcRect = imageRect(srcRectD);
	srcRect.clip(Common::Rect(src->_surface->w, src->_surface->h));

	imageBlit(me->_surface, imageRect(destD), src->_surface, srcRect, opts);
	me->flush();
}

// fill() and draw() take the same shape: a rectangle written as four numbers, as
// two points or as a rect, and then either a colour object or a parameter list.
struct ImageShapeArgs {
	Common::Rect rect;
	bool haveRect = false;
	// The corners as they were written -- a line runs from one to the other, and
	// that direction is not recoverable from the sorted rectangle.
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	Common::String shape;
	int lineSize = 0;
	Datum color;
	Datum bgColor;
};

static void imageShapeArgs(int nargs, ImageShapeArgs &out) {
	Common::Array<Datum> args;
	imageArgs(nargs, args);
	if (args.empty())
		return;

	Datum last = args.back();
	args.pop_back();

	if (args.size() >= 4) {
		out.x1 = args[0].asInt();
		out.y1 = args[1].asInt();
		out.x2 = args[2].asInt();
		out.y2 = args[3].asInt();
		out.haveRect = true;
	} else if (args.size() == 2) {
		imagePoint(args[0], out.x1, out.y1);
		imagePoint(args[1], out.x2, out.y2);
		out.haveRect = true;
	} else if (args.size() == 1) {
		Common::Rect r = imageRect(args[0]);
		out.x1 = r.left;
		out.y1 = r.top;
		out.x2 = r.right;
		out.y2 = r.bottom;
		out.haveRect = true;
	}

	if (out.haveRect)
		out.rect = imageMakeRect(out.x1, out.y1, out.x2, out.y2);

	if (last.type == PARRAY) {
		for (uint i = 0; i < last.u.parr->arr.size(); i++) {
			const PCell &cell = last.u.parr->arr[i];
			Common::String key = cell.p.asString();

			if (key.equalsIgnoreCase("shapeType"))
				out.shape = cell.v.asString();
			else if (key.equalsIgnoreCase("lineSize"))
				out.lineSize = cell.v.asInt();
			else if (key.equalsIgnoreCase("color"))
				out.color = cell.v;
			else if (key.equalsIgnoreCase("bgColor"))
				out.bgColor = cell.v;
			else
				debugC(5, kDebugLingoExec, "image: ignoring #%s", key.c_str());
		}
	} else {
		out.color = last;
	}
}

void LM::m_imageFill(int nargs) {
	// fill(color), fill(rect, colorOrParams), fill(point, point, colorOrParams)
	// or fill(left, top, right, bottom, colorOrParams). Without a parameter list
	// it is a filled rectangle with no outline.
	ImageShapeArgs args;
	imageShapeArgs(nargs, args);

	ImageObject *me = imageMe("image.fill()");
	if (!me || !me->_surface)
		return;

	Common::Rect rect = args.haveRect ? args.rect : Common::Rect(me->_surface->w, me->_surface->h);
	if (rect.isEmpty())
		return;

	uint32 color = imageColor(args.color, me->_surface->format);

	if (args.shape.equalsIgnoreCase("oval")) {
		me->_surface->drawEllipse(rect.left, rect.top, rect.right - 1, rect.bottom - 1, color, true);
	} else if (args.shape.equalsIgnoreCase("roundRect")) {
		me->_surface->drawRoundRect(rect, MIN(rect.width(), rect.height()) / 4, color, true);
	} else if (args.shape.equalsIgnoreCase("line")) {
		me->_surface->drawLine(args.x1, args.y1, args.x2, args.y2, color);
	} else {
		me->_surface->fillRect(rect, color);
	}

	// The border is drawn in #bgColor, which defaults to white.
	if (args.lineSize > 0) {
		uint32 border = args.bgColor.type == VOID
				? imageWhite(me->_surface->format)
				: imageColor(args.bgColor, me->_surface->format);

		for (int i = 0; i < args.lineSize; i++) {
			Common::Rect r = rect;
			r.grow(-i);
			if (r.isEmpty())
				break;

			if (args.shape.equalsIgnoreCase("oval"))
				me->_surface->drawEllipse(r.left, r.top, r.right - 1, r.bottom - 1, border, false);
			else
				me->_surface->frameRect(r, border);
		}
	}

	me->flush();
}

void LM::m_imageDraw(int nargs) {
	// draw(rect, colorOrParams), draw(point, point, colorOrParams) or the four
	// edges as separate arguments. **The default shape is a line**, not a
	// rectangle -- without a parameter list draw() connects the two corners.
	ImageShapeArgs args;
	imageShapeArgs(nargs, args);

	ImageObject *me = imageMe("image.draw()");
	if (!me || !me->_surface)
		return;

	uint32 color = imageColor(args.color, me->_surface->format);
	int lineSize = MAX(1, args.lineSize);

	if (args.shape.empty() || args.shape.equalsIgnoreCase("line")) {
		me->_surface->drawLine(args.x1, args.y1, args.x2, args.y2, color);
		me->flush();
		return;
	}

	if (args.shape.equalsIgnoreCase("roundRect"))
		debugC(5, kDebugLingoExec, "LM::m_imageDraw(): drawing a roundRect as a rectangle");

	for (int i = 0; i < lineSize; i++) {
		Common::Rect r = args.rect;
		r.grow(-i);
		if (r.isEmpty())
			break;

		if (args.shape.equalsIgnoreCase("oval"))
			me->_surface->drawEllipse(r.left, r.top, r.right - 1, r.bottom - 1, color, false);
		else
			me->_surface->frameRect(r, color);
	}

	me->flush();
}

void LM::m_imageGetPixel(int nargs) {
	// getPixel(x, y) or getPixel(point); a trailing #integer asks for the palette
	// index instead of a colour object.
	Common::Array<Datum> args;
	imageArgs(nargs, args);

	bool wantInteger = false;
	if (!args.empty() && args.back().type == SYMBOL) {
		wantInteger = args.back().asString().equalsIgnoreCase("integer");
		args.pop_back();
	}

	int x = 0, y = 0;
	if (args.size() >= 2) {
		x = args[0].asInt();
		y = args[1].asInt();
	} else if (args.size() == 1) {
		imagePoint(args[0], x, y);
	}

	ImageObject *me = imageMe("image.getPixel()");
	if (!me || !me->_surface || x < 0 || y < 0 || x >= me->_surface->w || y >= me->_surface->h) {
		// Outside the image Director answers -1.
		g_lingo->push(Datum(-1));
		return;
	}

	uint32 pixel = me->_surface->getPixel(x, y);

	if (me->_surface->format.bytesPerPixel == 1) {
		if (wantInteger)
			g_lingo->push(Datum((int)pixel));
		else
			g_lingo->push(Datum(new ColorObject((int)pixel)));
		return;
	}

	byte a, r, g, b;
	me->_surface->format.colorToARGB(pixel, a, r, g, b);
	if (wantInteger)
		g_lingo->push(Datum((int)g_director->_wm->findBestColor(r, g, b)));
	else
		g_lingo->push(Datum(new ColorObject(r, g, b)));
}

void LM::m_imageSetPixel(int nargs) {
	// setPixel(x, y, color) or setPixel(point, color)
	Common::Array<Datum> args;
	imageArgs(nargs, args);
	if (args.empty())
		return;

	Datum colorD = args.back();
	args.pop_back();

	int x = 0, y = 0;
	if (args.size() >= 2) {
		x = args[0].asInt();
		y = args[1].asInt();
	} else if (args.size() == 1) {
		imagePoint(args[0], x, y);
	}

	ImageObject *me = imageMe("image.setPixel()");
	if (!me || !me->_surface || x < 0 || y < 0 || x >= me->_surface->w || y >= me->_surface->h)
		return;

	me->_surface->setPixel(x, y, imageColor(colorD, me->_surface->format));
	me->flush();
}

void LM::m_imageExtractAlpha(int nargs) {
	g_lingo->dropStack(nargs);

	ImageObject *me = imageMe("image.extractAlpha()");
	if (!me || !me->_surface) {
		g_lingo->pushVoid();
		return;
	}

	// The alpha channel as a grey image. An image without one is opaque
	// throughout, so it comes back white.
	Graphics::ManagedSurface *alpha = new Graphics::ManagedSurface();
	alpha->create(me->_surface->w, me->_surface->h, me->_surface->format);

	bool hasAlpha = me->_surface->format.bytesPerPixel == 4 && me->_surface->format.aBits() > 0;
	for (int y = 0; y < me->_surface->h; y++) {
		for (int x = 0; x < me->_surface->w; x++) {
			byte a = 255, r, g, b;
			if (hasAlpha)
				me->_surface->format.colorToARGB(me->_surface->getPixel(x, y), a, r, g, b);

			if (alpha->format.bytesPerPixel == 1)
				alpha->setPixel(x, y, a > 127 ? 255 : 0);
			else
				alpha->setPixel(x, y, alpha->format.ARGBToColor(255, a, a, a));
		}
	}

	g_lingo->push(Datum(new ImageObject(alpha, true)));
}

void LM::m_imageTrimWhiteSpace(int nargs) {
	g_lingo->dropStack(nargs);

	ImageObject *me = imageMe("image.trimWhiteSpace()");
	if (!me || !me->_surface) {
		g_lingo->pushVoid();
		return;
	}

	// The picture without its white (or fully transparent) border.
	bool truecolor = me->_surface->format.bytesPerPixel == 4;
	uint32 white = truecolor ? me->_surface->format.ARGBToColor(255, 255, 255, 255) : 0;
	Common::Rect content;
	bool started = false;

	for (int y = 0; y < me->_surface->h; y++) {
		for (int x = 0; x < me->_surface->w; x++) {
			uint32 pixel = me->_surface->getPixel(x, y);
			bool blank = pixel == white;
			if (truecolor && !blank) {
				byte a, r, g, b;
				me->_surface->format.colorToARGB(pixel, a, r, g, b);
				blank = a == 0;
			}
			if (blank)
				continue;

			if (!started) {
				content = Common::Rect(x, y, x + 1, y + 1);
				started = true;
			} else {
				content.extend(Common::Rect(x, y, x + 1, y + 1));
			}
		}
	}

	Graphics::ManagedSurface *copy = new Graphics::ManagedSurface();
	if (started) {
		copy->create(content.width(), content.height(), me->_surface->format);
		copy->blitFrom(*me->_surface, content, Common::Point(0, 0));
	}

	g_lingo->push(Datum(new ImageObject(copy, true)));
}

// Both mask makers answer the same question -- which pixels of this image count
// -- and both hand back a picture that is white where they do. That is the
// reading copyPixels()'s #maskImage uses, and it is what extractAlpha() produces,
// which is how the games pass a mask in practice. Director calls these mask and
// matte objects and keeps them apart from images; here they are images, and the
// difference is not observable from Lingo.
static void imageCoverage(ImageObject *me, bool fromAlpha, int threshold) {
	Graphics::ManagedSurface *mask = new Graphics::ManagedSurface();
	mask->create(me->_surface->w, me->_surface->h, me->_surface->format);

	bool truecolor = me->_surface->format.bytesPerPixel == 4;
	uint32 white = imageWhite(me->_surface->format);
	uint32 black = truecolor ? me->_surface->format.ARGBToColor(255, 0, 0, 0) : 255;

	for (int y = 0; y < me->_surface->h; y++) {
		for (int x = 0; x < me->_surface->w; x++) {
			uint32 pixel = me->_surface->getPixel(x, y);
			bool covered;

			if (fromAlpha && truecolor) {
				byte a, r, g, b;
				me->_surface->format.colorToARGB(pixel, a, r, g, b);
				covered = a > threshold;
			} else {
				// A mask keys on white, as the mask ink does.
				covered = pixel != white;
			}

			mask->setPixel(x, y, covered ? white : black);
		}
	}

	g_lingo->push(Datum(new ImageObject(mask, true)));
}

void LM::m_imageCreateMatte(int nargs) {
	// createMatte({alphaThreshold}) -- 32-bit images with an alpha channel only.
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	int threshold = nargs >= 1 ? CLIP<int>(g_lingo->pop().asInt(), 0, 255) : 0;

	ImageObject *me = imageMe("image.createMatte()");
	if (!me || !me->_surface) {
		g_lingo->pushVoid();
		return;
	}

	imageCoverage(me, true, nargs >= 1 ? threshold : me->_alphaThreshold);
}

void LM::m_imageCreateMask(int nargs) {
	g_lingo->dropStack(nargs);

	ImageObject *me = imageMe("image.createMask()");
	if (!me || !me->_surface) {
		g_lingo->pushVoid();
		return;
	}

	imageCoverage(me, false, 0);
}

void LM::m_imageSetAlpha(int nargs) {
	// setAlpha(constant) or setAlpha(8bitImage): the alpha channel of a 32-bit
	// image, set from a number or taken from another image's brightness.
	// Answers TRUE when it could be done.
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Datum arg = nargs >= 1 ? g_lingo->pop() : Datum();

	ImageObject *me = imageMe("image.setAlpha()");
	if (!me || !me->_surface || me->_surface->format.bytesPerPixel != 4
			|| me->_surface->format.aBits() == 0) {
		g_lingo->push(Datum(0));
		return;
	}

	ImageObject *source = imageArg(arg);
	if (source && source->_surface
			&& (source->_surface->w != me->_surface->w || source->_surface->h != me->_surface->h)) {
		warning("LM::m_imageSetAlpha(): the alpha image is %dx%d, the image %dx%d",
				source->_surface->w, source->_surface->h, me->_surface->w, me->_surface->h);
		g_lingo->push(Datum(0));
		return;
	}

	int constant = source ? 0 : CLIP<int>(arg.asInt(), 0, 255);
	const Graphics::PixelFormat &format = me->_surface->format;

	for (int y = 0; y < me->_surface->h; y++) {
		for (int x = 0; x < me->_surface->w; x++) {
			byte a, r, g, b;
			format.colorToARGB(me->_surface->getPixel(x, y), a, r, g, b);
			a = source ? (byte)imageLuminance(source->_surface, x, y) : (byte)constant;
			me->_surface->setPixel(x, y, format.ARGBToColor(a, r, g, b));
		}
	}

	me->flush();
	g_lingo->push(Datum(1));
}

// Window

Common::String Window::asString() {
	return "window \"" + getName() + "\"";
}

bool Window::hasProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	return g_lingo->_theEntityFields.contains(fieldName) && hasField(g_lingo->_theEntityFields[fieldName]->field);
}

Datum Window::getProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return getField(g_lingo->_theEntityFields[fieldName]->field);
	}

	warning("Window::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Window::setProp(const Common::String &propName, const Datum &value, bool force) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		setField(g_lingo->_theEntityFields[fieldName]->field, value);
		return;
	}

	warning("Window::setProp: unknown property '%s'", propName.c_str());
}

bool Window::hasField(int field) {
	switch (field) {
	case kTheDrawRect:
	case kTheFileName:
	case kTheImage:
	case kTheModal:
	case kThePicture:
	case kTheRect:
	case kTheSourceRect:
	case kTheTitle:
	case kTheTitleVisible:
	case kTheVisible:
	case kTheWindowType:
		return true;
	default:
		break;
	}
	return false;
}

Datum Window::getField(int field) {
	switch (field) {
	case kTheTitle:
		return _window->getTitle();
	case kTheTitleVisible:
		return _window->isTitleVisible();
	case kTheVisible:
		return _window->isVisible();
	case kTheWindowType:
		return getWindowType();
	case kTheRect:
		return getStageRect();
	case kTheModal:
		return getModal();
	case kTheFileName:
		return getFileName();
	case kTheDrawRect:
		warning("Window::getField: poorly handled getting field 'drawRect'");
		ensureMovieIsLoaded();

		// TODO: This should allow stretching or panning
		return getStageRect();
	case kThePicture:
		ensureMovieIsLoaded();
		return getPicture();
	case kTheImage: {
			// A snapshot of the window, not a handle on it: Director's images of
			// the stage and of a MIAW are not references either, and painting
			// into one leaves the stage alone. TKKG 13 and 14 read it the way
			// the documentation describes, with
			// member("stageBitmap").image = (the stage).image.
			ensureMovieIsLoaded();
			Graphics::ManagedSurface *shot = new Graphics::ManagedSurface();
			if (getSurface() && getSurface()->w > 0 && getSurface()->h > 0)
				shot->copyFrom(*getSurface());
			return Datum(new ImageObject(shot, true));
		}
	case kTheSourceRect:
		ensureMovieIsLoaded();  // Remove fallthrough once implemented
		// fallthrough
	default:
		warning("Window::getField: unhandled field '%s'", g_lingo->field2str(field));
		return Datum();
	}
}

void Window::setField(int field, const Datum &value) {
	switch (field) {
	case kTheTitle:
		setTitle(value.asString());
		break;
	case kTheTitleVisible:
		setTitleVisible((bool)value.asInt());
		break;
	case kTheVisible:
		setVisible((bool)value.asInt());
		break;
	case kTheWindowType:
		setWindowType(value.asInt());
		break;
	case kTheDrawRect:
		warning("Window::setField: poorly handled setting field 'drawRect'");
		// fallthrough
	case kTheRect:
		setStageRect(value);
		break;
	case kTheModal:
		setModal((bool)value.asInt());
		break;
	case kTheFileName:
		setFileName(value.asString());
		break;
	case kTheImage:
		// Painting a whole image onto the stage at once.
		if (value.type == OBJECT && value.u.obj && value.u.obj->getObjType() == kImageObj) {
			ImageObject *image = static_cast<ImageObject *>(value.u.obj);
			if (image->_surface && getSurface()) {
				getSurface()->blitFrom(*image->_surface, Common::Point(0, 0));
				markSurfaceDirty();
			}
		} else {
			warning("Window::setField: the image needs an image object, got %s", value.type2str());
		}
		break;
	default:
		warning("Window::setField: unhandled field '%s'", g_lingo->field2str(field));
		break;
	}
}

void LM::m_close(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->setVisible(false);
}

void LM::m_forget(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	FArray *windowList = g_lingo->_windowList.u.farr;

	int windowIndex = -1;
	for (int i = 0; i < (int)windowList->arr.size(); i++) {
		if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>(windowList->arr[i].u.obj);
		if (window == me) {
			windowIndex = i;
			break;
		}
	}

	if (windowIndex == -1) {
		warning("m_forget: me object %s not found in window list", g_lingo->_state->me.asString().c_str());
		return;
	}

	if (windowIndex < (int)windowList->arr.size())
		windowList->arr.remove_at(windowIndex);

	// remove me from global vars
	for (auto &it : g_lingo->_globalvars) {
		if (it._value.type != OBJECT || it._value.u.obj->getObjType() != kWindowObj)
			continue;

		if (it._value.u.obj == me)
			g_lingo->_globalvars[it._key] = 0;
	}
	g_director->forgetWindow(me);
}

void LM::m_open(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	bool wasVisible = me->_window->isVisible();
	me->setVisible(true);

	if (!wasVisible)
		me->sendWindowEvent(kEventOpenWindow);
}

void LM::m_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LM::m_moveToFront(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->ensureMovieIsLoaded();

	bool wasActive = (g_director->_wm->getActiveWindow() == me->getId());
	g_director->_wm->setActiveWindow(me->getId());

	if (!wasActive)
		me->sendWindowEvent(kEventOpenWindow);
}

// Actor
/*
  collectChangeRects
  getAProp
  hitTest
  ilk
  mouseDown
  mouseHitTest
  mouseTrack
  mouseUp
  setAProp
  stepFrame
  updateRect
*/


/* TimeoutObject */

TimeoutObject::TimeoutObject(const Common::String &name) : Object<TimeoutObject>(name) {
	_objType = kTimeoutObj;
}

Common::String TimeoutObject::asString() {
	return Common::String::format("timeout(\"%s\")", _name.c_str());
}

bool TimeoutObject::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("name")
		|| propName.equalsIgnoreCase("period")
		|| propName.equalsIgnoreCase("time")
		|| propName.equalsIgnoreCase("timeoutHandler")
		|| propName.equalsIgnoreCase("target");
}

Datum TimeoutObject::getProp(const Common::String &propName) {
	if (propName.equalsIgnoreCase("name"))
		return Datum(_name);
	if (propName.equalsIgnoreCase("period"))
		return Datum(_period);
	// #time is the absolute millisecond at which the next message goes out, not
	// the remaining wait -- "Using Director MX", ch. 16.
	if (propName.equalsIgnoreCase("time"))
		return Datum((int)_time);
	if (propName.equalsIgnoreCase("timeoutHandler")) {
		Datum d(_timeoutHandler);
		d.type = SYMBOL;
		return d;
	}
	if (propName.equalsIgnoreCase("target"))
		return _target;

	warning("TimeoutObject::getProp(): unknown property '%s'", propName.c_str());
	return Datum();
}

Common::String TimeoutObject::getPropAt(uint32 index) {
	static const char *props[] = { "name", "period", "time", "timeoutHandler", "target" };
	return (index >= 1 && index <= 5) ? props[index - 1] : Common::String();
}

uint32 TimeoutObject::getPropCount() {
	return 5;
}

void TimeoutObject::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (propName.equalsIgnoreCase("period")) {
		// Changing the period reschedules from now, which is what a game
		// expects when it speeds a timer up or slows it down mid-run.
		_period = value.asInt();
		_time = g_system->getMillis() + _period;
		return;
	}
	if (propName.equalsIgnoreCase("time")) {
		_time = (uint32)value.asInt();
		return;
	}
	if (propName.equalsIgnoreCase("timeoutHandler")) {
		_timeoutHandler = value.asString();
		return;
	}
	if (propName.equalsIgnoreCase("target")) {
		_target = value;
		return;
	}
	if (propName.equalsIgnoreCase("name")) {
		_name = value.asString();
		return;
	}

	warning("TimeoutObject::setProp(): unknown property '%s'", propName.c_str());
}

void TimeoutObject::tick(uint32 now) {
	if (!_armed || _period <= 0 || now < _time)
		return;

	// Reschedule before calling: the handler may forget() this very object, and
	// it may also take longer than one period, in which case Director does not
	// queue up the missed calls.
	_time = now + _period;

	if (_timeoutHandler.empty())
		return;

	debugC(5, kDebugLingoExec, "TimeoutObject::tick(): firing %s on \"%s\"", _timeoutHandler.c_str(), _name.c_str());

	if (_target.type == OBJECT && _target.u.obj) {
		Symbol sym = _target.u.obj->getMethod(_timeoutHandler);
		if (sym.type == VOIDSYM) {
			warning("TimeoutObject::tick(): target of \"%s\" has no handler %s", _name.c_str(), _timeoutHandler.c_str());
			return;
		}
		g_lingo->push(_target);
		int frame = g_lingo->_state->callstack.size();
		LC::call(sym, 1, false);
		g_lingo->execute(frame);
		return;
	}

	// No target: the manual says Director then looks for the handler in the
	// movie scripts.
	Symbol sym = g_lingo->getHandler(_timeoutHandler);
	if (sym.type == VOIDSYM) {
		warning("TimeoutObject::tick(): no movie script handler %s for \"%s\"", _timeoutHandler.c_str(), _name.c_str());
		return;
	}
	int frame = g_lingo->_state->callstack.size();
	LC::call(sym, 0, false);
	g_lingo->execute(frame);
}

void LM::m_timeoutNew(int nargs) {
	TimeoutObject *me = static_cast<TimeoutObject *>(g_lingo->_state->me.u.obj);

	// timeOut("name").new(period, #handler) or .new(period, #handler, target)
	Datum target, handler, period;

	if (nargs > 3) {
		warning("LM::m_timeoutNew(): expected 2 or 3 arguments for \"%s\", got %d, dropping the surplus",
				me->getName().c_str(), nargs);
		while (nargs > 3) {
			g_lingo->pop();
			nargs--;
		}
	}
	// Arguments were pushed left to right, so they come off in reverse.
	if (nargs >= 3)
		target = g_lingo->pop();
	if (nargs >= 2)
		handler = g_lingo->pop();
	if (nargs >= 1)
		period = g_lingo->pop();

	me->_period = period.asInt();
	me->_timeoutHandler = (handler.type == VOID) ? Common::String() : handler.asString();
	me->_target = target;
	me->_time = g_system->getMillis() + me->_period;
	me->_armed = true;

	Movie *movie = g_director->getCurrentMovie();
	if (movie) {
		// A movie may call new() twice on the same name -- that restarts the one
		// timer, it does not make a second one.
		bool listed = false;
		for (auto &it : movie->_timeOutList) {
			if (it.type == OBJECT && it.u.obj == me) {
				listed = true;
				break;
			}
		}
		if (!listed)
			movie->_timeOutList.push_back(Datum(me));
	}

	debugC(3, kDebugLingoExec, "LM::m_timeoutNew(): armed \"%s\" every %d ms -> %s",
			me->getName().c_str(), me->_period, me->_timeoutHandler.c_str());

	g_lingo->push(g_lingo->_state->me);
}

void LM::m_timeoutForget(int nargs) {
	TimeoutObject *me = static_cast<TimeoutObject *>(g_lingo->_state->me.u.obj);

	// Disarm first: tickTimeouts() works on a snapshot, so a timer that forgets
	// itself from inside its own handler must stop by the flag as well as by
	// leaving the list.
	me->_armed = false;

	Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return;

	for (uint i = 0; i < movie->_timeOutList.size(); i++) {
		if (movie->_timeOutList[i].type == OBJECT && movie->_timeOutList[i].u.obj == me) {
			movie->_timeOutList.remove_at(i);
			return;
		}
	}
}

} // End of namespace Director
