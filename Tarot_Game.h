#ifndef TAROT_GAME_H_INCLUDED
#define TAROT_GAME_H_INCLUDED

#define NB_PARTIE_MAITRE    126

#define MAX_JOUEURS         4       //  Tarot à 4 seulement
#define MAX_CARTE_JOUEUR    24      //  18 + 6 du chien

#define NOMBRE_DONNES_PARTIE    15

//  Si plus faible que ce nombre, essaie toutes les possibilités
#define NOMBRE_DISTRIB_FIN_PARTIE_ATTAQUE   2000
#define MAX_CARTE_FIN_PARTIE_ATTAQUE        4
#define NOMBRE_DISTRIB_FIN_PARTIE_DEFENSE   2000
#define MAX_CARTE_FIN_PARTIE_DEFENSE        3


enum _Couleur_Carte {
    EXCUSE,
    ATOUT,
    TREFLE,
    CARREAU,
    PIQUE,
    COEUR,
    NB_COULEUR
};

struct _Carte_Jeu {
    int Couleur;        //  Type enum _Couleur_Carte
    int Valeur;         //  Valeur de la carte
    int Hauteur;        //  Relatif a la couleur
    int isPlayed;       //  Vrai : Carte jouée
    int Joueur;         //  Joueur ayant cette carte
    int Index;          //  Index carte (0..77)
};

enum _Hauteur_Carte {
    VALET = 11,
    CAVALIER = 12,
    DAME = 13,
    ROI = 14
};

enum _num_joueur {
    SUD = 0,
    EST = 1,
    NORD = 2,
    OUEST = 3,
    CHIEN = 4
};

enum _type_partie {
    ATTENTE_PARTIE = -1,
    PASSE = 0,
    PETITE = 1,
    GARDE = 2,
    GARDE_SANS = 3,
    GARDE_CONTRE = 4,
    NB_TYPE_PARTIE
};

enum StateJeu {
    JEU_VIDE = 0,
    JEU_ATTENTE_CONTRAT_J1,
    JEU_ATTENTE_CONTRAT_J2,
    JEU_ATTENTE_CONTRAT_J3,
    JEU_ATTENTE_CONTRAT_J4,
    JEU_FIN_CONTRAT,
    JEU_MONTRE_CHIEN,
    JEU_CHOIX_CARTES_CHIEN,
    JEU_PREMIER_PLI,
    JEU_MONTRE_POIGNEE,
    JEU_EN_COURS,
    JEU_FIN_PLI,
    JEU_ATTENTE_JEU_SUD,
    JEU_TERMINE,
    JEU_AFFICHE_POIGNEE,
    JEU_AFFICHE_ATOUT_ECART,
    JEU_CHOIX_POIGNEE,
    JEU_CHOIX_CARTE_POIGNEE,
    JEU_DECLARE_CHELEM,
    JEU_POSE_QUESTION_CHELEM,
    JEU_MESSAGE,
    JEU_AFFICHE_PLI_PRECEDENT,
    JEU_AFFICHE_SCORES,
    JEU_FIN_PARTIE,
    JEU_NOUVELLE_PARTIE,
    JEU_MESSAGE_NO_BUTTON
};

enum EtatChassePetit {
    CHASSE_TERMINEE = -2,
    CHASSE_POSSEDE_PETIT = -1,
    CHASSE_ATTENTE = 0,
    CHASSE_PRENEUR_DESSOUS,
    CHASSE_PRENEUR_GROS,
    CHASSE_DEFENSE_JOUE_21,
    CHASSE_DEFENSE_IMPAIR,
    CHASSE_DEFENSE_PAIR
};

enum _Type_Poignee {
    SANS_POIGNEE = 0,
    POIGNEE_NON_MONTREE,
    POIGNEE_SIMPLE,
    POIGNEE_DOUBLE,
    POIGNEE_TRIPLE
};


