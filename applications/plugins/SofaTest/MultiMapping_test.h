/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program; if not, write to the Free Software Foundation, Inc., 51  *
* Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.                   *
*******************************************************************************
*                            SOFA :: Applications                             *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/

/* Francois Faure, 2013 */
#ifndef SOFA_STANDARDTEST_MultiMapping_test_H
#define SOFA_STANDARDTEST_MultiMapping_test_H

#include "Sofa_test.h"
#include <sofa/component/init.h>
#include <sofa/core/MechanicalParams.h>
#include <sofa/simulation/common/VectorOperations.h>
#include <sofa/component/linearsolver/FullVector.h>
#include <sofa/component/linearsolver/EigenSparseMatrix.h>
#include <sofa/component/container/MechanicalObject.h>
#include <sofa/simulation/graph/DAGSimulation.h>
#include <plugins/SceneCreator/SceneCreator.h>
#include <sofa/helper/vector.h>


namespace sofa {

using std::cout;
using std::endl;
using helper::vector;
typedef std::size_t Index;


/** @brief Base class for the MultiMapping tests, directly adapted from Mapping_test.
 * @sa Mapping_test

  @author François Faure @date 2014
  */

template< class _MultiMapping>
struct MultiMapping_test : public Sofa_test<typename _MultiMapping::Real>
{
    typedef _MultiMapping Mapping;
    typedef typename Mapping::In In;
    typedef component::container::MechanicalObject<In> InDOFs;
    typedef typename InDOFs::Real  Real;
    typedef typename InDOFs::Deriv  InDeriv;
    typedef typename InDOFs::VecCoord  InVecCoord;
    typedef typename InDOFs::VecDeriv  InVecDeriv;
    typedef typename InDOFs::ReadVecCoord  ReadInVecCoord;
    typedef typename InDOFs::WriteVecCoord WriteInVecCoord;
    typedef typename InDOFs::ReadVecDeriv  ReadInVecDeriv;
    typedef typename InDOFs::WriteVecDeriv WriteInVecDeriv;
    typedef Data<InVecCoord> InDataVecCoord;
    typedef Data<InVecDeriv> InDataVecDeriv;

    typedef typename Mapping::Out Out;
    typedef component::container::MechanicalObject<Out> OutDOFs;
    typedef typename OutDOFs::Coord     OutCoord;
    typedef typename OutDOFs::Deriv     OutDeriv;
    typedef typename OutDOFs::VecCoord  OutVecCoord;
    typedef typename OutDOFs::VecDeriv  OutVecDeriv;
    typedef typename OutDOFs::ReadVecCoord  ReadOutVecCoord;
    typedef typename OutDOFs::WriteVecCoord WriteOutVecCoord;
    typedef typename OutDOFs::ReadVecDeriv  ReadOutVecDeriv;
    typedef typename OutDOFs::WriteVecDeriv WriteOutVecDeriv;
    typedef Data<OutVecCoord> OutDataVecCoord;
    typedef Data<OutVecDeriv> OutDataVecDeriv;

    typedef component::linearsolver::EigenSparseMatrix<In,Out> EigenSparseMatrix;


    core::MultiMapping<In,Out>* mapping; ///< the mapping to be tested
    vector<InDOFs*>  inDofs;  ///< mapping input
    OutDOFs* outDofs; ///< mapping output
    simulation::Node* root;         ///< Root of the scene graph, created by the constructor an re-used in the tests
    simulation::Simulation* simulation;  ///< created by the constructor an re-used in the tests
    Real deltaMax; ///< The maximum magnitude of the change of each scalar value of the small displacement is perturbation * numeric_limits<Real>::epsilon. This epsilon is 1.19209e-07 for float and 2.22045e-16 for double.
    Real errorMax;     ///< The test is successfull if the (infinite norm of the) difference is less than  maxError * numeric_limits<Real>::epsilon


    MultiMapping_test():deltaMax(1000),errorMax(10)
    {
        sofa::component::init();
        sofa::simulation::setSimulation(simulation = new sofa::simulation::graph::DAGSimulation());

    }

    /** Create scene with given number of parent states. Currently, only one child state is handled.
     * All the parents are set as child of the root node, while the child is in a child node.
    */
    void setupScene(int numParents)
    {
        root = simulation->createNewGraph("root").get();

        /// Child node
        simulation::Node::SPtr childNode = root->createChild("childNode");
        outDofs = modeling::addNew<OutDOFs>(childNode).get();
        mapping = modeling::addNew<Mapping>(childNode).get();
        mapping->addOutputModel(outDofs);

        /// Parent states, all added to the root node. This is not a simulable scene.
        for( int i=0; i<numParents; i++ )
        {
            typename InDOFs::SPtr inDof = modeling::addNew<InDOFs>(root);
            mapping->addInputModel( inDof.get() );
            inDofs.push_back(inDof.get());
        }

    }

