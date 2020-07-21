#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include "Tarot_Game.h"

const char *ConfigDir;
gchar *AppConfDir;
GError *Error = NULL;
GKeyFile *UserPrefFile;
int isConfModified;
gchar *FileConfName;
gchar *AppConfDir;

void ReadStyleJoueur(GKeyFile *UserPrefFile, int Position, const char *Key, int *Modified)
{
double *liste_double;
gsize liste_len;

    Error = NULL;
    memset(StyleJoueur[Position], 0, sizeof(double)*NB_ENTREE_STYLE_JEU);
    liste_double = g_key_file_get_double_list(UserPrefFile, "Style", Key, &liste_len, &Error);
    if ( liste_double == NULL  )
    {
        g_key_file_set_double_list(UserPrefFile, "Style", Key, StyleJoueur[Position], sizeof(double)*NB_ENTREE_STYLE_JEU);
        *Modified = 1;
    }
    if ( liste_len >= 1 )
        StyleJoueur[Position][dStyleLongues] = liste_double[0];
    if ( liste_len >= 2 )
        StyleJoueur[Position][dStyleNbPoints] = liste_double[1];
    if ( liste_len >= 3 )
        StyleJoueur[Position][dStylePetiteDernier] = liste_double[2];
    if ( liste_len >= 4 )
        StyleJoueur[Position][dStyleBout] = liste_double[3];
    if ( liste_len >= 5 )
        StyleJoueur[Position][dStyleNbAtout] = liste_double[4];
    if ( liste_len >= 6 )
        StyleJoueur[Position][dStyleRisque] = liste_double[5];
    if ( liste_len >= 7 )
        StyleJoueur[Position][dStyleAttChasse] = liste_double[6];
    if ( liste_len >= 8 )
        StyleJoueur[Position][dStyleAttPoints] = liste_double[7];
    if ( liste_len >= 9 )
        StyleJoueur[Position][dStyleAttPetit] = liste_double[8];
    if ( liste_len >= 10 )
        StyleJoueur[Position][dStyleDefChasse] = liste_double[9];
    if ( liste_len >= 11 )
        StyleJoueur[Position][dStyleDefPoints] = liste_double[10];
    if ( liste_len >= 12 )
        StyleJoueur[Position][dStyleDefDefausse] = liste_double[11];
    if ( liste_len != 12 )
    {
        g_key_file_set_double_list(UserPrefFile, "Style", Key, StyleJoueur[Position], NB_ENTREE_STYLE_JEU);
        *Modified = 1;
    }
}

void ReadContraintesDistribution(GKeyFile *UserPrefFile, int *Modified)
{
    memset(&ContraintesDistribution, 0, sizeof(struct _ContrainteDistrib));
    ContraintesDistribution.JeuFixeContrat = -1;        //  Pas de type de parties par défaut
    Error = NULL;
    ContraintesDistribution.MinBout = g_key_file_get_integer(UserPrefFile, "Distribution", "MinBout", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinBout", ContraintesDistribution.MinBout);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinAtout = g_key_file_get_integer(UserPrefFile, "Distribution", "MinAtout", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinAtout", ContraintesDistribution.MinAtout);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinRoi = g_key_file_get_integer(UserPrefFile, "Distribution", "MinRoi", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinRoi", ContraintesDistribution.MinRoi);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinTrefle = g_key_file_get_integer(UserPrefFile, "Distribution", "MinTrefle", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinTrefle", ContraintesDistribution.MinTrefle);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinCarreau = g_key_file_get_integer(UserPrefFile, "Distribution", "MinCarreau", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinCarreau", ContraintesDistribution.MinCarreau);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinCoeur = g_key_file_get_integer(UserPrefFile, "Distribution", "MinCoeur", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinCoeur", ContraintesDistribution.MinCoeur);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinPique = g_key_file_get_integer(UserPrefFile, "Distribution", "MinPique", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinPique", ContraintesDistribution.MinPique);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MinPoints = g_key_file_get_integer(UserPrefFile, "Distribution", "MinPoints", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MinPoints", ContraintesDistribution.MinPoints);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.MaxPoints = g_key_file_get_integer(UserPrefFile, "Distribution", "MaxPoints", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Distribution", "MaxPoints", ContraintesDistribution.MaxPoints);
        *Modified = 1;
    }
    Error = NULL;
    ContraintesDistribution.JeuFixeContrat = g_key_file_get_integer(UserPrefFile, "Distribution", "JeuFixeContrat", &Error);
    if ( Error != NULL  )
    {
        ContraintesDistribution.JeuFixeContrat = -1;
        g_key_file_set_integer(UserPrefFile, "Distribution", "JeuFixeContrat", ContraintesDistribution.JeuFixeContrat);
        *Modified = 1;
    }
}

