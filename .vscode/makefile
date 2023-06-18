#source file search paths
VPATH = ./src ../raven-set ../sqlite

	# compiler include search paths
INCS = -I../sqlite \
	-I../windex/include \
	-I../raven-set \
	-I../../code_ext/boost/boost1_78 

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
	$(ODIR)/sqlite3.o \
	$(ODIR)/cRunWatch.o

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

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

