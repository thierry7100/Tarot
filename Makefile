#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = -Wall -fexceptions `pkg-config gtk+-3.0 --cflags`
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = `pkg-config gtk+-3.0 --libs`

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g -D DEBUG=1 -D WIN=1
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/TarotGTK

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2 -D WIN=1
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/TarotGTK

OBJ_DEBUG = $(OBJDIR_DEBUG)/Tarot_Main_ui.o $(OBJDIR_DEBUG)/ScreenSize.o $(OBJDIR_DEBUG)/SplashScreen.o $(OBJDIR_DEBUG)/Table.o $(OBJDIR_DEBUG)/TarotFile.o $(OBJDIR_DEBUG)/TarotMessage.o $(OBJDIR_DEBUG)/ReglesTmp.o $(OBJDIR_DEBUG)/UserPreferences.o $(OBJDIR_DEBUG)/UtilJeu.o $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/resources.o $(OBJDIR_DEBUG)/JeuAttaque.o $(OBJDIR_DEBUG)/AffContrat.o $(OBJDIR_DEBUG)/AffResultat.o $(OBJDIR_DEBUG)/AffScores.o $(OBJDIR_DEBUG)/Cairo_util.o $(OBJDIR_DEBUG)/ChoixContrat.o $(OBJDIR_DEBUG)/Distribue.o $(OBJDIR_DEBUG)/DrawGameZone.o $(OBJDIR_DEBUG)/Ecart.o $(OBJDIR_DEBUG)/Heuristiques.o $(OBJDIR_DEBUG)/AffCartes.o $(OBJDIR_DEBUG)/JeuCommun.o $(OBJDIR_DEBUG)/JeuDefense.o $(OBJDIR_DEBUG)/MouseGameZone.o $(OBJDIR_DEBUG)/NomJoueurs.o $(OBJDIR_DEBUG)/PartieMaitre.o $(OBJDIR_DEBUG)/Poignee.o $(OBJDIR_DEBUG)/Proba.o $(OBJDIR_DEBUG)/Regle.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/Tarot_Main_ui.o $(OBJDIR_RELEASE)/ScreenSize.o $(OBJDIR_RELEASE)/SplashScreen.o $(OBJDIR_RELEASE)/Table.o $(OBJDIR_RELEASE)/TarotFile.o $(OBJDIR_RELEASE)/TarotMessage.o $(OBJDIR_RELEASE)/ReglesTmp.o $(OBJDIR_RELEASE)/UserPreferences.o $(OBJDIR_RELEASE)/UtilJeu.o $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/resources.o $(OBJDIR_RELEASE)/JeuAttaque.o $(OBJDIR_RELEASE)/AffContrat.o $(OBJDIR_RELEASE)/AffResultat.o $(OBJDIR_RELEASE)/AffScores.o $(OBJDIR_RELEASE)/Cairo_util.o $(OBJDIR_RELEASE)/ChoixContrat.o $(OBJDIR_RELEASE)/Distribue.o $(OBJDIR_RELEASE)/DrawGameZone.o $(OBJDIR_RELEASE)/Ecart.o $(OBJDIR_RELEASE)/Heuristiques.o $(OBJDIR_RELEASE)/AffCartes.o $(OBJDIR_RELEASE)/JeuCommun.o $(OBJDIR_RELEASE)/JeuDefense.o $(OBJDIR_RELEASE)/MouseGameZone.o $(OBJDIR_RELEASE)/NomJoueurs.o $(OBJDIR_RELEASE)/PartieMaitre.o $(OBJDIR_RELEASE)/Poignee.o $(OBJDIR_RELEASE)/Proba.o $(OBJDIR_RELEASE)/Regle.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/Tarot_Main_ui.o: Tarot_Main_ui.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Tarot_Main_ui.c -o $(OBJDIR_DEBUG)/Tarot_Main_ui.o

$(OBJDIR_DEBUG)/ScreenSize.o: ScreenSize.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ScreenSize.c -o $(OBJDIR_DEBUG)/ScreenSize.o

$(OBJDIR_DEBUG)/SplashScreen.o: SplashScreen.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c SplashScreen.c -o $(OBJDIR_DEBUG)/SplashScreen.o

$(OBJDIR_DEBUG)/Table.o: Table.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Table.c -o $(OBJDIR_DEBUG)/Table.o

$(OBJDIR_DEBUG)/TarotFile.o: TarotFile.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c TarotFile.c -o $(OBJDIR_DEBUG)/TarotFile.o