enum _type_timer {
    TIMER_NO = 0,
    TIMER_ENCHERES,
    TIMER_PARTIE
};
//  Variables dans le fichier de configuration

extern gchar *NomJoueurs[MAX_JOUEURS];
extern int FlagSignalisation;
extern int FlagPartieEnregistreeAttaque;
extern int FlagPartieEnregistreeDefense;
extern int totoPartieEnregistreeAttaque;
extern int totoPartieEnregistreeDefense;
extern int FlagAffichagePoints;
extern int FlagAffichageAtouts;
extern int AfficheJeuJoueurs[MAX_JOUEURS];
extern int MaitreJouee[NB_PARTIE_MAITRE];
extern int FlagSuggestionChien;
extern int FlagSuggestionCarte;
extern int FlagSuggestionPoignee;
extern int PassageAutoPliSuivant;
extern int DelaiPliAuto;
extern unsigned NbPartieMaitreJouee;


extern const int Startof[NB_COULEUR];
extern const int Endof[NB_COULEUR];
extern struct _Carte_Jeu Table[4];

#define ERREUR_PROBA_SURE       0.00001      //  Si erreur plus faible, considère sûr

struct _Jeu {

    struct _Carte_Jeu MyCarte[MAX_CARTE_JOUEUR];    //  Cartes du joueur
    //struct _Carte_Jeu CarteEcart[6];                //  Cartes mises à l'écart. Seul le preneur connait précisemment cela si Petite ou Garde
    int StatusCartes[78];               //  0 : ne possède pas la carte, 1 possède la carte, -1 carte déja jouée par un des joueurs
	double ForceMain[MAX_JOUEURS];		//	Force calculée (supposée) des différentes mains de la défense 0 faible, 1 fort
	double Obeissance[MAX_JOUEURS];		//	Valeur supposée de l'obeissance au joueur j. Si main forte, Obeissance = 1
	double ValEcart;                    //  Valeur de l'écart (sur si preneur et petite ou garde, sinon estimation)
	int NbCarte;                        //  Nombre de cartes en main pour le joueur
	int JoueurMainForte;			    //	Indique le joueur ayant la "main forte"
	int CouleurTenue;				    //	Indique la longue du preneur "tenue" par la défense


	int Flanc;						    //	Couleur de jeu préférentielle
	double ValFlanc[NB_COULEUR];		//	Valeurs des différents flancs (couleurs)

	int NbCouleur[NB_COULEUR];	    	//	Nombre de cartes dans chaque couleur


	double ValPetitAuBout0;			    //	Valeur initiale du petit au bout
	double ValPetitAuBout;			    //	Valeur actuelle du petit au bout

	int JoueurCouleur[NB_COULEUR];		//	Donne le n° du joueur ayant ouvert cette couleur.
	int CouleurEntameParJoueur[MAX_JOUEURS];	 //	Champ de bits des couleurs entamées par les joueurs
	int NbEntame[MAX_JOUEURS];				    //	Compte le nombre d'entames des joueurs

	int NbCoupeInit;				    //	Coupe "Initiales" (après écart) du preneur (d'après CoupeCouleur)
	int CoupeCouleur[NB_COULEUR];		//	1 si Coupe franche dans la couleur, -1 si Singlette

	int StatusChasse;				    //	Etat chasse au petit

	int IdxGagnant;					    //	Index du joueur ayant remporté le pli (0 = JoueurEntame)

	int ResteAtout;					    //	Nombre d'atouts toujours en jeu
	double AtoutPreneur;			    //	Nombre prévu d'atouts du preneur

	int NbDefausse[MAX_JOUEURS];	    //	Nombre de fois où le joueur j s'est défaussé
	int DefausseCouleurParJoueur[MAX_JOUEURS];          //	Champ de bits des couleurs où le joueur j s'est défaussé.
	int CouleurVoulueJoueur[MAX_JOUEURS][NB_COULEUR];	//	Volonté d'un joueur de jouer dans une couleur( defausse)
	int CouleurNonVoulueJoueur[MAX_JOUEURS][NB_COULEUR];//	Volonté d'un joueur de ne pas jouer dans une couleur
	int CouleurVoulue[MAX_JOUEURS];			    //	Somme des tableaux précédents par joueur

