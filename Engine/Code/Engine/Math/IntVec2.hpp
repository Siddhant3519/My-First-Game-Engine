#pragma once

//--------------------------------------------------------------------------------------------------
struct IntVec2
{
	// NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	int x = 0;
	int y = 0;

public:
	IntVec2() {};
	~IntVec2() {};
	IntVec2(IntVec2 const& copyFrom);
	explicit IntVec2(int initialX, int initialY);


	float			GetLength()					const;
	int				GetTaxicabLength()			const;
	int				GetLengthSquared()			const;
	float			GetOrientationRadians()		const;
	float			GetOrientationDegrees()		const;
	IntVec2 const	GetRotated90Degrees()		const;
	IntVec2 const	GetRotatedMinus90Degrees()	const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();
	void SetFromText(char const* text);

	// Operators (const)
	IntVec2	const	operator+(IntVec2 const& vecToAdd)		const; // IntVec2 + IntVec2
	IntVec2	const	operator-(IntVec2 const& vecToSubtract) const; // IntVec2 - IntVec2
	bool			operator==(const IntVec2& compare)		const; // IntVec2 == IntVec2
	bool			operator!=(const IntVec2& compare)		const; // IntVec2 != IntVec2

	// Operators (self-mutating / non-const)
	void operator=(IntVec2 const& copyFrom); // IntVec2 = IntVec2
};