#include "ofxSpaceColonization.h"
// http://www.jgallant.com/procedurally-generating-trees-with-space-colonization-algorithm-in-xna/
ofxSpaceColonization::ofxSpaceColonization(){
}

void ofxSpaceColonization::build(){
    if(!use3d){
        root_position = glm::vec3(ofGetWidth()/2, ofGetHeight(), 0);
        root_direction = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (leaves_positions.empty()) {
        leaves_positions = ofxSpaceColonizationHelper::genRandomLeavesPositions(ofGetWidth(), ofGetHeight(), 400, use3d, trunk_length);
    }
    shared_ptr<ofxSpaceColonizationBranch> root(new ofxSpaceColonizationBranch(root_direction));
    root->move(glm::vec3(0.0f,0.0f,0.0f), root_position);
    branches.push_back(root);

    for (auto vec:leaves_positions) {
        leaves.push_back(ofxSpaceColonizationLeaf(vec));
    }

    auto current = root;
    bool found = false;
    while (!found) {
        glm::vec3 cur = current->getPosition();
        for (auto l:leaves) {
            float distance = glm::distance(cur, l.getPosition());
            if (distance < max_dist) {
                found = true;
            }
        }

        if (!found) {
            shared_ptr<ofxSpaceColonizationBranch> nextBranch(new ofxSpaceColonizationBranch(current->getDirection()));
            if (!branches.empty()) {
                int lastInsertedBranchId = branches.size() -1;
                nextBranch->setParentByIndex(lastInsertedBranchId);
                nextBranch->move((current->getDirection() * branch_length ),
                                 branches.back()->getPosition());
            }
            addBranchToMesh(nextBranch);
            branches.push_back(nextBranch);
            current = branches.back();
        }
    }
}

void ofxSpaceColonization::grow(){
    if (!done_growing) {
        cout << branches.size() << endl;
        //process leaves
        for (int it=0;it<leaves.size();it++) {
            float record = 10000.0;

            auto closestBranchIndex = -1;
            for (int i=0;i<branches.size();i++) {
                auto distance = glm::distance(leaves[it].getPosition(), branches[i]->getPosition());
                auto vPos = branches[i]->getPosition();
                if (distance < min_dist) {
                    leaves[it].setReached(true);
                    closestBranchIndex = -1;
                    break;
                } else if (distance > max_dist){
                    //break;
                } else if ((closestBranchIndex < 0) || (distance < record)){
                    closestBranchIndex = i;
                    record = distance;
                }
            }

            //adjust direction and count
            if (closestBranchIndex>=0 && !leaves[it].isReached()) {
                auto dir = leaves[it].getPosition() + (-branches[closestBranchIndex]->getPosition());
                auto dirNorm = glm::normalize(dir);

                // here you should add some random force to avoid the situation
                // where a branch is stucked between the attraction of 2 leaves
                // equidistant
                branches[closestBranchIndex]->setDirection(branches[closestBranchIndex]->getDirection() + dirNorm);
                branches[closestBranchIndex]->count = branches[closestBranchIndex]->count + 1;
            }

            if (leaves[it].isReached()) {
                // TODO, maybe you can keep the leaves and draw just that one that get reached?
                leaves.erase(leaves.begin()+it);
            }
        }

        //Generate the new branches
        vector<shared_ptr<ofxSpaceColonizationBranch>> newBranches;
        for (int i = 0; i<branches.size(); i++) {
            if (branches[i]!= nullptr) {
                if (branches[i]->count > 0) {
                    auto newDir = branches[i]->getDirection() / (float(branches[i]->count + 1));
                    shared_ptr<ofxSpaceColonizationBranch> nextBranch(new ofxSpaceColonizationBranch(newDir));

                    nextBranch->setParentByIndex(i);
                    nextBranch->move(
                                     (glm::normalize(newDir) * branch_length),
                                      branches[i]->getPosition());
                    addBranchToMesh(nextBranch);
                    newBranches.push_back(nextBranch);
                }
                branches[i]->reset();
            }
        }
        branches.insert(branches.end(), newBranches.begin(), newBranches.end());
    }

}
void ofxSpaceColonization::setMinDist(int _min_dist){
    min_dist = _min_dist;
};

void ofxSpaceColonization::setMaxDist(int _max_dist){
    max_dist = _max_dist;
};

void ofxSpaceColonization::setBranchLength(int _length){
    branch_length = _length;
};

void ofxSpaceColonization::draw2d(){
    for (auto l:leaves) {
        l.draw2d();
    }

    for (int i = 0; i < branches.size(); i++) {
        float lineWidth = ofMap(i, 0, branches.size(), 20, 1);
        ofSetLineWidth(lineWidth);
        //branches[i]->draw();
        auto parentIndex = branches[i]->getIndexParent();
        auto parentPos = branches[parentIndex]->getPosition();
        auto pos = branches[i]->getPosition();

        ofDrawLine(parentPos.x, parentPos.y,
                   parentPos.z, pos.x,
                   pos.y, pos.z);
    }
}

void ofxSpaceColonization::addBranchToMesh(shared_ptr<ofxSpaceColonizationBranch> branch){
    //TODO
}

vector<ofxSpaceColonizationLeaf> ofxSpaceColonization::getLeaves() const{
    return this->leaves;
}

vector<shared_ptr<ofxSpaceColonizationBranch>> ofxSpaceColonization::getBranches() const{
    return this->branches;
}

