TOASMCPP=/usr/local/libexec/gcc/arm-apple-darwin/4.0.1/cc1plus.exe

OBJC_CC=/usr/local/bin/arm-apple-darwin-g++

ASM=arm-apple-darwin-as 

LD=arm-apple-darwin-g++

#preprocessor defines
PREPROC_DEF=-DIPHONE -DASEMANIP_READERONLY -DWIN32 -D_WINDOWS -DUSE_IPHONE_FILE_IMPLEMENTATION -DUSE_OGL -DUSE_TOUCH_SCREEN -DUSE_IPHONE_RES

#dirs to be included
INCLUDE_DIRS=-I../ \
-I../_iPhone/src/Sound \
-I../_iPhone/src \
-I../HG \
-I../Lib2D \
-I../Lib3D2 \
-I../Lib3DGL \
-I../Menu \
-I../System \
-I../Ui \
-I../ZLib \
-I../Sound


TOASMFLAGS=$(INCLUDE_DIRS) -w\
-quiet -fPIC -O5 -mmacosx-version-min=10.4 \
-mfpu=vfp -mcpu=arm1176jzf-s -D__private_extern__=extern -fsigned-char \
$(PREPROC_DEF) 

ASMFLAGS=-arch arm -force_cpusubtype_ALL 

OBJC_CC_FLAGS=-O3 $(INCLUDE_DIRS) $(PREPROC_DEF)

LDFLAGS=-lobjc \
	-framework CoreFoundation \
	-framework Foundation \
	-framework UIKit \
	-framework LayerKit \
	-framework CoreGraphics \
	-framework GraphicsServices \
	-framework CoreSurface \
	-framework CoreAudio \
	-framework OpenGLES \
	-larmfp \
	-L/usr/local/lib


#files needed to compile
OBJC_FILES=../_iPhone/src/main.mm \
../_iPhone/src/UIGLView.mm \
../_iPhone/src/GLoftApp.mm


