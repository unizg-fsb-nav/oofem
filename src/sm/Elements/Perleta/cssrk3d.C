/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2013   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "../sm/Elements/Perleta/cssrk3d.h"
#include "../sm/Materials/structuralms.h"
#include "Materials/structuralmaterial.h"
#include "node.h"
#include "material.h"
#include "crosssection.h"
#include "gausspoint.h"
#include "gaussintegrationrule.h"
#include "floatmatrix.h"
#include "intarray.h"
#include "floatarray.h"
#include "engngm.h"
#include "boundaryload.h"
#include "mathfem.h"
#include "fei2dquadlin.h"
#include "classfactory.h"
#include "elementinternaldofman.h"
#include "masterdof.h"
#include "../sm/CrossSections/structuralcrosssection.h"



#ifdef __OOFEG
 #include "oofeggraphiccontext.h"
#endif

namespace oofem {
REGISTER_Element(CSSRK3d);

FEI2dQuadLin CSSRK3d :: interp(1, 2);

CSSRK3d :: CSSRK3d(int n, Domain *aDomain) : StructuralElement(n, aDomain),

    lnodes(4)
{
    numberOfDofMans = 4;

    numberOfGaussPoints = 4;
    E=0;
    t=0;
    ni=0;
    a=0;
    b=0;
    K=0;
    referenceNode=0;
}

CSSRK3d :: ~CSSRK3d()
{ }

FEInterpolation *CSSRK3d :: giveInterpolation() const { return & interp; }

void
CSSRK3d :: computeBmatrixAt(GaussPoint *gp, FloatMatrix &answer, int li, int ui)
//
// Returns the [3x8] strain-displacement matrix {B} of the receiver,
// evaluated at gp.
// (epsilon_x,epsilon_y,gamma_xy) = B . r
// r = ( u1,v1,u2,v2,u3,v3,u4,v4)
{

    FloatArray Gcoords;
    double x,y;

    Gcoords   = gp->giveGlobalCoordinates();
    x   = Gcoords(0);
    y   = Gcoords(1);

    answer.resize(3, 12);
    answer.zero();

    answer.at(1, 1) =  (y-b)/(a*b) ;
    answer.at(1, 2) =  -ni*(2*x-a)/(2*a*b);
    answer.at(1, 3) = 0;
    answer.at(1, 4) =  -(y-b)/(a*b) ;
    answer.at(1, 5) =  ni*(2*x-a)/(2*a*b);
    answer.at(1, 6) = 0;
    answer.at(1, 7) =  y/(a*b);
    answer.at(1, 8) = -ni*(2*x-a)/(2*a*b);
    answer.at(1, 9) = 0;
    answer.at(1, 10) =  -( y / (a*b));
    answer.at(1, 11) =  ni*(2*x-a)/(2*a*b);
    answer.at(1, 12) = 0;


    answer.at(2, 1) =  -ni*(2*y-b)/(2*a*b);
    answer.at(2, 2) =  (x-a)/(a*b) ;
    answer.at(2, 3) = 0;
    answer.at(2, 4) =  ni*(2*y-b)/(2*a*b);
    answer.at(2, 5) =  -( x / (a*b));
    answer.at(2, 6) = 0;
    answer.at(2, 7) =  -ni*(2*y-b)/(2*a*b);;
    answer.at(2, 8) =   ( x / (a*b));
    answer.at(2, 9) = 0;
    answer.at(2, 10) =  ni*(2*y-b)/(2*a*b);
    answer.at(2, 11) =   -(x-a)/(a*b);
    answer.at(2, 12) = 0;


    answer.at(3, 1) =  -1/(2*b) ;
    answer.at(3, 2) =  -1/(2*a);
    answer.at(3, 3) = 0;
    answer.at(3, 4) =  -1/(2*b) ;
    answer.at(3, 5) =   1/(2*a);
    answer.at(3, 6) = 0;
    answer.at(3, 7) =   1/(2*b);
    answer.at(3, 8) =   1/(2*a) ;
    answer.at(3, 9) = 0;
    answer.at(3, 10) =   1/(2*b) ;
    answer.at(3, 11) =  -1/(2*a);
    answer.at(3, 12) = 0;
}


void CSSRK3d :: computeGaussPoints()
// Sets up the array of Gauss Points of the receiver.
{
    if ( integrationRulesArray.size() == 0 ) {
        // the gauss point is used only when methods from crosssection and/or material
        // classes are requested
        integrationRulesArray.resize( 1 );
        integrationRulesArray [ 0 ].reset( new GaussIntegrationRule(1, this, 1, 2) );
        this->giveCrossSection()->setupIntegrationPoints(* integrationRulesArray [ 0 ], this->numberOfGaussPoints, this);
        this->crossSectioncharacteristics();
        this->computeLength();
    }
}


void
CSSRK3d :: computeNmatrixAt(const FloatArray &iLocCoord, FloatMatrix &answer)
// Returns the displacement interpolation matrix {N} of the receiver, eva-
// luated at gp. Used for numerical calculation of consistent mass
// matrix. Must contain only interpolation for displacement terms,
// not for any rotations. (Inertia forces do not work on rotations).
// r = {u1,v1,u2,v2,u3,v3,u4,v4}^T
{
    FloatArray Gcoords;
    GaussPoint *gp = integrationRulesArray[0]->getIntegrationPoint(0);
    double x,y,x2,y2;

    Gcoords   = gp->giveGlobalCoordinates();

    x   = lnodes[0][0];
    y   = lnodes[0][1];
    x2  =x*x;
    y2  =y*y;

    answer.resize(2, 8);
    answer.zero();

    answer.at(1, 1) = ( (x-a)*(y-b))/(a*b);
    answer.at(1, 2) = -(y2-b*y+ni*x2-a*ni*x)/(2*a*b);
    answer.at(1, 3) = -(x*(y-b))/(a*b);
    answer.at(1, 4) = (y2-b*y+ni*x2-a*ni*x)/(2*a*b);
    answer.at(1, 5) =(x*y)/(a*b);
    answer.at(1, 6) = -(y2-b*y+ni*x2-a*ni*x)/(2*a*b);
    answer.at(1, 7) = -((x-a)*y)/(a*b);
    answer.at(1, 8) = (y2-b*y+ni*x2-a*ni*x)/(2*a*b);

    answer.at(2, 1) = -(ni*y2-b*ni*y+x2-a*x)/(2*a*b);
    answer.at(2, 2) =( (x-a)*(y-b))/(a*b);
    answer.at(2, 3) = (ni*y2-b*ni*y+x2-a*x)/(2*a*b);
    answer.at(2, 4) = -(x*(y-b))/(a*b);
    answer.at(2, 5) = -(ni*y2-b*ni*y+x2-a*x)/(2*a*b);
    answer.at(2, 6) =(x*y)/(a*b);
    answer.at(2, 7) = (ni*y2-b*ni*y+x2-a*x)/(2*a*b);
    answer.at(2, 8) = -((x-a)*y)/(a*b);
}

void
CSSRK3d :: crossSectioncharacteristics()
        //Computes E,ni,t
{

            if( E == 0. ) {
                GaussPoint *gp;
                gp= integrationRulesArray[0]->getIntegrationPoint(0);
                StructuralCrossSection * csec=this->giveStructuralCrossSection();
                StructuralMaterial *mat = dynamic_cast< StructuralMaterial * >(csec->giveMaterial(gp));
                t = csec->give(CS_Thickness, gp);
                E=mat->give('E', gp);
                ni=mat->give('n', gp);

            }
           ;
}

void
CSSRK3d :: computeLocalStiffnessMatrix(FloatMatrix &answer, MatResponseMode rMode, TimeStep *tStep)
// Returns the stiffness matrix of the receiver, expressed in the global
// axes. No integration over volume done, beam with constant material and crosssection
// parameters assumed.
{
    // compute clamped stifness
    this->computeClampedStiffnessMatrix(answer, rMode, tStep);
}


void
CSSRK3d :: computeStiffnessMatrix(FloatMatrix &answer, MatResponseMode rMode, TimeStep *tStep)
// Returns the stiffness matrix of the receiver, expressed in the global
// axes. No integration over volume done, beam with constant material and crosssection
// parameters assumed.
{
    // compute clamped stiffness
    this->computeLocalStiffnessMatrix(answer, rMode, tStep);
}


void

CSSRK3d :: computeClampedStiffnessMatrix(FloatMatrix &answer,

                                        MatResponseMode rMode, TimeStep *tStep)

{
// CSSRK3D stiffness matrix

     double a2,b2,ni2,tE8ni,tE24ni,tE8ni2;

     a2=a*a;

     b2=b*b;

     ni2=ni*ni;

                tE8ni2=t*E/(8*ni*ni-8);

                tE8ni=t*E/(8*ni-8);

                tE24ni=t*E/(24*a*b*(ni*ni-1));

     answer.resize(12, 12);

     answer.zero();

     answer.at(1,1)= (2*b2*ni2+3*a2*ni-8*b2-3*a2)* tE24ni;

     answer.at(1,2)= - tE8ni;

     answer.at(1,4)= -(2*b2*ni2-3*a2*ni-8*b2+3*a2)* tE24ni;

     answer.at(1,5)= -(3*ni-1)* tE8ni2;

     answer.at(1,7)= (2*b2*ni2-3*a2*ni+4*b2+3*a2)* tE24ni;

     answer.at(1,8)=  tE8ni;

     answer.at(1,10)= -(2*b2*ni2+3*a2*ni+4*b2-3*a2)* tE24ni;

     answer.at(1,11)= -answer.at(1,5);       //-answer.at(1,4);



     answer.at(2,2)= (2*a2*ni2+3*b2*ni-3*b2-8*a2)* tE24ni;

     answer.at(2,4)= -answer.at(1,5);       //-answer.at(1,4)

     answer.at(2,5)= -(2*a2*ni2+3*b2*ni-3*b2+4*a2)* tE24ni;

     answer.at(2,7)=  tE8ni;

     answer.at(2,8)= (2*a2*ni2-3*b2*ni+3*b2+4*a2)* tE24ni;

     answer.at(2,10)= answer.at(1,5);        //answer.at(1,4)

     answer.at(2,11)= -(2*a2*ni2-3*b2*ni+3*b2-8*a2)* tE24ni;




     answer.at(4,4)= answer.at(1,1);

     answer.at(4,5)=  tE8ni;

     answer.at(4,7)= answer.at(1,10);        //answer.at(1,7)

     answer.at(4,8)= answer.at(1,5);        //answer.at(1,4)

     answer.at(4,10)= answer.at(1,7);       //answer.at(1,5)

     answer.at(4,11)= - tE8ni;



     answer.at(5,5)= answer.at(2,2);

     answer.at(5,7)= -answer.at(1,5);       //-answer.at(1,4)

     answer.at(5,8)= answer.at(2,11);        //answer.at(2,8)

     answer.at(5,10)= - tE8ni;

     answer.at(5,11)= answer.at(2,8);       //answer.at(2,6)




     answer.at(7,7)= answer.at(1,1);

     answer.at(7,8)= - tE8ni;

     answer.at(7,10)= answer.at(1,4);       //answer.at(1,3)

     answer.at(7,11)= answer.at(1,5);       //answer.at(1,4)



     answer.at(8,8)= answer.at(2,2);

     answer.at(8,10)= -answer.at(1,5);      //-answer.at(1,4)

     answer.at(8,11)= answer.at(2,5);       //answer.at(2,4)





     answer.at(10,10)= answer.at(1,1);

     answer.at(10,11)=  tE8ni;



     answer.at(11,11)= answer.at(2,2);



     double max = answer.at(1,1);

     for (int i=1;i<12;i++){
         if (answer.at(i,i)>max){
             max=answer.at(i,i);
         }
         K= max/100000000;
     }

     answer.at(3,3)= K;
     answer.at(6,6)= K;
     answer.at(9,9)= K;
     answer.at(12,12)= K;


    answer.symmetrized();
}



double
CSSRK3d :: computeLength()
// Returns the edge lengths of the receiver.
{
    this->computeLCS();
    double  x12 , y12, x14, y14, x23, y23, x34, y34 ;

    if(a+b==0){
        x12=lnodes[1][0]-lnodes[0][0];
        y12=lnodes[1][1]-lnodes[0][1];
        x14=lnodes[3][0]-lnodes[0][0];
        y14=lnodes[3][1]-lnodes[0][1];
        x23=lnodes[2][0]-lnodes[1][0];
        y23=lnodes[2][1]-lnodes[1][1];
        x34=lnodes[2][0]-lnodes[3][0];
        y34=lnodes[2][1]-lnodes[3][1];

        a = (sqrt(x12*x12+y12*y12)+sqrt(x34*x34+y34*y34))/2;
        b = (sqrt(x14*x14+y14*y14)+sqrt(x23*x23+y23*y23))/2;
    }
  return a;
}


void
CSSRK3d :: initializeFrom(InputRecord &ir)
{
    numberOfGaussPoints = 4;
    StructuralElement :: initializeFrom(ir);

    if ( numberOfGaussPoints != 1 && numberOfGaussPoints != 4 && numberOfGaussPoints != 9 && numberOfGaussPoints != 16 && numberOfGaussPoints != 25 ) {
        numberOfGaussPoints = 4;
        OOFEM_WARNING("Number of Gauss points enforced to 1");
    }
}



void
CSSRK3d :: computeLCS()
{
    lcsMatrix.resize(3, 3); // Note! G -> L transformation matrix
    FloatArray e1, e2, e3, help;

    // compute e1' = [N2-N1]  and  help = [N4-N1]

	// Pointer operators removed because the project could not build. Test later to check if the method still works properly.
    e1.beDifferenceOf( this->giveNode(2)->giveCoordinates(), this->giveNode(1)->giveCoordinates() );
    help.beDifferenceOf( this->giveNode(4)->giveCoordinates(), this->giveNode(1)->giveCoordinates() );

    e1.normalize();
    e3.beVectorProductOf(e1, help);
    e3.normalize();
    e2.beVectorProductOf(e3, e1);
    for ( int i = 1; i <= 3; i++ ) {
        this->lcsMatrix.at(1, i) = e1.at(i);
        this->lcsMatrix.at(2, i) = e2.at(i);
        this->lcsMatrix.at(3, i) = e3.at(i);
    }
    for ( int i = 1; i <= 4; i++ ) {
		// Pointer operator removed because the project could not build. Test later to check if the method still works properly.
        this->lnodes [ i - 1 ].beProductOf( this->lcsMatrix, this->giveNode(i)->giveCoordinates() );
    }

}


bool
CSSRK3d :: computeGtoLRotationMatrix(FloatMatrix &answer)
{
    this->computeLCS();
    answer.resize(12, 12);
    answer.zero();

    for ( int i = 0; i <= 3; i++ ) { // Loops over nodes
        // In each node, transform global c.s. {D_u, D_v,D_w} into local c.s.
        answer.setSubMatrix(this->lcsMatrix, 1 + i * 3, 1 + i * 3);     // Displacements
    }

    return true;
}

int
CSSRK3d :: computeLoadGToLRotationMtrx(FloatMatrix &answer)
// Returns the rotation matrix of the receiver of the size [5,6]
// f(local) = T * f(global)
{
    this->computeLCS();

    answer.resize(3, 3);
    answer.zero();

    for ( int i = 1; i <= 3; i++ ) {
        answer.at(1, i) = lcsMatrix.at(1, i);
        answer.at(2, i) = lcsMatrix.at(2, i);
        answer.at(3, i) = lcsMatrix.at(3, i);
    }

    return 1;
}

void
CSSRK3d :: giveDofManDofIDMask(int inode, IntArray &answer) const
{
    answer = {
        D_u, D_v,D_w
    };
}

void
CSSRK3d :: updateLocalNumbering(EntityRenumberingFunctor &f)
{
    StructuralElement :: updateLocalNumbering(f);
    if ( this->referenceNode ) {
        this->referenceNode = f(this->referenceNode, ERS_DofManager);
    }
}


} // end namespace oofem