$(OBJDIR_DEBUG)/TarotMessage.o: TarotMessage.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c TarotMessage.c -o $(OBJDIR_DEBUG)/TarotMessage.o

$(OBJDIR_DEBUG)/ReglesTmp.o: ReglesTmp.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ReglesTmp.c -o $(OBJDIR_DEBUG)/ReglesTmp.o

$(OBJDIR_DEBUG)/UserPreferences.o: UserPreferences.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c UserPreferences.c -o $(OBJDIR_DEBUG)/UserPreferences.o

$(OBJDIR_DEBUG)/UtilJeu.o: UtilJeu.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c UtilJeu.c -o $(OBJDIR_DEBUG)/UtilJeu.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

$(OBJDIR_DEBUG)/resources.o: resources.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c resources.c -o $(OBJDIR_DEBUG)/resources.o

$(OBJDIR_DEBUG)/JeuAttaque.o: JeuAttaque.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c JeuAttaque.c -o $(OBJDIR_DEBUG)/JeuAttaque.o

$(OBJDIR_DEBUG)/AffContrat.o: AffContrat.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c AffContrat.c -o $(OBJDIR_DEBUG)/AffContrat.o

$(OBJDIR_DEBUG)/AffResultat.o: AffResultat.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c AffResultat.c -o $(OBJDIR_DEBUG)/AffResultat.o

$(OBJDIR_DEBUG)/AffScores.o: AffScores.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c AffScores.c -o $(OBJDIR_DEBUG)/AffScores.o

$(OBJDIR_DEBUG)/Cairo_util.o: Cairo_util.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Cairo_util.c -o $(OBJDIR_DEBUG)/Cairo_util.o

$(OBJDIR_DEBUG)/ChoixContrat.o: ChoixContrat.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ChoixContrat.c -o $(OBJDIR_DEBUG)/ChoixContrat.o

$(OBJDIR_DEBUG)/Distribue.o: Distribue.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Distribue.c -o $(OBJDIR_DEBUG)/Distribue.o

$(OBJDIR_DEBUG)/DrawGameZone.o: DrawGameZone.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c DrawGameZone.c -o $(OBJDIR_DEBUG)/DrawGameZone.o

$(OBJDIR_DEBUG)/Ecart.o: Ecart.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Ecart.c -o $(OBJDIR_DEBUG)/Ecart.o

$(OBJDIR_DEBUG)/Heuristiques.o: Heuristiques.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Heuristiques.c -o $(OBJDIR_DEBUG)/Heuristiques.o

$(OBJDIR_DEBUG)/AffCartes.o: AffCartes.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c AffCartes.c -o $(OBJDIR_DEBUG)/AffCartes.o

$(OBJDIR_DEBUG)/JeuCommun.o: JeuCommun.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c JeuCommun.c -o $(OBJDIR_DEBUG)/JeuCommun.o

$(OBJDIR_DEBUG)/JeuDefense.o: JeuDefense.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c JeuDefense.c -o $(OBJDIR_DEBUG)/JeuDefense.o

$(OBJDIR_DEBUG)/MouseGameZone.o: MouseGameZone.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c MouseGameZone.c -o $(OBJDIR_DEBUG)/MouseGameZone.o

$(OBJDIR_DEBUG)/NomJoueurs.o: NomJoueurs.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c NomJoueurs.c -o $(OBJDIR_DEBUG)/NomJoueurs.o

$(OBJDIR_DEBUG)/PartieMaitre.o: PartieMaitre.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c PartieMaitre.c -o $(OBJDIR_DEBUG)/PartieMaitre.o

$(OBJDIR_DEBUG)/Poignee.o: Poignee.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Poignee.c -o $(OBJDIR_DEBUG)/Poignee.o

$(OBJDIR_DEBUG)/Proba.o: Proba.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Proba.c -o $(OBJDIR_DEBUG)/Proba.o

$(OBJDIR_DEBUG)/Regle.o: Regle.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Regle.c -o $(OBJDIR_DEBUG)/Regle.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/Tarot_Main_ui.o: Tarot_Main_ui.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Tarot_Main_ui.c -o $(OBJDIR_RELEASE)/Tarot_Main_ui.o

$(OBJDIR_RELEASE)/ScreenSize.o: ScreenSize.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ScreenSize.c -o $(OBJDIR_RELEASE)/ScreenSize.o