CPP_FILES=\
../_iPhone/src/File_iPhone.cpp\
../_iPhone/src/GameEngine.cpp \
../_iPhone/src/IphoneUtil.cpp \
../_iPhone/src/Sound/SoundWrap.cpp \
../DevUtil.cpp \
../GapiMain.cpp \
../HG/ArchivedFileManager.cpp \
../HG/Car.cpp \
../HG/CarAnim.cpp \
../HG/CarAnimData.cpp \
../HG/CarBase.cpp \
../HG/CollideCar.cpp \
../HG/FPSCounter.cpp \
../HG/Functor.cpp \
../HG/GameCamera.cpp \
../HG/HighGear.cpp \
../HG/Intl.cpp \
../HG/Map.cpp \
../HG/Menu/Championship.cpp \
../HG/Menu/Loading.cpp \
../HG/minimap.cpp \
../HG/money_queue.cpp \
../HG/neon.cpp \
../HG/PlayingGame.cpp \
../HG/PointSystem.cpp \
../HG/Profile.cpp \
../HG/radar.cpp \
../HG/Random.cpp \
../HG/shadow.cpp \
../HG/Singleton.cpp \
../HG/skidmarks.cpp \
../HG/sparks.cpp \
../HG/Traffic.cpp \
../HG/trail.cpp \
../HG/Tuning.cpp \
../HG/WeatherFX.cpp \
../Keypad.cpp \
../Lib2D/aaline.cpp \
../Lib2D/AuroraSprite.cpp \
../Lib2D/BitFont.cpp \
../Lib2D/Bitmap.cpp \
../Lib2D/BitmapAlpha.cpp \
../Lib2D/BitmapPal.cpp \
../Lib2D/BitmapRLE.cpp \
../Lib2D/Font.cpp \
../Lib2D/Font2.cpp \
../Lib2D/FontWrapper.cpp \
../Lib2D/Image.cpp \
../Lib2D/L3texture.cpp \
../Lib2D/Lib2D.cpp \
../Lib2D/Rect.cpp \
../Lib3D2/Board3D.cpp \
../Lib3D2/Camera.cpp \
../Lib3D2/ConvexHull.cpp \
../Lib3D2/Face.cpp \
../Lib3D2/Font.cpp \
../Lib3D2/FSqrt.cpp \
../Lib3D2/IMath.cpp \
../Lib3D2/Keyframed.cpp \
../Lib3D2/Lib3d.cpp \
../Lib3D2/Malloc.cpp \
../Lib3D2/Matrix.cpp \
../Lib3D2/Mesh.cpp \
../Lib3D2/PoliceHelico.cpp \
../Lib3D2/rasterize.cpp \
../Lib3D2/Render.cpp \
../Lib3D2/skybox.cpp \
../Lib3D2/SoftLine.cpp \
../Lib3D2/Sort.cpp \
../Lib3D2/SunFX.cpp \
../Lib3D2/texture.cpp \
../Lib3D2/Vector.cpp \
../Menu/DemoGarageMenu.cpp \
../Menu/DemoMenu.cpp \
../Menu/GarageMenu.cpp \
../Menu/GlobeRenderer.cpp \
../Menu/LoadingScreen.cpp \
../Menu/MainMenuStack.cpp \
../Menu/MenuIcons.cpp \
../Menu/QuitYesNo.cpp \
../Menu/RaceSummary.cpp \
../Menu/SplashScreen.cpp \
../Menu/YesNoMenu.cpp \
../Sound/SoundManager.cpp \
../throw.cpp \
../Ui/Control.cpp \
../Ui/ListControl.cpp \
../Ui/MenuStack.cpp \
../Ui/Menu_.cpp \
../Ui/MessageOk.cpp \
../Ui/MessageYesNo.cpp \
../Ui/SliderControl.cpp \
../Ui/Style.cpp \
../Ui/Transition.cpp \
../Util.cpp \
../ZLib/adler32.cpp \
../ZLib/infblock.cpp \
../ZLib/infcodes.cpp \
../ZLib/inffast.cpp \
../ZLib/inflate.cpp \
../ZLib/inftrees.cpp \
../ZLib/infutil.cpp \
../ZLib/uncompr.cpp \
../Lib3DGL/Lib3DGL.cpp






GAME_OBJS=$(patsubst %.mm,%.o,$(OBJC_FILES)) $(patsubst %.cpp,%.oo,$(CPP_FILES))

all:    ASPHALT4_IPHONE

ASPHALT4_IPHONE:  $(GAME_OBJS)	
	@echo
	@echo [ MAKE LINKAGE ]
	@echo	
	$(LD) $(LDFLAGS) -v -o $@ $^
	cp -u -v -f ASPHALT4_IPHONE ./ASPHALT4_IPHONE.app/
	

%.o:	%.mm
	@echo
	@echo [ MAKE s --------------- $< --------------- ]
	@echo
	$(OBJC_CC) $(OBJC_CC_FLAGS) -c -o $@ $<
	
%.oo:	%.ss
	@echo
	@echo [ MAKE oo --------------- $< --------------- ]
	@echo
	$(ASM) $(ASMFLAGS) -o $@ $<

%.ss:	%.cpp
	@echo
	@echo [ MAKE ss --------------- $< --------------- ]
	@echo
	$(TOASMCPP) $(TOASMFLAGS) -o $@	$<

clean: cleanObjFiles cleanAsmFiles
	@echo
	@echo [ CLEAN --------------- ALL --------------- ]
	@echo
	rm -fr -v ASPHALT4_IPHONE
	
cleanObjFiles:
	@echo
	@echo [ CLEAN --------------- o oo --------------- ]
	@echo
	find ../ -name '*.o' | xargs rm -fr -v
	find ../ -name '*.oo' | xargs rm -fr -v	
	
cleanAsmFiles:
	@echo
	@echo [ CLEAN --------------- s ss --------------- ]
	@echo
	find ../ -name '*.s' | xargs rm -fr -v
	find ../ -name '*.ss' | xargs rm -fr -v	
	