    //    MultiMapping_test(std::string fileName):deltaMax(1000),errorMax(100)
    //    {
    //        sofa::component::init();
    //        sofa::simulation::setSimulation(simulation = new sofa::simulation::graph::DAGSimulation());

    //        /// Load the scene
    //        root = simulation->createNewGraph("root");
    //        root = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>( sofa::simulation::getSimulation()->load(fileName.c_str()));

    //        // InDofs
    //         inDofs = root->get<InDOFs>(root->SearchDown);

    //         // Get child nodes
    //         simulation::Node::SPtr patchNode = root->getChild("Patch");
    //         simulation::Node::SPtr elasticityNode = patchNode->getChild("Elasticity");

    //         // Add OutDofs
    //         outDofs = modeling::addNew<OutDOFs>(elasticityNode);

    //         // Add mapping to the scene
    //         mapping = modeling::addNew<Mapping>(elasticityNode).get();
    //         mapping->setModels(inDofs.get(),outDofs.get());

    //    }


    /** Returns OutCoord substraction a-b (should return a OutDeriv, but???)
      */
    virtual OutDeriv difference( const OutCoord& a, const OutCoord& b )
    {
        return Out::coordDifference(a,b);
    }

    //    /** Possible child force pre-treatment, does nothing by default
    //      */
    //    virtual OutVecDeriv preTreatment( const OutVecDeriv& f ) { return f; }


