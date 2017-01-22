#include "SceneNode.h"

SceneNode::SceneNode(Mesh *mesh, Vector4 colour, bool hasReflection, bool projectile) {
	this->mesh = mesh;
	this->colour = colour;
	this->hasReflection = hasReflection;
	this->projectile = projectile;
	this->fired = false;

	this->boundingRadius = 1.0f;
	this->distanceFromCamera = 0.0f;
	this->shininessFactor = 5.0f;
	this->radius = 1.0f;

	this->parent = NULL;
	this->rigidBody = NULL;
	this->simpleSpring = NULL;
	
	this->modelScale = Vector3(1, 1, 1);
}

SceneNode::~SceneNode(void) {
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
	delete rigidBody;
}

void SceneNode::AddChild(SceneNode *s) {
	children.push_back(s);
	s->parent = this;
}

void SceneNode::RemoveChild(SceneNode *s) {
	for (vector<SceneNode *>::const_iterator i = s->parent->GetChildIteratorStart(); i != s->parent->GetChildIteratorEnd(); ++i) {
		if (*i == s) {
			s->parent->children.erase(i);
			break;
		}
	}
	delete s;
}

void SceneNode::Update(float msec) {
	if (parent) { // This node has a parent...
		worldTransform = parent->worldTransform * transform;
	}
	else { // Root node, world transform is local transform!
		worldTransform = transform;
	}

	if (rigidBody) {
		worldTransform = rigidBody->GetModelMatrix();
	}

	for (vector<SceneNode *>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(msec);
	}
}