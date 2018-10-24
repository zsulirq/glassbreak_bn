CPPFLAGS += -O6

EAR_OBJS=Data/Config.o Data/Utils.o Data/DataReader.o Data/WavSource.o Data/PushSource.o Data/MicSource.o \
Features/Coeffs.o Features/Filter.o Features/Frame.o Features/Transform.o Features/Feature.o \
Search/AcousticScorer.o Search/Token.o Search/Search.o 

COMPILE_OBJS=Data/FileIO.o Network/HTKAcousticModel.o Network/Dictionary.o Network/FSTAssembly.o

LD_LIBRARY=-lportaudio

EAR=Ear
COMPILER=Compile

all: $(EAR_OBJS) $(COMPILE_OBJS) Ear.o Compile.o
	g++ -O6 $(EAR_OBJS) Ear.o $(LD_LIBRARY) -o $(EAR)
	g++ -O6 $(EAR_OBJS) $(COMPILE_OBJS) Compile.o $(LD_LIBRARY) -o $(COMPILER)

Data/FileIO.o : Data/FileIO.cpp
	g++ -o $@ -c $<

clean:
	-rm Data/*.o
	-rm Features/*.o
	-rm Search/*.o
	-rm Network/*.o
	-rm *.o
	-rm $(EAR)
	-rm $(COMPILER)