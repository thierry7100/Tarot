#ifndef CAIRO_UTIL_H_INCLUDED
#define CAIRO_UTIL_H_INCLUDED

enum _Position_Bec {
    BEC_DESSOUS = 0,
    BEC_DROITE = 1,
    BEC_DESSUS = 2,
    BEC_GAUCHE = 3
};

void CairoRoundedRectangle(cairo_t *cr, double x, double y, double Sx, double Sy, double RoundFactor, int Filled);
void CairoBulle(cairo_t *cr, double x, double y, double Sx, double Sy, double RoundFactor, double SizeBec, int Filled, int PositionBec);
#endif // CAIRO_UTIL_H_INCLUDED