    /** Test the mapping using the given values and small changes.
     * Return true in case of success, if all errors are below maxError*epsilon.
     * The mapping is initialized using the two first parameters,
     * then a new parent position is applied,
     * and the new child position is compared with the expected one.
     * Additionally, the Jacobian-related methods are tested using finite differences.
     *
     * The initialization values can used when the mapping is an embedding, e.g. to attach a mesh to a rigid object we compute the local coordinates of the vertices based on their world coordinates and the frame coordinates.
     * In other cases, such as mapping from pairs of points to distances, no initialization values are necessary, an one can use the same values as for testing, i.e. runTest( xp, expected_xc, xp, expected_xc).
     *
     *\param parentInit initial parent position
     *\param childInit initial child position
     *\param parentNew new parent position
     *\param expectedChildNew expected position of the child corresponding to the new parent position
     */
    bool runTest( const vector<InVecCoord>& parentNew,
                  const OutVecCoord& expectedChildNew)
    {
        typedef component::linearsolver::EigenSparseMatrix<In,Out> EigenSparseMatrix;
        core::MechanicalParams mparams;
        mparams.setKFactor(1.0);
        mparams.setSymmetricMatrix(false);

        // transfer the parent values in the parent states
        for( int i=0; i<parentNew.size(); i++ )
        {
            this->inDofs[i]->resize(parentNew[i].size());
            WriteInVecCoord xin = inDofs[i]->writePositions();
            copyToData(xin,parentNew[i]); // xin = parentNew[i]
        }

        /// Init
        sofa::simulation::getSimulation()->init(root);

        /// apply the mapping
        mapping->apply(&mparams, core::VecCoordId::position(), core::VecCoordId::position());
        mapping->applyJ(&mparams, core::VecDerivId::velocity(), core::VecDerivId::velocity());

        /// test apply: check if the child positions are the expected ones
        bool succeed=true;
        ReadOutVecCoord xout = outDofs->readPositions();
        for( Index i=0; i<xout.size(); i++ )
        {
            if( !this->isSmall( difference(xout[i],expectedChildNew[i]).norm(), errorMax ) ) {
                ADD_FAILURE() << "Position of mapped particle " << i << " is wrong: \n" << xout[i] <<"\nexpected: \n" << expectedChildNew[i];
                succeed = false;
            }
        }


        /// test applyJ and everything related to Jacobians
        const Index Nc=outDofs->getSize();
        vector<Index> Np(inDofs.size());
        for(Index i=0; i<Np.size(); i++)
            Np[i] = inDofs[i]->getSize();

        vector<InVecCoord> xp(Np.size()),xp1(Np.size());
        vector<InVecDeriv> vp(Np.size()),fp(Np.size()),dfp(Np.size()),fp2(Np.size());
        OutVecCoord xc(Nc),xc1(Nc);
        OutVecDeriv vc(Nc),fc(Nc);

        // get position data
        for(Index i=0; i<Np.size(); i++)
            copyFromData( xp[i],inDofs[i]->readPositions() );
        copyFromData( xc,  outDofs->readPositions() ); // positions and have already been propagated
        //        cout<<"parent positions xp = "<< xp << endl;
        //        cout<<"child  positions xc = "<< xc << endl;

        // set random child forces and propagate them to the parent
        for( unsigned i=0; i<Nc; i++ ){
            fc[i] = Out::randomDeriv( 1.0 );
            cout<<"random child forces  fc[" << i <<"] = "<<fc[i]<<endl;
        }
        for(Index p=0; p<Np.size(); p++) {
            fp2[p]=InVecDeriv(Np[p], InDeriv() ); // null vector of appropriate size
            WriteInVecDeriv fin = inDofs[p]->writeForces();
            copyToData( fin, fp2[p] );  // reset parent forces before accumulating child forces
        }
        WriteOutVecDeriv fout = outDofs->writeForces();
        copyToData( fout, fc );
        mapping->applyJT( &mparams, core::VecDerivId::force(), core::VecDerivId::force() );
        for(Index i=0; i<Np.size(); i++) copyFromData( fp[i], inDofs[i]->readForces() );
        //        cout<<"parent forces fp = "<<fp<<endl;

        // set small parent velocities and use them to update the child
        for( Index p=0; p<Np.size(); p++ ){
            vp[p].resize(Np[p]);
            xp1[p].resize(Np[p]);
            for( unsigned i=0; i<Np[p]; i++ ){
                vp[p][i] = In::randomDeriv( this->epsilon() * deltaMax );
                cout<<"parent velocities vp[" << p <<"] = " << vp[p] << endl;
                xp1[p][i] = xp[p][i] + vp[p][i];
                cout<<"new parent positions xp1["<< p <<"] = " << xp1[p] << endl;
            }
        }

        // propagate small velocity
        for( Index p=0; p<Np.size(); p++ ){
            WriteInVecDeriv vin = inDofs[p]->writeVelocities();
            copyToData( vin, vp[p] );
        }
        mapping->applyJ( &mparams, core::VecDerivId::velocity(), core::VecDerivId::velocity() );
        WriteOutVecDeriv vout = outDofs->writeVelocities();
        copyFromData( vc, vout);
        //        cout<<"child velocity vc = " << vc << endl;


        // apply geometric stiffness
        for( Index p=0; p<Np.size(); p++ ) {
            WriteInVecDeriv dxin = inDofs[p]->writeDx();
            copyToData( dxin, vp[p] );
            dfp[p] = InVecDeriv(Np[p], InDeriv() );
            WriteInVecDeriv fin = inDofs[p]->writeForces();
            copyToData( fin, dfp[p] );
        }
        mapping->applyDJT( &mparams, core::VecDerivId::force(), core::VecDerivId::force() );
        for( Index p=0; p<Np.size(); p++ ){
            copyFromData( dfp[p], inDofs[p]->readForces() ); // fp + df due to geometric stiffness
            cout<<"dfp["<< p <<"] = " << dfp[p] << endl;
        }

        // Jacobian will be obsolete after applying new positions
        const vector<defaulttype::BaseMatrix*>* J = mapping->getJs();
        OutVecDeriv Jv(Nc);
        for( Index p=0; p<Np.size(); p++ ){
            cout<<"J["<< p <<"] = "<< endl << *(*J)[p] << endl;
            EigenSparseMatrix* JJ = dynamic_cast<EigenSparseMatrix*>((*J)[p]);
            assert(JJ!=NULL);
            JJ->addMult(Jv,vp[p]);
        }

        // ================ test applyJT()
        vector<InVecDeriv> jfc(Np.size());
        for( Index p=0; p<Np.size(); p++ ) {
            jfc[p] = InVecDeriv( Np[p],InDeriv());
            EigenSparseMatrix* JJ = dynamic_cast<EigenSparseMatrix*>((*J)[p]);
            JJ->addMultTranspose(jfc[p],fc);
            if( this->vectorMaxDiff(jfc[p],fp[p])>this->epsilon()*errorMax ){
                succeed = false;
                ADD_FAILURE() << "applyJT test failed"<<endl<<"jfc["<< p <<"] = " << jfc[p] << endl<<" fp["<< p <<"] = " << fp[p] << endl;
            }
        }
        // ================ test getJs()
        // check that J.vp = vc
        if( this->vectorMaxDiff(Jv,vc)>this->epsilon()*errorMax ){
            succeed = false;
            for( Index p=0; p<Np.size(); p++ ) {
                cout<<"J["<< p <<"] = "<< endl << *(*J)[p] << endl;
                cout<<"vp["<< p <<"] = " << vp[p] << endl;
            }
            cout<<"Jvp = " << Jv << endl;
            cout<<"vc  = " << vc << endl;
            ADD_FAILURE() << "getJs() test failed"<<endl<<"Jvp = " << Jv << endl <<"vc  = " << vc << endl;
        }


        // compute parent forces from pre-treated child forces (in most cases, the pre-treatment does nothing)
        // the pre-treatement can be useful to be able to compute 2 comparable results of applyJT with a small displacement to test applyDJT
        for( Index p=0; p<Np.size(); p++ ) {
            fp[p].fill( InDeriv() );
            WriteInVecDeriv fin = inDofs[p]->writeForces();
            copyToData( fin, fp[p] );  // reset parent forces before accumulating child forces
        }
        copyToData( fout, fc );
        mapping->applyJT( &mparams, core::VecDerivId::force(), core::VecDerivId::force() );
        for( Index p=0; p<Np.size(); p++ )
            copyFromData( fp[p], inDofs[p]->readForces() );



        ///////////////////////////////


        // propagate small displacement
        for( Index p=0; p<Np.size(); p++ ){
            WriteInVecCoord pin = inDofs[p]->writePositions();
            copyToData( pin, xp1[p] );
            cout<<"new parent positions xp1["<< p << "] = " << xp1[p] << endl;
        }
        mapping->apply ( &mparams, core::VecCoordId::position(), core::VecCoordId::position() );
        WriteOutVecCoord pout = outDofs->writePositions();
        copyFromData( xc1, pout );
        //        cout<<"new child positions xc1 = " << xc1 << endl;

        // ================ test applyJ: compute the difference between propagated displacements and velocities
        OutVecDeriv dxc(Nc);
        for(unsigned i=0; i<Nc; i++ ){
            dxc[i] = difference( xc1[i], xc[i] );
        }
        if( this->vectorMaxDiff(dxc,vc)>this->epsilon()*errorMax ){
            succeed = false;
            ADD_FAILURE() << "applyJ test failed: the difference between child position change and child velocity (dt=1) should be less than  " << this->epsilon()*errorMax  << endl
                          << "position change = " << dxc << endl
                          << "velocity        = " << vc << endl;
        }



        // update parent force based on the same child forces
        for( Index p=0; p<Np.size(); p++ ){
            fp2[p].fill( InDeriv() );
            WriteInVecDeriv fin = inDofs[p]->writeForces();
            copyToData( fin, fp2[p] );  // reset parent forces before accumulating child forces
        }
        copyToData( fout, fc );
        mapping->applyJT( &mparams, core::VecDerivId::force(), core::VecDerivId::force() );
        vector<InVecDeriv> fp12(Np.size());
        for( Index p=0; p<Np.size(); p++ ){
            copyFromData( fp2[p], inDofs[p]->readForces() );
            cout<<"updated parent forces fp2["<< p <<"] = "<< fp2[p] << endl;
            fp12[p].resize(Np[p]);
            for(unsigned i=0; i<Np[p]; i++){
                fp12[p][i] = fp2[p][i]-fp[p][i];       // fp2 - fp
            }
            cout<<"fp2["<< p <<"] - fp["<< p <<"] = " << fp12[p] << endl;
            // ================ test applyDJT()
            if( this->vectorMaxDiff(dfp[p],fp12[p])>this->epsilon()*errorMax ){
                succeed = false;
                ADD_FAILURE() << "applyDJT test failed" << endl <<
                                 "dfp["<<p<<"]    = " << dfp[p] << endl <<
                                 "fp2["<<p<<"]-fp["<<p<<"] = " << fp12[p] << endl;
            }
        }




        return succeed;
    }

    virtual ~MultiMapping_test()
    {
        if (root!=NULL)
            sofa::simulation::getSimulation()->unload(root);
    }

    //    /// Get one EigenSparseMatrix out of a list. Error if not one single matrix in the list.
    //    static EigenSparseMatrix* getMatrix(const vector<sofa::defaulttype::BaseMatrix*>* matrices)
    //    {
    //        if( matrices->size() != 1 ){
    //            ADD_FAILURE()<< "Matrix list should have size == 1 in simple mappings";
    //        }
    //        EigenSparseMatrix* ei = dynamic_cast<EigenSparseMatrix*>((*matrices)[0] );
    //        if( ei == NULL ){
    //            ADD_FAILURE() << "getJs returns a matrix of non-EigenSparseMatrix type";
    //        }
    //        return ei;
    //    }



};

} // namespace sofa


#endif