void SaveConfFile()
{
    if ( !isConfModified ) return;
    printf("Fichier de configuration modifié, le sauve\n");
    Error = NULL;
    // Save as a file.
    if (!g_key_file_save_to_file (UserPrefFile, FileConfName, &Error))
    {
        g_warning ("Error saving key file: %s", Error->message);
        return;
    }
    isConfModified = 0;
}

//  Lecture des scores

void ReadScores(int *Modified)
{
int i;
char strMessage[64];
gsize Size;
int *IntList;

    memset(&ResPartie, 0, sizeof(ResPartie));
    Error = NULL;
    NumDonnes = 0;
    NumDonnes = g_key_file_get_integer(UserPrefFile, "Scores", "NumDonnes", &Error);
    if ( Error != NULL  )
    {
        g_key_file_set_integer(UserPrefFile, "Scores", "NumDonnes", NumDonnes);
        *Modified = 1;
    }
    for ( i = 0; i < NumDonnes; i++ )
    {
        snprintf(strMessage, 32, "Donne_%d", i+1);
        Error = NULL;
        IntList = g_key_file_get_integer_list(UserPrefFile, "Scores", strMessage, &Size, &Error);
        if ( Error == NULL && Size == 4 )
        {
            ResPartie[i].TypeEnregistre = IntList[0];
            ResPartie[i].Preneur = IntList[1];
            ResPartie[i].ScoreAttaque = IntList[2];
            ResPartie[i].ScoreDefense = IntList[3];
        }
    }
}