	int NbJoue[MAX_JOUEURS][NB_COULEUR];		//	Cartes jouées dans la couleur par le joueur

	double ProbCouleur[NB_COULEUR];			    //	Probabilité que le preneur possède de cette couleur
	double GuessProbCoupe[NB_COULEUR];		    //	Probabilité de coupe des couleurs par le preneur "devinées"
	double CalProbCoupe[NB_COULEUR];		    //	Probabilité de coupe des couleurs par le preneur "calculée"
	double ProbCoupe[NB_COULEUR];		        //	Probabilité de coupe d'une couleur par le preneur
	double TmpProbCoupe[NB_COULEUR];            //  Probabilité de coupe par le preneur, mis à jour pendant le pli
    double TmpProbCarte[5][78];			        //	Proba que le joueur j possède la carte c. Mis à jour pendant le pli
	double ProbCarte[5][78];			        //	Proba que le joueur j possède la carte c. Mis à jour entre les les plis
	double ConfianceProb[5][78];		        //	Confiance que l'on peut avoir dans la proba ProbCarte. Liée aux règles utilisées

	int NbAtout;                                //  Nombre d'atout du joueur
	int NbAtoutMaj;					            //	Nombre d'atouts forts ( >= 16 )
	double NbPoint;                             //  Points ne tenant compte que des forces des cartes
	double	NbPointDH;                          //  Points tenant compte de la distribution (coupes...)
	int NbBout;						            //	Nombre de bouts du joueur
	int TypePoignee[MAX_JOUEURS];		        //	Type de poignée annoncée par le joueur
	int PositionJoueur;                         //  Position du joueur pour cette structure _Jeu
	int PositionPreneur;                        //  Position du preneur
	int NbCouleurChien[NB_COULEUR];
	int NbAtoutSurCoupe;                        //  Nombre d'atouts utilisés par le preneur pour couper
    int PetitSurCoupe;                          //  Nombre de coupes avant de mettre le petit
	int		PetitImprenable;		            //	Vrai si le joueur possède le petit imprenable
};

//  Structure qui décrit la partie en cours.
//  Une seule structure est utilisée durant la partie "normale"
//  En fin de partie, quand le programme teste toutes les distributions possibles, d'autres structures sont allouées.

