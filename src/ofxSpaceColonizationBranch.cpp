#include "ofxSpaceColonizationBranch.h"


ofxSpaceColonizationBranch::ofxSpaceColonizationBranch(const ofVec3f _direction){
    direction = _direction;
    originalDirection = _direction;
}

void ofxSpaceColonizationBranch::move(ofVec3f pos){
    this->node.move(pos);
}

void ofxSpaceColonizationBranch::setParent(shared_ptr<ofxSpaceColonizationBranch> branch){
    this->node.setParent(branch->node);
}

ofVec3f ofxSpaceColonizationBranch::getPosition(){
    return this->node.getGlobalPosition();
}

void ofxSpaceColonizationBranch::reset(){
    this->direction = this->originalDirection;
    this->count = 0;
}

void ofxSpaceColonizationBranch::draw(){
    if(this->node.getParent() != nullptr){
        auto pos = this->node.getGlobalPosition();
        auto parentPos = this->node.getParent()->getGlobalPosition();
        ofDrawLine(pos.x, pos.y, parentPos.x, parentPos.y);
    }
}
