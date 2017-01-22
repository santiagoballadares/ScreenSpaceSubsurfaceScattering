#pragma once

#include "Vector3.h"
#include "RigidBody.h"
/*
class SimpleSpring {
public:
    SimpleSpring(RigidBody *rb, Vector3 fixedPoint) {
		m_rb = rb;
        m_fixedPoint = fixedPoint;

		m_lenght = (m_rb->m_position - fixedPoint).Length();
		m_ks = 3.0f;
        m_kd = 1.0f;
    }

void update() {
	Vector3 spring = m_rb->m_position - m_fixedPoint;
	float lenght = spring.Length();
 
    Vector3 restoreForce;	// without damping
	Vector3 springForce;	// with damping

	if (lenght != 0) {
		float displacement = lenght - m_lenght;
		Vector3 springN = spring / lenght;
		restoreForce = springN * (displacement * m_ks);

		Vector3 deltaV = m_rb->m_linearVelocity;
		float damperForce = Vector3::Dot(springN, deltaV);

		springForce = springN * ((displacement * m_ks) + m_kd * damperForce);

        // add spring force
		m_rb->AddForce(-springForce);
	}
}

protected:
	RigidBody *m_rb;
	Vector3 m_fixedPoint;
 
	float m_lenght;		// rest length
	float m_ks;			// stiffness
	float m_kd;			// damping
};
*/

class SimpleSpring {
public:
    SimpleSpring(RigidBody *rbA, Vector3 fixedPoint) {
        m_rbA = rbA;
		m_fixedPoint = fixedPoint;

		m_localPosA = rbA->m_position;
		m_localPosA.x += rbA->m_radius;

        m_ks = 0.5f;
		m_kd = 0.3f;

		// get world positions (rigid bodies)
		Vector3 p0 = ( m_rbA->m_orientation * m_localPosA ).ToMatrix().GetPositionVector() + m_rbA->m_position;

		m_lenght = (m_fixedPoint - p0).Length();
    }

	void update() {
		// world position for each spring point
		Vector3 p0 = ( m_rbA->m_orientation * m_localPosA ).ToMatrix().GetPositionVector() + m_rbA->m_position;

		// error
		float err = (m_fixedPoint - p0).Length() - m_lenght;

		Vector3 linVelA = m_rbA->m_linearVelocity;

		Vector3 forceDirection = m_fixedPoint - p0;
		forceDirection.Normalise();

		// calculate force form the spring (spring and damping)
		Vector3 force = forceDirection * (err * m_ks - Vector3::Dot(forceDirection, linVelA * m_kd) );

		m_rbA->AddForceSpring(p0,  force * 0.5f);
	}

protected:
	RigidBody *m_rbA;
	Vector3 m_fixedPoint;
	Vector3 m_localPosA;
	float m_lenght;		// rest lenght
 
	float m_ks;			// stiffness
	float m_kd;			// damping
};