struct _Tarot_Partie {
    int JoueurDistrib;                      //  Numéro du joueur qui a distribué
    int JoueurPreneur;                      //  Numéro du joueur qui a pris.
    int JoueurEntame;                       //  Joueur débutant le pli en cours
    int JoueurCourant;                      //  Position du joueur courant;
    int FlagAfficheJoueur[MAX_JOUEURS];     //  Vrai si les cartes doivent être montrées.
    int NbCarteJoueur[MAX_JOUEURS];         //  Nombre de cartes de chaque joueur
    int IdxCartesJoueur[MAX_JOUEURS][MAX_CARTE_JOUEUR];
    int IdxCartesChien[6];                  //  Index des cartes du chien
    int CartePli[MAX_JOUEURS];              //  Cartes jouées par chacun pour le pli en cours, si -1 rien de joué
    int AnnonceJoueur[MAX_JOUEURS];         //  Annonce de chaque joueur
    int TypePartie;                         //  Annonce la plus haute
    int isCarteLevee[MAX_CARTE_JOUEUR];     //  Vrai si carte levée, mise à l'écart ou montre poignée.
    int Carte2Joueur[78];                   //  Affectation Carte --> Joueur
    int Carte2JoueurDistrib[78];            //  Affectation Carte --> Joueur au moment distribution
    int CarteAuChien[78];                   //  Vrai si la carte a été vue au chien
    int CarteJouee[78];                     //  -1 si carte pas encore jouée, sinon n° du joueur
    int StateJeu;
    struct _Carte_Jeu CartesJeu[78];        //  Pour accès via l'index carte
    struct _Jeu JeuJoueur[MAX_JOUEURS];     //  Structure utilisée pour faire jouer un joueur
    int ListeAtoutPoignee[MAX_JOUEURS][15]; //  Atouts montrés dans la poignée par le joueur
    int NumAtoutPoignee[MAX_JOUEURS];       //  Nombre d'atouts dans la poignée du joueur
    int JoueurAffichePoignee;               //  Joueur affichant sa poignée
    int AffChoixJoueur;
    char *InfoMessage;                      //  Message à afficher si état JEU_MESSAGE
    int StateAfterMessage;
    double DeltaYMessage;
    double ProbaDistrib;                    //  Probabilité de la distribution en cours (fin de partie seulement)
    int PartiePetitImprenable;
    int TypeTimer;                          //  Type de timer en cours
    int NumPartieEnregistree;               //  Numéro partie enregistrée ou -1 si non
    int PreneurPartieEnregistree;           //  Preneur pour cette partie
    int NbCarteLevee;                       //  Nombre de cartes levées
    int NbAtoutEcart;                       //  Nombre d'atouts mis à l'écart
    int DecompteFinPli;                     //  Compteur attente fin de pli.
    int MemPlis[MAX_CARTE_JOUEUR][MAX_JOUEURS]; //  Mémorise les cartes jouées par chaque joueur
    int Entame[MAX_CARTE_JOUEUR];           //  Mémorise qui avait l'entame à chaque pli
    int NumPli;                             //  numéro du pli : de 0 à 17
    int NumBoutsPreneur;                    //  Nombre de bouts dans les plis du preneur
    int NumBoutsDefense;                    //  Nombre de bouts pour la défense.
    int NumAtoutsJoues;                     //  Nombre d'atouts joués
    int NumPointsPreneur;                   //  Nombre de points preneur
    int NumPointsDefense;                   //  Nombre de points défense
    int NumPlisPreneur;                     //  Nombre de plis du preneur
    int PointPetitAuBoutAttaque;            //  Points petit au bout
    int PointPetitAuBoutDefense;            //  Points petit au bout
    int PointsChelemAttaque;                //  Points pour le Chelem
    int PointsChelemDefense;                //  Points pour le Chelem
    int PointsPoigneeAttaque;               //  Points de la poignée éventuelle attaque
    int PointsPoigneeDefense;               //  Points de la poignée éventuelle attaque
    int PointsDefense;                      //  Points pour l'attaque
    int PointsAttaque;                      //  Points pour la défense
    int PointsMinPreneur;
    int PartieGagnee;
    int PoigneeMontree[MAX_JOUEURS];        //  Type de poignée montrée par les joueurs
    int ChelemDemande;                      //  Vrai si le preneur a demandé un Chelem
} ;

typedef struct _Tarot_Partie *TarotGame;
extern TarotGame CurrentGame;

enum e_Style_Jeu {
    dStyleLongues,              //  Favorise les longues pour prendre
    dStyleNbPoints,             //  Favorise les honneurs pour prendre
    dStylePetiteDernier,        //  Prise en dernier avec jeu faible
    dStyleBout,                 //  Favorise les bouts
    dStyleNbAtout,              //  Favorise le nombre d'atouts
    dStyleRisque,               //  Joueur aimant le risque (prise)
    dStyleAttChasse,            //  Intérêt pour chasse en attaque
    dStyleAttPoints,            //  Sauve rapidement les points en attaque
    dStyleAttPetit,             //  Sauve rapidement le petit en attaque
    dStyleDefChasse,            //  Intérêt pour chasse en défense
    dStyleDefPoints,            //  Sauve rapidement les points en défense
    dStyleDefDefausse,           //  Facilité de défausse
    NB_ENTREE_STYLE_JEU
};