void ReadUserPreferences()
{
int AppDirOK = 0;
int ConfFileOK = 0;
int Ret;
gchar *str_val;
int *liste_int;
gsize liste_len;

    ConfigDir = g_get_user_config_dir();
    isConfModified = 0;
    AppConfDir = g_build_filename(ConfigDir, "tarot", NULL);
    FileConfName = g_build_filename(ConfigDir,"tarot", "tarot.conf", NULL);
    printf("Config ir is : %s, ConfDir %s, ConfFile %s\n", ConfigDir, AppConfDir, FileConfName);
    //  Teste si le répertoire de configuration existe
    AppDirOK = g_file_test(AppConfDir, G_FILE_TEST_EXISTS);
    if ( !AppDirOK )
    {
        printf("AppDir n'existe pas, création");
        Ret = g_mkdir(AppConfDir, S_IRUSR|S_IWUSR|S_IXUSR);
        if ( Ret != 0 )
            printf("Ne peut créer le répertoire de configuration, erreur %s", strerror(errno));
        else
            AppDirOK = 1;    }
    if ( AppDirOK )
    {
        AppDirOK = g_file_test(AppConfDir, G_FILE_TEST_IS_DIR);
        if ( AppDirOK == 0 )
        {
            printf("Répertoire de configuration n'est pas un répertoire\n");
            return;
        }
    }
    //  Teste si fichier de configuration existe
    ConfFileOK = g_file_test(FileConfName, G_FILE_TEST_EXISTS);
    if ( !ConfFileOK )
    {
        printf("Fichier de configuration inexistant\n");
    }
    UserPrefFile = g_key_file_new ();
    if (!g_key_file_load_from_file (UserPrefFile, FileConfName, G_KEY_FILE_NONE, &Error))
    {
        if (!g_error_matches (Error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
            g_warning ("Error loading key file: %s", Error->message);
    }
    //  Lecture des noms des joueurs
    Error= NULL;
    str_val = g_key_file_get_string (UserPrefFile, "NomsJoueurs", "SUD", &Error);
    if ( str_val != NULL )
        NomJoueurs[SUD] = str_val;
    else
    {
        NomJoueurs[SUD] = "Thierry";
        g_key_file_set_string(UserPrefFile, "NomsJoueurs", "SUD", NomJoueurs[SUD]);
        isConfModified = 1;
    }
    Error= NULL;
    str_val = g_key_file_get_string (UserPrefFile, "NomsJoueurs", "OUEST", &Error);
    if ( str_val != NULL )
        NomJoueurs[OUEST] = str_val;
    else
    {
        NomJoueurs[OUEST] = "Soizic";
        g_key_file_set_string(UserPrefFile, "NomsJoueurs", "OUEST", NomJoueurs[OUEST]);
        isConfModified = 1;
    }
    Error= NULL;
    str_val = g_key_file_get_string (UserPrefFile, "NomsJoueurs", "EST", &Error);
    if ( str_val != NULL )
        NomJoueurs[EST] = str_val;
    else
    {
        NomJoueurs[EST] = "Marie-Claire";
        g_key_file_set_string(UserPrefFile, "NomsJoueurs", "EST", NomJoueurs[EST]);
        isConfModified = 1;
    Error= NULL;
    }
    str_val = g_key_file_get_string (UserPrefFile, "NomsJoueurs", "NORD", &Error);
    if ( str_val != NULL )
        NomJoueurs[NORD] = str_val;
    else
    {
        NomJoueurs[NORD] = "Catherine";
        g_key_file_set_string(UserPrefFile, "NomsJoueurs", "NORD", NomJoueurs[NORD]);
        isConfModified = 1;
    }
    //  Flags Jeu : Signalisation, parties enregistrées
    Error = NULL;
    FlagSignalisation = g_key_file_get_boolean(UserPrefFile, "Jeu", "Signalisation", &Error);
    if ( Error != NULL )
    {
        FlagSignalisation = 1;
        g_key_file_set_boolean(UserPrefFile, "Jeu", "Signalisation", FlagSignalisation);
        isConfModified = 1;
    }
    Error = NULL;
    FlagPartieEnregistreeAttaque = g_key_file_get_boolean(UserPrefFile, "Jeu", "PartieEnregistreeAttaque", &Error);
    if ( Error != NULL )
    {
        FlagPartieEnregistreeAttaque = 0;
        g_key_file_set_boolean(UserPrefFile, "Jeu", "PartieEnregistreeAttaque", FlagPartieEnregistreeAttaque);
        isConfModified = 1;
    }
    Error = NULL;
    FlagPartieEnregistreeDefense = g_key_file_get_boolean(UserPrefFile, "Jeu", "PartieEnregistreeDefense", &Error);
    if ( Error != NULL )
    {
        FlagPartieEnregistreeDefense = 0;
        g_key_file_set_boolean(UserPrefFile, "Jeu", "PartieEnregistreeDefense", FlagPartieEnregistreeDefense);
        isConfModified = 1;
    }
    //  Flags affichage
    Error = NULL;
    FlagAffichageAtouts = g_key_file_get_boolean(UserPrefFile, "Affichage", "Atouts", &Error);
    if ( Error != NULL )
    {
        FlagAffichageAtouts = 0;
        g_key_file_set_boolean(UserPrefFile, "Affichage", "Atouts", FlagAffichageAtouts);
        isConfModified = 1;
    }
    Error = NULL;
    FlagAffichagePoints = g_key_file_get_boolean(UserPrefFile, "Affichage", "Points", &Error);
    if ( Error != NULL )
    {
        FlagAffichagePoints = 0;
        g_key_file_set_boolean(UserPrefFile, "Affichage", "Points", FlagAffichagePoints);
        isConfModified = 1;
    }
    AfficheJeuJoueurs[SUD] = 1;
    Error = NULL;
    AfficheJeuJoueurs[EST] = g_key_file_get_boolean(UserPrefFile, "Affichage", "EST", &Error);
    if ( Error != NULL )
    {
        FlagAffichagePoints = 0;
        g_key_file_set_boolean(UserPrefFile, "Affichage", "EST", AfficheJeuJoueurs[EST]);
        isConfModified = 1;
    }
    Error = NULL;
    AfficheJeuJoueurs[NORD] = g_key_file_get_boolean(UserPrefFile, "Affichage", "NORD", &Error);
    if ( Error != NULL )
    {
        FlagAffichagePoints = 0;
        g_key_file_set_boolean(UserPrefFile, "Affichage", "NORD", AfficheJeuJoueurs[NORD]);
        isConfModified = 1;
    }
    Error = NULL;
    AfficheJeuJoueurs[OUEST] = g_key_file_get_boolean(UserPrefFile, "Affichage", "OUEST", &Error);
    if ( Error != NULL )
    {
        FlagAffichagePoints = 0;
        g_key_file_set_boolean(UserPrefFile, "Affichage", "OUEST", AfficheJeuJoueurs[OUEST]);
        isConfModified = 1;
    }
    //  Les parties enregistrées jouées récemment
    Error = NULL;
    liste_int = g_key_file_get_integer_list(UserPrefFile, "PartiesEnregistrees", "Age", &liste_len, &Error);
    if ( liste_int == NULL || liste_len != NB_PARTIE_MAITRE )
    {
        memset(MaitreJouee, 0, sizeof(MaitreJouee));
        g_key_file_set_integer_list(UserPrefFile, "PartiesEnregistrees", "Age", MaitreJouee, NB_PARTIE_MAITRE);
        isConfModified = 1;
    }
    Error = NULL;
    NbPartieMaitreJouee = g_key_file_get_integer(UserPrefFile, "PartiesEnregistrees", "NumJouees", &Error);
    if ( Error != NULL )
    {
        NbPartieMaitreJouee = 0;
        g_key_file_set_integer(UserPrefFile, "PartiesEnregistrees", "NumJouees", NbPartieMaitreJouee);
        isConfModified = 1;
    }
    Error = NULL;
    FlagSuggestionChien = g_key_file_get_boolean(UserPrefFile, "Preferences", "Chien", &Error);
    if ( Error != NULL )
    {
        FlagSuggestionChien = 1;
        g_key_file_set_boolean(UserPrefFile, "Preferences", "Chien", FlagSuggestionChien);
        isConfModified = 1;
    }
    Error = NULL;
    FlagSuggestionCarte = g_key_file_get_boolean(UserPrefFile, "Preferences", "Carte", &Error);
    if ( Error != NULL )
    {
        FlagSuggestionCarte = 1;
        g_key_file_set_boolean(UserPrefFile, "Preferences", "Carte", FlagSuggestionCarte);
        isConfModified = 1;
    }
    Error = NULL;
    FlagSuggestionPoignee = g_key_file_get_boolean(UserPrefFile, "Preferences", "Poignee", &Error);
    if ( Error != NULL )
    {
        FlagSuggestionPoignee = 1;
        g_key_file_set_boolean(UserPrefFile, "Preferences", "Poignee", FlagSuggestionPoignee);
        isConfModified = 1;
    }
    Error = NULL;
    PassageAutoPliSuivant = g_key_file_get_boolean(UserPrefFile, "Preferences", "PliSuivant", &Error);
    if ( Error != NULL )
    {
        PassageAutoPliSuivant = 1;
        g_key_file_set_boolean(UserPrefFile, "Preferences", "PliSuivant", PassageAutoPliSuivant);
        isConfModified = 1;
    }
    Error = NULL;
    DelaiPliAuto = g_key_file_get_integer(UserPrefFile, "Preferences", "DelaiPliSuivant", &Error);
    if ( Error != NULL )
    {
        DelaiPliAuto = DECOMPTE_FIN_PLI;
        g_key_file_set_integer(UserPrefFile, "Preferences", "DelaiPliSuivant", DelaiPliAuto);
        isConfModified = 1;
    }

    //  Les contraintes de distribution
    ReadContraintesDistribution(UserPrefFile, &isConfModified);
    //  Et enfin le style des joueurs
    ReadStyleJoueur(UserPrefFile, SUD, "SUD", &isConfModified);
    ReadStyleJoueur(UserPrefFile, EST, "EST", &isConfModified);
    ReadStyleJoueur(UserPrefFile, NORD, "NORD", &isConfModified);
    ReadStyleJoueur(UserPrefFile, OUEST, "OUEST", &isConfModified);

    ReadScores(&isConfModified);
    //  Si le fichier a été modifié (car incomplet), le sauve
    SaveConfFile();
}

void ChangePartieMaitreConf()
{
    Error = NULL;
    g_key_file_set_integer_list(UserPrefFile, "PartiesEnregistrees", "Age", MaitreJouee, NB_PARTIE_MAITRE);
    Error = NULL;
    g_key_file_set_integer(UserPrefFile, "PartiesEnregistrees", "NumJouees", NbPartieMaitreJouee);
    isConfModified = 1;
}

void ChangeUserConfigSignalisation(void)
{
    Error = NULL;
    g_key_file_set_boolean(UserPrefFile, "Jeu", "Signalisation", FlagSignalisation);
    isConfModified = 1;
}

void ChangeUserConfigModeEnregistre(void)
{
    Error = NULL;
    g_key_file_set_boolean(UserPrefFile, "Jeu", "PartieEnregistreeAttaque", FlagPartieEnregistreeAttaque);
    g_key_file_set_boolean(UserPrefFile, "Jeu", "PartieEnregistreeDefense", FlagPartieEnregistreeDefense);
    isConfModified = 1;
}

void ChangeUserConfigAffichage(void)
{
    Error = NULL;
    g_key_file_set_boolean(UserPrefFile, "Affichage", "NORD", AfficheJeuJoueurs[NORD]);
    g_key_file_set_boolean(UserPrefFile, "Affichage", "EST", AfficheJeuJoueurs[EST]);
    g_key_file_set_boolean(UserPrefFile, "Affichage", "OUEST", AfficheJeuJoueurs[OUEST]);
    g_key_file_set_boolean(UserPrefFile, "Affichage", "Points", FlagAffichagePoints);
    g_key_file_set_boolean(UserPrefFile, "Affichage", "Atouts", FlagAffichageAtouts);
    isConfModified = 1;
}

void ChangeUserPref(void)
{
    Error = NULL;
    g_key_file_set_boolean(UserPrefFile, "Preferences", "Chien", FlagSuggestionChien);
    g_key_file_set_boolean(UserPrefFile, "Preferences", "Poignee", FlagSuggestionPoignee);
    g_key_file_set_boolean(UserPrefFile, "Preferences", "Carte", FlagSuggestionCarte);
    g_key_file_set_boolean(UserPrefFile, "Preferences", "PliSuivant", PassageAutoPliSuivant);
    g_key_file_set_integer(UserPrefFile, "Preferences", "DelaiPliSuivant", DelaiPliAuto);
    isConfModified = 1;
}

void ChangeContraintesDistribution()
{
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinBout", ContraintesDistribution.MinBout);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinAtout", ContraintesDistribution.MinAtout);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinRoi", ContraintesDistribution.MinRoi);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinTrefle", ContraintesDistribution.MinTrefle);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinCarreau", ContraintesDistribution.MinCarreau);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinCoeur", ContraintesDistribution.MinCoeur);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinPique", ContraintesDistribution.MinPique);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MinPoints", ContraintesDistribution.MinPoints);
    g_key_file_set_integer(UserPrefFile, "Distribution", "MaxPoints", ContraintesDistribution.MaxPoints);
    g_key_file_set_integer(UserPrefFile, "Distribution", "JeuFixeContrat", ContraintesDistribution.JeuFixeContrat);
    isConfModified = 1;
}

