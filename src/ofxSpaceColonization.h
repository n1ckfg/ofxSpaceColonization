#pragma once

#include "ofMain.h"
#include "ofxSpaceColonizationBranch.h"
#include "ofxSpaceColonizationLeaf.h"
#include "ofxSpaceColonizationHelper.h"
#include "ofxBranchCylinder.h"

struct ofxSpaceColonizationOptions{
    int max_dist; // in the paper "radius of influence"
    int min_dist; // in the paper "kill distance"
    int trunk_length;
    glm::vec4 rootPosition;
    glm::vec3 rootDirection;
    bool use2d;
    int branchLength;
    bool doneGrowing;
    bool cap;
    float radius;
    int resolution;
    int textureRepeat;
    float radiusScale;
};

class ofxSpaceColonization : public of3dPrimitive {
public:
	ofxSpaceColonization();
    ofxSpaceColonization(ofxSpaceColonizationOptions opt);

    void build();
    void grow();
    void grow(glm::vec3 wind);
    void clear();
    vector<ofxSpaceColonizationLeaf> getLeaves() const;

    //remove all the setters once that the transition to opt is done
    void setLeavesPositions(vector<glm::vec3> leaves_positions);
    void setup(ofxSpaceColonizationOptions opt);
    ofxSpaceColonizationOptions options;

private:
    void makeSureThatThereAreLeaves();
    vector<glm::vec3> leaves_positions;
    void addBranchToMesh(shared_ptr<ofxSpaceColonizationBranch> branch,ofxBranchCylinderOptions opt);
    vector<ofxSpaceColonizationLeaf> leaves;
    vector<shared_ptr<ofxSpaceColonizationBranch>> branches;

};
