#ifndef FSBINTERPOLATORPLATE_H
#define FSBINTERPOLATORPLATE_H

#include "fei2dquadlin.h"

namespace oofem {
/**
 *
 */
class OOFEM_EXPORT FsbInterpolatorPlate : public FEI2dQuadLin
{
public:
    FsbInterpolatorPlate(int ind1, int ind2):
        FEI2dQuadLin(ind1, ind2)
    {}

    virtual void evalN(FloatArray &answer, const FloatArray &lcoords, const FEICellGeometry &cellgeo);
    virtual double evaldNdx(FloatMatrix &answer, const FloatArray &lcoords, const FEICellGeometry &cellgeo);
    void giveDerivatives(FloatMatrix &dn, const FloatArray &lc);
    virtual Element_Geometry_Type giveGeometryType() const { return EGT_quad_1; }
};

} // end namespace oofem

#endif // FSBINTERPOLATORPLATE_H
