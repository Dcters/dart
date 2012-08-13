#ifndef _MYWINDOW_
#define _MYWINDOW_

#include <stdarg.h>
#include "yui/Win3D.h"
#include "integration/EulerIntegrator.h"
#include "integration/RK4Integrator.h"
#include "collision/collision_skeleton.h"
#include "dynamics/SkeletonDynamics.h"
#include "dynamics/JointLimitDynamics.h"

using namespace std;

namespace dynamics{
    class SkeletonDynamics;
    class ContactDynamics;
}

namespace integration{
    class IntegrableSystem;
}

class MyWindow : public yui::Win3D, public integration::IntegrableSystem {
public:
 MyWindow(dynamics::SkeletonDynamics* _mList = 0, ...): Win3D() {
        mBackground[0] = 0.8;
        mBackground[1] = 0.2;
        mBackground[2] = 0.2;
        mBackground[3] = 1.0;
		
        mSim = false;
        mPlay = false;
        mSimFrame = 0;
        mPlayFrame = 0;
        mShowMarkers = true;

        mPersp = 30.f;
        mTrans[1] = 200.f;
        mTrans[0] = -200.f;
    
        mGravity = Eigen::Vector3d(0.0, -9.8, 0.0);
        mTimeStep = 1.0/1000.0;
        mForce = Eigen::Vector3d::Zero();
        mImpulseDuration = 0;
        mSelectedNode = 1;

        if (_mList) {
            mSkels.push_back(_mList);
            va_list ap;
            va_start(ap, _mList);
            while (true) {
                dynamics::SkeletonDynamics *skel = va_arg(ap, dynamics::SkeletonDynamics*);
                if(skel)
                    mSkels.push_back(skel);
                else
                    break;
            }
            va_end(ap);
        }
        
        int sumNDofs = 0;
        mIndices.push_back(sumNDofs);
        for (unsigned int i = 0; i < mSkels.size(); i++) {
            int nDofs = mSkels[i]->getNumDofs();
            sumNDofs += nDofs;
            mIndices.push_back(sumNDofs);
        }
        initDyn();
    }

    virtual void draw();
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void displayTimer(int _val);

    // Needed for integration
    virtual Eigen::VectorXd getState();
    virtual Eigen::VectorXd evalDeriv();
    virtual void setState(Eigen::VectorXd state);	
 protected:	

    // TO TEST DOFS
    double dDOF;
    ////////////////

    // Simulation parameters
    int mSimFrame;
    bool mSim;
    int mPlayFrame;
    bool mPlay;
    bool mShowMarkers;
    int mSelectedNode;
    integration::EulerIntegrator mIntegrator;
    std::vector<Eigen::VectorXd> mBakedStates;

    // Dynamic information
    std::vector<dynamics::SkeletonDynamics*> mSkels;
     dynamics::JointLimitDynamics *mJointLimitConstr;
    dynamics::ContactDynamics *mCollisionHandle;
    std::vector<Eigen::VectorXd> mDofVels;
    std::vector<Eigen::VectorXd> mDofs;

    Eigen::Vector3d mGravity;
    Eigen::Vector3d mForce;
    double mTimeStep;
    std::vector<int> mIndices;
    int mImpulseDuration;

    void initDyn();
    void setPose();
    void bake();
};

#endif
