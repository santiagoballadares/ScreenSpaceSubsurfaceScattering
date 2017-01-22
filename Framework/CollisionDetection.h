#pragma once
#include "CollisionData.h"
#include "SceneNode.h"
#include "MyTriangle.h"

extern RigidBody **rigidBodies;
extern int numRigidBodies;

class CollisionDetection
{
public:
	CollisionDetection()	{ collisionData = new CollisionData(); }
	~CollisionDetection()	{ delete collisionData; }

	void CheckSphereCollisions()
	{
		for (int i = 0; i < numRigidBodies - 1; ++i)
		{
			for (int j = i + 1; j < numRigidBodies; ++j)
			{

				bool hit = SphereSphereCollision(rigidBodies[i], rigidBodies[j], collisionData);

				if (hit)
				{
					// add forces
					//rigidBodies[temp[0]]->AddForce( collisionData->m_normal * 500.0f);
					//rigidBodies[temp[1]]->AddForce(-collisionData->m_normal * 500.0f);

					AddCollisionImpulse(*rigidBodies[i], *rigidBodies[j], *collisionData);

					updateShininess(n, rigidBodies[i], rigidBodies[j]);
				}

			}
		}

	}

	bool SphereSphereCollision(RigidBody *rb0, RigidBody *rb1, CollisionData *collisionData = NULL)
	{
		// get objects position vectors
		Vector3 posObject0 = rb0->m_position;
		Vector3 posObject1 = rb1->m_position;

		// get delta vector between the objects
		Vector3 delta = posObject0 - posObject1;

		// get squared distance
		float distSq = Vector3::Dot(delta, delta);

		// get sum of radiuses
		float sumRadius = rb0->m_radius + rb1->m_radius;
	
		// check distances
		if (distSq < sumRadius * sumRadius)
		{
			// add collision data
			if (collisionData)
			{
				collisionData->m_penetration		= sumRadius - sqrtf( distSq );
				collisionData->m_normal				= delta;
				collisionData->m_normal.Normalise();
				collisionData->m_point				= posObject0 - collisionData->m_normal * ( rb0->m_radius - collisionData->m_penetration * 0.5f );
			}
			return true;
		}
		else
		{
			return false;
		}
	}


	void CheckPlaneCollisions(int numVertices, Vector3 **vertices, float boundary)
	{
		for (int i = 17; i < numRigidBodies; ++i)
		{
			Vector3 point = rigidBodies[i]->m_position;

			if ( (point.x > 0.0f && point.x < boundary) && (point.z > 0.0f && point.z < boundary) && (point.y > -200.0))
			{
				for (int j = 0; j < numVertices; j+=3)
				{
					MyTriangle triangle = MyTriangle(	(*vertices[j  ]),
														(*vertices[j+1]),
														(*vertices[j+2]));

					bool hit = SpherePlaneCollision(rigidBodies[i], triangle, collisionData);
					
					if (hit) {
						AddCollisionImpulsePlane(*rigidBodies[i], *collisionData);
					}
				}
			}
		}
	}

	bool SpherePlaneCollision(RigidBody *rb0, MyTriangle &triangle, CollisionData *collisionData = NULL)
	{
		// get vectors from point (rigid body) to each vertex
		Vector3 vert1 = rb0->m_position - triangle.v1;
		Vector3 vert2 = rb0->m_position - triangle.v2;
		Vector3 vert3 = rb0->m_position - triangle.v3;

		// normalize vectors
		vert1.Normalise();
		vert2.Normalise();
		vert3.Normalise();

		// get angles from between vectors
		float totalAngles = 0.0f;

		totalAngles += acos(Vector3::Dot(vert1, vert2));   
		totalAngles += acos(Vector3::Dot(vert2, vert3));
		totalAngles += acos(Vector3::Dot(vert3, vert1));

		if (fabs(totalAngles - 2*PI) <= 0.1f)
		{
			float radius = rb0->m_radius;
			float distance = Vector3::Dot(rb0->m_position, triangle.GetNormal());
			float penetration = radius - distance;

			if (distance <= radius)
			{
				// add collision data
				if (collisionData)
				{
					collisionData->m_normal = triangle.GetNormal();
					collisionData->m_penetration = penetration;
					collisionData->m_point = rb0->m_position - triangle.GetNormal() * ( rb0->m_radius - penetration * 0.5f );
				}
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}


	void SetRoot(SceneNode *n)
	{
		this->n = n;
	}

	void updateShininess(SceneNode *n, RigidBody *rb0, RigidBody *rb1)
	{
		if ( n->HasReflection() && (n->GetRigidBody() == rb0 || n->GetRigidBody() == rb1) )
		{
			n->SetShininessFactor(5.0f);
			return;
		}

		for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		{
			updateShininess(*i, rb0, rb1);
		}
	}

protected:
	CollisionData *collisionData;
	SceneNode *n;
};