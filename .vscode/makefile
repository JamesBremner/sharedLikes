#source file search paths
VPATH = ./src ../raven-set ../sqlite ../sqliteClass/src

	# compiler include search paths
INCS = -I../sqlite \
	-I../windex/include \
	-I../raven-set \
	-I../../code_ext/boost/boost1_78 \
	-I../sqliteClass/src

LIBS = \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs \
	-L../../code_ext/boost/boost1_78/lib \
		-lboost_system-mgw11-mt-x64-1_78 \
		-lboost_program_options-mgw11-mt-x64-1_78 \
		# -lboost_thread-mgw11-mt-x64-1_7 \
	#-mwindows

static_link_flags = -static-libstdc++ -static-libgcc -static -lComctl32

# folders
ODIR = ./.vscode/obj

OPT = -g
CCflags = -std=c++17 $(OPT) -D_mingw_

# sources
OBJ_starter = \
	$(ODIR)/main.o \
	$(ODIR)/cSharedLikesDB.o \
	$(ODIR)/cSharedLikesDBClass.o \
	$(ODIR)/sqliteClass.o \
	$(ODIR)/sqlite3.o \
	$(ODIR)/cRunWatch.o

OBJ_test = \
	$(ODIR)/test.o \
	$(ODIR)/sqlite3.o \

### COMPILE

$(ODIR)/sqlite3.o: sqlite3.c
	gcc	 -c -o $@ $<

$(ODIR)/%.o: %.cpp  
	g++  $(CCflags) \
	-c -o $@ $< $(INCS)

starter: $(OBJ_starter)
	g++ $(OPT) \
	-o bin/likes.exe $^  \
	$(static_link_flags) $(LIBS)

test: $(OBJ_test)
	g++ $(OPT) \
	-o bin/test.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