void ChangeNomsJoueurs()
{
    //  Sauve les noms des joueurs
    g_key_file_set_string(UserPrefFile, "NomsJoueurs", "SUD", NomJoueurs[SUD]);
    g_key_file_set_string(UserPrefFile, "NomsJoueurs", "NORD", NomJoueurs[NORD]);
    g_key_file_set_string(UserPrefFile, "NomsJoueurs", "OUEST", NomJoueurs[OUEST]);
    g_key_file_set_string(UserPrefFile, "NomsJoueurs", "EST", NomJoueurs[EST]);
    isConfModified = 1;
}


//  Sauve les scores de la partie en cours

void SaveScores(void)
{
int i;
char strMessage[64];
int IntList[4];

    g_key_file_set_integer(UserPrefFile, "Scores", "NumDonnes", NumDonnes);
    for ( i = 0; i < NOMBRE_DONNES_PARTIE; i++ )
    {
        snprintf(strMessage, 32, "Donne_%d", i+1);
        if ( i < NumDonnes )
        {
            IntList[0] = ResPartie[i].TypeEnregistre;
            IntList[1] = ResPartie[i].Preneur;
            IntList[2] = ResPartie[i].ScoreAttaque;
            IntList[3] = ResPartie[i].ScoreDefense;
        }
        else
        {
            IntList[0] = 0;
            IntList[1] = 0;
            IntList[2] = 0;
            IntList[3] = 0;
        }
        g_key_file_set_integer_list(UserPrefFile, "Scores", strMessage, IntList, 4);
    }
    isConfModified = 1;
}

void SaveStyleJoueurs(void)
{
    g_key_file_set_double_list(UserPrefFile, "Style", "EST", StyleJoueur[EST], sizeof(double)*NB_ENTREE_STYLE_JEU);
    g_key_file_set_double_list(UserPrefFile, "Style", "NORD", StyleJoueur[NORD], sizeof(double)*NB_ENTREE_STYLE_JEU);
    g_key_file_set_double_list(UserPrefFile, "Style", "OUEST", StyleJoueur[OUEST], sizeof(double)*NB_ENTREE_STYLE_JEU);
    isConfModified = 1;
}
