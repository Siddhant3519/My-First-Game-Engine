#pragma once

//--------------------------------------------------------------------------------------------------
struct IntVec3
{
	// NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	int x = 0;
	int y = 0;
	int z = 0;

	IntVec3() {};
	~IntVec3() {};
	IntVec3(IntVec3 const& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	// Operators (const)
	IntVec3	const	operator+(IntVec3 const& vecToAdd)			const;		// IntVec3 + IntVec3
	IntVec3	const	operator-(IntVec3 const& vecToSubtract)		const;		// IntVec3 - IntVec3
	bool			operator==(IntVec3 const& compare)			const;		// IntVec3 == IntVec3
	bool			operator!=(IntVec3 const& compare)			const;		// IntVec3 != IntVec3

	// Operators (self-mutating / non-const)
	void operator=	(IntVec3 const& copyFrom);			// IntVec3 = IntVec3
	void operator+=	(IntVec3 const& vecToAdd);			// IntVec3 += IntVec3
	void operator-=	(IntVec3 const& vecToSubtract);		// IntVec3 -= IntVec3
	void operator*=	(int uniformScale);					// IntVec3 *= int
};