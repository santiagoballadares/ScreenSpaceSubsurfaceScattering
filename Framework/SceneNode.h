#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>

#include "RigidBody.h"
#include "SimpleSpring.h"
#include "Spring.h"

class SceneNode {
public:
	SceneNode(Mesh *m = NULL, Vector4 colour = Vector4(1, 1, 1, 1), bool hasReflection = false, bool projectile = false);
	~SceneNode(void);

	void SetTransform(const Matrix4 &matrix) { transform = matrix; }
	const Matrix4 & GetTransform() const { return transform; }

	void SetWorldTransform(const Matrix4 &matrix) { worldTransform = matrix; }
	Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }

	Mesh * GetMesh() const { return mesh; }
	void SetMesh(Mesh *m) { mesh = m; }

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	bool HasReflection() const { return hasReflection; }
	void SetHasReflection(bool r) { hasReflection = r; }

	float GetShininessFactor() const { return shininessFactor; }
	void SetShininessFactor(float s) { shininessFactor = s; }

	bool IsProjectile() const { return projectile; }
	void SetProjectile(bool p) { projectile = p; }

	bool Fired() const { return fired; }
	void SetFired(bool f) { fired = f; }

	float GetRadius() const { return radius; }
	void SetRadius(float rad) { radius = rad; }

	SceneNode * GetParent() { return parent; }

	RigidBody * GetRigidBody() { return rigidBody; }
	void SetRigidBody(RigidBody *rb) { rigidBody = rb; }

	SimpleSpring * GetSimpleSpring() { return simpleSpring; }
	void SetSimpleSpring(SimpleSpring *s) { simpleSpring = s; }
	
	static bool CompareByCameraDistance(SceneNode *a, SceneNode *b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	void AddChild(SceneNode *s);
	static void RemoveChild(SceneNode *s);

	virtual void Update(float msec);
	
	std::vector<SceneNode *>::const_iterator GetChildIteratorStart() { return children.begin(); }
	std::vector<SceneNode *>::const_iterator GetChildIteratorEnd() { return children.end(); }

protected:
	SceneNode	*parent;
	Mesh		*mesh;
	Matrix4		worldTransform;
	Matrix4		transform;
	Vector3		modelScale;
	Vector4		colour;
	std::vector<SceneNode *> children;

	RigidBody *rigidBody;
	SimpleSpring *simpleSpring;

	float boundingRadius;
	float distanceFromCamera;

	bool hasReflection;
	float shininessFactor;

	bool projectile;
	bool fired;

	float radius;
};