struct _Donne {
    int TypeEnregistre;         //  1 si partie enregistrée (compte pourcentage dans ce cas)
    int Preneur;
    int ScoreAttaque;
    int ScoreDefense;
};

extern int NumDonnes;
extern struct _Donne ResPartie[NOMBRE_DONNES_PARTIE];


extern const  double NbAtoutMoyen[4][4];
extern double StyleJoueur[MAX_JOUEURS][NB_ENTREE_STYLE_JEU];
extern int NbAtoutPoignee[4];
extern const char *strContrat[5];

void OpenDebugFile(const char *Name);
void CloseDebugFile();
void OutDebug(char *fmt, ...);
void ImprimeCarte(struct _Carte_Jeu *Carte);
char *strNomCarte(char *Buffer, int Index);

void DistribueJeu(TarotGame CurrentGame);
void DistribuePartieMaitre(TarotGame CurrentGame, int ChixFixe);
void RejouePartie(TarotGame CurrentGame);
void RejoueJeu(TarotGame CurrentGame);
void DistribuePartie(TarotGame CurrentGame);

void InitDistribue(TarotGame CurrentGame);
void AjouteCarteChienPreneur(TarotGame CurrentGame);
int isEcartable(TarotGame CurrentGame, int idx);
int EvalJeu(TarotGame CurrentGame, int Joueur);
void RepartitionCarte(TarotGame CurrentGame);
void MetCartesEcart(TarotGame CurrentGame);
int MakeEcart(TarotGame CurrentGame);
void DebutPartie(TarotGame CurrentGame);
void LeveCartesPossiblesSUD(TarotGame CurrentGame);
void BaisseCartesSUD(TarotGame CurrentGame);
int OkToShowPoignee(TarotGame CurrentGame);
void MonteAtoutsPoignee(TarotGame CurrentGame);
int isOKPoignee(TarotGame CurrentGame, int Index);
void RegardePoigneeJoueurs(TarotGame CurrentGame, int JoueurAvecPoignee);
void RegardeAtoutEcart(TarotGame CurrentGame, struct _Jeu *pJeu);
void IntegreInformationsChienJoueurs(TarotGame CurrentGame);
void SelectAtoutsLeveePoignee(TarotGame CurrentGame);
int NombrePointsContrat(TarotGame CurrentGame);
void ComptePointsFinPartie(TarotGame CurrentGame, int FlagFinPartie);
double ResultatPartieMaitre(TarotGame CurrentGame);
int CalcScoresPartie(double ScoreJoueurs[MAX_JOUEURS], int *JoueurGagnant);
int RegardeChelemPreneur(TarotGame CurrentGame);
int NbAtoutJoues(TarotGame CurrentGame);

//  Fonctions liées au probabilités
void InitProbaJoueurs(TarotGame CurrentGame);
void CheckProbaJoueurs(TarotGame CurrentGame);
void MonteProba(TarotGame CurrentGame, int Position, int Joueur, int IndexCarte, double mul, int FlagConfiance);
void BaisseProba(TarotGame CurrentGame, int Position, int Joueur, int IndexCarte, double ProbRegle);
void NormaliseProba(TarotGame CurrentGame, struct _Jeu *pJeu);
void RenormaliseMontant(struct _Jeu *pJeu, int Joueur, int NbCarteAttendues);
void RenormaliseDescendant(struct _Jeu *pJeu, int Joueur, int NbCarteAttendues);
double GetProbAtout(struct _Jeu *pJeu, int Joueur);
double ProbaExactementN(struct _Jeu *pJeu, int Joueur, int N, int Index, int Borne);
double GetProbAtout(struct _Jeu *pJeu, int joueur);
double ProbUnDefenseurSansAtout(struct _Jeu *pJeu);
void SetCarte2Position(struct _Jeu *pJeu, int Position, int Index);
void CopieProba2Tmp(struct _Jeu *pJeu);
int CheckProbaJeu(TarotGame CurrentGame, struct _Jeu *pJeu);
void Regle2Proba(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);
double ProbaMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Joueur, int Couleur);
void CalcTmpProbCoupe(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);
void BaisseTmpProb(struct _Jeu *pJeu, int posJoueur, int IndexCarte, double ProbRegle, double NumRegle);
void MonteTmpProb(struct _Jeu *pJeu, int posJoueur, int IndexCarte, double mul, double NumRegle);
void NormaliseTmpProba(TarotGame CurrentGame, struct _Jeu *pJeu, int indexJoueur);
void Attract_Proba(TarotGame CurrentGame, struct _Jeu *pJeu);

