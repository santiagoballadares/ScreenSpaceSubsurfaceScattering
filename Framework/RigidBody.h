#pragma once
#include "Vector3.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "CollisionData.h"
#include "MyPlane.h"

#define GRAVITY -9.8f
#define DAMPING_LINEAR 0.05f
#define DAMPING_ANGULAR 0.001f

class RigidBody
{
public:
	float m_radius;

	//<---------LINEAR----------------->
	Vector3     m_position;
	Vector3     m_linearVelocity;
	Vector3     m_force;
	float       m_invMass;

	//<----------ANGULAR--------------->
	Quaternion  m_orientation;
	Vector3     m_angularVelocity;
	Vector3     m_torque;
	Matrix4     m_invInertia;

	RigidBody()
	{
		Reset();
	}

	void Reset()
	{
		//<---------LINEAR-------------->
		m_position.ToZero();
		m_linearVelocity.ToZero();
		m_force.ToZero();
		m_invMass = 1.0f;

		//<----------ANGULAR--------------->
		m_orientation.ToIdentity();
		m_angularVelocity.ToZero();
		m_torque.ToZero();
		m_invInertia.ToIdentity();
	}

	void SetRadius(float radius) { m_radius = radius; }
	float GetRadius() { return m_radius; }

	//<---------LINEAR-------------->
	void SetPosition(const Vector3 position)					{ m_position = position; }
	Vector3 GetPosition() const									{ return m_position; }

	void SetLinearVelocity(const Vector3 linearVelocity)		{ m_linearVelocity = linearVelocity; }
	Vector3 GetLinearVelocity() const							{ return m_linearVelocity; }

	void SetForce(const Vector3 force)							{ m_force = force; }
	Vector3 GetForce() const									{ return m_force; }

	void SetInvMass(const float invMass)						{ m_invMass = invMass; }
	float GetInvMass() const									{ return m_invMass; }


	//<----------ANGULAR------------>
	void SetOrientation(const Quaternion orientation)			{ m_orientation = orientation; }
	Quaternion GetOrientation() const							{ return m_orientation; }

	void SetAngularVelocity(const Vector3 angularVelocity)		{ m_angularVelocity = angularVelocity; }
	Vector3 GetAngularVelocity() const							{ return m_angularVelocity; }

	void SetTorque(const Vector3 torque)						{ m_torque = torque; }
	Vector3 GetTorque() const									{ return m_torque; }

	void SetInvInertia(const Matrix4 invInertia)				{ m_invInertia = invInertia; }
	Matrix4 GetInvInertia() const								{ return m_invInertia; }


	void Integrate(float dt)
	{
		// check dt value
		if (dt > 0.25f)					dt = 0.25f;
		//if (m_force.Length() > 99.0f)	int stop = 0;

		//<------------------LINEAR----------------------->
		m_linearVelocity    += m_force * m_invMass * dt;
		m_position          += m_linearVelocity * dt;
		m_force.ToZero();

		//<------------------ANGULAR---------------------->
		Matrix4 worldInvInertia		= CreateWorldII();
		m_angularVelocity			+= (worldInvInertia * m_torque) * dt;
		m_orientation				+= Quaternion((m_angularVelocity * dt * 0.5f), 0.0f) * m_orientation;
		m_orientation.Normalise();
		m_torque.ToZero();
	}

	Matrix4 CreateWorldII()
	{
		Matrix4 orientationMatrix = m_orientation.ToMatrix();

		Matrix4 inverseOrientationMatrix = orientationMatrix.Transpose();

		Matrix4 inverseWorldInertiaMatrix = inverseOrientationMatrix * 
											m_invInertia * 
											orientationMatrix;
		return inverseWorldInertiaMatrix;
	}

	void AddForce(const Vector3 force)
	{
		m_force += force;
	}
	void AddAngForce(const Vector3 angForce)
	{
		m_angularVelocity += angForce;
	}

	void AddForceSpring(const Vector3 &worldPosForce, const Vector3 &directionMagnitude)
	{
		//<------------------LINEAR---------------------->
		m_force += directionMagnitude;

		//<------------------ANGULAR--------------------->
		Vector3 distance = worldPosForce - m_position;

		// clamp torque
		Vector3 torque = Vector3::Cross(distance, directionMagnitude);
		AddTorque(torque);
	}