$(OBJDIR_RELEASE)/SplashScreen.o: SplashScreen.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c SplashScreen.c -o $(OBJDIR_RELEASE)/SplashScreen.o

$(OBJDIR_RELEASE)/Table.o: Table.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Table.c -o $(OBJDIR_RELEASE)/Table.o

$(OBJDIR_RELEASE)/TarotFile.o: TarotFile.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c TarotFile.c -o $(OBJDIR_RELEASE)/TarotFile.o

$(OBJDIR_RELEASE)/TarotMessage.o: TarotMessage.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c TarotMessage.c -o $(OBJDIR_RELEASE)/TarotMessage.o

$(OBJDIR_RELEASE)/ReglesTmp.o: ReglesTmp.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ReglesTmp.c -o $(OBJDIR_RELEASE)/ReglesTmp.o

$(OBJDIR_RELEASE)/UserPreferences.o: UserPreferences.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c UserPreferences.c -o $(OBJDIR_RELEASE)/UserPreferences.o

$(OBJDIR_RELEASE)/UtilJeu.o: UtilJeu.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c UtilJeu.c -o $(OBJDIR_RELEASE)/UtilJeu.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/resources.o: resources.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c resources.c -o $(OBJDIR_RELEASE)/resources.o

$(OBJDIR_RELEASE)/JeuAttaque.o: JeuAttaque.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c JeuAttaque.c -o $(OBJDIR_RELEASE)/JeuAttaque.o

$(OBJDIR_RELEASE)/AffContrat.o: AffContrat.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c AffContrat.c -o $(OBJDIR_RELEASE)/AffContrat.o

$(OBJDIR_RELEASE)/AffResultat.o: AffResultat.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c AffResultat.c -o $(OBJDIR_RELEASE)/AffResultat.o

$(OBJDIR_RELEASE)/AffScores.o: AffScores.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c AffScores.c -o $(OBJDIR_RELEASE)/AffScores.o

$(OBJDIR_RELEASE)/Cairo_util.o: Cairo_util.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Cairo_util.c -o $(OBJDIR_RELEASE)/Cairo_util.o

$(OBJDIR_RELEASE)/ChoixContrat.o: ChoixContrat.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ChoixContrat.c -o $(OBJDIR_RELEASE)/ChoixContrat.o

$(OBJDIR_RELEASE)/Distribue.o: Distribue.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Distribue.c -o $(OBJDIR_RELEASE)/Distribue.o

$(OBJDIR_RELEASE)/DrawGameZone.o: DrawGameZone.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c DrawGameZone.c -o $(OBJDIR_RELEASE)/DrawGameZone.o

$(OBJDIR_RELEASE)/Ecart.o: Ecart.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Ecart.c -o $(OBJDIR_RELEASE)/Ecart.o

$(OBJDIR_RELEASE)/Heuristiques.o: Heuristiques.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Heuristiques.c -o $(OBJDIR_RELEASE)/Heuristiques.o

$(OBJDIR_RELEASE)/AffCartes.o: AffCartes.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c AffCartes.c -o $(OBJDIR_RELEASE)/AffCartes.o

$(OBJDIR_RELEASE)/JeuCommun.o: JeuCommun.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c JeuCommun.c -o $(OBJDIR_RELEASE)/JeuCommun.o

$(OBJDIR_RELEASE)/JeuDefense.o: JeuDefense.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c JeuDefense.c -o $(OBJDIR_RELEASE)/JeuDefense.o

$(OBJDIR_RELEASE)/MouseGameZone.o: MouseGameZone.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c MouseGameZone.c -o $(OBJDIR_RELEASE)/MouseGameZone.o

$(OBJDIR_RELEASE)/NomJoueurs.o: NomJoueurs.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c NomJoueurs.c -o $(OBJDIR_RELEASE)/NomJoueurs.o

$(OBJDIR_RELEASE)/PartieMaitre.o: PartieMaitre.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c PartieMaitre.c -o $(OBJDIR_RELEASE)/PartieMaitre.o

$(OBJDIR_RELEASE)/Poignee.o: Poignee.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Poignee.c -o $(OBJDIR_RELEASE)/Poignee.o

$(OBJDIR_RELEASE)/Proba.o: Proba.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Proba.c -o $(OBJDIR_RELEASE)/Proba.o

$(OBJDIR_RELEASE)/Regle.o: Regle.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Regle.c -o $(OBJDIR_RELEASE)/Regle.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release
