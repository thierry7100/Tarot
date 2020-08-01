#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include "Tarot_Ui_Objects.h"

//  Sauve une partie dans le fichier nommé Name

void SaveFile(const char *Name, TarotGame CurrentGame)
{
FILE *out;
int i, c;

    out = fopen(Name, "w");
    if ( out == NULL ) return;  //  Ne peut le faire, sortie
    fprintf(out, "[Distrib]\n");
    fprintf(out, "%d\n", CurrentGame->JoueurDistrib);
    fprintf(out, "[PartieEnregistree]\n");
    fprintf(out, "%d,%d\n", CurrentGame->NumPartieEnregistree, CurrentGame->PreneurPartieEnregistree);
    fprintf(out, "[Visible]\n");
    fprintf(out, "%d, %d, %d, %d\n", CurrentGame->FlagAfficheJoueur[0], CurrentGame->FlagAfficheJoueur[1], CurrentGame->FlagAfficheJoueur[2], CurrentGame->FlagAfficheJoueur[3]);
    //  Sauve les cartes pour chaque joueur
    fprintf(out, "[Cartes]\n");
    //  D'abord les atouts
    for ( i = 0; i < 22; i++)
    {
        fprintf(out, "%d,", CurrentGame->Carte2JoueurDistrib[i]);
    }
    fprintf(out, "\n");
    //  Puis chaque couleur
    for ( c = 0; c < 4; c++ )
    {
        for ( i = 0; i < 14; i++ )
        {
            fprintf(out, "%d,", CurrentGame->Carte2JoueurDistrib[c*14+i+22]);
        }
        fprintf(out, "\n");
    }
    fclose(out);
}

//  Read File until a start of section is found
//  Store the SectionName into the array, this array should be at least 128 characters wide
//  Return 1 if OK, -1 if an error occured

int LookForSection(FILE *In, char *SectionName)
{
char BufLine[1024];
int i;

    while ( 1 )
    {
        if ( fgets(BufLine, 1000, In) == NULL ) return -1;
        if ( BufLine[0] == '[' )
        {
            for ( i = 0; i < 127; i++ )
            {
                if ( BufLine[i+1] == ']' ) break;
                if ( BufLine[i+1] == '\n' ) break;
                if ( BufLine[i+1] == 0 ) break;
                SectionName[i] = BufLine[i+1];
            }
            SectionName[i] = 0;
            return 1;
        }
    }
    return -1;
}



//  Charge une partie enregistrée

int LoadFile(const char * Name, TarotGame CurrentGame)
{
FILE *InFile;
char SectionName[256];
int Res;
int dec, dec2;
int i, c;
char strMessage[64];

    InFile = fopen(Name, "r");
    if ( InFile == NULL ) return -1;
    memset(CurrentGame, 0, sizeof(*CurrentGame));       //  Efface la structure
    CurrentGame->FlagAfficheJoueur[0] = 1;              //  Par défaut SUD visible
#if DEBUG > 0
    OutDebug("Charge fichier %s\n", Name);
#endif // DEBUG
    while ( 1 )                         //  Quit loop on file end
    {
         //  Look for section
        Res = LookForSection(InFile, SectionName);
        if ( Res < 0 ) break;       //  Error, likely EOF
        printf("LoadFile, found section %s\n", SectionName);
        if ( strcmp(SectionName, "Distrib") == 0 )
        {
            Res = fscanf(InFile, "%d", &dec);
            if ( Res == 1 ) CurrentGame->JoueurDistrib = dec;
        }
        if ( strcmp(SectionName, "PartieEnregistree") == 0 )
        {
            Res = fscanf(InFile, "%d, %d", &dec, &dec2);
            if ( Res >= 1 ) CurrentGame->NumPartieEnregistree = dec;
            if ( Res >= 2 ) CurrentGame->PreneurPartieEnregistree = dec2;
        }
        if ( strcmp(SectionName, "Visible") == 0 )
        {
            Res = fscanf(InFile, "%d,%d,%d,%d", &CurrentGame->FlagAfficheJoueur[0], &CurrentGame->FlagAfficheJoueur[1], &CurrentGame->FlagAfficheJoueur[2], &CurrentGame->FlagAfficheJoueur[3]);
            if ( Res != 4 ) return -1;
        }
        else if ( strcmp(SectionName, "Cartes") == 0 )
        {
            // Lecture des atouts
            for ( i = 0; i < 21; i++ )
            {
                Res = fscanf(InFile, "%d,", &dec);
                if ( Res == 1) CurrentGame->Carte2JoueurDistrib[i] = dec;
                if ( Res <= 0 ) return -1;
            }
            //  Dernier atout
            Res = fscanf(InFile, "%d,\n", &dec);
            if ( Res <= 0 ) return -1;
            if ( Res == 1) CurrentGame->Carte2JoueurDistrib[i] = dec;
            //  Lecture des couleurs
            for ( c = 0; c < 4; c++ )
            {
                for ( i = 0; i < 13; i++ )
                {
                    Res = fscanf(InFile, "%d,", &dec);
                    if ( Res == 1) CurrentGame->Carte2JoueurDistrib[i+c*14+22] = dec;
                    if ( Res == 0 ) return -1;
                }
                //  Derniere carte : Roi
                Res = fscanf(InFile, "%d,\n", &dec);
                if ( Res == 1) CurrentGame->Carte2JoueurDistrib[c*14+35] = dec;
                if ( Res <= 0 ) return -1;
            }
            for ( i = 0; i < 78; i++ )
            {
                CurrentGame->Carte2Joueur[i] = CurrentGame->Carte2JoueurDistrib[i];
            }
            RepartitionCarte(CurrentGame);
            Res = EvalGames(CurrentGame);             //  Evalue la force des différents jeux
        }
    }
    fclose(InFile);
    if ( CurrentGame->NumPartieEnregistree >= 0 )
    {
        snprintf(strMessage, 60, "Partie n°%d", CurrentGame->NumPartieEnregistree+1);
        AfficheMsgStatusBarGen(strMessage);
#if DEBUG > 0
        OutDebug("Chargement OK, Partie %d, Preneur = %d\n", CurrentGame->NumPartieEnregistree, CurrentGame->PreneurPartieEnregistree);
#endif // DEBUG
    }
    return 1;
}