	void AddTorque(const Vector3 &worldAxisAndMagnitudeTorque)
	{
		Vector3 torque = worldAxisAndMagnitudeTorque;

		if ( torque.LengthSquared() > 0.000001f )
		{
			Vector3 torqueAxis = torque;
			torqueAxis.Normalise();

			float torquePower = torque.Length();

			if (torquePower < 0.0f)
				torquePower = 0.0f;
			if (torquePower > 5000.0f)
				torquePower = 5000.0f;

			m_torque += torqueAxis * torquePower;
		}
	}

	Matrix4 GetModelMatrix()
	{
		//return m_orientation.ToMatrix() * Matrix4::Translation(m_position);
		return Matrix4::Translation(m_position) * m_orientation.ToMatrix();
	}
};


extern RigidBody **rigidBodies;
extern int numRigidBodies;

extern bool activeGravity;

inline RigidBody * CreateRigidBody(const Matrix4 m, float radius = 10.0f, float mass = 1.0f)
{
	// create array
	if (rigidBodies == 0)
	{
		rigidBodies = new RigidBody*[100];
	}

	// create rigid body
	RigidBody *newRigidBody = new RigidBody();

	newRigidBody->m_position = m.GetPositionVector();

	newRigidBody->m_radius = radius;
	
	newRigidBody->m_invMass = 1.0f / mass;

	float inertia = 1.0f / ((2.0f/5.0f) * mass * radius * radius);

	Matrix4 mat = Matrix4();
	mat.values[0] = inertia;
	mat.values[5] = inertia;
	mat.values[10] = inertia;

	newRigidBody->m_invInertia = mat;

	// Add to array
	rigidBodies[numRigidBodies] = newRigidBody;
	++numRigidBodies;
	
	return newRigidBody;
}

inline void UpdatePhysics(float dt)
{
	// add gravity
	if (activeGravity)
	{
		for (int i = 0; i < numRigidBodies; ++i)
		{
			rigidBodies[i]->AddForce(Vector3(0.0f, GRAVITY , 0.0f) * (1.0f / rigidBodies[i]->GetInvMass()) * (dt));
		}
	}

	// add damping force
	for (int i = 0; i < numRigidBodies; ++i)
	{
		rigidBodies[i]->AddForce(-rigidBodies[i]->GetLinearVelocity() * DAMPING_LINEAR);

		rigidBodies[i]->AddAngForce(-rigidBodies[i]->GetAngularVelocity() * DAMPING_ANGULAR);
	}

	// update formula's variables
	for (int i = 0; i < numRigidBodies; ++i)
	{
		rigidBodies[i]->Integrate(dt);
	}
}