//  Heuristiques
void Heuristique_Petit_Proba(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);
void Heuristique_Chasse(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);
void Heuristique_Atout(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);
void Heuristique_Couleur(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable);

//  Fonctions de jeu communes attaque défense
void InitStructJeu(TarotGame CurrentGame, struct _Jeu *pJeu);
int IsJoue(TarotGame CurrentGame, int i);
int HasCarte(struct _Jeu *pJeu, int joueur, int IndexCarte);
int JoueurAvecPetit(TarotGame CurrentGame, struct _Jeu *pJeu);
double LongueurMoyenneCouleur(struct _Jeu *pJeu, int Joueur, int c);
int HasJoueCouleur(TarotGame CurrentGame, int Joueur, int Couleur );
int IsMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Color);
int NbMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Color);
int GetPlusFaible(struct _Jeu *pJeu, int c);
int GetPlusForte(struct _Jeu *pJeu, int c);
struct _Carte_Jeu *GetCartePlusFaible(struct _Jeu *pJeu, int c);
struct _Carte_Jeu *GetCartePlusForte(struct _Jeu *pJeu, int c);
int NbPossCouleur(struct _Jeu *pJeu, int Couleur);
int NbGrosAtout(TarotGame CurrentGame, struct _Jeu *pJeu, int Position, int N);
int NbAtoutMaitre(TarotGame CurrentGame, int Position);
int IsSigNOK(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur);
int NbNonHonneur(TarotGame CurrentGame, int Position, int Couleur);
int NbReste(TarotGame CurrentGame, int Position, int Couleur);
int ComptePoints(struct _Jeu *pJeu);
int isSup(struct _Carte_Jeu *pCarte1, struct _Carte_Jeu *pCarte2);
double AvgLongueur(struct _Jeu *pJeu, int Joueur, int Couleur);
double ProbCoupeJoueur(struct _Jeu *pJeu, int Joueur, int Couleur);
double ProbCoupeAvantPreneur(struct _Jeu *pJeu, int c);
double ProbCoupeApresPreneur(struct _Jeu *pJeu, int c);
double HasBarrage(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur);
int CarteMaitreNonJouee(TarotGame CurrentGame, int Couleur);
int CarteMaitreNonJoueeNoErr(TarotGame CurrentGame, int Couleur);
int MinLongueur(struct _Jeu *pJeu, int Joueur, int Couleur);
int MaxLongueur(struct _Jeu *pJeu, int Joueur, int Couleur);
double CalcPointRestantAJouer(TarotGame CurrentGame, int Couleur);
int IsMaitre(TarotGame CurrentGame, struct _Jeu *pJeu, struct _Carte_Jeu *pCarte);
double ForcePreneurCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur);
int MinPlisPreneur(TarotGame CurrentGame, struct _Jeu *pJeu);
int NbBoutDefense(TarotGame CurrentGame, int Position);
int NbCouleurOuverte(struct _Jeu *pJeu);
int CompteCouleurJoueur(struct _Jeu *pJeu, int Joueur);
double EvalCoupSecond(TarotGame CurrentGame, struct _Jeu *pJeu);
double EvalCoupTroisieme(TarotGame CurrentGame, struct _Jeu *pJeu);
double EvalCoup(TarotGame CurrentGame, struct _Jeu *pJeu, int *iGagnant);
void MakeCarte(struct _Carte_Jeu *Carte, int Index);
int Carte2Index(struct _Jeu *pJeu, int Couleur, int Hauteur);
int ComptePlusGrand(TarotGame CurrentGame, int c, int h);
double PointsRestants(struct _Jeu *pJeu, int Joueur, int Couleur);
double CalcPointRestant(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur, int idxGagnant, int PosP);
double GetPointMinHonneur(struct _Jeu *pJeu, int c);
int HasPetit(struct _Jeu *pJeu);
int HasExcuse(struct _Jeu *pJeu);
double PlisPossibles(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur,  double *TousUtiles);
int CompteHonneur(TarotGame CurrentGame, int Joueur, int ResParCouleur[]);
void CalcTmpProbCarte(TarotGame CurrentGame);
int LookForSigDescendant(TarotGame CurrentGame, int PosJoueur, int Couleur);
double ProbRestantAuDessus(struct _Jeu *pJeu, int h, int pos, int Idx);
int LookForPetit(TarotGame CurrentGame, int joueur);
int NextAtout(TarotGame CurrentGame, int Index);
int AtoutMaitre(TarotGame CurrentGame);
double nbDistrib(TarotGame CurrentGame, struct _Jeu *pJeu);
void GainPetitAuBout(TarotGame CurrentGame, struct _Jeu *pJeu);
int FinPartie(TarotGame CurrentGame);

