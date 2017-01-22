#pragma once

#include "Vector3.h"
#include "RigidBody.h"
/*
class Spring {
public:
	Spring(RigidBody *rb0, RigidBody *rb1) {
	m_rb0 = rb0;
    m_rb1 = rb1;

	m_lenght = (m_rb0->m_position - m_rb1->m_position).Length();
	m_ks = 0.4f;
    m_kd = 0.45f;
}

void update() {
	Vector3 spring = m_rb0->m_position - m_rb1->m_position;
	float lenght = spring.Length();
 
    Vector3 restoreForce;	// without damping
	Vector3 springForce;	// with damping

	if (lenght != 0) {
		float displacement = lenght - m_lenght;
		Vector3 springN = spring / lenght;
		restoreForce = springN * (displacement * m_ks);

		Vector3 deltaV = m_rb0->m_linearVelocity - m_rb1->m_linearVelocity;
		float damperForce = Vector3::Dot(springN, deltaV);

		springForce = springN * ((displacement * m_ks) + m_kd * damperForce);

        // add spring force
		m_rb0->AddForce(-springForce);
		m_rb1->AddForce( springForce);
	}
}

protected:
	RigidBody *m_rb0;
	RigidBody *m_rb1;
 
	float m_lenght;		// rest length
	float m_ks;			// stiffness
	float m_kd;			// damping
};*/

class Spring {
public:
    Spring(RigidBody *rbA, Vector3 localPosA, RigidBody *rbB, Vector3 localPosB) {
        m_rbA = rbA;
        m_rbB = rbB;
		m_localPosA = localPosA;
		m_localPosB = localPosB;

        m_ks = 30.0f;
		m_kd = 10.0f;

		// get world positions (rigid bodies)
		Vector3 p0 = ( m_rbA->m_orientation.ToMatrix().GetPositionVector() * m_localPosA ) + m_rbA->m_position;
		Vector3 p1 = ( m_rbB->m_orientation.ToMatrix().GetPositionVector() * m_localPosB ) + m_rbB->m_position;

		m_lenght = (p1 - p0).Length();
    }

	void update() {
		// world position for each spring point
		Vector3 p0 = ( m_rbA->m_orientation.ToMatrix().GetPositionVector() * m_localPosA ) + m_rbA->m_position;
		Vector3 p1 = ( m_rbB->m_orientation.ToMatrix().GetPositionVector() * m_localPosB ) + m_rbB->m_position;

		// error
		float err = (p1 - p0).Length() - m_lenght;

		Vector3 linVelA = m_rbA->m_linearVelocity;
		Vector3 linVelB = m_rbB->m_linearVelocity;

		Vector3 forceDirection = p1 - p0;
		forceDirection.Normalise();

		// calculate force form the spring (spring and damping)
		Vector3 force = forceDirection * (err * m_ks - Vector3::Dot(forceDirection, (linVelA - linVelB) * m_kd) );

		m_rbA->AddForceSpring(p0,  force * 0.5f);
		m_rbB->AddForceSpring(p1, -force * 0.5f);
	}

protected:
	RigidBody *m_rbA;
	RigidBody *m_rbB;
	Vector3 m_localPosA;
	Vector3 m_localPosB;
	float m_lenght;		// rest lenght
 
	float m_ks;			// stiffness
	float m_kd;			// damping
};
