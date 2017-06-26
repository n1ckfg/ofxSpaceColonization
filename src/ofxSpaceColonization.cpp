#include "ofxSpaceColonization.h"

static const ofxSpaceColonizationOptions defaultSpaceColOptions = {
    150,                             // max_dist
    10,                              // min_dist
    150,                             // trunk_length
    glm::vec4(0.0f,0.0f,0.0f, 1.0f), // rootPosition
    glm::vec3(0.0f, 1.0f, 0.0f),     // rootDirection
    false,                           // use2d
    7,                               // branchLength
    false,                           // done growing (is it still used? check)
    false,                           // cap
    2.0,                             // radius;
    16,                              // resolution;
    1,                               // textureRepeat;
    0.9997                           // radiusScale;
};

ofxSpaceColonization::ofxSpaceColonization(){
    setup(defaultSpaceColOptions);
}

ofxSpaceColonization::ofxSpaceColonization(ofxSpaceColonizationOptions _opt){
    setup(_opt);
}

void ofxSpaceColonization::setup(ofxSpaceColonizationOptions _opt){
    //this->mesh = getMesh();
    this->options = _opt;
    this->current_radius = _opt.radius;
};

void ofxSpaceColonization::build(){
    if (options.use2d) {
        //TODO set this variable in the opt struct of the 2D example and remove this condition
        options.rootPosition = glm::vec4(ofGetWidth()/2, ofGetHeight(), 0, 1.0);
        options.rootDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (leaves_positions.empty()) {
        leaves_positions =
            ofxSpaceColonizationHelper::genRandomLeavesPositions(ofGetWidth(), ofGetHeight(), 400, options.use2d, options.trunk_length);
    }




    glm::vec4 endPoint = glm::vec4(0.0f,1.0f,0.0f, 1.0);
    glm::quat orientation;
    shared_ptr<ofxSpaceColonizationBranch> root(new ofxSpaceColonizationBranch(options.rootPosition, endPoint, orientation, glm::vec3(0.0f, 1.0f, 0.0f)));
    branches.push_back(root);


    for (auto vec:leaves_positions) {
        leaves.push_back(ofxSpaceColonizationLeaf(vec));
    }

    auto current = root;
    bool found = false;
    // we build the trunk,adding a branch after another, until we do not reach the foliage

    while (!found) {
        glm::vec3 cur = glm::vec3(current->getEndPos());
        for (auto l:leaves) {
            float distance = glm::distance(cur, l.getPosition());
            if (distance < options.max_dist) {
                found = true;
            }
        }

        float radius_top;
        if (!found && !branches.empty()) {
            glm::vec3 parentDir = current->getEndDirection();
            glm::vec3 parentPos = glm::vec3(branches.back()->getEndPos());
            glm::quat parentOrientation = branches.back()->getEndOrientation();
            glm::vec3 newDir = parentDir;
            glm::vec3 newPos = parentPos + (newDir * options.branchLength);

            shared_ptr<ofxSpaceColonizationBranch> nextBranch(
                new ofxSpaceColonizationBranch(glm::vec4(parentPos, 1.0), glm::vec4(newPos, 1.0), parentOrientation, parentDir));
            int lastInsertedBranchId = branches.size() -1;
            nextBranch->setParentByIndex(lastInsertedBranchId);
            branches.push_back(nextBranch);
            cout << nextBranch << endl;
            current = branches.back();
            radius_top = this->current_radius * options.radiusScale;
            auto opt = ofxBranchCylinderOptions({
                options.cap,
                this->current_radius,
                radius_top,
                options.resolution,
                options.textureRepeat });
            addBranchToMesh(nextBranch,opt);
            this->current_radius = radius_top;
        }
    }
}

void ofxSpaceColonization::grow(){
    float record = -1;
    if (!options.doneGrowing) {
        //If no leaves left, we are done
        if (leaves.size() == 0) {
            options.doneGrowing = true;
            return;
        }
        //process leaves
        for (int it=0;it<leaves.size();it++) {
            //float record = 10000.0;
            //Find the nearest branch for this leaf
            auto closestBranchIndex = -1;

            for (int i=0;i<branches.size();i++) {
                auto distance = glm::distance(leaves[it].getPosition(),
                                              glm::vec3(branches[i]->getEndPos()));
                if (distance < options.min_dist) {
                    leaves[it].setReached(true);
                    closestBranchIndex = -1;
                    break;
                } else if (distance > options.max_dist){
                    //break;
                } else if ((closestBranchIndex < 0) || (distance < record)){
                    closestBranchIndex = i;
                    record = distance;
                }
            }

            //adjust direction and count
            if (closestBranchIndex>=0 && !leaves[it].isReached()) {
                auto dir = leaves[it].getPosition() - (glm::vec3(branches[closestBranchIndex]->getEndPos()));
                auto dirNorm = glm::normalize(dir);
                // here you should add some random force to avoid the situation
                // where a branch is stucked between the attraction of 2 leaves
                // equidistant
                branches[closestBranchIndex]->correctNextBranchDirection(dirNorm);
                branches[closestBranchIndex]->incrementCounterBy(1);
            }

            if (leaves[it].isReached()) {
                // TODO, maybe you can keep the leaves and draw just that one that get reached?
                leaves.erase(leaves.begin()+it);
            }
        }

        //Generate the new branches
        vector<shared_ptr<ofxSpaceColonizationBranch>> newBranches;
        float radius_top;
        for (int i = 0; i<branches.size(); i++) {
            if (branches[i] != nullptr) {
                if (branches[i]->getCount() > 0) {
                    glm::vec3 parentDir = branches[i]->getEndDirection();
                    glm::vec3 parentPos = glm::vec3(branches[i]->getEndPos());
                    glm::quat parentOrientation = branches[i]->getEndOrientation();
                    glm::vec3 nextBranchDir = branches[i]->getNextBranchDirectionDirection();
                    glm::vec3 newDir = glm::normalize(nextBranchDir / (float(branches[i]->getCount() + 1)));
                    glm::vec3 newPos = parentPos + (newDir * options.branchLength);

                    shared_ptr<ofxSpaceColonizationBranch> nextBranch(
                                                                      new ofxSpaceColonizationBranch(glm::vec4(parentPos, 1.0), glm::vec4(newPos, 1.0), parentOrientation, parentDir));
                    nextBranch->setParentByIndex(i);
                    radius_top = this->current_radius * options.radiusScale;
                    auto opt = ofxBranchCylinderOptions({
                        options.cap,
                        this->current_radius,
                        radius_top,
                        options.resolution,
                        options.textureRepeat });
                    addBranchToMesh(nextBranch,opt);
                    newBranches.push_back(nextBranch);
                    this->current_radius = radius_top;
                }
                branches[i]->reset();
            }
        }
        branches.insert(branches.end(), newBranches.begin(), newBranches.end());
    }

}

void ofxSpaceColonization::addBranchToMesh(shared_ptr<ofxSpaceColonizationBranch> branch, ofxBranchCylinderOptions opt){
    ofxBranchCylinder::putIntoMesh(branch, this->getMesh(), opt);
}

vector<ofxSpaceColonizationLeaf> ofxSpaceColonization::getLeaves() const{
    return this->leaves;
}

int ofxSpaceColonization::getSizeBranches() const{
    return this->branches.size();
}

void ofxSpaceColonization::clear(){
    leaves.clear();
    branches.clear();
    leaves_positions.clear();
    clearMesh();
    branches.clear();
};

void ofxSpaceColonization::clearMesh(){
    current_radius = options.radius;
    getMesh().clear();
};