inline void AddCollisionImpulse(RigidBody &rb0, RigidBody &rb1, CollisionData &collisionData)
{
	float invMass0 = ( (1.0f / rb0.m_invMass) > 1000.0f) ? 0.0f : rb0.m_invMass;
	float invMass1 = ( (1.0f / rb1.m_invMass) > 1000.0f) ? 0.0f : rb1.m_invMass;

	const Matrix4 worldInvInertia0 = rb0.m_invInertia;
	const Matrix4 worldInvInertia1 = rb1.m_invInertia;

	// return if both objects are not movable
	if ( (invMass0 + invMass1) == 0.0f )
	{
		return;
	}

	Vector3 r0 = collisionData.m_point - rb0.m_position;
	Vector3 r1 = collisionData.m_point - rb1.m_position;

	Vector3 v0 = rb0.m_linearVelocity + Vector3::Cross(rb0.m_angularVelocity, r0);
	Vector3 v1 = rb1.m_linearVelocity + Vector3::Cross(rb1.m_angularVelocity, r1);

	Vector3 dv = v0 - v1;

	// If the objects are moving away from each other we dont need to apply an impulse
	float relativeMovement = -Vector3::Dot(dv, collisionData.m_normal);
	if (relativeMovement < -0.01f)
	{
		return;
	}

	
	// Normal impulse

	// Coefficient of Restitution
	float e = 0.0f;

	float normDiv =	Vector3::Dot(collisionData.m_normal, collisionData.m_normal) * 
					(	( invMass0 + invMass1 ) + 
						Vector3::Dot(	collisionData.m_normal, 
										Vector3::Cross( worldInvInertia0 * Vector3::Cross(r0, collisionData.m_normal ), r0) + 
										Vector3::Cross( worldInvInertia1 * Vector3::Cross(r1, collisionData.m_normal ), r1) 
									)
					);

	float jn = -1 * (1+e) * Vector3::Dot(dv, collisionData.m_normal) / normDiv;

	// Hack fix to stop sinking - bias impulse proportional to penetration distance
	jn = jn + (collisionData.m_penetration * 1.5f);

	rb0.m_linearVelocity	+= collisionData.m_normal * jn * invMass0;
	rb0.m_angularVelocity	+= worldInvInertia0 * Vector3::Cross(r0, collisionData.m_normal * jn);

	rb1.m_linearVelocity	-= collisionData.m_normal * jn * invMass1;
	rb1.m_angularVelocity	-= worldInvInertia1 * Vector3::Cross(r1, collisionData.m_normal * jn);



	// Tangent impulse

	// Work out our tangent vector, with is perpendicular to our collision normal
	Vector3 tangent = Vector3();
	tangent = dv - ( collisionData.m_normal * Vector3::Dot(dv, collisionData.m_normal));
	tangent.Normalise();

	float tangDiv = invMass0 + invMass1 +  
					Vector3::Dot(	tangent, 
									Vector3::Cross(( rb0.m_invInertia * Vector3::Cross(r0, tangent) ), r0) +
									Vector3::Cross(( rb1.m_invInertia * Vector3::Cross(r1, tangent) ), r1)
								);

	float jt = -1 * Vector3::Dot(dv, tangent) / tangDiv;
	// Clamp min/max tangental component

	// Apply contact impulse
	rb0.m_linearVelocity += tangent * jt * invMass0;
	rb0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, tangent * jt);
						
	rb1.m_linearVelocity -= tangent * jt * invMass1;
	rb1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, tangent * jt);
}

inline void AddCollisionImpulsePlane(RigidBody &rb0, CollisionData &collisionData)
{
	// Normal impulse

	// Coefficient of Restitution
	float e = 0.5f;

	float invMass0 = ( (1.0f / rb0.m_invMass) > 1000.0f) ? 0.0f : rb0.m_invMass;

	const Matrix4 worldInvInertia0 = rb0.m_invInertia;

	Vector3 r0 = collisionData.m_point - rb0.m_position;

	Vector3 v0 = rb0.m_linearVelocity + Vector3::Cross(rb0.m_angularVelocity, r0);

	Vector3 dv = v0;

	// If the objects are moving away from each other we dont need to apply an impulse
	float relativeMovement = -Vector3::Dot(dv, collisionData.m_normal);
	if (relativeMovement < -0.01f) 
	{
		return;
	}

	float normDiv =	Vector3::Dot(collisionData.m_normal, collisionData.m_normal) * 
					(	invMass0 + 
						Vector3::Dot(	collisionData.m_normal, 
										Vector3::Cross( worldInvInertia0 * Vector3::Cross(r0, collisionData.m_normal ), r0)
									)
					);

	float jn = -1 * (1+e) * Vector3::Dot(dv, collisionData.m_normal) / normDiv;
				
	// Hack fix to stop sinking - bias impulse proportional to penetration distance
	jn = jn + (collisionData.m_penetration * 1.5f);

	rb0.m_linearVelocity += collisionData.m_normal * jn * invMass0;
	rb0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, collisionData.m_normal * jn);


	// Tangent impulse

	// Work out our tangent vector, with is perpendicular to our collision normal
	Vector3 tangent = Vector3();
	tangent = dv - ( collisionData.m_normal * Vector3::Dot(dv, collisionData.m_normal));
	tangent.Normalise();
            
	float tangDiv = invMass0 +  
					Vector3::Dot(	tangent, 
									Vector3::Cross(( rb0.m_invInertia * Vector3::Cross(r0, tangent) ), r0)
								);

	float jt = -1 * Vector3::Dot(dv, tangent) / tangDiv;
	// Clamp min/max tangental component
            
	// Apply contact impulse
	rb0.m_linearVelocity += tangent * jt * invMass0;
	rb0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, tangent * jt);
}