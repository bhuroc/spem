
include $(VMLROOT)/lib/vmlcommon.mk

EYELINK_LIB:=/libpath:$(VMLROOT)/lib eyelink_core.lib 

INCLUDES+=$(BVL_INCLUDE) $(VML_INCLUDE) $(OPTO_INCLUDE) $(TCL_INCLUDE)
CXXFLAGS += /MDd /GR /RTC1 /EHs /Zi /Fd"vc70.pdb" /D"COIN_DLL" 
LINKARGS += /DEBUG /PDB:"info.pdb" 

LIBS=$(BVL_LIB) $(VML_LIB) $(COIN3D_LIB) $(OPTO_LIB) $(TCL_LIB) $(EYELINK_LIB)
LIBS += user32.lib gdi32.lib opengl32.lib glu32.lib comdlg32.lib winmm.lib

SRCS=main.cpp TrackingDataSource.cc App.cc State.cc Datum.cc Trial.cc ExperimentManager.cc Eyelink.cc EyelinkCollector.cc EyePositionTracer.cc version.cc RobotActor.cc Trajectory.cc TimeSampler.cc SpaceSampler.cc RigidBodyAdapter.cc 
OBJS:=$(patsubst %.cpp, %.obj, $(filter  %.cpp, $(SRCS)))\
     $(patsubst %.cc, %.obj, $(filter  %.cc,$(SRCS)))\
     $(patsubst %.rc, %.res, $(filter  %.rc, $(SRCS)))

TARGET=spem

all:$(TARGET) 

$(TARGET): $(OBJS)
	$(LINK) $(LINKARGS) $^ $(LIBS) 
	mt -nologo -manifest $@.exe.manifest -outputresource:$@.exe\;1

.PHONY: clean depend

depend:
	makedepend -o.obj -Y -- $(SRCS)

clean:
	rm -f $(TARGET).exe $(OBJS) *.ilk *.pdb *.suo *.manifest


# DO NOT DELETE