//  Fonctions liées à la Table de jeu

int CarteValide(struct _Jeu *pJeu, int IndexCarte, int JoueurEntame);
void PoseCarte(TarotGame CurrentGame, int IndexCarte);
void RamassePli(TarotGame CurrentGame, int ModeFinPartie);
int PlusFortAtoutJoue(int Position);
int Gagnant(int idxPosition);
int GainPli(TarotGame CurrentGame, int PosTable);
int PointsPetitAuBout(TarotGame CurrentGame);
int isCarteInPli(TarotGame CurrentGame, int nPli, int IndexCarte );

//  Jeu en attaque
void JoueEnPremierAttaque(TarotGame CurrentGame);
void JoueAttaqueSecond(TarotGame CurrentGame);
void JoueAttaqueTroisieme(TarotGame CurrentGame);
void JoueAttaqueDernier(TarotGame CurrentGame);
double CalcProbaChelem(TarotGame CurrentGame);
double ProbaPetitPrenable(struct _Jeu *pJeu);
void InitPetitAuBout(TarotGame CurrentGame);

//  Jeu en défense
void InitProbaCoupes(TarotGame CurrentGame, struct _Jeu *pJeu);
void CalcProbCoupe(TarotGame CurrentGame, struct _Jeu *pJeu, int JoueurEntame);
void EntameDefense(TarotGame CurrentGame);
void JoueEnPremierDefense(TarotGame CurrentGame);
void JoueDefenseSecond(TarotGame CurrentGame);
void JoueDefenseTroisieme(TarotGame CurrentGame);
void JoueDefenseDernier(TarotGame CurrentGame);
int JoueDefausseEnSecond(TarotGame CurrentGame, struct _Jeu *pJeu);
double ProbGainCoup(TarotGame CurrentGame, struct _Jeu *pJeu, int indexTable, int CarteDef);
int JoueDefenseGagnant(TarotGame CurrentGame, struct _Jeu *pJeu);
int DefPerdante(TarotGame CurrentGame, struct _Jeu *pJeu);
int DefGagnant(TarotGame CurrentGame, struct _Jeu *pJeu);
int DefGeneral(TarotGame CurrentGame, struct _Jeu *pJeu, double ProbGain, int MinPliP);

#endif // TAROT_GAME_H_INCLUDED
